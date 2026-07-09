#include "droneops/Server.hpp"
#include "droneops/Package.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iostream>
#include <memory>

using json = nlohmann::json;

namespace droneops::server {

WebServer::WebServer(droneops::sqlite::SqliteStore& store, const std::string& web_root, const std::string& cert_path, const std::string& key_path)
    : store_(store), web_root_(web_root) {
    if (!cert_path.empty() && !key_path.empty()) {
        srv_ = std::make_unique<httplib::SSLServer>(cert_path.c_str(), key_path.c_str());
        if (!srv_->is_valid()) {
            std::cerr << "Erro ao carregar certificados SSL. Verifique os caminhos.\n";
            exit(1);
        }
    } else {
        srv_ = std::make_unique<httplib::Server>();
    }
    setupRoutes();
}

WebServer::~WebServer() {}

void WebServer::start(int port) {
    std::cout << "DroneOps server listening on " << (dynamic_cast<httplib::SSLServer*>(srv_.get()) ? "https" : "http") << "://0.0.0.0:" << port << '\n';
    srv_->listen("0.0.0.0", port);
}

void WebServer::stop() {
    srv_->stop();
}

void WebServer::setupRoutes() {
    srv_->set_mount_point("/static", web_root_);

    srv_->Get("/", [this](const httplib::Request&, httplib::Response& res) {
        std::filesystem::path index = std::filesystem::path(web_root_) / "index.html";
        std::ifstream file(index);
        if (file) {
            std::ostringstream ss;
            ss << file.rdbuf();
            res.set_content(ss.str(), "text/html");
        } else {
            res.status = 404;
        }
    });

    srv_->Get("/api/version", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"version": "v0.5.0"})", "application/json");
    });

    // Fase 4: Network Check (ping externo)
    srv_->Get("/api/system/network", [](const httplib::Request&, httplib::Response& res) {
        // Tenta se conectar a um dominio super estavel e de rapida resposta para verificar internet
        httplib::Client cli("http://captive.apple.com");
        cli.set_connection_timeout(1, 0); // 1 segundo
        cli.set_read_timeout(1, 0);
        auto cli_res = cli.Get("/generate_204");
        
        bool is_online = (cli_res && cli_res->status == 200);
        res.set_content(is_online ? R"({"online": true})" : R"({"online": false})", "application/json");
    });

    // API REST
    srv_->Get(R"(/api/missions/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        try {
            auto mission = store_.getMission(id);
            json j = {
                {"mission_id", mission.mission_id},
                {"evidence_paths", mission.evidence_paths},
                {"occurrences", mission.occurrences},
                {"final_decision", mission.final_decision},
                {"sync_status", mission.sync_status}
            };
            
            std::vector<bool> pre, post;
            for(const auto& item : mission.preflight_checklist) pre.push_back(item.checked);
            for(const auto& item : mission.postflight_checklist) post.push_back(item.checked);
            j["preflight"] = pre;
            j["postflight"] = post;

            res.set_content(j.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 404;
            std::string err = std::string(R"({"error": ")") + e.what() + "\"}";
            res.set_content(err, "application/json");
        }
    });

    srv_->Post("/api/missions", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            MissionRecord m;
            m.mission_id = j.value("mission_id", "");
            m.project_id = j.value("project_id", "");
            m.campaign_id = j.value("campaign_id", "");
            m.area_id = j.value("area_id", "");
            m.responsible = j.value("responsible", "");
            m.operator_id = j.value("operator_id", "");
            m.aircraft_id = j.value("aircraft_id", "");
            m.drone_qr = j.value("drone_qr", "");
            m.sensor_id = j.value("sensor_id", "");
            m.planned_date = j.value("planned_date", "");
            m.evidence_paths = j.value("evidence_paths", "");
            m.occurrences = j.value("occurrences", "");
            m.final_decision = j.value("final_decision", "rascunho");

            if (j.contains("preflight") && j["preflight"].is_array()) {
                for (const auto& val : j["preflight"]) {
                    m.preflight_checklist.push_back({"", "", val.get<bool>(), true});
                }
            }
            if (j.contains("postflight") && j["postflight"].is_array()) {
                for (const auto& val : j["postflight"]) {
                    m.postflight_checklist.push_back({"", "", val.get<bool>(), true});
                }
            }

            store_.saveMission(m);
            std::cout << "Missao " << m.mission_id << " salva via API.\n";

            res.status = 200;
            res.set_content(R"({"status": "ok"})", "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            std::string err = std::string(R"({"error": ")") + e.what() + "\"}";
            res.set_content(err, "application/json");
        }
    });

    // Fase 5: Upload de Evidências (Fotos Físicas)
    srv_->Post(R"(/api/missions/(.+)/evidence)", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        try {
            auto mission = store_.getMission(id);
            std::filesystem::path fotos_dir = "out/campanha/evidencias/fotos";
            std::filesystem::create_directories(fotos_dir);

            std::string saved_paths = mission.photo_paths;
            bool added = false;

            if (req.is_multipart_form_data()) {
                for (auto it = req.files.find("files"); it != req.files.end(); ++it) {
                    const auto& file = it->second;
                    if (file.name == "files") {
                        std::filesystem::path out_path = fotos_dir / file.filename;
                        std::ofstream out(out_path, std::ios::binary);
                        out.write(file.content.data(), file.content.size());
                        out.close();

                        if (!saved_paths.empty()) saved_paths += ",";
                        saved_paths += "evidencias/fotos/" + file.filename;
                        added = true;
                    }
                }
            }

            if (added) {
                mission.photo_paths = saved_paths;
                store_.saveMission(mission);
            }
            res.set_content(R"({"status": "uploaded"})", "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            std::string err = std::string(R"({"error": ")") + e.what() + "\"}";
            res.set_content(err, "application/json");
        }
    });

    // Fase 4: Empacotamento
    srv_->Post(R"(/api/missions/(.+)/package)", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        try {
            auto mission = store_.getMission(id);
            std::filesystem::path out_dir = "out/pacotes_prontos/" + id;
            droneops::package::writeMissionPackage(".", out_dir, mission);
            
            // Marca como pendente de sincronizacao
            mission.sync_status = "pendente_sincronizacao";
            store_.saveMission(mission);

            res.set_content(R"({"status": "packaged", "path": ")" + out_dir.string() + "\"}", "application/json");
        } catch (const std::exception& e) {
            res.status = 404;
            std::string err = std::string(R"({"error": ")") + e.what() + "\"}";
            res.set_content(err, "application/json");
        }
    });

    // Fase 4: Sincronização Simulada (Upload)
    srv_->Post("/api/system/sync", [this](const httplib::Request&, httplib::Response& res) {
        try {
            // Pega todas as missoes
            auto missions = store_.getAllMissions();
            int synced = 0;
            for(auto& m : missions) {
                if(m.sync_status == "pendente_sincronizacao") {
                    // Aqui faria o POST real do pacote ZIP para o SISTER-Observa
                    // Vamos simular sucesso
                    m.sync_status = "sincronizado";
                    store_.saveMission(m);
                    synced++;
                }
            }
            res.set_content("{\"synced\": " + std::to_string(synced) + "}", "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(R"({"error": "Falha na sincronizacao"})", "application/json");
        }
    });
}

} // namespace droneops::server

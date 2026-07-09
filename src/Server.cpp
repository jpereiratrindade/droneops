#include "droneops/Server.hpp"
#include "droneops/Package.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iostream>

using json = nlohmann::json;

namespace droneops::server {

WebServer::WebServer(droneops::sqlite::SqliteStore& store, const std::string& web_root)
    : store_(store), web_root_(web_root) {
    setupRoutes();
}

void WebServer::start(int port) {
    std::cout << "Iniciando servidor web na porta " << port << "...\n";
    std::cout << "Arquivos estaticos em: " << web_root_ << "\n";
    std::cout << "Acesse: http://127.0.0.1:" << port << "\n";
    srv_.listen("0.0.0.0", port);
}

void WebServer::stop() {
    srv_.stop();
}

void WebServer::setupRoutes() {
    // Servir diretorio estatico
    auto ret = srv_.set_mount_point("/static", web_root_);
    if (!ret) {
        std::cerr << "Aviso: Nao foi possivel montar diretorio estatico: " << web_root_ << "\n";
    }

    srv_.Get("/", [this](const httplib::Request&, httplib::Response& res) {
        res.set_redirect("/static/index.html");
    });

    srv_.Get("/api/version", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"version": "v0.3.1"})", "application/json");
    });

    // Fase 4: Network Check (ping externo)
    srv_.Get("/api/system/network", [](const httplib::Request&, httplib::Response& res) {
        // Tenta se conectar a um dominio super estavel e de rapida resposta para verificar internet
        httplib::Client cli("http://captive.apple.com");
        cli.set_connection_timeout(1, 0); // 1 segundo
        cli.set_read_timeout(1, 0);
        auto cli_res = cli.Get("/generate_204");
        
        bool is_online = (cli_res && cli_res->status == 200);
        res.set_content(is_online ? R"({"online": true})" : R"({"online": false})", "application/json");
    });

    // API REST
    srv_.Get(R"(/api/missions/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        try {
            auto mission = store_.getMission(id);
            json j;
            j["mission_id"] = mission.mission_id;
            j["project_id"] = mission.project_id;
            j["campaign_id"] = mission.campaign_id;
            j["area_id"] = mission.area_id;
            j["responsible"] = mission.responsible;
            j["operator_id"] = mission.operator_id;
            j["aircraft_id"] = mission.aircraft_id;
            j["drone_qr"] = mission.drone_qr;
            j["sensor_id"] = mission.sensor_id;
            j["planned_date"] = mission.planned_date;
            j["evidence_paths"] = mission.evidence_paths;
            j["occurrences"] = mission.occurrences;
            j["final_decision"] = mission.final_decision;
            
            // preflight
            j["preflight"] = json::array();
            for (const auto& item : mission.preflight_checklist) {
                j["preflight"].push_back(item.checked);
            }
            // postflight
            j["postflight"] = json::array();
            for (const auto& item : mission.postflight_checklist) {
                j["postflight"].push_back(item.checked);
            }

            res.set_content(j.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 404;
            res.set_content(R"({"error": "Nao encontrado"})", "application/json");
        }
    });

    srv_.Post("/api/missions", [this](const httplib::Request& req, httplib::Response& res) {
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

    // Fase 4: Empacotamento
    srv_.Post(R"(/api/missions/(.+)/package)", [this](const httplib::Request& req, httplib::Response& res) {
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
    srv_.Post("/api/system/sync", [this](const httplib::Request&, httplib::Response& res) {
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

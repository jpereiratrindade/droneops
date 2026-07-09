#include "droneops/Server.hpp"
#include <nlohmann/json.hpp>
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
        res.set_content(R"({"version": "v0.3.0"})", "application/json");
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
}

} // namespace droneops::server

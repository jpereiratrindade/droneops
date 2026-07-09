#include "droneops/Sqlite.hpp"

#include <sqlite3.h>
#include <stdexcept>
#include <sstream>

namespace droneops::sqlite {

SqliteStore::SqliteStore(const std::filesystem::path& db_path) {
    if (sqlite3_open(db_path.string().c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error("Não foi possível abrir o banco SQLite: " + std::string(sqlite3_errmsg(db_)));
    }
}

SqliteStore::~SqliteStore() {
    if (db_) {
        sqlite3_close(db_);
    }
}

void SqliteStore::initSchema() {
    const char* schema = R"(
        CREATE TABLE IF NOT EXISTS missions (
            mission_id TEXT PRIMARY KEY,
            project_id TEXT,
            campaign_id TEXT,
            area_id TEXT,
            objective TEXT,
            planned_date TEXT,
            planned_time TEXT,
            responsible TEXT,
            pilot TEXT,
            observer TEXT,
            aircraft_id TEXT,
            sensor_id TEXT,
            battery_ids TEXT,
            weather TEXT,
            authorization_ref TEXT,
            operator_id TEXT,
            drone_qr TEXT,
            aro_ref TEXT,
            gps_latitude TEXT,
            gps_longitude TEXT,
            gps_accuracy_m TEXT,
            photo_paths TEXT,
            final_decision TEXT,
            flight_plan_path TEXT,
            map_path TEXT,
            evidence_paths TEXT,
            flight_log_paths TEXT,
            occurrences TEXT,
            notes TEXT,
            status_text TEXT,
            preflight_json TEXT,
            postflight_json TEXT
        );
    )";

    char* errmsg = nullptr;
    if (sqlite3_exec(db_, schema, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        std::string err = errmsg;
        sqlite3_free(errmsg);
        throw std::runtime_error("Falha ao inicializar schema SQLite: " + err);
    }
}

// Simples serializador JSON manual (já que não temos dependência de json no core ainda)
std::string SqliteStore::serializeChecklist(const std::vector<ChecklistItem>& checklist) const {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < checklist.size(); ++i) {
        oss << "{\"id\":\"" << checklist[i].id << "\",\"checked\":" << (checklist[i].checked ? "true" : "false") << "}";
        if (i + 1 < checklist.size()) oss << ",";
    }
    oss << "]";
    return oss.str();
}

// Simples deserializador JSON manual 
std::vector<ChecklistItem> SqliteStore::deserializeChecklist(const std::string& json_str) const {
    // Para simplificar no MVP e evitar parser complexo, se precisar recuperamos estado
    // Retorna vazio ou implementa parser simples depois. Para o MVP do DroneOps,
    // o `Validator` e `Csv` não persistem os detalhes internos da checklist em si, 
    // ou se o fizerem, poderemos usar uma lib json.
    return {};
}

void SqliteStore::saveMission(const MissionRecord& r) {
    const char* sql = R"(
        INSERT OR REPLACE INTO missions (
            mission_id, project_id, campaign_id, area_id, objective, planned_date, planned_time,
            responsible, pilot, observer, aircraft_id, sensor_id, battery_ids, weather,
            authorization_ref, operator_id, drone_qr, aro_ref, gps_latitude, gps_longitude,
            gps_accuracy_m, photo_paths, final_decision, flight_plan_path, map_path,
            evidence_paths, flight_log_paths, occurrences, notes, status_text,
            preflight_json, postflight_json
        ) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Falha ao preparar INSERT: " + std::string(sqlite3_errmsg(db_)));
    }

    sqlite3_bind_text(stmt, 1, r.mission_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, r.project_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, r.campaign_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, r.area_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, r.objective.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, r.planned_date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, r.planned_time.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, r.responsible.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, r.pilot.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 10, r.observer.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, r.aircraft_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 12, r.sensor_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 13, r.battery_ids.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 14, r.weather.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 15, r.authorization_ref.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 16, r.operator_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 17, r.drone_qr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 18, r.aro_ref.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 19, r.gps_latitude.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 20, r.gps_longitude.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 21, r.gps_accuracy_m.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 22, r.photo_paths.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 23, r.final_decision.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 24, r.flight_plan_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 25, r.map_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 26, r.evidence_paths.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 27, r.flight_log_paths.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 28, r.occurrences.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 29, r.notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 30, r.status_text.c_str(), -1, SQLITE_STATIC);

    std::string pre = serializeChecklist(r.preflight_checklist);
    std::string pos = serializeChecklist(r.postflight_checklist);
    sqlite3_bind_text(stmt, 31, pre.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 32, pos.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Falha ao executar INSERT: " + std::string(sqlite3_errmsg(db_)));
    }
    sqlite3_finalize(stmt);
}

std::vector<MissionRecord> SqliteStore::getAllMissions() {
    std::vector<MissionRecord> result;
    const char* sql = "SELECT * FROM missions;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Falha ao preparar SELECT: " + std::string(sqlite3_errmsg(db_)));
    }

    auto get_str = [](sqlite3_stmt* s, int col) -> std::string {
        const unsigned char* text = sqlite3_column_text(s, col);
        return text ? reinterpret_cast<const char*>(text) : "";
    };

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        MissionRecord r;
        r.mission_id = get_str(stmt, 0);
        r.project_id = get_str(stmt, 1);
        r.campaign_id = get_str(stmt, 2);
        r.area_id = get_str(stmt, 3);
        r.objective = get_str(stmt, 4);
        r.planned_date = get_str(stmt, 5);
        r.planned_time = get_str(stmt, 6);
        r.responsible = get_str(stmt, 7);
        r.pilot = get_str(stmt, 8);
        r.observer = get_str(stmt, 9);
        r.aircraft_id = get_str(stmt, 10);
        r.sensor_id = get_str(stmt, 11);
        r.battery_ids = get_str(stmt, 12);
        r.weather = get_str(stmt, 13);
        r.authorization_ref = get_str(stmt, 14);
        r.operator_id = get_str(stmt, 15);
        r.drone_qr = get_str(stmt, 16);
        r.aro_ref = get_str(stmt, 17);
        r.gps_latitude = get_str(stmt, 18);
        r.gps_longitude = get_str(stmt, 19);
        r.gps_accuracy_m = get_str(stmt, 20);
        r.photo_paths = get_str(stmt, 21);
        r.final_decision = get_str(stmt, 22);
        r.flight_plan_path = get_str(stmt, 23);
        r.map_path = get_str(stmt, 24);
        r.evidence_paths = get_str(stmt, 25);
        r.flight_log_paths = get_str(stmt, 26);
        r.occurrences = get_str(stmt, 27);
        r.notes = get_str(stmt, 28);
        r.status_text = get_str(stmt, 29);
        
        r.preflight_checklist = deserializeChecklist(get_str(stmt, 30));
        r.postflight_checklist = deserializeChecklist(get_str(stmt, 31));

        result.push_back(r);
    }
    sqlite3_finalize(stmt);
    return result;
}

MissionRecord SqliteStore::getMission(const std::string& mission_id) {
    const char* sql = "SELECT * FROM missions WHERE mission_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Falha ao preparar SELECT: " + std::string(sqlite3_errmsg(db_)));
    }
    sqlite3_bind_text(stmt, 1, mission_id.c_str(), -1, SQLITE_STATIC);

    auto get_str = [](sqlite3_stmt* s, int col) -> std::string {
        const unsigned char* text = sqlite3_column_text(s, col);
        return text ? reinterpret_cast<const char*>(text) : "";
    };

    MissionRecord r;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        r.mission_id = get_str(stmt, 0);
        r.project_id = get_str(stmt, 1);
        r.campaign_id = get_str(stmt, 2);
        r.area_id = get_str(stmt, 3);
        r.objective = get_str(stmt, 4);
        r.planned_date = get_str(stmt, 5);
        r.planned_time = get_str(stmt, 6);
        r.responsible = get_str(stmt, 7);
        r.pilot = get_str(stmt, 8);
        r.observer = get_str(stmt, 9);
        r.aircraft_id = get_str(stmt, 10);
        r.sensor_id = get_str(stmt, 11);
        r.battery_ids = get_str(stmt, 12);
        r.weather = get_str(stmt, 13);
        r.authorization_ref = get_str(stmt, 14);
        r.operator_id = get_str(stmt, 15);
        r.drone_qr = get_str(stmt, 16);
        r.aro_ref = get_str(stmt, 17);
        r.gps_latitude = get_str(stmt, 18);
        r.gps_longitude = get_str(stmt, 19);
        r.gps_accuracy_m = get_str(stmt, 20);
        r.photo_paths = get_str(stmt, 21);
        r.final_decision = get_str(stmt, 22);
        r.flight_plan_path = get_str(stmt, 23);
        r.map_path = get_str(stmt, 24);
        r.evidence_paths = get_str(stmt, 25);
        r.flight_log_paths = get_str(stmt, 26);
        r.occurrences = get_str(stmt, 27);
        r.notes = get_str(stmt, 28);
        r.status_text = get_str(stmt, 29);
        
        r.preflight_checklist = deserializeChecklist(get_str(stmt, 30));
        r.postflight_checklist = deserializeChecklist(get_str(stmt, 31));
    } else {
        throw std::runtime_error("Missão não encontrada: " + mission_id);
    }
    sqlite3_finalize(stmt);
    return r;
}

} // namespace droneops::sqlite

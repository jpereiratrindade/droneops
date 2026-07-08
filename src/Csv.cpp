#include "droneops/Csv.hpp"

#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>

namespace droneops::csv {
namespace {

std::string csvEscape(const std::string& value) {
    if (value.find_first_of(",\"\n\r") == std::string::npos) {
        return value;
    }
    std::string escaped = "\"";
    for (const char c : value) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped.push_back(c);
        }
    }
    escaped.push_back('"');
    return escaped;
}

std::string jsonEscape(const std::string& value) {
    std::string out;
    for (unsigned char c : value) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out.push_back(static_cast<char>(c));
        }
    }
    return out;
}

void ensureOutput(const std::filesystem::path& output) {
    if (output.has_parent_path()) {
        std::filesystem::create_directories(output.parent_path());
    }
}

void writeCsvRow(std::ostream& out, const std::vector<std::string>& values) {
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            out << ',';
        }
        out << csvEscape(values[i]);
    }
    out << '\n';
}

MissionRecord rowToRecord(const std::vector<std::string>& headers,
                          const std::vector<std::string>& row,
                          const std::size_t source_line) {
    std::map<std::string, std::string> values;
    for (std::size_t i = 0; i < headers.size() && i < row.size(); ++i) {
        values[headers[i]] = row[i];
    }

    MissionRecord record;
    record.source_line = source_line;
    record.project_id = values["project_id"];
    record.campaign_id = values["campaign_id"];
    record.area_id = values["area_id"];
    record.mission_id = values["mission_id"];
    record.objective = values["objective"];
    record.planned_date = values["planned_date"];
    record.planned_time = values["planned_time"];
    record.responsible = values["responsible"];
    record.pilot = values["pilot"];
    record.observer = values["observer"];
    record.aircraft_id = values["aircraft_id"];
    record.sensor_id = values["sensor_id"];
    record.battery_ids = values["battery_ids"];
    record.weather = values["weather"];
    record.authorization_ref = values["authorization_ref"];
    record.operator_id = values["operator_id"];
    record.drone_qr = values["drone_qr"];
    record.aro_ref = values["aro_ref"];
    record.gps_latitude = values["gps_latitude"];
    record.gps_longitude = values["gps_longitude"];
    record.gps_accuracy_m = values["gps_accuracy_m"];
    record.photo_paths = values["photo_paths"];
    record.final_decision = values["final_decision"];
    record.flight_plan_path = values["flight_plan_path"];
    record.map_path = values["map_path"];
    record.evidence_paths = values["evidence_paths"];
    record.flight_log_paths = values["flight_log_paths"];
    record.occurrences = values["occurrences"];
    record.notes = values["notes"];
    record.status_text = values["status"].empty() ? "rascunho" : values["status"];
    record.status = mission_status_from_string(record.status_text);
    return record;
}

std::vector<std::string> recordToRow(const MissionRecord& record) {
    return {
        record.project_id,
        record.campaign_id,
        record.area_id,
        record.mission_id,
        record.objective,
        record.planned_date,
        record.planned_time,
        record.responsible,
        record.pilot,
        record.observer,
        record.aircraft_id,
        record.sensor_id,
        record.battery_ids,
        record.weather,
        record.authorization_ref,
        record.operator_id,
        record.drone_qr,
        record.aro_ref,
        record.gps_latitude,
        record.gps_longitude,
        record.gps_accuracy_m,
        record.photo_paths,
        record.final_decision,
        record.flight_plan_path,
        record.map_path,
        record.evidence_paths,
        record.flight_log_paths,
        record.occurrences,
        record.notes,
        std::string(to_string(calculate_status(record))),
    };
}

} // namespace

std::vector<std::string> defaultMissionHeaders() {
    return {
        "project_id",
        "campaign_id",
        "area_id",
        "mission_id",
        "objective",
        "planned_date",
        "planned_time",
        "responsible",
        "pilot",
        "observer",
        "aircraft_id",
        "sensor_id",
        "battery_ids",
        "weather",
        "authorization_ref",
        "operator_id",
        "drone_qr",
        "aro_ref",
        "gps_latitude",
        "gps_longitude",
        "gps_accuracy_m",
        "photo_paths",
        "final_decision",
        "flight_plan_path",
        "map_path",
        "evidence_paths",
        "flight_log_paths",
        "occurrences",
        "notes",
        "status",
    };
}

std::vector<std::string> parseCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (quoted) {
            if (c == '"' && i + 1 < line.size() && line[i + 1] == '"') {
                current.push_back('"');
                ++i;
            } else if (c == '"') {
                quoted = false;
            } else {
                current.push_back(c);
            }
        } else {
            if (c == '"') {
                quoted = true;
            } else if (c == ',') {
                fields.push_back(current);
                current.clear();
            } else {
                current.push_back(c);
            }
        }
    }
    fields.push_back(current);
    return fields;
}

ReadResult readMissions(const std::filesystem::path& input) {
    std::ifstream in(input);
    if (!in) {
        throw std::runtime_error("nao foi possivel abrir CSV: " + input.string());
    }

    ReadResult result;
    std::string line;
    std::size_t line_number = 0;
    std::vector<std::string> headers;
    while (std::getline(in, line)) {
        ++line_number;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line_number == 1) {
            headers = parseCsvLine(line);
            continue;
        }
        if (line.empty()) {
            continue;
        }
        result.records.push_back(rowToRecord(headers, parseCsvLine(line), line_number));
    }
    return result;
}

void writeMissionsCsv(const std::filesystem::path& output,
                      const std::vector<MissionRecord>& records) {
    ensureOutput(output);
    std::ofstream out(output);
    if (!out) {
        throw std::runtime_error("nao foi possivel escrever CSV: " + output.string());
    }
    writeCsvRow(out, defaultMissionHeaders());
    for (const auto& record : records) {
        writeCsvRow(out, recordToRow(record));
    }
}

void writeMissionsJsonl(const std::filesystem::path& output,
                        const std::vector<MissionRecord>& records) {
    ensureOutput(output);
    std::ofstream out(output);
    if (!out) {
        throw std::runtime_error("nao foi possivel escrever JSONL: " + output.string());
    }
    for (const auto& record : records) {
        out << "{\"project_id\":\"" << jsonEscape(record.project_id)
            << "\",\"campaign_id\":\"" << jsonEscape(record.campaign_id)
            << "\",\"area_id\":\"" << jsonEscape(record.area_id)
            << "\",\"mission_id\":\"" << jsonEscape(record.mission_id)
            << "\",\"responsible\":\"" << jsonEscape(record.responsible)
            << "\",\"aircraft_id\":\"" << jsonEscape(record.aircraft_id)
            << "\",\"sensor_id\":\"" << jsonEscape(record.sensor_id)
            << "\",\"operator_id\":\"" << jsonEscape(record.operator_id)
            << "\",\"drone_qr\":\"" << jsonEscape(record.drone_qr)
            << "\",\"aro_ref\":\"" << jsonEscape(record.aro_ref)
            << "\",\"gps_latitude\":\"" << jsonEscape(record.gps_latitude)
            << "\",\"gps_longitude\":\"" << jsonEscape(record.gps_longitude)
            << "\",\"gps_accuracy_m\":\"" << jsonEscape(record.gps_accuracy_m)
            << "\",\"photo_paths\":\"" << jsonEscape(record.photo_paths)
            << "\",\"final_decision\":\"" << jsonEscape(record.final_decision)
            << "\",\"status\":\"" << to_string(calculate_status(record)) << "\"}\n";
    }
}

void writeTemplateCsv(const std::filesystem::path& output) {
    MissionRecord record;
    record.project_id = "SISTER-CAMPO";
    record.campaign_id = "CAMP-2026-001";
    record.area_id = "AREA-001";
    record.mission_id = "DO-001";
    record.objective = "Mapeamento de reconhecimento";
    record.planned_date = "2026-07-08";
    record.planned_time = "09:00";
    record.responsible = "responsavel_operacional";
    record.operator_id = "responsavel_operacional";
    record.pilot = "piloto";
    record.observer = "observador";
    record.aircraft_id = "DRONE-001";
    record.sensor_id = "RGB-001";
    record.battery_ids = "BAT-001;BAT-002";
    record.weather = "ceu claro; vento baixo";
    record.drone_qr = "DRONE-001";
    record.aro_ref = "SISTER-ARO-2026-001";
    record.gps_latitude = "-30.000000";
    record.gps_longitude = "-51.000000";
    record.gps_accuracy_m = "10";
    record.photo_paths = "evidencias/fotos/drone.jpg";
    record.final_decision = "apto_para_voo";
    record.flight_plan_path = "droneops/planos_voo/DO-001.plan";
    record.map_path = "droneops/mapas/DO-001.geojson";
    writeMissionsCsv(output, {record});
}

} // namespace droneops::csv

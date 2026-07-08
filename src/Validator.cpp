#include "droneops/Validator.hpp"

#include <regex>
#include <unordered_map>

namespace droneops::validator {
namespace {

void requireField(std::vector<ValidationIssue>& issues,
                  const MissionRecord& record,
                  const std::string& field,
                  const std::string& value) {
    if (value.empty()) {
        issues.push_back({Severity::Error, record.source_line, field, "campo obrigatorio ausente"});
    }
}

std::string duplicateKey(const MissionRecord& record) {
    return record.project_id + "|" + record.campaign_id + "|" + record.area_id + "|" + record.mission_id;
}

} // namespace

std::vector<ValidationIssue> validateMissions(const std::vector<MissionRecord>& records) {
    std::vector<ValidationIssue> issues;
    const std::regex iso_date(R"(^\d{4}-\d{2}-\d{2}$)");
    std::unordered_map<std::string, std::size_t> first_seen;

    for (const auto& record : records) {
        requireField(issues, record, "project_id", record.project_id);
        requireField(issues, record, "campaign_id", record.campaign_id);
        requireField(issues, record, "area_id", record.area_id);
        requireField(issues, record, "mission_id", record.mission_id);
        requireField(issues, record, "responsible", record.responsible);

        if (!record.planned_date.empty() && !std::regex_match(record.planned_date, iso_date)) {
            issues.push_back({Severity::Error, record.source_line, "planned_date",
                              "deve estar no formato ISO YYYY-MM-DD"});
        }

        if (record.aircraft_id.empty()) {
            issues.push_back({Severity::Warning, record.source_line, "aircraft_id",
                              "aeronave ainda nao identificada"});
        }
        if (record.objective.empty()) {
            issues.push_back({Severity::Warning, record.source_line, "objective",
                              "objetivo da missao vazio"});
        }
        if (record.authorization_ref.empty()) {
            issues.push_back({Severity::Warning, record.source_line, "authorization_ref",
                              "documento/autorizacao nao anexado ou nao informado"});
        }
        if (record.operator_id.empty()) {
            issues.push_back({Severity::Warning, record.source_line, "operator_id",
                              "operador ainda nao identificado no certificado de protocolo"});
        }
        if (record.drone_qr.empty()) {
            issues.push_back({Severity::Warning, record.source_line, "drone_qr",
                              "drone ainda nao identificado por QR Code ou confirmacao manual"});
        }
        if (record.gps_latitude.empty() || record.gps_longitude.empty()) {
            issues.push_back({Severity::Warning, record.source_line, "gps",
                              "localizacao GPS do celular ainda nao registrada"});
        }
        if (record.photo_paths.empty()) {
            issues.push_back({Severity::Warning, record.source_line, "photo_paths",
                              "fotos de comprovacao ainda nao anexadas"});
        }
        if (record.final_decision == "bloqueado" || record.final_decision == "voo_nao_autorizado"
            || record.final_decision == "cancelado") {
            issues.push_back({Severity::Error, record.source_line, "final_decision",
                              "decisao final bloqueia a operacao ate revisao"});
        }

        if (has_open_occurrence(record)) {
            issues.push_back({Severity::Error, record.source_line, "occurrences",
                              "ocorrencia aberta bloqueia conclusao automatica"});
        }

        if (!record.project_id.empty() && !record.campaign_id.empty() && !record.area_id.empty()
            && !record.mission_id.empty()) {
            const auto key = duplicateKey(record);
            const auto [it, inserted] = first_seen.emplace(key, record.source_line);
            if (!inserted) {
                issues.push_back({Severity::Error, record.source_line, "duplicate_key",
                                  "missao duplicada; primeira ocorrencia na linha "
                                      + std::to_string(it->second)});
            }
        }
    }

    return issues;
}

} // namespace droneops::validator

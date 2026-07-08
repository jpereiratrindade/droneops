#include "droneops/Mission.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace droneops {
namespace {

std::string lower_ascii(std::string_view value) {
    std::string out;
    out.reserve(value.size());
    for (unsigned char c : value) {
        out.push_back(static_cast<char>(std::tolower(c)));
    }
    return out;
}

bool is_truthy_path_list(const std::string& value) {
    return std::ranges::any_of(value, [](unsigned char c) {
        return !std::isspace(c) && c != ';' && c != ',';
    });
}

} // namespace

std::string_view to_string(const MissionStatus status) {
    switch (status) {
    case MissionStatus::Draft:
        return "rascunho";
    case MissionStatus::Planned:
        return "planejada";
    case MissionStatus::Ready:
        return "apta";
    case MissionStatus::Executed:
        return "executada";
    case MissionStatus::Reviewed:
        return "revisada";
    case MissionStatus::Blocked:
        return "bloqueada";
    case MissionStatus::Packaged:
        return "empacotada";
    case MissionStatus::Synced:
        return "sincronizada";
    }
    return "bloqueada";
}

MissionStatus mission_status_from_string(const std::string_view value) {
    const auto normalized = lower_ascii(value);
    if (normalized == "planejada") {
        return MissionStatus::Planned;
    }
    if (normalized == "apta") {
        return MissionStatus::Ready;
    }
    if (normalized == "executada") {
        return MissionStatus::Executed;
    }
    if (normalized == "revisada") {
        return MissionStatus::Reviewed;
    }
    if (normalized == "bloqueada") {
        return MissionStatus::Blocked;
    }
    if (normalized == "empacotada") {
        return MissionStatus::Packaged;
    }
    if (normalized == "sincronizada") {
        return MissionStatus::Synced;
    }
    return MissionStatus::Draft;
}

bool required_items_checked(const std::vector<ChecklistItem>& items) {
    for (const auto& item : items) {
        if (item.required && !item.checked) {
            return false;
        }
    }
    return true;
}

bool has_open_occurrence(const MissionRecord& mission) {
    return is_truthy_path_list(mission.occurrences);
}

bool has_minimum_identity(const MissionRecord& mission) {
    return !mission.project_id.empty() && !mission.campaign_id.empty() && !mission.area_id.empty()
        && !mission.mission_id.empty() && !mission.responsible.empty();
}

bool has_execution_evidence(const MissionRecord& mission) {
    return is_truthy_path_list(mission.evidence_paths) || is_truthy_path_list(mission.flight_log_paths);
}

MissionStatus calculate_status(const MissionRecord& mission) {
    if (!has_minimum_identity(mission) || has_open_occurrence(mission)) {
        return MissionStatus::Blocked;
    }

    if (mission.aircraft_id.empty() || mission.objective.empty() || mission.planned_date.empty()) {
        return MissionStatus::Draft;
    }

    if (mission.preflight_checklist.empty() || !required_items_checked(mission.preflight_checklist)) {
        return MissionStatus::Planned;
    }

    if (!has_execution_evidence(mission)) {
        return MissionStatus::Ready;
    }

    if (mission.postflight_checklist.empty() || !required_items_checked(mission.postflight_checklist)) {
        return MissionStatus::Executed;
    }

    return MissionStatus::Reviewed;
}

std::vector<ChecklistItem> default_preflight_checklist() {
    return {
        {"pre-001", "Responsavel operacional definido", false, true},
        {"pre-002", "Aeronave identificada", false, true},
        {"pre-003", "Baterias verificadas", false, true},
        {"pre-004", "Plano de voo/documento anexado quando aplicavel", false, true},
        {"pre-005", "Condicoes ambientais registradas", false, true},
    };
}

std::vector<ChecklistItem> default_postflight_checklist() {
    return {
        {"post-001", "Logs de voo preservados", false, true},
        {"post-002", "Evidencias principais anexadas", false, true},
        {"post-003", "Ocorrencias registradas ou confirmadas ausentes", false, true},
    };
}

} // namespace droneops


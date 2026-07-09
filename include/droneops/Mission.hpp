#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace droneops {

enum class Severity {
    Error,
    Warning
};

struct ValidationIssue {
    Severity severity{Severity::Error};
    std::size_t line{0};
    std::string field;
    std::string message;
};

enum class MissionStatus {
    Draft,
    Planned,
    Ready,
    Executed,
    Reviewed,
    Blocked,
    Packaged,
    Synced
};

struct ChecklistItem {
    std::string id;
    std::string label;
    bool checked{false};
    bool required{true};
};

struct MissionRecord {
    std::size_t source_line{0};

    std::string project_id;
    std::string campaign_id;
    std::string area_id;
    std::string mission_id;
    std::string objective;
    std::string planned_date;
    std::string planned_time;
    std::string responsible;
    std::string pilot;
    std::string observer;
    std::string aircraft_id;
    std::string sensor_id;
    std::string battery_ids;
    std::string weather;
    std::string authorization_ref;
    std::string operator_id;
    std::string drone_qr;
    std::string aro_ref;
    std::string gps_latitude;
    std::string gps_longitude;
    std::string gps_accuracy_m;
    std::string photo_paths;
    std::string final_decision;
    std::string flight_plan_path;
    std::string map_path;
    std::string evidence_paths;
    std::string flight_log_paths;
    std::string occurrences;
    std::string notes;
    std::string status_text{"rascunho"};
    std::string sync_status{"nao_sincronizado"};

    std::vector<ChecklistItem> preflight_checklist;
    std::vector<ChecklistItem> postflight_checklist;
    MissionStatus status{MissionStatus::Draft};
};

std::string_view to_string(MissionStatus status);
MissionStatus mission_status_from_string(std::string_view value);

bool required_items_checked(const std::vector<ChecklistItem>& items);
bool has_open_occurrence(const MissionRecord& mission);
bool has_minimum_identity(const MissionRecord& mission);
bool has_execution_evidence(const MissionRecord& mission);
MissionStatus calculate_status(const MissionRecord& mission);

std::vector<ChecklistItem> default_preflight_checklist();
std::vector<ChecklistItem> default_postflight_checklist();

} // namespace droneops

#include "droneops/Validator.hpp"

#include <cstdlib>
#include <iostream>

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "Falha: " << message << '\n';
        std::exit(1);
    }
}

bool hasErrorFor(const std::vector<droneops::ValidationIssue>& issues, const std::string& field) {
    for (const auto& issue : issues) {
        if (issue.severity == droneops::Severity::Error && issue.field == field) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    droneops::MissionRecord mission;
    mission.source_line = 2;
    mission.project_id = "SISTER-CAMPO";
    mission.campaign_id = "CAMP-2026-001";
    mission.area_id = "AREA-001";
    mission.mission_id = "DO-001";
    mission.responsible = "Operador";
    mission.planned_date = "08/07/2026";

    const auto issues = droneops::validator::validateMissions({mission});
    expect(hasErrorFor(issues, "planned_date"), "data fora do ISO deve gerar erro");

    mission.planned_date = "2026-07-08";
    const auto duplicate_issues = droneops::validator::validateMissions({mission, mission});
    expect(hasErrorFor(duplicate_issues, "duplicate_key"), "missao duplicada deve gerar erro");
}


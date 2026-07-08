#include "droneops/Mission.hpp"

#include <cstdlib>
#include <iostream>

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "Falha: " << message << '\n';
        std::exit(1);
    }
}

droneops::MissionRecord baseMission() {
    droneops::MissionRecord mission;
    mission.project_id = "SISTER-CAMPO";
    mission.campaign_id = "CAMP-2026-001";
    mission.area_id = "AREA-001";
    mission.mission_id = "DO-001";
    mission.objective = "Mapeamento";
    mission.planned_date = "2026-07-08";
    mission.responsible = "Operador";
    mission.aircraft_id = "DRONE-001";
    mission.preflight_checklist = {
        {"pre-001", "Responsavel", true, true},
        {"pre-002", "Baterias", true, true},
    };
    return mission;
}

} // namespace

int main() {
    auto mission = baseMission();
    expect(droneops::calculate_status(mission) == droneops::MissionStatus::Ready,
           "missao apta sem evidencias de execucao");

    mission.flight_log_paths = "droneops/logs_voo/DO-001.log";
    expect(droneops::calculate_status(mission) == droneops::MissionStatus::Executed,
           "missao executada com log sem pos-voo");

    mission.postflight_checklist = {
        {"post-001", "Logs preservados", true, true},
        {"post-002", "Evidencias anexadas", true, true},
    };
    expect(droneops::calculate_status(mission) == droneops::MissionStatus::Reviewed,
           "missao revisada com pos-voo completo");

    mission.occurrences = "Falha operacional";
    expect(droneops::calculate_status(mission) == droneops::MissionStatus::Blocked,
           "ocorrencia aberta bloqueia missao");
}


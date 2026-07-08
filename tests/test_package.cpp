#include "droneops/Package.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "Falha: " << message << '\n';
        std::exit(1);
    }
}

} // namespace

int main() {
    const auto root = std::filesystem::temp_directory_path() / "droneops_package_test";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);

    droneops::MissionRecord mission;
    mission.project_id = "SISTER-CAMPO";
    mission.campaign_id = "CAMP-2026-001";
    mission.area_id = "AREA-001";
    mission.mission_id = "DO-001";
    mission.responsible = "Operador";

    droneops::package::writeMissionPackage(root, root / "pacote", mission);

    expect(std::filesystem::exists(root / "pacote" / "manifest.json"), "manifesto deve ser gerado");
    expect(std::filesystem::exists(root / "pacote" / "droneops" / "missao_DO-001.json"),
           "json de missao deve ser gerado");
    expect(std::filesystem::exists(root / "pacote" / "droneops" / "certificado_protocolo_DO-001.json"),
           "certificado de protocolo deve ser gerado");

    const auto manifest = droneops::package::buildManifest(root / "pacote", mission);
    expect(!manifest.files.empty(), "manifesto deve listar arquivos");

    std::filesystem::remove_all(root);
}

#include "droneops/Package.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace droneops::package {
namespace {

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

std::string hex64(const std::uint64_t value) {
    std::ostringstream out;
    out << std::hex << std::setw(16) << std::setfill('0') << value;
    return out.str();
}

void writeTextFile(const std::filesystem::path& path, const std::string& text) {
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("nao foi possivel escrever arquivo: " + path.string());
    }
    out << text;
}

std::string missionJson(const MissionRecord& mission) {
    std::ostringstream out;
    out << "{\n"
        << "  \"project_id\": \"" << jsonEscape(mission.project_id) << "\",\n"
        << "  \"campaign_id\": \"" << jsonEscape(mission.campaign_id) << "\",\n"
        << "  \"area_id\": \"" << jsonEscape(mission.area_id) << "\",\n"
        << "  \"mission_id\": \"" << jsonEscape(mission.mission_id) << "\",\n"
        << "  \"objective\": \"" << jsonEscape(mission.objective) << "\",\n"
        << "  \"planned_date\": \"" << jsonEscape(mission.planned_date) << "\",\n"
        << "  \"responsible\": \"" << jsonEscape(mission.responsible) << "\",\n"
        << "  \"aircraft_id\": \"" << jsonEscape(mission.aircraft_id) << "\",\n"
        << "  \"sensor_id\": \"" << jsonEscape(mission.sensor_id) << "\",\n"
        << "  \"authorization_ref\": \"" << jsonEscape(mission.authorization_ref) << "\",\n"
        << "  \"operator_id\": \"" << jsonEscape(mission.operator_id) << "\",\n"
        << "  \"drone_qr\": \"" << jsonEscape(mission.drone_qr) << "\",\n"
        << "  \"aro_ref\": \"" << jsonEscape(mission.aro_ref) << "\",\n"
        << "  \"gps_latitude\": \"" << jsonEscape(mission.gps_latitude) << "\",\n"
        << "  \"gps_longitude\": \"" << jsonEscape(mission.gps_longitude) << "\",\n"
        << "  \"gps_accuracy_m\": \"" << jsonEscape(mission.gps_accuracy_m) << "\",\n"
        << "  \"photo_paths\": \"" << jsonEscape(mission.photo_paths) << "\",\n"
        << "  \"final_decision\": \"" << jsonEscape(mission.final_decision) << "\",\n"
        << "  \"status\": \"" << to_string(calculate_status(mission)) << "\"\n"
        << "}\n";
    return out.str();
}

std::string protocolCertificateJson(const MissionRecord& mission) {
    std::ostringstream out;
    out << "{\n"
        << "  \"schema\": \"droneops.protocol_certificate.v1\",\n"
        << "  \"mission_id\": \"" << jsonEscape(mission.mission_id) << "\",\n"
        << "  \"operator_id\": \"" << jsonEscape(mission.operator_id.empty() ? mission.responsible : mission.operator_id) << "\",\n"
        << "  \"drone_qr\": \"" << jsonEscape(mission.drone_qr.empty() ? mission.aircraft_id : mission.drone_qr) << "\",\n"
        << "  \"aro_ref\": \"" << jsonEscape(mission.aro_ref) << "\",\n"
        << "  \"latitude\": \"" << jsonEscape(mission.gps_latitude) << "\",\n"
        << "  \"longitude\": \"" << jsonEscape(mission.gps_longitude) << "\",\n"
        << "  \"accuracy_m\": \"" << jsonEscape(mission.gps_accuracy_m) << "\",\n"
        << "  \"photo_paths\": \"" << jsonEscape(mission.photo_paths) << "\",\n"
        << "  \"final_decision\": \"" << jsonEscape(mission.final_decision.empty() ? "rascunho" : mission.final_decision) << "\"\n"
        << "}\n";
    return out.str();
}

} // namespace

std::string stable_file_hash(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("nao foi possivel abrir arquivo para hash: " + path.string());
    }

    // FNV-1a 64-bit: simples, deterministico e suficiente para manifesto inicial.
    std::uint64_t hash = 14695981039346656037ULL;
    char c = 0;
    while (in.get(c)) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ULL;
    }
    return "fnv1a64:" + hex64(hash);
}

PackageManifest buildManifest(const std::filesystem::path& package_root,
                              const MissionRecord& mission) {
    PackageManifest manifest;
    manifest.package_id = "PKG-DRONEOPS-" + mission.mission_id;
    manifest.project_id = mission.project_id;
    manifest.campaign_id = mission.campaign_id;
    manifest.area_id = mission.area_id;
    manifest.mission_id = mission.mission_id;
    manifest.human_responsible = mission.responsible;
    manifest.validation_status = has_open_occurrence(mission) ? "bloqueado" : "validado";

    for (const auto& entry : std::filesystem::recursive_directory_iterator(package_root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto relative = std::filesystem::relative(entry.path(), package_root).generic_string();
        if (relative == "manifest.json") {
            continue;
        }
        manifest.files.push_back({relative, entry.file_size(), stable_file_hash(entry.path())});
    }
    return manifest;
}

void writeManifestJson(const std::filesystem::path& output,
                       const PackageManifest& manifest) {
    if (output.has_parent_path()) {
        std::filesystem::create_directories(output.parent_path());
    }
    std::ofstream out(output);
    if (!out) {
        throw std::runtime_error("nao foi possivel escrever manifesto: " + output.string());
    }

    out << "{\n"
        << "  \"package_id\": \"" << jsonEscape(manifest.package_id) << "\",\n"
        << "  \"droneops_version\": \"" << jsonEscape(manifest.droneops_version) << "\",\n"
        << "  \"camposync_contract_version\": \"" << jsonEscape(manifest.camposync_contract_version) << "\",\n"
        << "  \"camponode_id\": \"" << jsonEscape(manifest.camponode_id) << "\",\n"
        << "  \"project_id\": \"" << jsonEscape(manifest.project_id) << "\",\n"
        << "  \"campaign_id\": \"" << jsonEscape(manifest.campaign_id) << "\",\n"
        << "  \"area_id\": \"" << jsonEscape(manifest.area_id) << "\",\n"
        << "  \"mission_id\": \"" << jsonEscape(manifest.mission_id) << "\",\n"
        << "  \"origin_module\": \"" << jsonEscape(manifest.origin_module) << "\",\n"
        << "  \"human_responsible\": \"" << jsonEscape(manifest.human_responsible) << "\",\n"
        << "  \"validation_status\": \"" << jsonEscape(manifest.validation_status) << "\",\n"
        << "  \"sync_status\": \"" << jsonEscape(manifest.sync_status) << "\",\n"
        << "  \"expected_destination\": \"" << jsonEscape(manifest.expected_destination) << "\",\n"
        << "  \"files\": [\n";

    for (std::size_t i = 0; i < manifest.files.size(); ++i) {
        const auto& file = manifest.files[i];
        out << "    {\"path\": \"" << jsonEscape(file.path) << "\", \"size\": " << file.size
            << ", \"hash\": \"" << jsonEscape(file.hash) << "\"}";
        if (i + 1 < manifest.files.size()) {
            out << ',';
        }
        out << '\n';
    }
    out << "  ]\n}\n";
}

void writeMissionPackage(const std::filesystem::path& campaign_dir,
                         const std::filesystem::path& output_dir,
                         const MissionRecord& mission) {
    std::filesystem::create_directories(output_dir / "droneops" / "checklists");
    std::filesystem::create_directories(output_dir / "droneops" / "mapas");
    std::filesystem::create_directories(output_dir / "droneops" / "planos_voo");
    std::filesystem::create_directories(output_dir / "droneops" / "logs_voo");
    std::filesystem::create_directories(output_dir / "evidencias" / "documentos");
    std::filesystem::create_directories(output_dir / "logs");

    writeTextFile(output_dir / "metadata.json",
                  "{\n  \"module\": \"droneops\",\n  \"source_dir\": \""
                      + jsonEscape(campaign_dir.string()) + "\"\n}\n");
    writeTextFile(output_dir / "droneops" / ("missao_" + mission.mission_id + ".json"),
                  missionJson(mission));
    writeTextFile(output_dir / "droneops" / ("certificado_protocolo_" + mission.mission_id + ".json"),
                  protocolCertificateJson(mission));
    writeTextFile(output_dir / "logs" / "package.log",
                  "pacote CampoSync gerado para missao " + mission.mission_id + "\n");

    const auto manifest = buildManifest(output_dir, mission);
    writeManifestJson(output_dir / "manifest.json", manifest);
}

} // namespace droneops::package

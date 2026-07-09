#include "droneops/Csv.hpp"
#include "droneops/Package.hpp"
#include "droneops/Sqlite.hpp"
#include "droneops/Validator.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void printHelp() {
    std::cout
        << "droneops - planejamento, checklist e CampoSync para missoes com drones\n\n"
        << "Uso:\n"
        << "  droneops init --dir examples\n"
        << "  droneops init-mission --dir missao_DO001 --project SISTER-CAMPO --campaign CAMP-2026-001 "
           "--area AREA-001 --mission DO-001 --responsible \"Nome\" --aircraft DRONE-001 "
           "[--db missao_DO001/droneops.db]\n"
        << "  droneops validate --input missao_DO001/droneops/missoes.csv --out missao_DO001/validacao\n"
        << "  droneops validate --db missao_DO001/droneops.db --out missao_DO001/validacao\n"
        << "  droneops package --dir missao_DO001 --mission DO-001 --out missao_DO001/pacote [--db missao_DO001/droneops.db]\n"
        << "  droneops help\n\n"
        << "O DroneOps registra operacao e evidencias, mas nao substitui autorizacao, norma ou "
        << "responsavel humano.\n";
}

std::string optionValue(const std::vector<std::string>& args, const std::string& name) {
    for (std::size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == name) {
            return args[i + 1];
        }
    }
    return {};
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

std::string severityName(const droneops::Severity severity) {
    return severity == droneops::Severity::Error ? "ERRO" : "AVISO";
}

void writeValidationReport(const std::filesystem::path& output,
                           const std::vector<droneops::MissionRecord>& records,
                           const std::vector<droneops::ValidationIssue>& issues) {
    if (output.has_parent_path()) {
        std::filesystem::create_directories(output.parent_path());
    }
    std::ofstream out(output);
    if (!out) {
        throw std::runtime_error("nao foi possivel escrever relatorio: " + output.string());
    }

    out << "# Relatorio de validacao DroneOps\n\n";
    out << "- Missoes lidas: " << records.size() << "\n";
    out << "- Ocorrencias de validacao: " << issues.size() << "\n\n";

    out << "## Status das missoes\n\n";
    out << "| Missao | Projeto | Campanha | Area | Status |\n";
    out << "| --- | --- | --- | --- | --- |\n";
    for (const auto& record : records) {
        out << "| " << record.mission_id << " | " << record.project_id << " | " << record.campaign_id
            << " | " << record.area_id << " | " << droneops::to_string(droneops::calculate_status(record))
            << " |\n";
    }

    out << "\n## Pendencias\n\n";
    if (issues.empty()) {
        out << "Nenhuma pendencia encontrada.\n";
        return;
    }
    out << "| Severidade | Linha | Campo | Mensagem |\n";
    out << "| --- | ---: | --- | --- |\n";
    for (const auto& issue : issues) {
        out << "| " << severityName(issue.severity) << " | " << issue.line << " | " << issue.field
            << " | " << issue.message << " |\n";
    }
}

int runInit(const std::vector<std::string>& args) {
    const auto dir_arg = optionValue(args, "--dir");
    const std::filesystem::path dir = dir_arg.empty() ? "examples" : dir_arg;
    std::filesystem::create_directories(dir);

    droneops::csv::writeTemplateCsv(dir / "template_missoes.csv");
    writeTextFile(dir / "README_EXEMPLOS.md",
                  "# Exemplos DroneOps\n\n"
                  "- `template_missoes.csv`: planilha minima de missoes.\n\n"
                  "Comandos:\n\n"
                  "```sh\n"
                  "droneops validate --input examples/template_missoes.csv --out out\n"
                  "```\n");
    std::cout << "Arquivos de exemplo gerados em " << dir << '\n';
    return 0;
}

int runInitMission(const std::vector<std::string>& args) {
    const auto dir_arg = optionValue(args, "--dir");
    const auto project = optionValue(args, "--project");
    const auto campaign = optionValue(args, "--campaign");
    const auto area = optionValue(args, "--area");
    const auto mission_id = optionValue(args, "--mission");
    const auto responsible = optionValue(args, "--responsible");
    const auto aircraft = optionValue(args, "--aircraft");
    const auto sensor = optionValue(args, "--sensor");
    const auto db_arg = optionValue(args, "--db");

    if (dir_arg.empty() || project.empty() || campaign.empty() || area.empty() || mission_id.empty()
        || responsible.empty() || aircraft.empty()) {
        throw std::runtime_error(
            "init-mission requer --dir, --project, --campaign, --area, --mission, --responsible e --aircraft");
    }

    const std::filesystem::path dir(dir_arg);
    std::filesystem::create_directories(dir / "droneops" / "checklists");
    std::filesystem::create_directories(dir / "droneops" / "mapas");
    std::filesystem::create_directories(dir / "droneops" / "planos_voo");
    std::filesystem::create_directories(dir / "droneops" / "logs_voo");
    std::filesystem::create_directories(dir / "evidencias" / "documentos");
    std::filesystem::create_directories(dir / "evidencias" / "fotos");
    std::filesystem::create_directories(dir / "logs");

    droneops::MissionRecord mission;
    mission.project_id = project;
    mission.campaign_id = campaign;
    mission.area_id = area;
    mission.mission_id = mission_id;
    mission.objective = "Definir objetivo da missao";
    mission.planned_date = "2026-07-08";
    mission.responsible = responsible;
    mission.operator_id = responsible;
    mission.aircraft_id = aircraft;
    mission.sensor_id = sensor;
    mission.drone_qr = aircraft;
    mission.aro_ref = "informar ARO vigente";
    mission.final_decision = "rascunho";
    mission.photo_paths = "evidencias/fotos/";
    mission.flight_plan_path = "droneops/planos_voo/" + mission_id + ".plan";
    mission.map_path = "droneops/mapas/" + mission_id + ".geojson";

    if (!db_arg.empty()) {
        droneops::sqlite::SqliteStore store(db_arg);
        store.initSchema();
        store.saveMission(mission);
        std::cout << "Banco SQLite inicializado em " << db_arg << '\n';
    } else {
        droneops::csv::writeMissionsCsv(dir / "droneops" / "missoes.csv", {mission});
        std::cout << "Planilha gerada em: " << (dir / "droneops" / "missoes.csv") << '\n';
    }

    writeTextFile(dir / "droneops" / "checklists" / (mission_id + "_pre_voo.md"),
                  "# Checklist pre-voo\n\n"
                  "- [ ] Responsavel operacional definido\n"
                  "- [ ] Aeronave identificada\n"
                  "- [ ] Baterias verificadas\n"
                  "- [ ] Plano/documento de voo anexado quando aplicavel\n"
                  "- [ ] Condicoes ambientais registradas\n");
    writeTextFile(dir / "droneops" / "checklists" / (mission_id + "_pos_voo.md"),
                  "# Checklist pos-voo\n\n"
                  "- [ ] Logs de voo preservados\n"
                  "- [ ] Evidencias principais anexadas\n"
                  "- [ ] Ocorrencias registradas ou confirmadas ausentes\n");
    writeTextFile(dir / "README_MISSAO.md",
                  "# Missao DroneOps\n\n"
                  "- Projeto: `" + project + "`\n"
                  "- Campanha: `" + campaign + "`\n"
                  "- Area: `" + area + "`\n"
                  "- Missao: `" + mission_id + "`\n"
                  "- Responsavel: `" + responsible + "`\n"
                  "- Aeronave: `" + aircraft + "`\n\n"
                  "## Certificacao de protocolo\n\n"
                  "1. Identifique operador e drone por QR Code ou entrada manual.\n"
                  "2. Registre GPS do celular no local da aplicacao.\n"
                  "3. Anexe fotos em `evidencias/fotos/`.\n"
                  "4. Preencha a decisao final em `droneops/missoes.csv`.\n\n"
                  "## Validar\n\n"
                  "```sh\n"
                  "droneops validate " + (!db_arg.empty() ? "--db " + db_arg : "--input " + (dir / "droneops" / "missoes.csv").string())
                      + " --out " + (dir / "validacao").string() + "\n"
                  "```\n\n"
                  "## Gerar pacote CampoSync\n\n"
                  "```sh\n"
                  "droneops package --dir " + dir.string() + " --mission " + mission_id
                      + (!db_arg.empty() ? " --db " + db_arg : "") + " --out " + (dir / "pacote").string() + "\n"
                  "```\n");

    std::cout << "Missao DroneOps inicializada em " << dir << '\n';
    return 0;
}

int runValidate(const std::vector<std::string>& args) {
    const auto input = optionValue(args, "--input");
    const auto db_arg = optionValue(args, "--db");
    const auto out_dir = optionValue(args, "--out");
    if ((input.empty() && db_arg.empty()) || out_dir.empty()) {
        throw std::runtime_error("validate requer --input ou --db, e --out");
    }

    std::vector<droneops::MissionRecord> records;
    std::vector<droneops::ValidationIssue> issues;

    if (!db_arg.empty()) {
        droneops::sqlite::SqliteStore store(db_arg);
        records = store.getAllMissions();
    } else {
        auto read = droneops::csv::readMissions(input);
        records = read.records;
        issues.insert(issues.begin(), read.issues.begin(), read.issues.end());
    }

    auto domain_issues = droneops::validator::validateMissions(records);
    issues.insert(issues.end(), domain_issues.begin(), domain_issues.end());

    const std::filesystem::path out_path(out_dir);
    std::filesystem::create_directories(out_path);
    droneops::csv::writeMissionsCsv(out_path / "missoes_normalizadas.csv", records);
    droneops::csv::writeMissionsJsonl(out_path / "missoes_normalizadas.jsonl", records);
    writeValidationReport(out_path / "relatorio_validacao.md", records, issues);

    std::cout << "Validacao DroneOps concluida em " << out_path << '\n';
    return 0;
}

int runPackage(const std::vector<std::string>& args) {
    const auto dir_arg = optionValue(args, "--dir");
    const auto mission_id = optionValue(args, "--mission");
    const auto db_arg = optionValue(args, "--db");
    const auto out_arg = optionValue(args, "--out");
    if (dir_arg.empty() || mission_id.empty() || out_arg.empty()) {
        throw std::runtime_error("package requer --dir, --mission e --out");
    }

    const std::filesystem::path dir(dir_arg);
    std::vector<droneops::MissionRecord> records;

    if (!db_arg.empty()) {
        droneops::sqlite::SqliteStore store(db_arg);
        records = store.getAllMissions();
    } else {
        auto read = droneops::csv::readMissions(dir / "droneops" / "missoes.csv");
        records = read.records;
    }

    for (const auto& mission : records) {
        if (mission.mission_id == mission_id) {
            droneops::package::writeMissionPackage(dir, out_arg, mission);
            std::cout << "Pacote CampoSync gerado em " << out_arg << '\n';
            return 0;
        }
    }

    throw std::runtime_error("missao nao encontrada: " + mission_id);
}

} // namespace

int main(int argc, char** argv) {
    try {
        const std::vector<std::string> args(argv + 1, argv + argc);
        if (args.empty() || args[0] == "help" || args[0] == "--help" || args[0] == "-h") {
            printHelp();
            return 0;
        }
        const auto command = args[0];
        if (command == "init") {
            return runInit(args);
        }
        if (command == "init-mission") {
            return runInitMission(args);
        }
        if (command == "validate") {
            return runValidate(args);
        }
        if (command == "package") {
            return runPackage(args);
        }
        throw std::runtime_error("comando desconhecido: " + command);
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << '\n';
        return 1;
    }
}

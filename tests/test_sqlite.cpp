#include "droneops/Sqlite.hpp"
#include <cassert>
#include <iostream>

void test_sqlite_store() {
    // Banco em memoria
    droneops::sqlite::SqliteStore store(":memory:");
    store.initSchema();

    droneops::MissionRecord m1;
    m1.mission_id = "DO-001";
    m1.project_id = "PROJ-1";
    m1.campaign_id = "CAMP-1";
    m1.status_text = "rascunho";
    store.saveMission(m1);

    droneops::MissionRecord m2;
    m2.mission_id = "DO-002";
    m2.project_id = "PROJ-2";
    m2.campaign_id = "CAMP-2";
    m2.status_text = "bloqueado";
    store.saveMission(m2);

    auto missions = store.getAllMissions();
    assert(missions.size() == 2);
    
    auto retrieved = store.getMission("DO-001");
    assert(retrieved.project_id == "PROJ-1");
    assert(retrieved.status_text == "rascunho");

    auto retrieved2 = store.getMission("DO-002");
    assert(retrieved2.campaign_id == "CAMP-2");
    
    // Atualizar
    m1.status_text = "executada";
    store.saveMission(m1);
    
    auto updated = store.getMission("DO-001");
    assert(updated.status_text == "executada");
    assert(updated.project_id == "PROJ-1");

    std::cout << "test_sqlite_store passed.\n";
}

int main() {
    test_sqlite_store();
    return 0;
}

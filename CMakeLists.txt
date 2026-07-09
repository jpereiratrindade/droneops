cmake_minimum_required(VERSION 3.20)

project(droneops VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(droneops_core
    src/Csv.cpp
    src/Mission.cpp
    src/Package.cpp
    src/Validator.cpp
    src/Sqlite.cpp
    src/Server.cpp
)

include(FetchContent)
FetchContent_Declare(
    sqlite3
    URL https://sqlite.org/2024/sqlite-amalgamation-3450100.zip
    URL_HASH SHA256=5592243caf28b2cdef41e6ab58d25d653dfc53deded8450eb66072c929f030c4
)
FetchContent_MakeAvailable(sqlite3)

# sqlite-amalgamation doesn't provide a CMake target directly by default,
# so we compile it as a static library
add_library(sqlite3_static STATIC 
    ${sqlite3_SOURCE_DIR}/sqlite3.c
)
target_include_directories(sqlite3_static PUBLIC ${sqlite3_SOURCE_DIR})

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    httplib
    URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.15.3.tar.gz
    URL_HASH SHA256=2121bbf38871bb2aafb5f7f2b9b94705366170909f434428352187cb0216124e
)
FetchContent_MakeAvailable(httplib)

target_link_libraries(droneops_core PUBLIC sqlite3_static nlohmann_json::nlohmann_json httplib::httplib)

target_include_directories(droneops_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(droneops_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(droneops src/main.cpp)
target_link_libraries(droneops PRIVATE droneops_core)
target_compile_options(droneops PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()

add_executable(test_mission tests/test_mission.cpp)
target_link_libraries(test_mission PRIVATE droneops_core)
add_test(NAME test_mission COMMAND test_mission)

add_executable(test_validator tests/test_validator.cpp)
target_link_libraries(test_validator PRIVATE droneops_core)
add_test(NAME test_validator COMMAND test_validator)

add_executable(test_package tests/test_package.cpp)
target_link_libraries(test_package PRIVATE droneops_core)
add_test(NAME test_package COMMAND test_package)

add_executable(test_sqlite tests/test_sqlite.cpp)
target_link_libraries(test_sqlite PRIVATE droneops_core)
add_test(NAME test_sqlite COMMAND test_sqlite)

add_test(
    NAME test_cli_init_mission
    COMMAND droneops init-mission
        --dir ${CMAKE_CURRENT_BINARY_DIR}/mission_DO001
        --project SISTER-CAMPO
        --campaign CAMP-2026-001
        --area AREA-001
        --mission DO-001
        --responsible "Operador"
        --aircraft DRONE-001
        --sensor RGB-001
)

add_test(
    NAME test_cli_validate
    COMMAND droneops validate
        --input ${CMAKE_CURRENT_BINARY_DIR}/mission_DO001/droneops/missoes.csv
        --out ${CMAKE_CURRENT_BINARY_DIR}/mission_DO001/validacao
)
set_tests_properties(test_cli_validate PROPERTIES DEPENDS test_cli_init_mission)

add_test(
    NAME test_cli_package
    COMMAND droneops package
        --dir ${CMAKE_CURRENT_BINARY_DIR}/mission_DO001
        --mission DO-001
        --out ${CMAKE_CURRENT_BINARY_DIR}/mission_DO001/pacote
)
set_tests_properties(test_cli_package PROPERTIES DEPENDS test_cli_validate)


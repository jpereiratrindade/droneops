#pragma once

#include "droneops/Sqlite.hpp"
#include <httplib.h>
#include <memory>
#include <string>

namespace droneops::server {

class WebServer {
public:
    WebServer(droneops::sqlite::SqliteStore& store, const std::string& web_root, const std::string& cert_path = "", const std::string& key_path = "");
    ~WebServer();

    void start(int port);
    void stop();

private:
    droneops::sqlite::SqliteStore& store_;
    std::string web_root_;
    std::unique_ptr<httplib::Server> srv_;

    void setupRoutes();
};

} // namespace droneops::server

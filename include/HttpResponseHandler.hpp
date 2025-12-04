#pragma once

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h> //stat
#include <unistd.h>
#include <ctime>
#include <filesystem>  

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "ConfigBuilder.hpp"
// #include "Server.hpp"
#include "Cgi.hpp"

class HttpResponseHandler {
public:
    HttpResponse handleRequest(HttpRequest& req, const config::ServerConfig* vh);

private:
    HttpResponse handleGET(HttpRequest& req, const config::ServerConfig* vh);
    HttpResponse handlePOST(HttpRequest& req, const config::ServerConfig* vh);
    HttpResponse handleDELETE(HttpRequest& req, const config::ServerConfig* vh);
    //helpers:
    const config::LocationConfig* findLocationConfig (const config::ServerConfig* vh, const std::string& uri_raw);
    HttpResponse parseCGIOutput(const std::string& out);
    std::string mapUriToPath(const config::LocationConfig* loc, const std::string& uri_raw);
};

//int stat(const char *pathname, struct stat *statbuf); Retrieve information about a file (size, type, permissions, timestamps, etc.) without opening it.
// return 0 → success, statbuf filled.
// -1 → error (errno set), e.g., file does not exist.
// struct stat {
//     st_mode; //file type & permissions.
//     st_size; //file size.
//     st_mmine;  //ast modification time, etc.
// }
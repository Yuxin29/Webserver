#pragma once

#include <string>
#include <fstream>
#include <map>
#include <sys/types.h>
#include <sys/stat.h> //stat
#include <unistd.h>

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include "ConfigBuilder.hpp"

//int stat(const char *pathname, struct stat *statbuf); Retrieve information about a file (size, type, permissions, timestamps, etc.) without opening it.
// return 0 → success, statbuf filled.
// -1 → error (errno set), e.g., file does not exist.
// struct stat {
//     st_mode; //file type & permissions.
//     st_size; //file size.
//     st_mmine;  //ast modification time, etc.
// }

class HttpResponseHandler {
public:
    HttpResponse handleRequest(const HttpRequest& req, const config::ServerConfig* vh);

private:
    HttpResponse handleGET(const HttpRequest& req, const config::ServerConfig* vh);
    HttpResponse handlePOST(const HttpRequest& req, const config::ServerConfig* vh);
    HttpResponse handleDELETE(const HttpRequest& req, const config::ServerConfig* vh);
};
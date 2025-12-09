#pragma once

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h> //stat
#include <unistd.h>
#include <ctime>
#include <filesystem>
#include <algorithm>

#include "HttpResponse.hpp"
#include "ConfigBuilder.hpp"
#include "Cgi.hpp"

/**
 * @class HttpResponseHandler
 * @brief Handles HTTP requests and generates appropriate HTTP responses.
 *
 * This class processes HttpRequest objects based on the HTTP method (GET, POST, DELETE)
    * and the server configuration (virtual hosts, locations). It generates HttpResponse objects
    * that represent the server's response to the client's request.
 *
 * Usage example:
 * @code
    * HttpResponseHandler handler;
    * HttpResponse res = handler.handleRequest(req, vh);
 * @endcode
 */
class HttpResponseHandler {
private:
    // --------------------
    // Internal Utility Methods
    // --------------------
    HttpResponse                    parseCGIOutput(const std::string& out, const HttpRequest& req);
    HttpResponse                    generateAutoIndex(const std::string& dirPath, HttpRequest& req);
    // --------------------
    //  InternalHandlers for different HTTP methods
    // --------------------
    HttpResponse                    handleGET(HttpRequest& req, const config::ServerConfig* vh);
    HttpResponse                    handlePOST(HttpRequest& req, const config::ServerConfig* vh);
    HttpResponse                    handleDELETE(HttpRequest& req, const config::ServerConfig* vh);

public:
    // --------------------
    //   Public Handler Methods
    // --------------------
    HttpResponse                    handleRequest(HttpRequest& req, const config::ServerConfig* vh);
};

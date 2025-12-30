#pragma once

#include "ConfigBuilder.hpp"
#include "HttpResponse.hpp"

#include <filesystem>
#include <algorithm>
#include <ctime>
#include <sys/stat.h>

namespace fs = std::filesystem;

namespace httpUtils {
    bool                            isMethodAllowed(const config::LocationConfig* loc, const std::string& method);
    bool                            shouldKeepAlive(const HttpRequest& req);

    std::string                     trim_space(std::string str);
    std::string                     normalizeHeaderKey(const std::string& key);
    std::string                     getMimeType(const std::string& path);
    std::string                     formatTime(std::time_t t);
    std::string                     mapUriToPath(const config::LocationConfig* loc, const std::string& uri_raw);
    std::string                     getIndexFile(const std::string& dirPath, const config::LocationConfig* lc);

    const config::LocationConfig*   findLocationConfig(const config::ServerConfig* vh, const std::string& uri_raw, const std::string& method = "");
    bool                            isCgiRequest(HttpRequest& request, const config::ServerConfig& vh);
}

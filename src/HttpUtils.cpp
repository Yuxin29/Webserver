#include "HttpUtils.hpp"

// a helper to trim empty space
std::string trim_space(std::string str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

namespace fs = std::filesystem; // Alias for filesystem

// Function: map file extension to MIME type
std::string getMimeType(const std::string& path) {
    static const std::map<std::string, std::string> mimeMap = 
    {
        {".html","text/html"},
        {".htm","text/html"},
        {".css","text/css"}, 
        {".js","application/javascript"},
        {".json","application/json"},
        {".png","image/png"},
        {".jpg","image/jpeg"},
        {".jpeg","image/jpeg"},
        {".gif","image/gif"},
        {".txt","text/plain"}
    };
    auto ext = fs::path(path).extension().string();  // get file extension
    auto it = mimeMap.find(ext);                     // lookup in map
    return it != mimeMap.end() ? it->second : "application/octet-stream"; // default MIME
}

// Format time as GMT string for Last-Modified header
std::string formatTime(std::time_t t) {
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT"); // gmtime -> UTC, put_time 格式化
    return ss.str();
}
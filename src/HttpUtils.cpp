#include "HttpUtils.hpp"

namespace fs = std::filesystem; // Alias for filesystem

namespace httpUtils{

/**
 * @brief Checks if the HTTP method is allowed in the given LocationConfig
 *
 * @param  loc pointer to the LocationConfig
 * @param  method the HTTP method to check (e.g., "GET", "POST")
 * @return bool true if the method is allowed, false otherwise
 *
 * @note used to validate if a request method is permitted for a specific location
 */
bool isMethodAllowed(const config::LocationConfig* loc, const std::string& method){
   if (!loc)
      return false;
   for (std::vector<std::string>::const_iterator it = loc->methods.begin(); it != loc->methods.end(); ++it){
      if (*it == method)
         return true;
   }
   return false;
}

/**
 * @brief Determines if connection should be kept alive based on request headers and HTTP version
 *
 * @param req the HttpRequest object
 * @return bool true if connection should stay alive, false if it should close
 *
 * @note HTTP/1.1 defaults to keep-alive unless client sends "Connection: close"
 *       HTTP/1.0 defaults to close unless client sends "Connection: keep-alive" -> filtered out in validation stage
 * @note Connection header value is Case-insensitivity (all valid) -> Convert to lowercase
 *
 * @example of connection headerlines
 * Connection: Keep-Alive
 * Connection: CLOSE
 * Connection: KeEp-AlIvE
 */
bool shouldKeepAlive(const HttpRequest& req){
   std::string version = req.getVersion();
   const std::map<std::string, std::string>& headers = req.getHeaders();

   auto it = headers.find("Connection");
   if (it != headers.end()) {
      std::string connValue = it->second;
      std::transform(connValue.begin(), connValue.end(), connValue.begin(), ::tolower);
      if (connValue.find("close") != std::string::npos)
         return false;
      if (connValue.find("keep-alive") != std::string::npos)
         return true;
   }

   // HTTP/1.1 default behavior: keep connection alive
   return true;
}

/**
 * @brief Trims the empty space \t at the beginning and the end of a string
 *
 * @param str a string with possible '\t' at the beginning and end
 * @return a string without any '\t' at the beginning or end
 *
 * @note Currently supports 400, 405, 413, 431. Default is 400.
 */
std::string trim_space(std::string str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

/**
 * @brief  Gets the MIME type based on the file extension
 *
 * @param string the file path
 * @return string the corresponding MIME type
 *
 * @note             MIME type: Multipurpose Internet Mail Extension type:
 * @example_format:  type / subtype
 * @example          text/html
 */
std::string getMimeType(const std::string& path)
{
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
   auto ext = fs::path(path).extension().string();                         // get file extension
   auto it = mimeMap.find(ext);                                            // lookup in map
   return it != mimeMap.end() ? it->second : "application/octet-stream";   // default MIME
}

/**
 * @brief  Formats a time_t value into a GMT string
 * @param  t time_t value
 * @return string formatted GMT time
 *
 * @note Example input: 7777236666646: from 1970/1/1 to now in seconds
 * @note Example output: "Wed, 21 Oct 2015 07:28:00 GMT", used for Last-Modified header
 */
std::string formatTime(std::time_t t) {
   std::ostringstream ss;
   ss << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT");
   return ss.str();
}

/**
 * @brief   Maps a request URI to a filesystem path based on the LocationConfig
 *
 * @param   loc pointer to the LocationConfig
 * @param   uri_raw the request URI
 * @return  string the corresponding filesystem path
 *
 * @note    used to translate request URIs into actual file paths on the server
 * @note    path traversal should not be allowed
 * @note    use std::filesystem::canonical to get the real path and check if it is under root
 */
std::string mapUriToPath(const config::LocationConfig* loc, const std::string& uri_raw)
{
   std::string rel = uri_raw;
   if (rel.find(loc->path) == 0)
      rel = rel.substr(loc->path.length());
   if (!rel.empty() && rel[0] == '/')
      rel = rel.substr(1);

   fs::path full = fs::absolute(loc->root) / rel;

   std::error_code ec;
   if (fs::exists(full))
   {
      fs::path canon = fs::canonical(full, ec);
      if (!ec)
         return canon.string();
   }
   return full.string();
}

/**
 * @brief   Gets the first existing index file from the directory based on LocationConfig
 *
 * @param   dirPath the directory path
 * @param   lc pointer to the LocationConfig
 * @return  string the first found index file name, or empty string if none found
 *
 * @note    used to implement index file lookup when serving directories
 */
std::string getIndexFile(const std::string& dirPath, const config::LocationConfig* lc)
{
    struct stat st;
    for (size_t i = 0; i < lc->index.size(); ++i)
    {
        std::string candidate = dirPath + "/" + lc->index[i];
        if (stat(candidate.c_str(), &st) == 0 && S_ISREG(st.st_mode))
            return lc->index[i]; // return filename only
    }
    return ""; // no index file found
}

/**
 * @brief Finds the best/longest matching LocationConfig for a given URI
 *
 * @param vh pointer to the ServerConfig (virtual host)
 * @param uri_raw the request URI
 * @return  const pointer to the best matching LocationConfig, or NULL if none found
 *
 * @note  used to map request URIs to server location blocks based on longest prefix match
 */
const config::LocationConfig* findLocationConfig(const config::ServerConfig* vh, const std::string& uri_raw)
{
   std::string uri = uri_raw;
   size_t qpos = uri.find('?');  // yuxin need to reconsider
   if (qpos != std::string::npos)
      uri = uri.substr(0, qpos);

   const config::LocationConfig* best = nullptr;
   size_t bestLen = 0;

   for (size_t i = 0; i < vh->locations.size(); i++) {
      const config::LocationConfig& loc = vh->locations[i];
      // rfind == 0 → prefix match
      if (uri.rfind(loc.path, 0) == 0){
         if (loc.path.length() > bestLen) {
            best = &loc;
            bestLen = loc.path.length();
         }
      }
   }
   return best;
}
}
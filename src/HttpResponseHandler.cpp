#include "HttpResponseHandler.hpp"
#include "Server.hpp"

/**
 * @brief Trims the empty space \t at the beginning and the end of a string
 *
 * @param str a string with possible '\t' at the beginning and end
 * @return a string without any '\t' at the beginning or end
 *
 * @note Currently supports 400, 405, 413, 431. Default is 400.
 */
static std::string trim_space(std::string str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

namespace fs = std::filesystem; // Alias for filesystem

/**
 * @brief  Gets the MIME type based on the file extension
 *
 * @param string path the file path
 * @return string the corresponding MIME type
 *
 * @note MIME type: Multipurpose Internet Mail Extension type: format: type / subtype: example text/html
 */
static std::string getMimeType(const std::string& path) {
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

/**
 * @brief  Formats a time_t value into a GMT string
 * @param  t time_t value
 * @return string formatted GMT time
 *
 * @note Example format: "Wed, 21 Oct 2015 07:28:00 GMT", used for Last-Modified header
 */
static std::string formatTime(std::time_t t) {
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT");
    return ss.str();
}

/**
 * @brief parse the output from CGI execution into an HttpResponse object
 *
 * @param out raw output string from CGI execution of lin
 * @return HttpResponse object representing the CGI response
 *
 * @note used when CGI script is executed and its output needs to be converted into an HTTP response
 * @example
 * Status: 200 OK\r\n
 * Content-Type: text/html\r\n
 * \r\n
 * <html>...</html>
 */
HttpResponse HttpResponseHandler::parseCGIOutput(const std::string& out){
   //it is after last header valuse and then the empty line
   size_t pos = out.find("\r\n\r\n");
   if (pos == std::string::npos)
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>Invalid CGI Output</h1>", {}, false, false);
   
   std::string headersString = out.substr(0, pos);
   std::string bodyString = out.substr(pos + 4);

   std::string statusCode = "200";
   std::string statusMsg = "OK";

   std::map<std::string, std::string>  headersMap;

   std::istringstream ss(headersString);
   std::string       line;

   while (std::getline(ss, line))
   {
      if (!line.empty() && line.back() == '\r') //getline removes \n but not \related
         line.pop_back();
      size_t dd = line.find(":");
      std::string key = line.substr(0, dd);
      std::string val = line.substr(dd + 1);
      key = trim_space(key);
      val = trim_space(val);

      // check if it is special status code in headerlines
      // eg: Status: 404 Not Found
      if (key == "Status") {
         size_t space = val.find(" ");
         statusCode = val.substr(0, space);
         statusMsg  = val.substr(space + 1);
      }
      else
         headersMap[key] = val;
   }
   // manually setup this one
   headersMap["Content-Length"] = std::to_string(bodyString.size());
      return HttpResponse("HTTP/1.1", std::stoi(statusCode), statusMsg, bodyString, headersMap, true, true);
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
const config::LocationConfig* HttpResponseHandler::findLocationConfig (const config::ServerConfig* vh, const std::string& uri_raw)
{
   std::string uri = uri_raw;
   size_t qpos = uri.find('?');
   if (qpos != std::string::npos)
      uri = uri.substr(0, qpos);
   
   const config::LocationConfig* best = NULL;
   size_t bestLen = 0;

   for (size_t i = 0; i < vh->locations.size(); i++) {
      const config::LocationConfig& loc = vh->locations[i];
      if (uri.rfind(loc.path, 0) == 0)  // rfind == 0 → prefix match
      {
         if (loc.path.length() > bestLen) {
            best = &loc;
            bestLen = loc.path.length();
         }
      }
   }
   return best;
}

/**
 * @brief   Maps a request URI to a filesystem path based on the LocationConfig
 *
 * @param   loc pointer to the LocationConfig
 * @param   uri_raw the request URI
 * @return  string the corresponding filesystem path
 *
 * @note    used to translate request URIs into actual file paths on the server
 */
std::string HttpResponseHandler::mapUriToPath(const config::LocationConfig* loc, const std::string& uri_raw)
{
   if (!loc)
     return "";

   std::string uri = uri_raw;
   size_t qpos = uri.find('?');
   if (qpos != std::string::npos)
      uri = uri.substr(0, qpos); 
   // remove prefix from URI
   std::string relative = uri.substr(loc->path.length());
   // Case 1: request points to a directory
   if (relative.empty() || relative == "/")
   {
      // index list is not empty → use first index
      if (!loc->index.empty())
         return loc->root + "/" + loc->index[0];

      // no index defined → return directory itself
      return loc->root;
   }
   // Case 2: regular file
   return loc->root + relative;
}

/**
 * @brief   Handles an HTTP GET request and generates the appropriate HttpResponse
 *
 * @param   req the HttpRequest object representing the client's GET request
 * @param   vh pointer to the ServerConfig (virtual host)   
 * @return  HttpResponse object representing the server's response to the GET request
 *
 * @note    need to check CGI first, it has priority over static files
 * @note    follows steps: map URI to path, check file existence and permissions, determine MIME type, read file content, build response
 *
 * @example request:
 * GET /hello HTTP/1.1
 * Host: example.com
 * User-Agent: curl/7.81.0             
 * Accept: text/plain 
 * @example response:
 * HTTP/1.1 200 OK
 * Date: Thu, 21 Nov 2025 10:05:00 GMT
 * Server: ExampleServer/1.0
 * Content-Type: text/plain
 * Content-Length: 13
 *
 * Hello, world!
 */
HttpResponse HttpResponseHandler::handleGET(HttpRequest& req, const config::ServerConfig* vh){
   // get the request URI: uniform Resource Identifier, _path in the request
   std::string uri = req.getPath();
   // First check if it is cgi
   const config::LocationConfig* lc = findLocationConfig(vh, uri);
   if (!lc)
      return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>404 Not Found</h1>", std::map<std::string, std::string>(), false, false);
   CGI cgi(req, *lc);
   if (cgi.isCGI()){
      std::string cgi_output = cgi.execute();
      if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
         return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>40412 Not Found</h1>", std::map<std::string, std::string>(), false, false);
      return parseCGIOutput(cgi_output);   
   }
   // map URI to path. for example: /hello → filesystem path (e.g., /var/www/html/hello).
   std::string fullpath = mapUriToPath(lc, uri);
   // Checked if the file exists, is readable, and is a regular file: exits(), is_regular_file, access(R_OK)
   struct stat st;
   if (stat(fullpath.c_str(), &st) < 0)
      return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>40413 Not Found</h1>", std::map<std::string, std::string>(), false, false);
   if (access(fullpath.c_str(), R_OK) < 0)
      return HttpResponse("HTTP/1.1", 403, "Forbidden", "<h1>40331 Forbidden</h1>", std::map<std::string, std::string>(), false, false);
   if (!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode))
      return HttpResponse("HTTP/1.1", 404, "Forbidden", "<h1>40332 Forbidden</h1>", std::map<std::string, std::string>(), false, false);
   // Determined MIME type (text/plain for .txt or plain text).
   std::string mine_type = getMimeType(fullpath);
   if (mine_type.empty())
      mine_type = "text/html";
   // Read file content → sent as response body. using: std::ifstream ifs(fullpath.c_str(), std::ios::binary);
   std::ifstream ifs(fullpath.c_str(), std::ios::binary);
   if (!ifs.is_open())
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Cannot open file</h1>", {}, false, false);
   ifs.seekg(0, std::ios::end);
   std::streamsize size = ifs.tellg();
   ifs.seekg(0, std::ios::beg);
   std::string body(size, '\0');
   ifs.read(&body[0], size);
   ifs.close();
   // Filled headers like Date and Server. maybe more headers
   std::map<std::string, std::string> headers;
   headers["Content-Type"] = mine_type;
   headers["Content-Length"] = std::to_string(body.size());
   headers["Server"] = "MiniWebserv/1.0";
   headers["Last-Modified"] = formatTime(st.st_mtime);
   // Build HttpResponse object with status 200 OK and the file content as body
   return HttpResponse("HTTP/1.1", 200, "OK", body, headers, true, true);
}

/**
 * @brief   Handles an HTTP POST request and generates the appropriate HttpResponse
 *
 * @param   req the HttpRequest object representing the client's GET request
 * @param   vh pointer to the ServerConfig (virtual host)   
 * @return  HttpResponse object representing the server's response to the POST request
 *
 * @note    need to check CGI first, it has priority over static files
 * @note    follows steps:  determine target resource, read request body, validate, process data, generate response
 *
 * @example request:
   * POST /submit-data HTTP/1.1
   * Host: example.com
   * Content-Type: application/json
   * Content-Length: 27
 * @example response:
   * HTTP/1.1 201 Created
   * Content-Type: application/json
   * Content-Length: 23
   *
   * {"status":"success"}
 */
HttpResponse HttpResponseHandler::handlePOST(HttpRequest& req, const config::ServerConfig* vh){
   // Server receives POST /submit-data.
   std::string uri = req.getPath();
   // Determines the target resource:Typically a CGI script, an upload handler, or a location block. Example: /var/www/html/submit-data (or routed to CGI)
   const config::LocationConfig* lc = findLocationConfig(vh, uri);
   if (!lc)
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error: No Location Match</h1>", {}, false, false);
   CGI cgi(req, *lc); 
   if (cgi.isCGI()) {
      std::string cgi_output = cgi.execute();
      if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
         return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 CGI execute failed</h1>", {}, false, false);
      return parseCGIOutput(cgi_output);
   }
   //  If not CGI, assume static file upload handler:
   std::string uploadDir = "./uploads";
   std::string filename = "upload_" + std::to_string(time(NULL)) + "_" + std::to_string(rand() % 1000) + ".dat";
   std::string savepath = uploadDir + "/" + filename;
   //  Ensures upload directory exists
   struct stat st;
   if (stat(uploadDir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)){
      if (mkdir(uploadDir.c_str(), 0755) != 0)
         return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error: Cannot create upload directory</h1>", {}, false, false);
   }
   // Reads request body:
   // - Content-Length = 27 → read exactly 27 bytes.
   // - Body = {"name":"Alice","age":30}
   std::string body = req.getBody();
   // Validates:
   // - Validate Content-Type
   // - Optional: Check if JSON is valid

   // Processes the data:
   // - Example: store in a database, write to a file, pass to CGI, etc.
   std::ofstream ofs(savepath.c_str(), std::ios::binary);
   if (!ofs.is_open())
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error 6</h1>", std::map<std::string, std::string>(), false, false);
   ofs.write(body.c_str(), body.size());
   ofs.close();
   // Generates response: Set headers (Content-Type, Content-Length, Date, Server)
   std::map<std::string, std::string> headers;
   headers["Content-Length"] = std::to_string(body.size());
   // - Set status code (201 Created, 200 OK, 400 Bad Request…)
   return HttpResponse("HTTP/1.1", 200, "Created", "{\"status\":\"success\"}", std::map<std::string, std::string>(), true, true);
}

/**
 * @brief   Handles an HTTP DELETE request and generates the appropriate HttpResponse
 *
 * @param   req the HttpRequest object representing the client's GET request
 * @param   vh pointer to the ServerConfig (virtual host)   
 * @return  HttpResponse object representing the server's response to the DELETE request
 *
 * @note    need to check CGI first, it has priority over static files
 * @note    follows steps:  determine target resource, read request body, validate, process data, generate response
 *
 * @example request:
   * DELETE /files/file1.txt HTTP/1.1
   * Host: example.com
   * User-Agent: curl/7.81.0
 * @example response:
   * HTTP/1.1 204 No Content
   * Date: Thu, 21 Nov 2025 11:00:00 GMT
   * Server: ExampleServer/1.0
   *
   * {"status":"success"}
 */
HttpResponse HttpResponseHandler::handleDELETE(HttpRequest& req, const config::ServerConfig* vh){
   // Server receives DELETE /files/file1.txt.
   std::string uri = req.getPath();
   
   // first check CGI, below are fake code
   const config::LocationConfig* lc = findLocationConfig(vh, uri);
   if (!lc) 
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error: No Location Match</h1>", {}, false, false);
   CGI cgi(req, *lc); 
   if (cgi.isCGI()) {
        std::string cgi_output = cgi.execute();
        if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
            return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 CGI exuc failed</h1>", {}, false, false);
        return parseCGIOutput(cgi_output);
   }
   // 2. otherwie, it is a static delete. Maps path: /files/file1.txt → /var/www/html/files/file1.txt
   std::string fullpath = mapUriToPath(lc, uri);
   // Validates: Does file exist? Is it allowed to delete this path? (check directory permissions)? Is DELETE method allowed in this location?
   struct stat st;
   if (stat(fullpath.c_str(), &st) < 0)
      return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>404 Not Found</h1>", std::map<std::string, std::string>(), false, false);
   if (access(fullpath.c_str(), W_OK) < 0) //to delete it, we need to have the writing right
      return HttpResponse("HTTP/1.1", 40333, "Forbidden", "<h1>403 Forbidden</h1>", std::map<std::string, std::string>(), false, false);
   if (!S_ISREG(st.st_mode))
      return HttpResponse("HTTP/1.1", 40333, "Forbidden", "<h1>403 Forbidden</h1>", std::map<std::string, std::string>(), false, false);
   // Attempts deletion: - unlink("/var/www/html/files/file1.txt")
   if (unlink(fullpath.c_str()) < 0)   //sth worng: (io err, permission)
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error</h1>", std::map<std::string, std::string>(), false, false);
   // Generates response:
   std::map<std::string, std::string> headers;
   headers["Content-Length"] = "0";  // No response body
   // - If success → 204 No Content (most common) - Or 200 OK with optional message
   return HttpResponse("HTTP/1.1", 204, "No content", "", std::map<std::string, std::string>(), true, true);
}


/**
 * @brief  Handles the HTTP request and generates the appropriate response
 *
 * @param  req HttpRequest object representing the client's request
 * @param  vh pointer to the ServerConfig for the virtual host
 * @return HttpResponse object representing the server's response
 
 * @note for the server to handle the request based on method type
 */
HttpResponse HttpResponseHandler::handleRequest(HttpRequest& req, const config::ServerConfig* vh){
   if (!vh)
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "", {}, false, false);
   if (req.getMethod() == "GET")
     return handleGET(req, vh);
   else if (req.getMethod() == "POST")
      return handlePOST(req, vh);
   else if (req.getMethod() == "DELETE")
      return handleDELETE(req, vh);
   // not supposed to reach here, already filtered in parser validation
   return HttpResponse("HTTP/1.1", 405, "Method Not Allowed", "", {}, false, false);
}

//int stat(const char *pathname, struct stat *statbuf); Retrieve information about a file (size, type, permissions, timestamps, etc.) without opening it.
// return 0 → success, statbuf filled.
// -1 → error (errno set), e.g., file does not exist.
// struct stat {
//     st_mode; //file type & permissions.
//     st_size; //file size.
//     st_mmine;  //ast modification time, etc.
// }
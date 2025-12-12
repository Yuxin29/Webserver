#include "HttpResponseHandler.hpp"
#include <iostream>  //debug

// --------------------
// static utility functions
// --------------------
namespace {

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
namespace fs = std::filesystem; // Alias for filesystem
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
   std::string root = loc->root; // e.g., "./sites/static"
   fs::path rootPath = fs::absolute(root);

   std::string cleanUri = uri_raw;
   if (!cleanUri.empty() && cleanUri[0] == '/')
      cleanUri = cleanUri.substr(1);
   fs::path fullPath = rootPath / cleanUri;
   std::error_code ec;

   // path exists → canonicalize
   if (fs::exists(fullPath)) {
      fs::path canonicalPath = fs::canonical(fullPath, ec);
      if (ec)
         return ""; // should never happen for existing path
      // security: ensure canonical path is inside root
      fs::path rel = fs::relative(canonicalPath, rootPath, ec);
      if (ec || (!rel.empty() && rel.string().substr(0, 2) == ".."))
         return "";
      return canonicalPath.string();
   }
   // Path does NOT exist → return full path anyway (for 404 later)
   return fullPath.string();
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

// --------------------
// Internal Utility Methods
// --------------------
/**
 * @brief parse the output from CGI execution into an HttpResponse object
 *
 * @param out raw output string from CGI execution of lin
 * @param req the HttpRequest object (used to determine keep-alive)
 * @return HttpResponse object representing the CGI response
 *
 * @note used when CGI script is executed and its output needs to be converted into an HTTP response
 * @note CGI :common gateway Interface
 * @note CGI path is the filesystem path to the cgi executable program:
 *
 * @example
 * Status: 200 OK\r\n
 * Content-Type: text/html\r\n
 * \r\n
 * <html>...</html>
 */
HttpResponse HttpResponseHandler::parseCGIOutput(const std::string& out, const HttpRequest& req, const config::ServerConfig* vh){
   //it is after last header valuse and then the empty line
   //Note from lucio, should check both "\r\n\r\n"(for python) and "\n\n" (for php and bash)
   size_t pos = out.find("\r\n\r\n");
   if (pos == std::string::npos){
      pos = out.find("\n\n");
      if (pos == std::string::npos){
         return makeErrorResponse(500, vh);
      }
   }

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
      return HttpResponse("HTTP/1.1", std::stoi(statusCode), statusMsg, bodyString, headersMap, shouldKeepAlive(req), true);
}

/**
 * @brief   Generates an HTML directory listing for autoindex
 *
 * @param   dirPath the directory path
 * @param   req the HttpRequest object (used for path in links)
 * @return  HttpResponse object containing the autoindex HTML page
 *
 * @note    used to provide directory listings when autoindex is enabled
 */
HttpResponse HttpResponseHandler::generateAutoIndex(const std::string& dirPath, HttpRequest& req)
{
   namespace fs = std::filesystem;
   std::string body = "<html><head><title>Index of " + req.getPath() + "</title></head><body>";
   body += "<h1>Index of " + req.getPath() + "</h1><ul>";

   for (const auto& entry : fs::directory_iterator(dirPath))
   {
      std::string name = entry.path().filename().string();
      body += "<li><a href=\"" + req.getPath();
      if (req.getPath().back() != '/')
         body += "/";
      body += name + "\">" + name + "</a></li>";
   }

   body += "</ul></body></html>";

   std::map<std::string, std::string> headers;
   headers["Content-Type"] = "text/html";
   headers["Content-Length"] = std::to_string(body.size());
   headers["Server"] = "MiniWebserv/1.0";
   headers["Date"] = formatTime(time(NULL));

   return HttpResponse("HTTP/1.1", 200, "OK", body, headers, shouldKeepAlive(req), true);
}

// --------------------
//  InternalHandlers for different HTTP methods
// --------------------
/**
 * @brief parse the output from CGI execution into an HttpResponse object

 * @brief   Handles an HTTP GET request and generates the appropriate HttpResponse
 *
 * @param   req the HttpRequest object representing the client's GET request
 * @param   vh pointer to the ServerConfig (virtual host)
 * @return  HttpResponse object representing the server's response to the GET request
 *
 * @note    need to check CGI first, it has priority over static files
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
HttpResponse HttpResponseHandler::handleGET(HttpRequest& req, const config::ServerConfig* vh)
{
   // get the request URI: uniform Resource Identifier, _path in the request
   std::string uri = req.getPath();

   // First find LocationConfig check if it is cgi
   const config::LocationConfig* lc = findLocationConfig(vh, uri);
   if (!lc)
      return makeErrorResponse(404, vh);

   if (!isMethodAllowed(lc, "GET"))
      return makeErrorResponse(405, vh);

   CGI cgi(req, *lc);
   if (cgi.isCGI()){
      std::string cgi_output = cgi.execute();
      if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
         return makeErrorResponse(500, vh);
      return parseCGIOutput(cgi_output, req, vh);   
   }

   // map URI to path. for example: /hello → filesystem path (e.g., /var/www/html/hello).
   std::string fullpath = mapUriToPath(lc, uri);
   if (fullpath.empty())
      return makeErrorResponse(403, vh);

   // Checked if the file exists, is readable, and is a regular file: exits(), is_regular_file, access(R_OK)
   struct stat st;
   if (stat(fullpath.c_str(), &st) < 0)
      return makeErrorResponse(404, vh);

   if (S_ISDIR(st.st_mode)) {
      std::cout << "debug, fullpath: "<< fullpath << std::endl;
      // try index files
      std::string index_file = getIndexFile(fullpath, lc);
      if (!index_file.empty()) {
         fullpath += "/" + index_file;
         if (stat(fullpath.c_str(), &st) < 0 || !S_ISREG(st.st_mode))
            return makeErrorResponse(404, vh);
      }
      else
      {
         if (!lc->autoindex)
            return makeErrorResponse(403, vh); //check correct error code
         // autoindex enabled → return HTML directory listing
         return generateAutoIndex(fullpath, req);
      }
   }
   if (access(fullpath.c_str(), R_OK) < 0)
      return makeErrorResponse(403, vh);
   if (!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode))
      return makeErrorResponse(403, vh);

   // Determined MIME type (text/plain for .txt or plain text).
   std::string mime_type = getMimeType(fullpath);
   // if (mime_type.empty())
   //    mime_type = "text/html";

   // Read file content → sent as response body.
   // using: std::ifstream ifs(fullpath.c_str(), std::ios::binary);
   std::ifstream ifs(fullpath.c_str(), std::ios::binary);
   if (!ifs.is_open())
      return makeErrorResponse(500, vh);

   ifs.seekg(0, std::ios::end);
   std::streamsize size = ifs.tellg();
   if (size < 0)
      return makeErrorResponse(500, vh);
   ifs.seekg(0, std::ios::beg);
   std::string body(size, '\0');
   ifs.read(&body[0], size);
   ifs.close();

   // Filled headers like Date and Server. maybe more headers
   std::map<std::string, std::string> headers;
   headers["Content-Type"] = mime_type;
   headers["Content-Length"] = std::to_string(body.size());
   headers["Server"] = "MiniWebserv/1.0";
   headers["Last-Modified"] = formatTime(st.st_mtime);
   headers["Date"] = formatTime(time(NULL));

   return HttpResponse("HTTP/1.1", 200, "OK", body, headers, shouldKeepAlive(req), true);
}

/**
 * @brief   Handles an HTTP POST request and generates the appropriate HttpResponse
 *
 * @param   req the HttpRequest object representing the client's GET request
 * @param   vh pointer to the ServerConfig (virtual host)
 * @return  HttpResponse object representing the server's response to the POST request
 *
 * @note    need to check CGI first, it has priority over static files
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

   // Determines the target resource:Typically a CGI script, an upload handler, or a location block.
   // Example: /var/www/html/submit-data (or routed to CGI)
   const config::LocationConfig* lc = findLocationConfig(vh, uri);
   if (!lc)
   {
      //std::cout << "debug 3" << uri << std::endl;
      return makeErrorResponse(404, vh);
   }
   if (!isMethodAllowed(lc, "POST"))
      return makeErrorResponse(405, vh);
   CGI cgi(req, *lc); 
   if (cgi.isCGI()) {
      std::string cgi_output = cgi.execute();
      if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
         return makeErrorResponse(500, vh);
      return parseCGIOutput(cgi_output, req, vh);
   }

   //  If not CGI, assume static file upload handler:
   // yuxin need to check, should the upload_dir be in root?
   std::string uploadDir = lc->upload_dir; // NOTE FROM LIN date:10/12 change this to lc->upload_dir
   if (uploadDir.empty())
       return makeErrorResponse(500, vh); 
   std::string filename = "upload_" + std::to_string(time(NULL)) + "_" + std::to_string(rand() % 1000) + ".dat";
   std::string savepath = uploadDir + "/" + filename;

   //  Ensures upload directory exists
   struct stat st;
   if (stat(uploadDir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)){
      if (mkdir(uploadDir.c_str(), 0755) != 0)
         return makeErrorResponse(500, vh);
   }

   // Reads request body:
   // - Content-Length = 27 → read exactly 27 bytes.
   // - Body = {"name":"Alice","age":30}
   std::string body = req.getBody();

   // Validates:
   // - Validate Content-Type if necessary.

   // Processes the data:
   // - Example: store in a database, write to a file, pass to CGI, etc.
   std::ofstream ofs(savepath.c_str(), std::ios::binary);
   if (!ofs.is_open())
      return makeErrorResponse(500, vh);
   ofs.write(body.c_str(), body.size());
   ofs.close();

   // Generates response: Set headers (Content-Type, Content-Length, Date, Server)
   std::map<std::string, std::string> headers;
   std::string responseBody = "{\"status\":\"success\"}";
   headers["Content-Length"] = std::to_string(responseBody.size());
   headers["Content-Type"] = "application/json";

   return HttpResponse("HTTP/1.1", 201, "Created", responseBody, std::map<std::string, std::string>(), shouldKeepAlive(req), true);
}

/**
 * @brief   Handles an HTTP DELETE request and generates the appropriate HttpResponse
 *
 * @param   req the HttpRequest object representing the client's GET request
 * @param   vh pointer to the ServerConfig (virtual host)
 * @return  HttpResponse object representing the server's response to the DELETE request
 *
 * @note    need to check CGI first, it has priority over static files
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
   if (!lc) {
      //std::cout << "debug 4" << uri << std::endl;
      return makeErrorResponse(404, vh);
   }
   if (!isMethodAllowed(lc, "DELETE"))
      return makeErrorResponse(405, vh);
   CGI cgi(req, *lc); 
   if (cgi.isCGI()) {
         std::string cgi_output = cgi.execute();
         if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
            return makeErrorResponse(500, vh);
         return parseCGIOutput(cgi_output, req, vh);
   }

   // otherwie, it is a static delete. Maps path: /files/file1.txt → /var/www/html/files/file1.txt
   std::string fullpath = mapUriToPath(lc, uri);
   if (fullpath.empty())
      return makeErrorResponse(403, vh);

   // Validates:
   // Does file exist?
   // Is it allowed to delete this path? (check directory permissions)?
   //  Is DELETE method allowed in this location?
   struct stat st;
   if (stat(fullpath.c_str(), &st) < 0){
      //std::cout << "debug 5" << uri << std::endl;
      return makeErrorResponse(404, vh);
   }
   if (!S_ISREG(st.st_mode))
      return makeErrorResponse(403, vh);
   if (access(fullpath.c_str(), W_OK) < 0)
      return makeErrorResponse(403, vh);

   // Attempts deletion: - unlink("/var/www/html/files/file1.txt") //sth worng: (io err, permission)
   if (unlink(fullpath.c_str()) < 0)
      return makeErrorResponse(500, vh);

   // Generates response:
   std::map<std::string, std::string> headers;
   headers["Content-Length"] = "0";  // No response body

   // - If success → 204 No Content (most common) - Or 200 OK with optional message
   return HttpResponse("HTTP/1.1", 204, "No Content", "", std::map<std::string, std::string>(), shouldKeepAlive(req), true);
}

// --------------------
//   Public Handler Methods
// --------------------
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

static const std::map<int, std::string> STATUS_REASON = {
   {400, "Bad Request"},
   {403, "Forbidden"},
   {404, "Not Found"},
   {405, "Method Not Allowed"},
   {408, "Request Timeout"},
   {413, "Payload Too Large"},
   {414, "URI Too Long"},
   {431, "Request Header Fields Too Large"},
   {500, "Internal Server Error"},
};

std::string loadFile(const std::string& path)
{
   std::ifstream file(path.c_str());
   if (!file.is_open())
      return ""; // return empty so makeErrorResponse() can fallback

   std::stringstream buffer;
   buffer << file.rdbuf();
   return buffer.str();
}

/**
 * @brief   Generates an HTTP error response with the specified status code
 *
 * @param   status the HTTP status code for the error response (e.g., 404, 500)
 * @return  HttpResponse object representing the error response
 *
 * @note    This function creates a simple HTML error page corresponding to the given status code.
 *          If a static HTML file for the error exists in "static/errors/", it will be used as the body.
 *          Otherwise, a default HTML message will be generated.
 *
 * @example response:
   * HTTP/1.1 404 Not Found
   * Content-Type: text/html
   * Content-Length: 23
   *
   * <h1>404 Not Found</h1>
 */
HttpResponse makeErrorResponse(int status, const config::ServerConfig* vh)
{
   // Find reason phrase
   std::string reason;
   if (STATUS_REASON.count(status))
      reason = STATUS_REASON.at(status);
   else {
      status = 500;
      reason = "Internal Server Error";
   }

   // Load static HTML file hard-coded path: static/errors/404.html, need to implement later
   //std::string path = "sites/static/errors/" + std::to_string(status) + ".html";
   std::string body;
   if (vh && vh->errorPages.count(status)){
      std::string path = vh->errorPages.at(status);
      body = loadFile(vh->errorPages.at(status));
   }

   // Fallback
   if (body.empty())
      body = "<h1>" + std::to_string(status) + " " + reason + "</h1>";

   std::map<std::string, std::string> headers;
   headers["Content-Type"] = "text/html";

   return HttpResponse("HTTP/1.1", status, reason, body, headers, false, false);
}

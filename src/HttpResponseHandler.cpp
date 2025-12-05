#include "HttpResponseHandler.hpp"
#include "Server.hpp"

// a helper to trim empty space
static std::string trim_space(std::string str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

namespace fs = std::filesystem; // Alias for filesystem

// Function: map file extension to MIME type
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

// Format time as GMT string for Last-Modified header (not used yet but might be needed later)
// static std::string formatTime(std::time_t t) {
//     std::ostringstream ss;
//     ss << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT"); // gmtime -> UTC, put_time 格式化
//     return ss.str();
// }

// this is the public over-all call for LUCIO server to use
HttpResponse HttpResponseHandler::handleRequest(HttpRequest& req, const config::ServerConfig* vh)
{
   if (!vh)
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "", {}, false, false);
   if (req.getMethod() == "GET")
     return handleGET(req, vh);
   else if (req.getMethod() == "POST")
      return handlePOST(req, vh);
   else if (req.getMethod() == "DELETE")
      return handleDELETE(req, vh);
   return HttpResponse("HTTP/1.1", 405, "Method Not Allowed", "", {}, false, false);
}

// the out here comes from lin CGIExecute, it is a string
// it can be like this: need to comfirm with lin
// Content-Type: text/html
// Status: 200 OK                   headers
//                                    empty lines
// <html> ... </html>               body
HttpResponse HttpResponseHandler::parseCGIOutput(const std::string& out){
   size_t pos = out.find("\r\n\r\n");   //it is after ok and then the empty line
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
      if (!line.empty() && line.back() == '\r')
         line.pop_back(); //getline removes \n but not \related

      size_t dd = line.find(":");
      std::string key = line.substr(0, dd);
      std::string val = line.substr(dd + 1);
      key = trim_space(key);
      val = trim_space(val);

      // check if it is special status code in headerlines
      // eg: Status: 404 Not Found
      if (key == "Status") 
      {
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

// longest match
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

/*
------------------------------ Http request example of GET ------------------------------
GET /hello HTTP/1.1
Host: example.com
User-Agent: curl/7.81.0
Accept: text/plain

Http request example of GET
HTTP/1.1 200 OK
Date: Thu, 21 Nov 2025 10:05:00 GMT
Server: ExampleServer/1.0
Content-Type: text/plain
Content-Length: 13

Hello, world!
*/
//lin configuration/webserv.conf -> root, location, index,, error_page, cgi_path, upload_path and so on.
// need to check CGI first, it has priority over static files
HttpResponse HttpResponseHandler::handleGET(HttpRequest& req, const config::ServerConfig* vh){
   // 0. Server received GET /hello. /hello is supposed to be file
   std::string uri = req.getPath(); // URI: uniform Resource Identifier, _path in the request

   // 0. First check if it is cgi
   const config::LocationConfig* lc = findLocationConfig(vh, uri);
   if (!lc)
      return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>40411 Not Found</h1>", std::map<std::string, std::string>(), false, false);
   CGI cgi(req, *lc);
   if (cgi.isCGI()){
      std::string cgi_output = cgi.execute();
      if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
         return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>40412 Not Found</h1>", std::map<std::string, std::string>(), false, false);
      return parseCGIOutput(cgi_output);   
   }

   // 2. Mapped /hello → filesystem path (e.g., /var/www/html/hello).
   std::string fullpath = mapUriToPath(lc, uri);
   //std::cout << "fullpath = " << fullpath << std::endl;

   // 3. Checked if the file exists, is readable, and is a regular file: exits(), is_regular_file, access(R_OK)
   struct stat st;
   if (stat(fullpath.c_str(), &st) < 0)
      return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>40413 Not Found</h1>", std::map<std::string, std::string>(), false, false);
   if (access(fullpath.c_str(), R_OK) < 0)
      return HttpResponse("HTTP/1.1", 403, "Forbidden", "<h1>40331 Forbidden</h1>", std::map<std::string, std::string>(), false, false);
   if (!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode))
      return HttpResponse("HTTP/1.1", 404, "Forbidden", "<h1>40332 Forbidden</h1>", std::map<std::string, std::string>(), false, false);

   // 4. Determined MIME type (text/plain for .txt or plain text).
   std::string mine_type = getMimeType(fullpath);
   if (mine_type.empty())
      mine_type = "text/html";

   // 5. Got file size (13) → set Content-Length.

   // 6. Read file content → sent as response body.
   //std::ifstream ifs(fullpath.c_str(), std::ios::binary);
   std::ifstream ifs(fullpath.c_str(), std::ios::binary);
   if (!ifs.is_open())
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Cannot open file</h1>", {}, false, false);
   ifs.seekg(0, std::ios::end);
   std::streamsize size = ifs.tellg();
   ifs.seekg(0, std::ios::beg);

   std::string body(size, '\0');
   ifs.read(&body[0], size);
   ifs.close();
   //std::cout << "[DEBUG] file size = " << size << ", body size = " << body.size() << std::endl;

   // 7. Filled headers like Date and Server.
   std::map<std::string, std::string> headers;
   headers["Content-Type"] = mine_type;
   headers["Content-Length"] = std::to_string(body.size());
   headers["Server"] = "MiniWebserv/1.0";

   return HttpResponse("HTTP/1.1", 200, "OK", body, headers, true, true);
}

/*
------------------------------ Http request example of POST ------------------------------
POST /submit-data HTTP/1.1
Host: example.com
Content-Type: application/json
Content-Length: 27

HTTP/1.1 201 Created
Content-Type: application/json
Content-Length: 23

{"status":"success"}
*/
HttpResponse HttpResponseHandler::handlePOST(HttpRequest& req, const config::ServerConfig* vh){
   // 0. Server receives POST /submit-data.
   std::string uri = req.getPath();

   // Determines the target resource:
   // 1.Typically a CGI script, an upload handler, or a location block.
   // - Example: /var/www/html/submit-data (or routed to CGI)
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

   // 2. Other wise, it is a static one
   std::string uploadDir = "./uploads";
   //std::string uploadDir = mapUriToPath(lc, uri);
   std::string filename = "upload_" + std::to_string(time(NULL)) + "_" + std::to_string(rand() % 1000) + ".dat";
   std::string savepath = uploadDir + "/" + filename;

   // if save path is not existing, should we create it??? yes
   struct stat st;
   if (stat(uploadDir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
   {
      if (mkdir(uploadDir.c_str(), 0755) != 0)
         return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error: Cannot create upload directory</h1>", {}, false, false);
   }

   // 3. Reads request body:
   // - Content-Length = 27 → read exactly 27 bytes.
   // - Body = {"name":"Alice","age":30}
   std::string body = req.getBody();

   // 4. Validates:
   // - Validate Content-Type
   // - Optional: Check if JSON is valid

   // 5. Processes the data:
   // - Example: store in a database, write to a file, pass to CGI, etc.
   std::ofstream ofs(savepath.c_str(), std::ios::binary);
   // if (!ofs.is_open())
   //    return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error 6</h1>", std::map<std::string, std::string>(), false, false);  //when I try to post, here it goes: 500 Internal Server Error 6
   ofs.write(body.c_str(), body.size());
   ofs.close();

   // 6. Generates response:
   // - Set status code (201 Created, 200 OK, 400 Bad Request…)
   // - Set headers (Content-Type, Content-Length, Date, Server)
   std::map<std::string, std::string> headers;
   headers["Content-Length"] = std::to_string(body.size());

   return HttpResponse("HTTP/1.1", 222, "Created", "{\"status\":\"success\"}", std::map<std::string, std::string>(), true, true);
}

/*
------------------------------ Http request example of DELETE ------------------------------
DELETE /files/file1.txt HTTP/1.1
Host: example.com
User-Agent: curl/7.81.0

HTTP/1.1 204 No Content
Date: Thu, 21 Nov 2025 11:00:00 GMT
Server: ExampleServer/1.0
*/
HttpResponse HttpResponseHandler::handleDELETE(HttpRequest& req, const config::ServerConfig* vh){
   // 0. Server receives DELETE /files/file1.txt.
   std::string uri = req.getPath();
   
   // 1. first check CGI, below are fake code
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

   // 2. otherwie, it is a static delete. Maps path:
   // - /files/file1.txt → /var/www/html/files/file1.txt
   std::string fullpath = mapUriToPath(lc, uri);
   //std::cout << "fullpath = " << fullpath << std::endl;

   // 3. Validates:
   // - Does file exist? Is it allowed to delete this path? (check directory permissions)? Is DELETE method allowed in this location?
   struct stat st;
   if (stat(fullpath.c_str(), &st) < 0)
      return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>404 Not Found</h1>", std::map<std::string, std::string>(), false, false);
   if (access(fullpath.c_str(), W_OK) < 0) //to delete it, we need to have the writing right
      return HttpResponse("HTTP/1.1", 40333, "Forbidden", "<h1>403 Forbidden</h1>", std::map<std::string, std::string>(), false, false);
   if (!S_ISREG(st.st_mode))
      return HttpResponse("HTTP/1.1", 40333, "Forbidden", "<h1>403 Forbidden</h1>", std::map<std::string, std::string>(), false, false);

   // 5. Attempts deletion:
   // - unlink("/var/www/html/files/file1.txt")
   if (unlink(fullpath.c_str()) < 0)   //sth worng: (io err, permission)
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error</h1>", std::map<std::string, std::string>(), false, false);

   // 6. Generates response:
   // - If success → 204 No Content (most common)
   // - Or 200 OK with optional message
   std::map<std::string, std::string> headers;
   headers["Content-Length"] = "0";  // No response body

   return HttpResponse("HTTP/1.1", 204, "No content", "", std::map<std::string, std::string>(), true, true);
}

// lin's cgi 
// class CGI
// {
// public:
// 	CGI(const HttpRequest& req, const config::LocationConfig& lc);
// 	bool isCGI()const;
// 	std::string execute();
// };


#include "HttpResponseHandler.hpp"
#include "Server.hpp"
#include "Cgi.hpp"

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

// Format time as GMT string for Last-Modified header
// not used yet but might be needed later
// static std::string formatTime(std::time_t t) {
//     std::ostringstream ss;
//     ss << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT"); // gmtime -> UTC, put_time 格式化
//     return ss.str();
// }

// this is the public over all call
// for LUCIO to use
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
   // const config::LocationConfig* lc = findLocationConfig(vh, uri);
   // if (!lc)
   //    return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>40411 Not Found</h1>", std::map<std::string, std::string>(), false, false);
   // CGI cgi(req, *lc);
   // if (cgi.isCGI()){
   //    std::string cgi_output = cgi.execute();
   //    if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
   //       return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>40411 Not Found</h1>", std::map<std::string, std::string>(), false, false);
   //    return parseCGIOutput(cgi_output);   
   // }

   // 2. Mapped /hello → filesystem path (e.g., /var/www/html/hello).
   std::string fullpath;
   if (uri == "/")
      fullpath = vh->root + "/index.html";
   else
      fullpath = vh->root + uri;
   //std::cout << "fullpath = " << fullpath << std::endl;

   // 3. Checked if the file exists, is readable, and is a regular file: exits(), is_regular_file, access(R_OK)
   struct stat st;
   if (stat(fullpath.c_str(), &st) < 0)
      return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>40411 Not Found</h1>", std::map<std::string, std::string>(), false, false);
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
   // below are fake code, waiting for lins CGI
   // const config::LocationConfig* lc = findLocationConfig(vh, uri);
   // if (!lc) 
   //    return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error: No Location Match</h1>", {}, false, false);
   // CGI cgi(req, *lc); 
   // if (cgi.isCGI()) {
   //      std::string cgi_output = cgi.execute();
   //      if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
   //          return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 CGI 执行失败</h1>", {}, false, false);
   //      return parseCGIOutput(cgi_output);
   // }

   // 2. Other wise, it is a static one
   std::string  uploadDir = vh->root; //fake one, hard-coded, ask lin later: should be according to to lin configuration/webserv.conf
   std::string filename = "upload_" + std::to_string(time(NULL)) + "_" + std::to_string(rand() % 1000) + ".dat";
   std::string savepath = uploadDir + "/" + filename;
   // if save path is not existing, should we create it???

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
   if (!ofs.is_open())
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error</h1>", std::map<std::string, std::string>(), false, false);
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
   // const config::LocationConfig* lc = findLocationConfig(vh, uri);
   // if (!lc) 
   //    return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error: No Location Match</h1>", {}, false, false);
   // CGI cgi(req, *lc); 
   // if (cgi.isCGI()) {
   //      std::string cgi_output = cgi.execute();
   //      if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
   //          return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 CGI 执行失败</h1>", {}, false, false);
   //      return parseCGIOutput(cgi_output);
   // }

   // 2. otherwie, it is a static delete. Maps path:
   // - /files/file1.txt → /var/www/html/files/file1.txt
   std::string fullpath;
   if (uri == "/")
      fullpath = vh->root + "/index.html";
   else
      fullpath = vh->root + uri;
   std::cout << "fullpath = " << fullpath << std::endl;

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
   if (unlink(fullpath.c_str()) < 0)
      //sth worng: (io err, permission)
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error</h1>", std::map<std::string, std::string>(), false, false);

   // 6. Generates response:
   // - If success → 204 No Content (most common)
   // - Or 200 OK with optional message
   std::map<std::string, std::string> headers;
   headers["Content-Length"] = "0";  // No response body
   headers["Content-Type"]   = "text/plain";

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



#include "HttpResponseHandler.hpp" 
#include "HttpUtils.hpp"

// this is the public over all call
// for LUCIO to use
HttpResponse HttpResponseHandler::handleRequest(const HttpRequest& req)
{
    if (req.getMethod() == "GET")
        return handleGET(req);
    else if (req.getMethod() == "POST")
        return handlePOST(req);
    else if (req.getMethod() == "DELETE")
        return handleDELETE(req);
    return HttpResponse("HTTP/1.1", 405, "Method Not Allowed", "", {});
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
HttpResponse HttpResponseHandler::handleGET(const HttpRequest& req){
   // 1. Server received GET /hello. /hello is supposed to be file
   std::string uri = req.getrequestPath(); // URI: uniform Resource Identifier, _path in the request

   // 2. Mapped /hello → filesystem path (e.g., /var/www/html/hello).
   const std::string root = "linConfig/root"; //fake one, hard-coded, ask lin later: should be according to to lin configuration/webserv.conf
   std::string fullpath = root + uri;

   // 3. Checked if the file exists, is readable, and is a regular file: exits(), is_regular_file, access(R_OK)
   struct stat st;

   if (stat(fullpath.c_str(), &st) < 0) 
      return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>404 Not Found</h1>", std::map<std::string, std::string>());
   if (access(fullpath.c_str(), R_OK) < 0)
      return HttpResponse("HTTP/1.1", 403, "Forbidden", "<h1>403 Forbidden</h1>", std::map<std::string, std::string>());
   if (!S_ISREG(st.st_mode))
      return HttpResponse("HTTP/1.1", 404, "Forbidden", "<h1>403 Forbidden</h1>", std::map<std::string, std::string>());
   
   // 4. Determined MIME type (text/plain for .txt or plain text).
   std::string mine_type = getMimeType(fullpath);

   // 5. Got file size (13) → set Content-Length.

   // 6. Read file content → sent as response body.
   std::ifstream ifs(fullpath.c_str(), std::ios::binary);
   std::string body((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>(ifs));

   // 7. Filled headers like Date and Server.
   std::map<std::string, std::string> headers;
   headers["Content-Type"] = mine_type;
   headers["Content-Length"] = std::to_string(body.size());
   // ... 

   return HttpResponse("HTTP/1.1", 200, "OK", body, headers);
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
HttpResponse HttpResponseHandler::handlePOST(const HttpRequest& req){
   // 1. Server receives POST /submit-data.
   std::string uri = req.getrequestPath();
   
   // 2. Determines the target resource:
   // - Typically a CGI script, an upload handler, or a location block.
   // - Example: /var/www/html/submit-data (or routed to CGI)
   const std::string root = "linConfig/static/uploads"; //fake one, hard-coded, ask lin later: should be according to to lin configuration/webserv.conf
   std::string savepath = root + uri;
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
      return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error</h1>", std::map<std::string, std::string>());
   ofs.write(body.c_str(), body.size());
   ofs.close();

   // 6. Generates response:
   // - Set status code (201 Created, 200 OK, 400 Bad Request…)
   // - Set headers (Content-Type, Content-Length, Date, Server)
   std::map<std::string, std::string> headers;
   headers["Content-Length"] = std::to_string(body.size());
   
   return HttpResponse("HTTP/1.1", 222, "Created", "{\"status\":\"success\"}", std::map<std::string, std::string>());
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
HttpResponse HttpResponseHandler::handleDELETE(const HttpRequest& /*req*/){
   // 1. Server receives DELETE /files/file1.txt.
   
   
   // 2. Maps path:
   // - /files/file1.txt → /var/www/html/files/file1.txt
   
   
   // 3. Validates:
   // - Does file exist?
   // - Is it allowed to delete this path? (check directory permissions)
   // - Is DELETE method allowed in this location?
   
   
   // 4. If file does not exist → return 404 Not Found.
   // If file exists but permission denied → 403 Forbidden.
   
   
   // 5. Attempts deletion:
   // - unlink("/var/www/html/files/file1.txt")
   
   
   // 6. Generates response:
   // - If success → 204 No Content (most common)
   // - Or 200 OK with optional message
   return HttpResponse("HTTP/1.1", 333, "delete_test", "DELETE", std::map<std::string, std::string>());
}

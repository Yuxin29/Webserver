#include "HttpResponseHandler.hpp"
#include <iostream>  //debug

namespace fs = std::filesystem; // Alias for filesystem

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
      key = httpUtils::trim_space(key);
      val = httpUtils::trim_space(val);

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
   for (std::map<std::string, std::string>::const_iterator it = req.getHeaders().begin(); it != req.getHeaders().end(); ++it){
      const std::string& key = it->first;
      const std::string& value = it->second;
      if (key ==  "Content-Type" )
      {
         headersMap["Content-Type"] = value;
         return HttpResponse("HTTP/1.1", std::stoi(statusCode), statusMsg, bodyString, headersMap, httpUtils::shouldKeepAlive(req), true);
      }
   }
   headersMap["Content-Type"] = "text/html";
   return HttpResponse("HTTP/1.1", std::stoi(statusCode), statusMsg, bodyString, headersMap, httpUtils::shouldKeepAlive(req), true);
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
    headers["Date"] = httpUtils::formatTime(time(NULL));

    return HttpResponse("HTTP/1.1", 200, "OK", body, headers, httpUtils::shouldKeepAlive(req), true);
}

// --------------------
//  InternalHandlers for different HTTP methods
// --------------------
HttpResponse HttpResponseHandler::handleGET(HttpRequest& req, const config::ServerConfig* vh)
{
   // get the request URI: uniform Resource Identifier, _path in the request
   std::string uri = req.getPath();

   // First find LocationConfig check if it is cgi
   const config::LocationConfig* lc = httpUtils::findLocationConfig(vh, uri);
   if (!lc)
      return makeErrorResponse(404, vh);
   if (!httpUtils::isMethodAllowed(lc, "GET"))
      return makeErrorResponse(405, vh);
   CGI cgi(req, *lc);
   if (cgi.isCGI()){
      std::string cgi_output = cgi.execute();
      if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
         return makeErrorResponse(500, vh);
      return parseCGIOutput(cgi_output, req, vh);
   }

   // map URI to path. for example: /hello → filesystem path (e.g., /var/www/html/hello).
   std::string fullpath = httpUtils::mapUriToPath(lc, uri);

   // Checked if the file exists, is readable, and is a regular file: exits(), is_regular_file, access(R_OK)
   struct stat st;
   if (stat(fullpath.c_str(), &st) == 0) 
   {
      if (S_ISDIR(st.st_mode))
      {
         // URI does NOT end with '/'
         if (uri.empty() || uri.back() != '/')
            return makeRedirect301(uri + "/", vh);
         //try index files
         std::string index_file = httpUtils::getIndexFile(fullpath, lc);
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
   }
   else
      return makeErrorResponse(404, vh);
   if (access(fullpath.c_str(), R_OK) < 0)
      return makeErrorResponse(403, vh);
   if (!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode))
      return makeErrorResponse(403, vh);

   // Determined MIME type (text/plain for .txt or plain text).
   std::string mime_type = httpUtils::getMimeType(fullpath);
   
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
   headers["Last-Modified"] = httpUtils::formatTime(st.st_mtime);
   headers["Date"] = httpUtils::formatTime(time(NULL));

   return HttpResponse("HTTP/1.1", 200, "OK", body, headers, httpUtils::shouldKeepAlive(req), true);
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
   const config::LocationConfig* lc = httpUtils::findLocationConfig(vh, uri);
   if (!lc)
      return makeErrorResponse(404, vh);
   if (!httpUtils::isMethodAllowed(lc, "POST"))
      return makeErrorResponse(405, vh);
   CGI cgi(req, *lc);
   if (cgi.isCGI()) {
      std::string cgi_output = cgi.execute();
      if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
         return makeErrorResponse(500, vh);
      return parseCGIOutput(cgi_output, req, vh);
   }

   //  If not CGI, assume static file upload handler:
   std::string uploadDir = lc->upload_dir;
   std::cout << "upload filder: " << lc->upload_dir << std::endl;
   if (uploadDir.empty())
       return makeErrorResponse(500, vh);
   
   // yuxin need to consider filename conflict
   // HTTP POST itself does not automatically preserve an original filename. 
   // The client must send a filename (commonly via multipart/form-data Content-Disposition filename="..." or a custom header). 
   // The server should not blindly trust that filename 
   // — generate a safe server-side name and optionally preserve only the extension or use Content-Type to infer an extension.
   // std::string filename = "upload_" + std::to_string(time(NULL)) + "_" + std::to_string(rand() % 1000) + ".dat";
   std::string filename = "upload_" + std::to_string(time(NULL)) + "_" + std::to_string(rand() % 1000);
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
   ofs.write(body.data(), body.size());
   ofs.close();

   // Generates response: Set headers (Content-Type, Content-Length, Date, Server)
   std::map<std::string, std::string> headers;
   std::string responseBody = "{\"status\":\"success\"}";
   headers["Content-Length"] = std::to_string(responseBody.size());
   headers["Content-Type"] = "application/json";

   return HttpResponse("HTTP/1.1", 201, "Created", responseBody, headers, httpUtils::shouldKeepAlive(req), true);
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
   const config::LocationConfig* lc = httpUtils::findLocationConfig(vh, uri);
   if (!lc)
      return makeErrorResponse(404, vh);
   if (!httpUtils::isMethodAllowed(lc, "DELETE"))
      return makeErrorResponse(405, vh);
   CGI cgi(req, *lc);
   if (cgi.isCGI()) {
         std::string cgi_output = cgi.execute();
         if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
            return makeErrorResponse(500, vh);
         return parseCGIOutput(cgi_output, req, vh);
   }

   // otherwie, it is a static delete. Maps path: /files/file1.txt → /var/www/html/files/file1.txt
   std::string fullpath = httpUtils::mapUriToPath(lc, uri);

   // Validates:
   // Does file exist?
   // Is it allowed to delete this path? (check directory permissions)?
   //  Is DELETE method allowed in this location?
   struct stat st;
   if (stat(fullpath.c_str(), &st) < 0)
      return makeErrorResponse(404, vh);
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
   return HttpResponse("HTTP/1.1", 204, "No Content", "", std::map<std::string, std::string>(), httpUtils::shouldKeepAlive(req), true);
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
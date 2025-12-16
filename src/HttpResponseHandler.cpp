#include "HttpResponseHandler.hpp"
#include <iostream>  //debug

namespace fs = std::filesystem; // Alias for filesystem


static bool extractMultipartFile(const HttpRequest& req, std::string& outFileData, std::string& outFileName)
{
	// Get Content-Type
	if (!req.getHeaders().count("content-type"))
		return false;

	std::string ct = req.getHeaders().at("content-type");
	size_t bpos = ct.find("boundary=");
	if (bpos == std::string::npos)
		return false;

	std::string boundary = ct.substr(bpos + 9);
   size_t semiPos = boundary.find(";");
   if (semiPos != std::string::npos){
      boundary = boundary.substr(0, semiPos);
   }
   boundary = httpUtils::trim_space(boundary);

   std::string marker = "--" + boundary;
	const std::string& body = req.getBody();

	// Find first boundary
	size_t partStart = body.find(marker);
	if (partStart == std::string::npos)
		return false;
	partStart += marker.size();
	if (body.compare(partStart, 2, "\r\n") == 0)
		partStart += 2;

	/// Find header end
    size_t headersEnd = body.find("\r\n\r\n", partStart);
    if (headersEnd == std::string::npos)
        return false;
    
    // Extract headers section
    std::string headers = body.substr(partStart, headersEnd - partStart);
    size_t filenamePos = headers.find("filename=\"");
    if (filenamePos != std::string::npos) {
        filenamePos += 10;  // Skip 'filename="'
        size_t filenameEnd = headers.find("\"", filenamePos);
        if (filenameEnd != std::string::npos) {
            outFileName = headers.substr(filenamePos, filenameEnd - filenamePos);
        }
    }
    
    // If no filename found, use default
    if (outFileName.empty()) {
        outFileName = "upload_" + std::to_string(time(NULL)) + ".dat";
    }


	size_t dataStart = headersEnd + 4; // skip CRLF CRLF

   size_t markerPos = body.find( marker, dataStart);
	if (markerPos == std::string::npos)
	{
		std::cout << "5\n";
		return false;
	}
	size_t dataEnd = markerPos;
	if (dataEnd >= 2 && body.compare(dataEnd - 2, 2, "\r\n") == 0)
		dataEnd -= 2;
	// std::string boundaryWithCRLF = "\r\n" + marker;
	// size_t markerPos = body.find(boundaryWithCRLF, dataStart);
	// if (markerPos == std::string::npos){
   //       std::string boundaryWithCRLF = "\n" + marker;
   //       markerPos = body.find(boundaryWithCRLF, dataStart);
   //       if (markerPos == std::string::npos)
   //          return false;
   // }
		
	// size_t dataEnd = markerPos;
	// if (dataEnd >= 2 && body.compare(dataEnd - 2, 2, "\r\n") == 0)
	// 	dataEnd -= 2;
	// Extract file bytes
	outFileData.assign(body.data() + dataStart, dataEnd - dataStart);
   std::cout << "Extracted file: " << outFileName 
              << " (" << outFileData.size() << " bytes)" << std::endl;
	return true;
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
   size_t sepLen = 4; // NOTE 15.12, when \r\n\r\n, it the length should be 4
   if (pos == std::string::npos){
      pos = out.find("\n\n");
      sepLen = 2; // NOTE 15.12, when \n\n, it the length should be 2
      if (pos == std::string::npos){
         return makeErrorResponse(500, vh);
      }
   }

   std::string headersString = out.substr(0, pos);
   std::string bodyString = out.substr(pos + sepLen); // NOTE 15.12

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
      if (key == "status") {
         size_t space = val.find(" ");
         statusCode = val.substr(0, space);
         statusMsg  = val.substr(space + 1);
      }
      else
         headersMap[key] = val;
   }
   // manually setup this one
   headersMap["Content-Length"] = std::to_string(bodyString.size());

   // Check if CGI already set Content-Type, otherwise use default
   if (headersMap.find("Content-Type") == headersMap.end()) {
      headersMap["Content-Type"] = "text/html; charset=UTF-8";
   }

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
    headers["content-type"] = "text/html";
    headers["content-length"] = std::to_string(body.size());
    headers["server"] = "MiniWebserv/1.0";
    headers["date"] = httpUtils::formatTime(time(NULL));

    return HttpResponse("HTTP/1.1", 200, "OK", body, headers, httpUtils::shouldKeepAlive(req), true);
}

// --------------------
//  InternalHandlers for different HTTP methods
// --------------------
HttpResponse HttpResponseHandler::handleGET(HttpRequest& req, const config::ServerConfig* vh)
{
   // get the request URI: uniform Resource Identifier, _path in the request
   std::string fullUri = req.getPath();  // Full URI including query string

   // Split URI from query parameters
   std::string uri = fullUri;
   size_t queryPos = fullUri.find('?');
   if (queryPos != std::string::npos) {
      uri = fullUri.substr(0, queryPos);
   }
   if (httpUtils::isCgiRequest(req, *vh)){
      const config::LocationConfig* lc = httpUtils::findLocationConfig(vh, uri, "GET");
      if (!lc){
         return makeErrorResponse(403, vh);
      }
      CGI cgi(req, *lc);
      if (!cgi.isAllowedCgi()){
        return makeErrorResponse(403, vh);
      }
      std::string cgi_output = cgi.execute();
      if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
         return makeErrorResponse(500, vh);
      return parseCGIOutput(cgi_output, req, vh);
   }

    // First find LocationConfig check if it is cgi
   const config::LocationConfig* lc = httpUtils::findLocationConfig(vh, uri, "GET");
   if (!lc)
      return makeErrorResponse(404, vh);

   // If location path ends with / but uri doesn't, normalize uri for path mapping
   if (lc->path.length() > 1 && lc->path.back() == '/' && !uri.empty() && uri.back() != '/') {
      // Check if uri matches location without trailing slash
      std::string locWithoutSlash = lc->path.substr(0, lc->path.length() - 1);
      if (uri == locWithoutSlash) {
         uri += "/";  // Add trailing slash for consistent path mapping
      }
   }
   // Check for configured redirect
   if (!lc->redirect.empty()) {
      return makeRedirect301(lc->redirect, vh);
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

         // Now check method after redirect handled
         if (!httpUtils::isMethodAllowed(lc, "GET"))
            return makeErrorResponse(405, vh);

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
               return makeErrorResponse(404, vh); // No index file and autoindex disabled
            // autoindex enabled → return HTML directory listing
            return generateAutoIndex(fullpath, req);
         }
      }
      // Not a directory - check method for regular files
      else if (!httpUtils::isMethodAllowed(lc, "GET"))
         return makeErrorResponse(405, vh);
   }
   else
      return makeErrorResponse(404, vh);
   if (access(fullpath.c_str(), R_OK) < 0)
      return makeErrorResponse(403, vh);
   if (!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode))
      return makeErrorResponse(403, vh);

   // Determined MIME type (text/plain for .txt or plain text).
   std::string mime_type = httpUtils::getMimeType(fullpath);

   // Check if download should be forced (via ?download query parameter)
   bool forceDownload = (fullUri.find("?download") != std::string::npos);

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
   headers["Cache-Control"] = "no-cache, no-store, must-revalidate";
   headers["Pragma"] = "no-cache";
   headers["Expires"] = "0";

   // Add Content-Disposition for forced downloads
   if (forceDownload) {
      size_t lastSlash = fullpath.find_last_of('/');
      std::string filename = (lastSlash != std::string::npos)
         ? fullpath.substr(lastSlash + 1)
         : "download";
      headers["content-disposition"] = "attachment; filename=\"" + filename + "\"";
   }
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
HttpResponse HttpResponseHandler::handlePOST(HttpRequest& req, const config::ServerConfig* vh)
{
	std::string fullUri = req.getPath();  // Full URI including query string
	// Split URI from query parameters
	std::string uri = fullUri;
	size_t queryPos = fullUri.find('?');
	if (queryPos != std::string::npos) {
		uri = fullUri.substr(0, queryPos);
	}
	if (httpUtils::isCgiRequest(req, *vh)){
			const config::LocationConfig* lc = httpUtils::findLocationConfig(vh, uri, "POST");
		if (!lc){
			return makeErrorResponse(403, vh);
		}
      if (!httpUtils::isMethodAllowed(lc, "POST"))
         return makeErrorResponse(405, vh);
		CGI cgi(req, *lc);
		if (!cgi.isAllowedCgi()){
		return makeErrorResponse(403, vh);
		}
		std::string cgi_output = cgi.execute();
		std::cout << cgi_output ;
		if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
			return makeErrorResponse(500, vh);
		return parseCGIOutput(cgi_output, req, vh);
	}
	//Non-CGI POST handling (raw body only)
	const config::LocationConfig* lc = httpUtils::findLocationConfig(vh, uri, "POST");
   if (!lc){
      return makeErrorResponse(404, vh);
   }
   if (!httpUtils::isMethodAllowed(lc, "POST"))
      return makeErrorResponse(405, vh);
	std::string ct;
	if (req.getHeaders().count("content-type"))
		ct = req.getHeaders().at("content-type");
	if (ct.find("multipart/form-data") != std::string::npos)
	{
		std::string fileData;
      std::string fileName;

		if (!extractMultipartFile(req, fileData, fileName))
			return makeErrorResponse(400, vh);

		std::string fullPath = lc->upload_dir;
      if (!fullPath.empty() && fullPath.back() != '/') {
         fullPath += "/";
      }
      fullPath += fileName;
      std::cout << "Uploading to: " << fullPath << std::endl;
		std::ofstream ofs(fullPath.c_str(), std::ios::binary);
		ofs.write(fileData.data(), fileData.size());
		ofs.close();
      std::string responseBody = "File uploaded successfully: " + fileName;
      std::map<std::string, std::string> headers;
      headers["Content-Type"] = "text/plain";
      headers["Content-Length"] = std::to_string(responseBody.size());
      return HttpResponse("HTTP/1.1", 200, "Created", responseBody, headers, httpUtils::shouldKeepAlive(req), true);

	}
	return HttpResponse("HTTP/1.1", 201, "Created", "hello", std::map<std::string, std::string>(), httpUtils::shouldKeepAlive(req), true);
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
   // Server receives DELETE /files/file1.txt?version=2 HTTP/1.1
   std::string fullUri = req.getPath();

   // remove the ? part: DELETE /files/file1.txt
   std::string uri = fullUri;
   size_t queryPos = fullUri.find('?');
   if (queryPos != std::string::npos)
      uri = fullUri.substr(0, queryPos);

   // first check CGI, Lucio said not necessary
   // if (httpUtils::isCgiRequest(req, *vh)){
   //    const config::LocationConfig* lc = httpUtils::findLocationConfig(vh, uri);
   //    if (!lc){
   //       return makeErrorResponse(404, vh);
   //    }
   //    CGI cgi(req, *lc);
   //    if (!cgi.isAllowedCgi()){
   //      return makeErrorResponse(403, vh);
   //    }
   //    std::string cgi_output = cgi.execute();
   //    if (cgi_output.empty() || cgi_output == "CGI_EXECUTE_FAILED")
   //       return makeErrorResponse(500, vh);
   //    return parseCGIOutput(cgi_output, req, vh);
   // }

   // otherwie, it is a static delete.
   const config::LocationConfig* lc = httpUtils::findLocationConfig(vh, uri);

   if (!lc)
      return makeErrorResponse(404, vh);
   if (!httpUtils::isMethodAllowed(lc, "DELETE"))
      return makeErrorResponse(405, vh);

   // DELETE should NOT auto-fix trailing slash (only for GET): /files → /files/

   // Check for configured redirect
   if (!lc->redirect.empty())
      return makeErrorResponse(307, vh);

   // Maps path: /files/file1.txt → /var/www/html/files/file1.txt: path to disk path
   std::string fullpath = httpUtils::mapUriToPath(lc, uri);

   // Validates: lstat instead of stat: not follwoing symbolic link
   // Does file exist?
   // Is it allowed to delete this path? (check directory permissions)?
   //  Is DELETE method allowed in this location?
   struct stat st;
   if (lstat(fullpath.c_str(), &st) < 0)
      return makeErrorResponse(404, vh);
   if (S_ISLNK(st.st_mode))
      return makeErrorResponse(403, vh);
   if (!S_ISREG(st.st_mode))
      return makeErrorResponse(403, vh);

   // Attempts deletion: - unlink("/var/www/html/files/file1.txt")
   if (unlink(fullpath.c_str()) < 0)
   {
      // file not existing
      if (errno == ENOENT)
         return makeErrorResponse(404, vh);
      // not access   
      if (errno == EACCES || errno == EPERM)
         return makeErrorResponse(403, vh);
      // is dir or not empty dir
      if (errno == EISDIR || errno == ENOTEMPTY)
         return makeErrorResponse(409, vh);
      return makeErrorResponse(500, vh);
   }

   // Generates response:
   std::map<std::string, std::string> headers;
   headers["content-length"] = "0";  // No response body

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
HttpResponse   HttpResponseHandler::handleRequest(HttpRequest& req, const config::ServerConfig* vh) {
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
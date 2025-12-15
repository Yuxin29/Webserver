#include "HttpResponse.hpp"

// --------------------
//   Serialization
// --------------------
/**
 * @brief builds the HTTP response string
 *
 * @param void
 * @return a string representing the full HTTP response
 *
 * @note constructs the response string including status line, headers, and body
 */
std::string HttpResponse::buildResponseString(){
    std::ostringstream response;

    response << _version << " " << _status << " " << _reason << "\r\n";

    bool hasContentLength = false;
    // iterates though the _responseHeaders map container and check if there is Content-Length
    for (const auto &header : _responseHeaders) {
        if (header.first == "Content-Length")
            hasContentLength = true;
        response << header.first << ": " << header.second << "\r\n";
    }
    if (!hasContentLength) {
        response << "Content-Length: " << _body.size() << "\r\n";
    }
    // Add Connection header based on _keepConnectionAlive flag
    if (_responseHeaders.find("Connection") == _responseHeaders.end()) {
        if (_keepConnectionAlive)
            response << "Connection: keep-alive\r\n";
        else
            response << "Connection: close\r\n";
    }
    response << "\r\n";
    response << _body;
    return response.str();
}

static const std::map<int, std::string> STATUS_REASON = {
    {301, "Moved Permanently"},
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

// HttpResponse makeErrorResponse(int status, const config::ServerConfig* vh)
// {
//    // Find reason phrase
//    std::string reason;
//    if (STATUS_REASON.count(status))
//       reason = STATUS_REASON.at(status);
//    else {
//       status = 500;
//       reason = "Internal Server Error";
//    }

//    // Load custom error page if configured
//    std::string body;
//    if (vh && vh->errorPages.count(status)){
//       const std::string& path = vh->errorPages.at(status);
//       body = loadFile(path);
      
//       // Log if custom error page fails to load
//       if (body.empty()) {
//          std::cerr << "Warning: Failed to load custom error page: " 
//                    << path << " for status " << status << std::endl;
//       }
//    }

//    // Fallback to default HTML
//    if (body.empty()) {
//       body = "<!DOCTYPE html>\n"
//              "<html>\n"
//              "<head><title>" + std::to_string(status) + " " + reason + "</title></head>\n"
//              "<body>\n"
//              "<h1>" + std::to_string(status) + " " + reason + "</h1>\n"
//              "<p>The server encountered an error processing your request.</p>\n"
//              "</body>\n"
//              "</html>";
//    }

//    // Build headers
//    std::map<std::string, std::string> headers;
//    headers["Content-Type"] = "text/html; charset=UTF-8";
//    headers["Content-Length"] = std::to_string(body.size());
//    headers["Cache-Control"] = "no-cache, no-store, must-revalidate";
//    headers["Pragma"] = "no-cache";
//    headers["Expires"] = "0";
   
//    // Optional: Add server identification
//    // headers["Server"] = "webserv/1.0";
   
//    // Optional: Security headers
//    // headers["X-Content-Type-Options"] = "nosniff";
   
//    return HttpResponse("HTTP/1.1", status, reason, body, headers, false, false);
// }

HttpResponse makeRedirect301(const std::string& location, const config::ServerConfig* vh)
{
   std::string body;
   if (vh && vh->errorPages.count(301)) {
      body = loadFile(vh->errorPages.at(301));
   }
   if (body.empty())
      body = "<h1>301 Moved Permanently</h1>";

   std::map<std::string, std::string> headers;
   headers["Location"] = location;

   return HttpResponse("HTTP/1.1", 301, "Moved Permanently", body, headers, false, true);
}		// yuxin need to tell lin
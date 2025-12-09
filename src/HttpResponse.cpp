#include "HttpResponse.hpp"

// --------------------
//        Getters
// --------------------
const std::string&  HttpResponse::getVersion() const{
    return _version;
}
    
int HttpResponse::getStatus() const{
    return _status;
}

const std::string&  HttpResponse::getReason() const{
    return _reason;
}

const std::string& HttpResponse::getBody() const{
    return _body;
}

const std::map<std::string, std::string>&  HttpResponse::getHeaders() const{
    return _responseHeaders;
}

bool HttpResponse::isKeepAlive() const{
    return _keepConnectionAlive;
}

bool    HttpResponse::isRequestComplete() const{
    return _requestComplete;
}

// --------------------
//        Setters
// --------------------
void    HttpResponse::setVersion(const std::string &v){
    _version = v;
}

void    HttpResponse::setStatus(const int &s){
    _status = s;
}

void    HttpResponse::setReason(const std::string &r){
    _reason = r;
}
    
void    HttpResponse::setBody(const std::string &b){
    _body = b;
}

void    HttpResponse::addHeader(const std::string& k, const std::string& v){
    _responseHeaders[k] = v;
}

void    HttpResponse::setKeepAlive(const bool &alive){    
    _keepConnectionAlive = alive;
}

void    HttpResponse::setRequestComplete(const bool &complete){
    _requestComplete = complete;
}

// --------------------
//    Constructors
// --------------------
/**
 * @brief constructor for HttpResponse class
 *
 * @param strs and map of strings
 * @return an HttpResponse object
 *
 * @note initializes all member variables of the HttpResponse class
 */
HttpResponse::HttpResponse(const std::string& version, const int& status, const std::string& reason, const std::string& body, const std::map<std::string, std::string>& responseHeaders, bool alive, bool complete)
{
    _version = version;
    _status = status;
    _reason = reason;
    _body = body;
    _responseHeaders = responseHeaders;
    _keepConnectionAlive = alive;
    _requestComplete = complete;
}

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

/**
 * @brief Builds an HTTP error response for a given status code.
 *
 * @param status HTTP status code (e.g., 400, 405, 413, 431)
 * @return HttpResponse object representing the error page
 *
 * @note Currently supports 400, 405, 413, 414(addable, not mandatory), 431. Default is 400.
    @note Example error page format:
 */
HttpResponse reqParsingErrorResponse(int status)
{
    std::map<std::string, std::string> headers;

    switch (status)
    {
        case 400:
            return HttpResponse("HTTP/1.1", status, "Bad Request", "<h1>400 Bad Request</h1>", headers, false, false);
        case 405:
            return HttpResponse("HTTP/1.1", status, "Method Not Allowed", "<h1>405 Method Not Allowed</h1>", headers, false, false);
        case 413:
            return HttpResponse("HTTP/1.1", status, "Payload Too Large", "<h1>413 Payload Too Large</h1>", headers, false, false);
        case 414:
            return HttpResponse("HTTP/1.1", status, "URI Too Long", "<h1>414 URI Too Long</h1>", headers, false, false);
        case 431:
            return HttpResponse("HTTP/1.1", status, "Request Header Fields Too Large", "<h1>431 Request Header Fields Too Large</h1>", headers, false, false);
        default:
            return HttpResponse("HTTP/1.1", status, "Bad Request", "<h1>400 Bad Request</h1>", headers, false, false);
    }
}

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
HttpResponse resHandlerErrorResponse(int code){
    switch (code) {
        case 400:
            return HttpResponse("HTTP/1.1", 400, "Bad Request", "<h1>400 Bad Request</h1>", {}, false, false);
        case 403:
            return HttpResponse("HTTP/1.1", 403, "Forbidden", "<h1>403 Forbidden</h1>", {}, false, false);
        case 404:
            return HttpResponse("HTTP/1.1", 404, "Not Found", "<h1>404 Not Found</h1>", {}, false, false);
        case 405:
            return HttpResponse("HTTP/1.1", 405, "Method Not Allowed", "<h1>405 Method Not Allowed</h1>", {}, false, false);
        case 408:
            return HttpResponse("HTTP/1.1", 408, "Connection Time-out", "<h1>408 Request Time-out</h1>", {}, false, false);
        case 500:
        default:
            return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<h1>500 Internal Server Error</h1>", {}, false, false);
    }
}
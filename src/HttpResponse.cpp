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
    // if not set be the user, by default, the connection is closed
    if (_responseHeaders.find("Connection") == _responseHeaders.end()) {
        response << "Connection: close\r\n";
    }
    response << "\r\n";
    response << _body;
    return response.str();
}
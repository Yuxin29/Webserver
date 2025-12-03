#include "HttpRequest.hpp"

//just getters
const std::string& HttpRequest::getMethod() const{
    return _method;
}

const std::string& HttpRequest::getPath() const{
    return _requestPath;
}   

const std::string& HttpRequest::getVersion() const{
    return _version;
}   

const std::map<std::string, std::string>&  HttpRequest::getHeaders() const{
    return _requestHeaders;
}

const std::string& HttpRequest::getBody() const{
    return _body;
}

void HttpRequest::setMethod(const std::string &m){
    _method = m;
}

void HttpRequest::setPath(const std::string &p){
    _requestPath = p;
}

void HttpRequest::setVersion(const std::string &v){
    _version = v;
}

void HttpRequest::addHeader(const std::string& k, const std::string& v){ 
    _requestHeaders[k] = v;
}

void HttpRequest::setBody(const std::string &b){
    _body = b;
}
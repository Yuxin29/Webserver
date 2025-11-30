#include "HttpRequest.hpp"

//just getters
std::string& HttpRequest::getMethod(){
    return _method;
}

std::string& HttpRequest::getPath(){
    return _requestPath;
}   

std::string& HttpRequest::getVersion(){
    return _version;
}   

std::map<std::string, std::string>&  HttpRequest::getHeaders(){
    return _requestHeaders;
}

std::string& HttpRequest::getBody(){
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
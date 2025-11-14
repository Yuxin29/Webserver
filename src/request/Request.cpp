#include "Request.hpp"

//the for Request class
HttpRequest::HttpRequest(const std::string& method, const std::string& requestpath, const std::string& version, const std::string& body, const std::map<std::string, std::string>& requestHeaders)
    :_method(method), _requestPath(requestpath), _version(version), _body(body), _requestHeaders(requestHeaders){
}

HttpRequest::HttpRequest(const HttpRequest& other)
    : _method(other._method), _requestPath(other._requestPath), _version(other._version), _body(other._body), _requestHeaders(other._requestHeaders){
}            
    
HttpRequest& HttpRequest::operator=(const HttpRequest &other) = delete{
}

HttpRequest::~HttpRequest(){
}

std::string HttpRequest::getMethod(){
    return _method;
}

std::string HttpRequest::getrequestPath(){
    return _requestPath;
}   

std::string HttpRequest::getVersion(){
    return _version;
}   

std::map<std::string, std::string>  HttpRequest::getrequestHeaders{
    return _body;
}
std::string HttpRequest::getBody(){
    return _requestHeaders;
}

HttpResponse(const std::HttpResponse& version, const std::string& status, const std::string& phase, const std::string& body, const std::map<std::string, std::string>& requestHeaders);

//the for Request class
HttpRequest::HttpRequest(const std::string& method, const std::string& requestpath, const std::string& version, const std::string& body, const std::map<std::string, std::string>& requestHeaders)
    :_method(method), _requestPath(requestpath), _version(version), _body(body), _requestHeaders(requestHeaders){
}

HttpRequest::HttpRequest(const HttpRequest& other)
    : _method(other._method), _requestPath(other._requestPath), _version(other._version), _body(other._body), _requestHeaders(other._requestHeaders){
}            
    
HttpRequest& HttpRequest::operator=(const HttpRequest &other) = delete{
}

HttpRequest::~HttpRequest(){
}

std::string HttpRequest::getMethod(){
    return _method;
}

std::string HttpRequest::getrequestPath(){
    return _requestPath;
}   

std::string HttpRequest::getVersion(){
    return _version;
}   

std::map<std::string, std::string>  HttpRequest::getrequestHeaders{
    return _body;
}
std::string HttpRequest::getBody(){
    return _requestHeaders;
}
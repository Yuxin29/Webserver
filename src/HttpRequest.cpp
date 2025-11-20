#include "HttpRequest.hpp"

/*  ******************************HttpRequest********************************  */
HttpRequest::HttpRequest(const std::string& method, const std::string& requestpath, const std::string& version, const std::string& body, const std::map<std::string, std::string>& requestHeaders)
    :_method(method), _requestPath(requestpath), _version(version), _body(body), _requestHeaders(requestHeaders){
}

HttpRequest::HttpRequest(const HttpRequest& other)
    : _method(other._method), _requestPath(other._requestPath), _version(other._version), _body(other._body), _requestHeaders(other._requestHeaders){
}            

HttpRequest::~HttpRequest(){
}

const std::string HttpRequest::getMethod(){
    return _method;
}

const std::string HttpRequest::getrequestPath(){
    return _requestPath;
}   

const std::string HttpRequest::getVersion(){
    return _version;
}   

const std::map<std::string, std::string>  HttpRequest::getrequestHeaders(){
    return _requestHeaders;
}

const std::string HttpRequest::getBody(){
    return _body;
}
/*  ******************************HttpRequest********************************  */

#include "HttpRequest.hpp"

//just getters
const std::string& HttpRequest::getMethod() const{
    return _method;
}

const std::string& HttpRequest::getrequestPath() const{
    return _requestPath;
}   

const std::string& HttpRequest::getVersion() const{
    return _version;
}   

const std::map<std::string, std::string>&  HttpRequest::getrequestHeaders() const{
    return _requestHeaders;
}

const std::string& HttpRequest::getBody() const{
    return _body;
}
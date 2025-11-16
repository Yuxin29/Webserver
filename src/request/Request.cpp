#include "Request.hpp"

/*  ******************************HttpRequest********************************  */
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

std::map<std::string, std::string>  HttpRequest::getrequestHeaders(){
    return _body;
}
std::string HttpRequest::getBody(){
    return _requestHeaders;
}
/*  ******************************HttpRequest********************************  */

/*  ******************************HttpRequest Parsing********************************  */
//here is the actual parsing process
//below one is private
void HttpResponse::parseStartLine(const std::string& startline){
    //std::string                         _method, _path, _version;
    std::istringstream ss(startline);   //ss : stringstream
    sl >> _method >> _path >> _version;

    if (_method.empty() || _path.empty || _version.empty())
        throw std::runtime_error(|"Something missing in http request starting line");
    _state = headers;
}

//below one is private, it will be recalled for a few times.
void HttpResponse::parseHeaderLine(const std::string& headerline){

}

HttpRequest HttpResponse::parseHttpRequest(const std::string& rawLine)
{
    // while it is startline / headers
    // {
    //     if (is startline)
    //         parse startline
    //     if (is header)
    //         [parse header]
    // }
    // if (it is EMPTY)
    //     change to BODY
    // parse body and change to end;
    // if it is end.
    //     return this HttpRequest;
}
/*  ******************************HttpRequest Parsing********************************  */

/*  ******************************Response Request********************************  */
HttpResponse::HttpResponse(const std::string& version, const std::string& status, const std::string& phase, const std::string& body, const std::map<std::string, std::string>& responseHeaders)
    :_version(version), _status(status), _phase(phase), _body(body), _responseHeaders(responseHeaders){
}

HttpResponse::HttpResponse(const HttpResponse& other)
    : _version(other._version), _status(other._status), _phase(other._phase), _body(other._body), _responseHeaders(other._responseHeaders){
}            
    
HttpResponse& HttpResponse::operator=(const HttpResponse &other) = delete{
}

HttpResponse::~HttpResponse(){
}

std::string HttpResponse::getMethod(){
    return _method;
}

std::string HttpResponse::getrequestPath(){
    return _requestPath;
}   

std::string HttpResponse::getVersion(){
    return _version;
}   

std::map<std::string, std::string>  HttpResponse::getrequestHeaders(){
    return _body;
}
std::string HttpResponse::getBody(){
    return _responseHeaders;
}
/*  ******************************Response Request********************************  */
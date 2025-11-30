#pragma once

#include <string>
#include <map>

// <start-line>\r\n
// <header1: value>\r\n
// <header2: value>\r\n
// ...\r\n
// \r\n
// <body>
class HttpRequest{
private:
    std::string                         _method;
    std::string                         _requestPath;
    std::string                         _version;
    std::map<std::string, std::string>  _requestHeaders;
    std::string                         _body;

public:
    //Orthodox not all needed: defined just here
    HttpRequest() {}
    // HttpRequest(const std::string& method, const std::string& requestpath, const std::string& version, const std::string& body, const std::map<std::string, std::string>& requestHeaders)
    // : _method(method), _requestPath(requestpath), _version(version), _body(body), _requestHeaders(requestHeaders) {}

    //getters
    std::string&                         getMethod();
    std::string&                         getPath();
    std::string&                         getVersion();
    std::map<std::string, std::string>&  getHeaders();
    std::string&                         getBody();

    //setters
    void setMethod(const std::string &m);
    void setPath(const std::string &m);
    void setVersion(const std::string &m);
    void addHeader(const std::string& k, const std::string& v);
    void setBody(const std::string &m);
};

// method required by this subject:    GET, POST, DELETE

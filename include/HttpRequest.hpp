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
    const std::string                         _method;
    const std::string                         _requestPath;
    const std::string                         _version;
    const std::string                         _body;
    const std::map<std::string, std::string>  _requestHeaders;

public:
    //Orthodox not all needed: defined just here
    HttpRequest() {}
    HttpRequest(const std::string& method, const std::string& requestpath, const std::string& version, const std::string& body, const std::map<std::string, std::string>& requestHeaders)
    : _method(method), _requestPath(requestpath), _version(version), _body(body), _requestHeaders(requestHeaders) {}

    //getters
    const std::string&                         getMethod() const;
    const std::string&                         getrequestPath() const;
    const std::string&                         getVersion() const;
    const std::map<std::string, std::string>&  getrequestHeaders() const;
    const std::string&                         getBody() const;
};

// method required by this subject:    GET, POST, DELETE

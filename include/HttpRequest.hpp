#pragma once

#include <string>
#include <iostream>
#include <stdexcept>
#include <map>
#include <sstream>

// method required by this subject:    GET, POST, DELETE

// a Request class: method, target/path, version, headers map, body buffer, maybe query params and parsed cookies.
// <start-line>\r\n
// <header1: value>\r\n
// <header2: value>\r\n
// ...\r\n
// \r\n
// <body>
// 1. <start-line>\r\n
// Method sp _path sp version crlf(Carriage Return and Line Feed)
// Get /index.html HTTP/1.1\r\n
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

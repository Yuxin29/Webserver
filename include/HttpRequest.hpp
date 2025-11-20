#pragma once

#include <string>
#include <iostream>
#include <stdexcept>
#include <map>
#include <sstream>

// -----------------------------------------------------------------------------------------------------
// method required by this subject: 
// enum Method{
//     GET,
//     POST,
//     DELETE
// };

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
    //Orthodox * 4 
    HttpRequest() {}
    HttpRequest(const std::string& method, const std::string& requestpath, const std::string& version, const std::string& body, const std::map<std::string, std::string>& requestHeaders);
    HttpRequest(const HttpRequest& other);               
    HttpRequest& operator=(const HttpRequest &other) = delete;
    ~HttpRequest();

    //getters
    const std::string                         getMethod();
    const std::string                         getrequestPath();
    const std::string                         getVersion();
    const std::map<std::string, std::string>  getrequestHeaders();
    const std::string                         getBody();
};

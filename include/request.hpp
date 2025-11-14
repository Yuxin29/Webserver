#program once

#include <string>
#include <iostream>
#include <stdexcept>
#include <map>
//maybe more

//  ├─ HttpRequest.hpp             // // response builder
//  ├─ HttpParser.hpp              // state machine parser
//  ├─ HttpResponse.hpp            // response builder
//  ├─ StatusCodes.hpp          // constant table
//  ├─ Headers.hpp              // utility header functions
//  ├─ MimeTypes.hpp            // text/html, image/png, etc.

enum Method{
    GET
    POST
    DELETE
};

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
    std::string                         _method;
    std::string                         _requestPath;
    std::string                         _version;
    std::map<std::string, std::string>  _requestHeaders;
    std::string                         _body;

public:
    //Orthodox * 4 

    //getters
    std::string                         getMethod();
    std::string                         getrequestPath();
    std::string                         getVersion();
    std::map<std::string, std::string>  getrequestHeaders;
    std::string                         getBody();
};

//Accepts raw bytes (from non-blocking reads) and advances through states:
class HttpParser
{

}

// <status-line>\r\n
// <header1: value>\r\n
// ...\r\n
// \r\n
// <body>
class HttpResponse{
private:
    std::string                         _method;
    std::string                         _requestPath;
    std::string                         _version;
    std::map<std::string, std::string>  _responseHeaders;
    std::string                         _body;

public:
    //Orthodox * 4 

    //getters
    std::string                         getMethod();
    std::string                         getrequestPath();
    std::string                         getVersion();
    std::map<std::string, std::string>  getrequestHeaders;
    std::string                         getBody();
};
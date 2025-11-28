#pragma once

#include <map>
#include <sstream>

//constom status_code
// 200 OK
// 404 NOT FOUND 
// 500 INTERNAL SERVER ERROR 
// 413 Payload too large 

//Headers:
// --> must have "Content-Length"
// --> can have  "Content-type": MIME type，eg text/html, text/plain）
// --> Connection: close / Connection: keep-alive 

//Body
// file content from GET or data from POST
// size must be the same as Content-Length

// Example of a HttpResponse
// HTTP/1.1 <status_code> <reason_phrase>\r\n
// Header1: value1\r\n
// Header2: value2\r\n
// ...
// \r\n
// <body>
class HttpResponse{
public:
    std::string                         _version;
    int                                 _status;
    std::string                         _reason;
    std::string                         _body;
    std::map<std::string, std::string>  _responseHeaders;
    bool                                _keepConnectionAlive;
    bool                                _requestComplete;

    HttpResponse() {};
    HttpResponse(const std::string& version, const int& status, const std::string& reason, const std::string& body, const std::map<std::string, std::string>& responseHeaders, bool alive, bool complete);
    
    std::string buildResponseString();
};

// how to check the content of a full HTTP request and respongs
// eg: curl -v http://localhost:8080/test.txt
/*
> GET /test.txt HTTP/1.1
> Host: localhost:8080
> User-Agent: curl/7.68.0
> Accept: *
< HTTP/1.1 200 OK
< Content-Length: 12
< Content-Type: text/plain
< Connection: keep-alive
< Hello World
*/

// > Request  
// < Reponse 
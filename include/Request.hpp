#pragma once

#include <string>
#include <iostream>
#include <stdexcept>
#include <map>
#include <sstream>
//maybe more

//  ├─ HttpRequest.hpp             // // response builder
//  ├─ HttpParser.hpp              // state machine parser
//  ├─ HttpResponse.hpp            // response builder
//  ├─ StatusCodes.hpp          // constant table
//  ├─ Headers.hpp              // utility header functions
//  ├─ MimeTypes.hpp            // text/html, image/png, etc.

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
    HttpRequest(const std::string& method, const std::string& requestpath, const std::string& version, const std::string& body, const std::map<std::string, std::string>& requestHeaders);
    HttpRequest(const HttpRequest& other);               
    HttpRequest& operator=(const HttpRequest &other);
    ~HttpRequest();

    //getters
    std::string                         getMethod();
    std::string                         getrequestPath();
    std::string                         getVersion();
    std::map<std::string, std::string>  getrequestHeaders();
    std::string                         getBody();
};

// -----------------------------------------------------------------------------------------------------
// this is used in both HttpRequest and HttpResponse
enum State{
    START_LINE,
    HEADERS,
    EMPTY,   // might not be neccessary ??
    BODY,
    DONE
};

//Accepts raw bytes (from non-blocking reads) and advances through states:
// I might recerive something from Lucio like this:
GET /index.html HTTP/1.1\r\n
// Host: example.com\r\n
// Content-Length: 5\r\n
// \r\n
// hello
// in this class, I put it into HttpRequest class
// I will use State Machine here
// check state -> get input -> do a thin -> change to next state
class HttpParser{
private:
    //should I have these repreating here? maybe i do, they r not const
    std::string                         _method, _path, _version;
    std::map<std::string, std::string>  _responseHeaders;
    std::string                         _body;
    //buffering here
    std::string                         _buffer;
    size_t                              _bufferLength;

    void parseStartLine(const std::string& startline);
    void parseHeaderLine(const std::string& headerline);

public:
    State   _state;
    //default construction ft. _buffer("") is not necessary
    HttpParser() : _state(START_LINE), _buffer(""), _bufferLength(0){};

    HttpRequest parseHttpRequest(const std::string& rawLine);
};

// -----------------------------------------------------------------------------------------------------

// <status-line>\r\n
// <header1: value>\r\n
// ...\r\n
// \r\n
// <body>
// 1.<status-line>\r\n
// VERSION SP STATUS_CODE SP REASON_PHRASE CRLF
// HTTP/1.1 200 OK\r\needs
class HttpResponse{
private:
    std::string                         _version;
    std::string                         _status;
    std::string                         _phase;
    std::map<std::string, std::string>  _responseHeaders;
    std::string                         _body;

public:
    //Orthodox * 4 
    HttpResponse(const std::string& version, const std::string& status, const std::string& phase, const std::string& body, const std::map<std::string, std::string>& responseHeaders);
    HttpResponse(const HttpResponse& other);               
    HttpResponse& operator=(const HttpResponse &other);
    ~HttpResponse();

    //getters
    std::string                         getVersion();
    std::string                         getStatus();
    std::string                         getPhase();
    std::map<std::string, std::string>  getrequestHeaders();
    std::string                         getBody();
};
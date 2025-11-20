#pragma once

#include <string>
#include <iostream>
#include <stdexcept>
#include <map>
#include <sstream>

class HttpRequest;

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
// GET /index.html HTTP/1.1\r\n
// Host: example.com\r\n
// Content-Length: 5\r\n
// \r\n
// hello
// in this class, I put it into HttpRequest class
// I will use State Machine here
// check state -> get input -> do a thin -> change to next state
class HttpParser{
private:
    std::string                         _method, _path, _version;
    std::map<std::string, std::string>  _requestHeaders;
    std::string                         _body;

    std::string                         _buffer;
    size_t                              _bodyLength;

    bool validateRequestMethod();    //  GET, POST, DELETE  ---> 405
    bool validateHttpVersion();      //  Only accept HTTP/1.1
    void parseStartLine(const std::string& startline);

    // bool validateMandatoryHeaders(); // Host
    // bool validateRepeatingHeaders(); // eg: multiple Hosts
    // bool validateContentLength();    // eg: like minus number ---> 300 or too big(Payload Too Large) ---> 413
    bool validateHeadersSetup();
    void parseHeaderLine(const std::string& headerline);

    bool validateBody();             // Post must have body, body must fullfill ContentLength
    void parseBody(size_t pos);

public:
    State   _state = START_LINE;
    HttpParser() :_bodyLength(0){};

    HttpRequest parseHttpRequest(const std::string& rawLine);
};

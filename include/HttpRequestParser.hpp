#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <map>

#include "HttpRequest.hpp"

// this is used in both HttpRequest and HttpResponse
enum State{
    START_LINE,
    HEADERS,
    EMPTY,   // might not be neccessary ??
    BODY,
    DONE,
    ERROR //Lucio addition, set the status to error and generate an error response.

};

// Accepts raw bytes (from non-blocking reads) from Lucio and advances through states:
// I will use State Machine here
// check state -> get input -> do a thing -> change to next state
class HttpParser{
private:
    std::string                         _method, _path, _version;
    int                                 _errStatus = 200;
    std::map<std::string, std::string>  _requestHeaders;
    std::string                         _body;

    std::string                         _buffer;
    size_t                              _bodyLength;

    bool validateStartLine();
    void parseStartLine(const std::string& startline);

    bool validateHeaders();
    void parseHeaderLine(const std::string& headerline);

    bool validateBody();             // Post must have body, body must fullfill ContentLength
    void parseBody(size_t pos);

public:
    State   _state = START_LINE;
    HttpParser() :_bodyLength(0){};

    HttpRequest parseHttpRequest(const std::string& rawLine);
};

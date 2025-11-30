#pragma once

#include <iostream>
#include <sstream>

#include "HttpRequest.hpp"  // map, string here

// this is used in HttpRequest
enum State{
    START_LINE,
    HEADERS,
    EMPTY,   // might not be neccessary ??
    BODY,
    DONE,
    ERROR
};

// Accepts raw bytes (from non-blocking reads) from Lucio and advances through states:
// I will use State Machine here
// check state -> get input -> do a thing -> change to next state
class HttpParser{
private:
    HttpRequest                         _req;
    int                                 _errStatus = 0; //set the err to specific error code(404) and generate an error response.
    State                               _state = START_LINE;

    std::string                         _buffer;
    size_t                              _bodyLength;

    bool validateStartLine();
    void parseStartLine(const std::string& startline);
    bool validateHeaders();
    void parseHeaderLine(const std::string& headerline);
    bool validateBody();
    void parseBody(size_t pos);
public:
    HttpParser() :_bodyLength(0){};
    int             getState();
    int             getErrStatus();
    HttpRequest     parseHttpRequest(const std::string& rawLine);
};

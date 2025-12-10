#pragma once

#include <iostream>
#include <sstream>
#include <set>

#include "HttpRequest.hpp"  // map, string here

/**
 * @enum State
 * @brief Represents the internal parsing state of the HTTP request parser.
 *
 * @note The parser transitions through these states as raw data is fed into it:
 */
enum State{
    START_LINE,     ///< Parsing the initial request line ("GET / HTTP/1.1").
    HEADERS,        ///< Reading header lines.  
    BODY,           ///< Parsing the message body (POST, PUT, etc.).
    DONE,           ///< Successfully finished parsing the request.
    ERROR           ///< Encountered malformed syntax or invalid structure.
};

/**
 * @class HttpParser
 * @brief Incrementally parses raw HTTP request data using a state machine.
 *
 * @note This parser accepts raw bytes (often from non-blocking socket reads) and processes them according to HTTP request grammar. 
 * @note The parser maintains an internal state machine: check state -> get input -> do a thing -> change to next state
 *
 * Usage example:
 * @code
 * HttpParser parser;
 * while (parser.getState() != DONE && parser.getState() != ERROR) {
 *     parser.parseHttpRequest(rawDataChunk);
 * }
 * HttpRequest req = parser.getState() == DONE ? parser.getRequest() : ...;
 * @endcode
 */
class HttpParser{
private:
    HttpRequest             _req;                   ///< The HttpRequest object being constructed   
    int                     _errStatus = 0;         ///< HTTP error status code if parsing fails
    State                   _state = START_LINE;    ///< Current state of the parser

    std::string             _buffer;                ///< The resulting parsed HttpRequest object
    size_t                  _bodyLength;            ///< Expected length of the message body (from Content-Length header)

    // --------------------
    //  Internal Validation Methods
    // --------------------
    bool                     validateStartLine();
    bool                     validateHeaders();
    bool                     validateBody();
    // --------------------
    //  Internal Parsing Methods
    // --------------------
    void                    parseStartLine(const std::string& startline);
    void                    parseHeaderLine(const std::string& headerline);
    void                    parseBody(size_t pos);

public:
    // --------------------
    // Public Parsing Methods
    // --------------------
    HttpRequest             parseHttpRequest(const std::string& rawLine);
    // --------------------
    //      Getters
    // --------------------
    int                     getState() {return _state;}
    int                     getErrStatus() {return _errStatus; }
};
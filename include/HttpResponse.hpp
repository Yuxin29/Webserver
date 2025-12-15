#pragma once

#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "ConfigBuilder.hpp"
#include "CGI.hpp"

/**
 * @class HttpResponse
 * @brief Represents an HTTP response that will be sent to the client.
 *
 * This class contains the HTTP version, status line, headers, and body.
 * It can serialize itself into a properly formatted HTTP response string.
 *
 * Example response format:
 * @code
 * HTTP/1.1 200 OK\r\n
 * Content-Type: text/html\r\n
 * Content-Length: 13\r\n
 * Connection: keep-alive\r\n
 * \r\n
 * Hello, world!
 * @endcode
 *
 * @note Required / recommended response components:
 * - **Status Codes** commonly used:
 *      - 200 OK
 *      - 404 Not Found
 *      - 500 Internal Server Error
 *      - 413 Payload Too Large
 *
 * @note Mandatory / optional headers:
 * - **Content-Length** is required.
 * - **Content-Type** is recommended (e.g., text/html, text/plain, image/png).
 * - **Connection** can be `"close"` or `"keep-alive"`.
 *
 * @note Body:
 * - Contains file content (GET) or POST data.
 * - Its size **must match Content-Length** exactly.
  * @note _keepConnectionAlive:
 * - By default, HTTP/1.1 connections are persistent (keep-alive), unless the server sends Connection: close in its response.
 * - This means the TCP connection can be reused for multiple requests.
 */
class HttpResponse{
private:
    std::string                             _version;                       ///< HTTP version string (e.g., "HTTP/1.1")
    int                                     _status;                        ///< HTTP status code (e.g., 200, 404, 500)
    std::string                             _reason;                        ///< Reason phrase corresponding to the status code (e.g., "OK", "Not Found")
    std::string                             _body;                          ///< Message body of the response
    std::map<std::string, std::string>      _responseHeaders;               ///< Key-value map of response headers

    bool                                    _keepConnectionAlive = true;    ///< Whether to keep the connection alive (HTTP/1.1 default, HTTP1.1 keeps connection alive unless there is an error OR client requests comes with a Connection:close)
    bool                                    _requestComplete;               ///< Whether the request has been completely processed
public:
    // --------------------
    //        Getters
    // --------------------
    const std::string&                         getVersion() const           { return _version; }
    int                                        getStatus() const            { return _status; }
    const std::string&                         getReason() const            { return _reason; }
    const std::string&                         getBody() const              { return _body; }
    const std::map<std::string, std::string>&  getHeaders() const           { return _responseHeaders; }
    bool                                       isKeepAlive() const          { return _keepConnectionAlive; }
    bool                                       isRequestComplete() const    { return _requestComplete; }

    // --------------------
    //        Setters
    // --------------------
    void        setVersion(const std::string &v)                            { _version = v; }
    void        setStatus(const int &s)                                     { _status = s; }
    void        setReason(const std::string &r)                             { _reason = r; }
    void        setBody(const std::string &b)                               { _body = b; }
    void        addHeader(const std::string& k, const std::string& v)       { _responseHeaders[k] = v; }
    void        setKeepAlive(const bool &alive)                             { _keepConnectionAlive = alive; }
    void        setRequestComplete(const bool &complete)                    { _requestComplete = complete; }

    // --------------------
    //    Constructors
    // --------------------
    HttpResponse() {};
    HttpResponse(const std::string& version, const int& status, const std::string& reason,
                    const std::string& body,
                    const std::map<std::string, std::string>& responseHeaders,
                    bool alive, bool complete)
    {
    _version = version; _status = status; _reason = reason;
    _body = body;
    _responseHeaders = responseHeaders;
    _keepConnectionAlive = alive; _requestComplete = complete;}

    // --------------------
    //   Serialization
    // --------------------
    std::string buildResponseString();
};

// --------------------
//   Error Response Helpers
// --------------------
std::string loadFile(const std::string& path);

HttpResponse makeErrorResponse(int status, const config::ServerConfig* vh);
HttpResponse makeRedirect301(const std::string& location, const config::ServerConfig* vh);
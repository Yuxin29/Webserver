#pragma once

#include <string>
#include <map>

/**
 * @class HttpRequest
 * @brief Represents a parsed HTTP request received by the server.
 *
 * This class stores the HTTP request line (method, path, version),
 * the request headers, and the optional message body. 
 * It is used by the HTTP parser and passed into the request handlers (GET/POST/DELETE).
 *
 * @note The supported HTTP methods according to the project specification are: GET; POST; DELETE
 *
 * Example request format:
 * @code
 * GET /index.html HTTP/1.1\r\n
 * Host: localhost\r\n
 * User-Agent: curl/7.81.0\r\n
 * \r\n
 * <body>
 * @endcode
 *
 * Edge case for Http request
 * @code
 * GET  HTTP/1.1
 * Host: localhost:8080
 * User-Agent: curl/8.3.0
 * Accept: stars/stars
 * \r\n
 * @endcode
 * here the request path is empty:
 * - Nginx and Apache tolerate this: they treat an empty request-target as / (the root).
 * - Some stricter HTTP parsers will return 400 Bad Request because the request-target is required by the spec (RFC 9110).  YUXIN WANTS TO FOLLOW THIS
    // yuxin need to reconsider
 */
class HttpRequest{
private:
    std::string                             _method;                ///< HTTP method (GET, POST, DELETE)
    std::string                             _requestPath;           ///< URI path extracted from the request line
    std::string                             _version;               ///< HTTP version string (e.g., "HTTP/1.1")
    std::map<std::string, std::string>      _requestHeaders;        ///< Key-value map of request headers
    std::string                             _body;                  ///< Optional message body (POST, some PUT, etc.)
public:
    // --------------------
    //        Getters
    // --------------------
    const std::string&                         getMethod() const    { return _method;}
    const std::string&                         getPath() const      { return _requestPath;}
    const std::string&                         getVersion() const   { return _version;}
    const std::map<std::string, std::string>&  getHeaders() const   { return _requestHeaders;}
    const std::string&                         getBody() const      { return _body;}

    // --------------------
    //        Setters
    // --------------------
    void    setMethod(const std::string &m)                         { _method = m; }
    void    setPath(const std::string &m)                           { _requestPath = m; }
    void    setVersion(const std::string &m)                        { _version = m; }
    void    addHeader(const std::string& k, const std::string& v)   { _requestHeaders[k] = v; }
    void    setBody(const std::string &m)                           { _body = m; }
};

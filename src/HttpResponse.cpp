#include "HttpResponse.hpp"

/**
 * @brief constructor for HttpResponse class
 *
 * @param strs and map of strings
 * @return an HttpResponse object
 *
 * @note initializes all member variables of the HttpResponse class
 */
HttpResponse::HttpResponse(const std::string& version, const int& status, const std::string& reason, const std::string& body, const std::map<std::string, std::string>& responseHeaders, bool alive, bool complete)
{
    _version = version;
    _status = status;
    _reason = reason;
    _body = body;
    _responseHeaders = responseHeaders;
    _keepConnectionAlive = alive;
    _requestComplete = complete;
}

/**
 * @brief builds the HTTP response string
 *
 * @param void
 * @return a string representing the full HTTP response
 *
 * @note constructs the response string including status line, headers, and body
 */
std::string HttpResponse::buildResponseString(){
    std::ostringstream response;  //out string stream

    response << _version << " " << _status << " " << _reason << "\r\n";
    
    // Headers
    bool hasContentLength = false;  // check Content-Length exit
    // iterates though the _responseHeaders map container
    for (const auto &header : _responseHeaders) {
        if (header.first == "Content-Length")
            hasContentLength = true;
        response << header.first << ": " << header.second << "\r\n";
    }
    if (!hasContentLength) {
        response << "Content-Length: " << _body.size() << "\r\n";
    }
    // if not set be the user, by default, the connection is closed
    if (_responseHeaders.find("Connection") == _responseHeaders.end()) {
        response << "Connection: close\r\n";
    }
    response << "\r\n";
    response << _body;

    return response.str();
}
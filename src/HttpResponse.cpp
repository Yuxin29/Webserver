#include "HttpResponse.hpp"

// --------------------
//   Serialization
// --------------------
/**
 * @brief builds the HTTP response string
 *
 * @param void
 * @return a string representing the full HTTP response
 *
 * @note constructs the response string including status line, headers, and body
 */
std::string HttpResponse::buildResponseString(){
    std::ostringstream response;

    response << _version << " " << _status << " " << _reason << "\r\n";

    bool hasContentLength = false;
    // iterates though the _responseHeaders map container and check if there is Content-Length
    for (const auto &header : _responseHeaders) {
        if (header.first == "Content-Length")
            hasContentLength = true;
        response << header.first << ": " << header.second << "\r\n";
    }
    if (!hasContentLength) {
        response << "Content-Length: " << _body.size() << "\r\n";
    }
    // Add Connection header based on _keepConnectionAlive flag
    if (_responseHeaders.find("Connection") == _responseHeaders.end()) {
        if (_keepConnectionAlive)
            response << "Connection: keep-alive\r\n";
        else
            response << "Connection: close\r\n";
    }
    response << "\r\n";
    response << _body;
    return response.str();
}

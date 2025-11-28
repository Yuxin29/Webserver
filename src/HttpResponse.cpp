#include "HttpResponse.hpp"

// std::string                         _version;
// int                                 _status;
// std::string                         _reason;
// std::string                         _body;
// std::map<std::string, std::string>  _responseHeaders;

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

#include "HttpResponseHandler.hpp" 

// this is the public over all call
HttpResponse HttpResponseHandler::handleRequest(const HttpRequest& req)
{
    if (req.getMethod() == "GET")
        return handleGET(req);
    else if (req.getMethod() == "POST")
        return handlePOST(req);
    else if (req.getMethod() == "DELETE")
        return handleDELETE(req);
    return HttpResponse("HTTP/1.1", 405, "Method Not Allowed", "", {});
}


// HttpResponse::HttpResponse(const std::string& version, const int& status, const std::string& reason, const std::string& body, const std::map<std::string, std::string>& responseHeaders)
// {
//     _version = version;
//     _status = status;
//     _reason = reason;
//     _body = body;
//     _responseHeaders = responseHeaders;
// }

// Example of a HttpResponse
// HTTP/1.1 <status_code> <reason_phrase>\r\n
// Header1: value1\r\n
// Header2: value2\r\n
// ...
// \r\n
// <body>

HttpResponse HttpResponseHandler::handleGET(const HttpRequest& /*req*/){
    return HttpResponse("HTTP/1.1", 111, "get_test", "GET", std::map<std::string, std::string>());
}

HttpResponse HttpResponseHandler::handlePOST(const HttpRequest& /*req*/){
    return HttpResponse("HTTP/1.1", 222, "post_test", "POST", std::map<std::string, std::string>());
}

HttpResponse HttpResponseHandler::handleDELETE(const HttpRequest& /*req*/){
    return HttpResponse("HTTP/1.1", 333, "delete_test", "DELETE", std::map<std::string, std::string>());
}

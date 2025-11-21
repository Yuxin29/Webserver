
#include "HttpResponseHandler.hpp" 
#include "HttpUtils.hpp"

// this is the public over all call
// for LUCIO to use
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

/*
------------------------------ Http request example of GET ------------------------------
GET /hello HTTP/1.1
Host: example.com
User-Agent: curl/7.81.0
Accept: text/plain

Http request example of GET
HTTP/1.1 200 OK
Date: Thu, 21 Nov 2025 10:05:00 GMT
Server: ExampleServer/1.0
Content-Type: text/plain
Content-Length: 13

Hello, world!
------------------------------ What happens here ------------------------------
1. Server received GET /hello. /hello is supposed to be file
2. Mapped /hello → filesystem path (e.g., /var/www/html/hello).
3. Checked if the file exists, is readable, and is a regular file.
    exits(), is_regular_file, access(R_OK)
4. Determined MIME type (text/plain for .txt or plain text).
5. Got file size (13) → set Content-Length.
6. Filled headers like Date and Server.
7. Read file content → sent as response body.
*/
HttpResponse HttpResponseHandler::handleGET(const HttpRequest& /*req*/){
    return HttpResponse("HTTP/1.1", 111, "get_test", "GET", std::map<std::string, std::string>());
}

HttpResponse HttpResponseHandler::handlePOST(const HttpRequest& /*req*/){
    return HttpResponse("HTTP/1.1", 222, "post_test", "POST", std::map<std::string, std::string>());
}

HttpResponse HttpResponseHandler::handleDELETE(const HttpRequest& /*req*/){
    return HttpResponse("HTTP/1.1", 333, "delete_test", "DELETE", std::map<std::string, std::string>());
}

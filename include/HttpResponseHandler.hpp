#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <string>

class HttpResponseHandler {
public:
    HttpResponse handleRequest(const HttpRequest& req);

private:
    HttpResponse handleGET(const HttpRequest& req);
    HttpResponse handlePOST(const HttpRequest& req);
    HttpResponse handleDELETE(const HttpRequest& req);
};
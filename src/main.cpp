#include "Server.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponse.hpp"

//testing for yuxin request part, not the real main, can be commented if anyone is merging to main
int main() {
    std::cout << " ------------------------ testing http request ----------------------" << std::endl;
    HttpParser parser;

    // Simulate receiving HTTP request in multiple broken chunks, 
    // std::string chunk1 = "GET /index.html HTTP/1.1\r\nHost: loca";
    // std::string chunk2 = "HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl\r\nContent-Length: 11\r\n\r\n";
    // std::string chunk3 = "Hello World!";

    // Simulate receiving HTTP request in multiple clean and full chunks
    std::string chunk1 = "GET /index.html HTTP/1.1\r\n";
    std::string chunk2 = "Host: localhost\r\n";
    std::string chunk3 = "\r\n";

    // First chunk
    HttpRequest req1 = parser.parseHttpRequest(chunk1);
    if (parser._state == DONE)
        std::cout << "Request fully parsed after chunk1 (unexpected!)\n";
    else
        std::cout << "Parser waiting for more data...\n";
    std::cout << "--------------" << std::endl;

    // Second chunk
    HttpRequest req2 = parser.parseHttpRequest(chunk2);
    if (parser._state == DONE)
        std::cout << "Request fully parsed after chunk1 (unexpected!)\n";
    else
        std::cout << "Parser waiting for more data...\n";
    std::cout << "--------------" << std::endl;

    // Third chunk (body continuation)
    HttpRequest req3 = parser.parseHttpRequest(chunk3);
    if (parser._state != DONE)
        std::cout << "Parser still waiting for body...\n";
    else {
        std::cout << "Request parsed after chunk3!\n";
        std::cout << "Method: " << req3.getMethod() << "\n";
        std::cout << "Path: " << req3.getrequestPath() << "\n";
        std::cout << "Version: " << req3.getVersion() << "\n";
        std::cout << "Headers:\n";
        for (auto &h : req3.getrequestHeaders()) {
            std::cout << "  " << h.first << ": " << h.second << "\n";
        }
        std::cout << "Body: " << req3.getBody() << "\n";
    }
    std::cout << "--------------" << std::endl;

    std::cout << " ------------------------ testing http request ----------------------" << std::endl; 
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "text/plain";

    HttpResponse res {
        "HTTP/1.1",   // _version
        200,        // _status
        "OK",         // _reason
        "Hello World\n",  // _body
        headers
    };

    std::cout << res.buildResponse() << std::endl;
    return 0;
}
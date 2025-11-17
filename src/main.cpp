#include "Server.hpp"
#include "Request.hpp"

//testing for yuxin request part, not the real main, can be commented if anyone is merging to main
int main() {
    HttpParser parser;

    // Simulate receiving HTTP request in multiple chunks
    std::string chunk1 = "GET /index.html HTTP/1.1\r\nHost: loca";
    std::string chunk2 = "lhost\r\nUser-Agent: curl\r\n\r\nHello ";
    std::string chunk3 = "World!";

    // First chunk
    HttpRequest req1 = parser.parseHttpRequest(chunk1);
    if (parser._state == DONE) {
        std::cout << "Request fully parsed after chunk1 (unexpected!)\n";
    } else {
        std::cout << "Parser waiting for more data...\n";
    }
    std::cout << "--------------" << std::endl;

    // Second chunk
    HttpRequest req2 = parser.parseHttpRequest(chunk2);
    if (parser._state != DONE) {
        std::cout << "Parser still waiting for body...\n";
    } else {
        std::cout << "Request parsed after chunk2!\n";
        std::cout << "Method: " << req2.getMethod() << "\n";
        std::cout << "Path: " << req2.getrequestPath() << "\n";
        std::cout << "Version: " << req2.getVersion() << "\n";
        std::cout << "Headers:\n";
        for (auto &h : req2.getrequestHeaders()) {
            std::cout << "  " << h.first << ": " << h.second << "\n";
        }
        std::cout << "Body: " << req2.getBody() << "\n";
    }
    std::cout << "--------------" << std::endl;

    // Third chunk (body continuation)
    HttpRequest req3 = parser.parseHttpRequest(chunk3);
    if (parser._state != DONE) {
        std::cout << "Parser still waiting for body...\n";
    } else {
        std::cout << "Request parsed after chunk2!\n";
        std::cout << "Method: " << req2.getMethod() << "\n";
        std::cout << "Path: " << req2.getrequestPath() << "\n";
        std::cout << "Version: " << req2.getVersion() << "\n";
        std::cout << "Headers:\n";
        for (auto &h : req2.getrequestHeaders()) {
            std::cout << "  " << h.first << ": " << h.second << "\n";
        }
        std::cout << "Body: " << req2.getBody() << "\n";
    }
    std::cout << "--------------" << std::endl;

    return 0;
}
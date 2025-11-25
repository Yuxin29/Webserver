//Lucio Part
#include "Webserver.hpp"
#include "Config.hpp"
#include "utils.hpp"

//Yuxin Part
#include "Server.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponse.hpp"
#include "HttpResponseHandler.hpp"

// //Lucio Main
// int main(int argc, char **argv){
// 	try {
// 		Configuration configFile;
// 		if (argc == 1){
// 			if (!configFile.load(utils::DEFAULT_CONFIG_PATH)){
// 				return utils::returnErrorMessage(utils::FAILED_TO_LOAD_DEFAULT_PATH);
// 			}
// 		} 
// 		else if (argc == 2){
// 			(void)argv;
// 			/* Maybe a:
// 				validateFile()
// 				parseFile()
// 				validateDataInFile()
// 			*/
// 		}
// 		else {
// 			return utils::returnErrorMessage(utils::WRONG_ARGUMENTS);
// 		}
// 		Webserver miniNginx;
// 		if (miniNginx.createServers(configFile) == utils::FAILURE){
// 			return utils::returnErrorMessage(utils::FAILED_TO_CREATE_SERVERS);
// 		}
// 		if (miniNginx.runWebserver() == utils::FAILURE){
// 			return utils::returnErrorMessage(utils::ERROR_RUNNING_SERVERS);
// 		}
// 		return utils::SUCCESS;
// 	} catch (const std::exception& e){
// 		std::cerr << "Error: " << e.what() << std::endl;
// 		return utils::FAILURE;
// 	}
// }

// Yuxin Main
int main() {
    std::cout << " ------------------------ testing http request ----------------------" << std::endl;
    HttpParser parser;
    std::cout << parser._state << std::endl;

    // Simulate receiving HTTP request in multiple broken chunks, 
    // std::string chunk1 = "GET /index.html HTTP/1.1\r\nHost: loca";
    // std::string chunk2 = "HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl\r\nContent-Length: 11\r\n\r\n";
    // std::string chunk3 = "Hello World!";

    // Simulate receiving HTTP request in multiple clean and full chunks
    // std::string chunk1 = "GET /index.html HTTP/1.1\r\n";
    // std::string chunk2 = "Host: localhost\r\n";
    // std::string chunk3 = "\r\n";

    // Simulate receiving HTTP request in multiple clean and full chunks
    std::string chunk1 = "";
    std::string chunk2 = "";
    std::string chunk3 = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n DELETE ";

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

    std::cout << " ------------------------ testing http reponse ----------------------" << std::endl; 
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

    std::cout << " ------------------------ testing HttpResponseHandler ----------------------" << std::endl; 
    // Example HTTP requests
    std::map<std::string, std::string> headers2;
    headers2["Host"] = "localhost";

    // GET request
    HttpRequest getReq("GET", "/index.html", "HTTP/1.1", "", headers2);
    HttpResponseHandler handler;
    HttpResponse getRes = handler.handleRequest(getReq);
    std::cout << "GET response version: " << getRes._version << std::endl;
    std::cout << "GET response status: " << getRes._status << std::endl;
    std::cout << "GET response reason: " << getRes._reason << std::endl;
    std::cout << "GET response body: " << getRes._body << std::endl;
    //std::cout << "GET response responseHeaders: " << getRes._responseHeaders << std::endl;
    std::cout << "--------------" << std::endl;
    
    // POST request
    HttpRequest postReq("POST", "/submit", "HTTP/1.1", "name=John&age=30", headers2);
    HttpResponse postRes = handler.handleRequest(postReq);
    std::cout << "POST response version: " << postRes._version << std::endl;
    std::cout << "POST response status: " << postRes._status << std::endl;
    std::cout << "POST response reason: " << postRes._reason << std::endl;
    std::cout << "POST response body: " << postRes._body << std::endl;
    //std::cout << "POST response status: " << postRes._responseHeaders << std::endl;
    std::cout << "--------------" << std::endl;

    // DELETE request
    HttpRequest delReq("DELETE", "/data.txt", "HTTP/1.1", "", headers2);
    HttpResponse delRes = handler.handleRequest(delReq);
    std::cout << "DELETE response version: " << delRes._version << std::endl;
    std::cout << "DELETE response status: " << delRes._status << std::endl;
    std::cout << "DELETE response reason: " << delRes._reason << std::endl;
    std::cout << "DELETE response body: " << delRes._body << std::endl;
    //std::cout << "DELETE response status: " << postRes._responseHeaders << std::endl;
    std::cout << "--------------" << std::endl;

    return 0;
}

//lin main
// #include "ConfigBuilder.hpp"
// #include <iostream>

// using namespace config;

// static void printVector(const std::vector<std::string>& v) {
//     for (size_t i = 0; i < v.size(); i++)
//         std::cout << "    - " << v[i] << "\n";
// }

// static void printErrorPages(const std::map<int, std::string>& m) {
//     for (std::map<int, std::string>::const_iterator it = m.begin();
//          it != m.end(); it++)
//     {
//         std::cout << "    " << it->first << " => " << it->second << "\n";
//     }
// }
// int main() {
//     try {
//         // load config file
//         Parser parser("configuration/webserv.conf");

//         // parse into AST
//         std::vector<ServerNode> servers = parser.parse();
// 		//convert AST to config
// 		std::vector<ServerConfig> cfgs = ConfigBuilder::build(servers);

//         // print results
//          for (size_t i = 0; i < cfgs.size(); i++) {
//             const ServerConfig& s = cfgs[i];
//             std::cout << "=============================\n";
//             std::cout << "        SERVER " << i + 1 << "\n";
//             std::cout << "=============================\n";

//             std::cout << "Host: " << s.host << "\n";
//             std::cout << "Port: " << s.port << "\n";

//             std::cout << "Root: " << s.root << "\n";

//             std::cout << "Index:\n";
//             printVector(s.index);

//             std::cout << "Client Max Body Size: " << s.client_max_body_size << " bytes\n";

//             std::cout << "Server Names:\n";
//             printVector(s.server_names);

//             std::cout << "Error Pages:\n";
//             printErrorPages(s.error_pages);

//             std::cout << "\nLocations:\n";
//             for (size_t j = 0; j < s.locations.size(); j++) {
//                 const LocationConfig& loc = s.locations[j];
//                 std::cout << "\n  --- Location " << j + 1 << " ---\n";
//                 std::cout << "  Path: " << loc.path << "\n";
//                 std::cout << "  Root: " << loc.root << "\n";

//                 std::cout << "  Index:\n";
//                 printVector(loc.index);

//                 std::cout << "  Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
//                 std::cout << "  Redirect: " << loc.redirect << "\n";
//                 std::cout << "  CGI Pass: " << loc.cgi_pass << "\n";
//                 std::cout << "  CGI Ext: " << loc.cgi_ext << "\n";
//                 std::cout << "  Upload Dir: " << loc.upload_dir << "\n";

//                 std::cout << "  Client Max Body Size: "
//                           << loc.client_max_body_size << " bytes\n";

//                 std::cout << "  Methods:\n";
//                 printVector(loc.methods);
//             }

//             std::cout << "\n";
//         }

//     } catch (const std::exception& e) {
//         std::cerr << "Error while parsing config: " << e.what() << "\n";
//         return 1;
//     }
// }


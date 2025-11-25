#include "Http.hpp"

void Http::setName(const std::string& name) {
	_name = name;
}

httpResponse Http::processRequest(const std::string& request, const Configuration::ServerBlock& server) {
	httpResponse response;
	(void)server; // Will use this later for routing
	
	// Simple parsing - extract first line
	std::istringstream stream(request);
	std::string method, path, version;
	stream >> method >> path >> version;
	
	// Check if it's a valid HTTP request
	if (method.empty() || path.empty()) {
		response.statusCode = 400;
		response.responseData = 
			"HTTP/1.1 400 Bad Request\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 45\r\n"
			"Connection: close\r\n"
			"\r\n"
			"<html><body><h1>400 Bad Request</h1></body></html>";
		response.keepConnectionAlive = false;
		return response;
	}
	
	// For testing: return simple 200 OK with request info
	std::ostringstream body;
	body << "<html><body>"
	     << "<h1>Webserver Working!</h1>"
	     << "<p><b>Method:</b> " << method << "</p>"
	     << "<p><b>Path:</b> " << path << "</p>"
	     << "<p><b>Version:</b> " << version << "</p>"
	     << "<p><b>Server Block Port:</b> " << server.port << "</p>"
	     << "</body></html>";
	
	std::string bodyStr = body.str();
	
	// Check Connection header for keep-alive
	bool keepAlive = false;
	if (version == "HTTP/1.1") {
		// HTTP/1.1 defaults to keep-alive
		keepAlive = true;
		// Check if client explicitly wants to close
		if (request.find("Connection: close") != std::string::npos) {
			keepAlive = false;
		}
	} else if (version == "HTTP/1.0") {
		// HTTP/1.0 defaults to close
		keepAlive = false;
		// Check if client explicitly wants keep-alive
		if (request.find("Connection: keep-alive") != std::string::npos) {
			keepAlive = true;
		}
	}
	
	// Build response
	std::ostringstream responseStream;
	responseStream << "HTTP/1.1 200 OK\r\n"
	               << "Content-Type: text/html\r\n"
	               << "Content-Length: " << bodyStr.length() << "\r\n"
	               << "Connection: " << (keepAlive ? "keep-alive" : "close") << "\r\n"
	               << "\r\n"
	               << bodyStr;
	
	response.statusCode = 200;
	response.responseData = responseStream.str();
	response.keepConnectionAlive = keepAlive;
	
	return response;
}

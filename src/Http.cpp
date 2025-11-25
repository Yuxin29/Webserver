#include "Http.hpp"

void Http::setName(const std::string& name) {
	_name = name;
}

httpResponse Http::processRequest(const std::string& request, const ServerConfig& server) {
	httpResponse response;
	response.requestComplete = true;
	
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
	
	// Check for POST body - simple Content-Length check
	if (method == "POST") {
		size_t clPos = request.find("Content-Length:");
		if (clPos != std::string::npos) {
			size_t start = clPos + 15;
			size_t end = request.find("\r\n", start);
			if (end != std::string::npos) {
				int contentLength = std::stoi(request.substr(start, end - start));
				size_t bodyStart = request.find("\r\n\r\n");
				if (bodyStart != std::string::npos) {
					size_t bodyReceived = request.size() - (bodyStart + 4);
					if (bodyReceived < static_cast<size_t>(contentLength)) {
						response.requestComplete = false;
						response.statusCode = 0;
						response.responseData = "";
						response.keepConnectionAlive = false;
						return response;
					}
				}
			}
		}
	}
	
	// Build response body with server info
	std::ostringstream body;
	body << "<html><head><title>Webserver Test</title></head><body>"
	     << "<h1>🚀 Webserver Working!</h1>"
	     << "<h2>Request Info:</h2>"
	     << "<p><b>Method:</b> " << method << "</p>"
	     << "<p><b>Path:</b> " << path << "</p>"
	     << "<p><b>Version:</b> " << version << "</p>"
	     << "<h2>Server Config:</h2>"
	     << "<p><b>Host:</b> " << server.host << ":" << server.port << "</p>";
	
	if (!server.serverNames.empty()) {
		body << "<p><b>Server Names:</b> ";
		for (size_t i = 0; i < server.serverNames.size(); i++) {
			body << server.serverNames[i];
			if (i < server.serverNames.size() - 1) body << ", ";
		}
		body << "</p>";
	}
	
	body << "<p><b>Root:</b> " << server.root << "</p>"
	     << "<p><b>Max Body Size:</b> " << server.clientMaxBodySize << " bytes</p>"
	     << "<p><b>Locations:</b> " << server.locations.size() << "</p>";
	
	body << "</body></html>";
	
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

#pragma once
#include <iostream>

//Placeholder for http class that will process the requests and return responses
class Http{

	private:
		std::string _name;

	public:
		void setName(const std::string& name);

		// maybe needed for Http
    // parseRequest(rawData) → Request object
    // isRequestComplete(rawData) → bool
    
    // // HTTP routing
    // routeRequest(Request, ServerBlock) → response string
    
    // // Request handling
    // serveStaticFile(Request, ServerBlock) → response string
    // handleCGI(Request, ServerBlock) → response string
    // handleDELETE(Request, ServerBlock) → response string
    
    // // Response generation
    // generateResponse(statusCode, headers, body) → string
    // generateErrorResponse(errorCode, ServerBlock) → string
    
    // // HTTP validation
    // validateMethod(method) → bool
    // validateHeaders(headers) → bool

	// std::string Http::processRequest(const std::string& rawData, 
    //                              const ServerBlock& server) {
    // // 1. Parse into Request object (Http's job)
    // Request req = parseRequest(rawData);
    
    // // 2. Find matching location (could use Server::findLocation helper)
    // const LocationBlock* location = findBestMatch(req.getPath(), server);
    
    // // 3. Route based on location config (Http's job)
    // if (location->cgiPass) {
    //     return handleCGI(req, server, *location);
    // } else {
    //     return serveStaticFile(req, server, *location);
    // }
	// }
};
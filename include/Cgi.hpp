#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include "ConfigBuilder.hpp"
#include "HttpRequest.hpp"

class CGI
{
private:
	//from server.locConfig
	std::string _cgiPass;
	std::string _cgiExt;

	//request
	std::string _scriptPath;  //real CGI path
	std::string _method;      //GET/POST
	std::string _query;       //QUERY_STRING
	std::string _body;        //POST body
	std::map<std::string, std::string> _header;
	std::string _contentType;
	std::string _serverName;

public:
	CGI(const HttpRequest& req, const config::LocationConfig& lc);
	bool isCGI()const;
	std::string execute();
};

#endif

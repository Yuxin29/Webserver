#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include "ConfigBuilder.hpp"
#include "HttpRequest.hpp"

class CGI
{
private:
	std::string 						_cgiPass;
	std::string							_cgiExt;
	std::string 						_scriptPath;
	std::string 						_method; 
	std::string 						_query;       
	std::string 						_body;
	std::map<std::string, std::string> 	_header;
	std::string 						_contentType;
	std::string 						_serverName;

public:
	CGI(const HttpRequest& req, const config::LocationConfig& lc);
	bool isAllowedCgi()const;
	std::string execute();
};

#endif
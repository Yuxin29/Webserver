#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include "ConfigBuilder.hpp"

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

public:
	CGI();
	bool isCGI()const;
};

#endif

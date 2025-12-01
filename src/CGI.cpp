#include "Cgi.hpp"

CGI::CGI(const HttpRequest& req, const config::LocationConfig& lc)
:_cgiPass(lc.cgi_pass),
_cgiExt(lc.cgiExt),
_method(req.getMethod()),
 _query("")
{
	std::string raw = req.getrequestPath();
	size_t pos = raw.find('?');
	if(pos != std::string::npos){
		_scriptPath = lc.root + raw.substr(0, pos);
		_query = raw.substr(pos + 1);
	}
	else{
		_scriptPath = lc.root + raw;
		_query = "";
	}
}

bool CGI::isCGI()const
{

}

std::string CGI::execute()
{

}

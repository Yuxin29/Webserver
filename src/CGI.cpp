#include "Cgi.hpp"

//if the whole http request is: GET /cgi-bin/test.py?name=Bob HTTP/1.1
//scriptpath = root +/cgi-bin/test.py
//query_path = name=Bob
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
	if (_cgiPass.empty())
		return false;
	if (_method != "GET" && _method != "POST")
		return false;
	//like: _scriptPath = "./sites/cgi/test.py"; _cgiExt = ".py";
	if (!_scriptPath.endwith(_cgiExt))
		return false;
	//exist and readable?
	if(access(_scriptPath.c_str(), R_OK) != 0)
		return false;
	return true;
}

/*
pipe(stdin), for writing POST body
pipe(stdout)
fork()
child: dup2 → execve()
parent: write → read → parse
*/
std::string CGI::execute()
{

}

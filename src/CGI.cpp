#include "Cgi.hpp"
#include <unistd.h> //pipe, dup2, fork, execve, access
#include <string>
#include <sys/wait.h>
#include <cstring> //strerror
#include "HttpRequest.hpp"
#include <iostream>

//if the whole http request is: GET /cgi-bin/test.py?name=Bob HTTP/1.1
//scriptpath = root +/cgi-bin/test.py
//query_path = name=Bob
CGI::CGI(const HttpRequest& req, const config::LocationConfig& lc)
:_cgiPass(lc.cgiPass),
_cgiExt(lc.cgiExt),
_method(req.getMethod()),
 _query(""),
 _body(req.getBody())
{
	std::string raw = req.getPath();
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
	// if (!_scriptPath.ends_with(_cgiExt))
	// 	return false;
	//exist and readable?
	if(access(_scriptPath.c_str(), R_OK) != 0)
		return false;
	return true;
}

/*
pipe(stdin), for writing POST body
pipe(stdout)
fork()
child: dup2 → execve(cgi_pass, argv, envp)
argv = [ _cgiPass, _scriptPath ]
envp = [
  "REQUEST_METHOD=GET",
  "QUERY_STRING=xxx",
  NULL
]
but these types are just std::string, we should use char*[] in the execve()
parent: write → read → parse
close childs pipe ends, write body (empty for GET), read stdout into string, waitpid()

CGI environment is NOT OS environment.
OS environment (like PATH, HOME) is different.
CGI requires HTTP metadata:
Request method
Query string
Body size
Script path
Protocol
None of these exist in the OS environment.

*/
std::string CGI::execute()
{
	// int	stdin_pipe[2];
	// int stdout_pipe[2];

	// if(pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0){
	// 	std::cerr << "[CGI] pipe() failed: " << strerror(errno) << std::endl;
	// 	return "";
	// }
	// pid_t pid = fork();
	// if(pid < 0){
	// 	std::cerr << "[CGI] fork() failed: " << strerror(errno) << std::endl;
	// 	_exit(42); //confused about code??
	// }
	// if(pid == 0){
	// 	if(dup2(stdin_pipe[0], STDIN_FILENO) < 0 || dup2(stdout_pipe[1], STDOUT_FILENO)){
	// 		std::cerr << "[CGI] dup2() failed: " << strerror(errno) << std::endl;
	// 		return "";
	// 	}
	// 	// close all pipe ends not used
	// 	close(stdin_pipe[1]);
	// 	close(stdout_pipe[0]);
	// 	// execve(cgiPass, argv, envp)
	// 	std::vector<std::string> envStrings;
	// 	std::vector<char*> env;
	// 	envStrings.push_back("REQUEST_METHOD="+_method);
	// 	envStrings.push_back("QUERY_STRING="+_query);
	// 	//...
	// 	for(auto& s : envStrings)
	// 		env.push_back(const_cast<char *>(s.c_str()));
	// 	env.push_back(NULL);
	// 	std::vector<char*> argv;
	// 	argv.push_back(const_cast<char *>(_cgiPass.c_str()));
	// 	argv.push_back(const_cast<char *>(_scriptPath.c_str()));
	// 	argv.push_back(NULL);
	// 	execve(_cgiPass.c_str(), argv.data(), env.data()); //argv.data() returns: char** (pointer to first element of vector)
	// 	std::cerr << "[CGI] execve failed: " << strerror(errno) << std::endl;
	// 	_exit(42); //confused about code??
	// }
	// else{
	// 	close(stdin_pipe[0]);
	// 	close(stdout_pipe[1]);
	// 	//Write POST body (GET writes nothing)
	// 	//Step 3.3 — CLOSE stdin write-end (CRITICAL)
	// 	//Read CGI output from stdout_pipe[0]
	// 	//Wait for the child process
	// 	//return
	// }
	return nullptr;
}

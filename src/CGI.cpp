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
_body(req.getBody()),
_header(req.getHeaders()),
_contentType(""),
_serverName("")
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
	//count() and at() come from std::map
	if (_header.count("Content-Type")) //count can check if Content-Type exits in map
		_contentType = _header.at("Content-Type"); //retrieves the value for the Content-type
	if (_header.count("Host"))
		_serverName = _header.at("Host");
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
	int	stdin_pipe[2];
	int stdout_pipe[2];

	if(pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0){
		std::cerr << "[CGI] pipe() failed: " << strerror(errno) << std::endl;
		return "";
	}
	pid_t pid = fork();
	if(pid < 0){
		std::cerr << "[CGI] fork() failed: " << strerror(errno) << std::endl;
		return "";
	}
	if(pid == 0){
		if(dup2(stdin_pipe[0], STDIN_FILENO) < 0 || dup2(stdout_pipe[1], STDOUT_FILENO) < 0){
			std::cerr << "[CGI] dup2() failed: " << strerror(errno) << std::endl;
			_exit(42);
		}
		// close all pipe ends not used
		close(stdin_pipe[1]);
		close(stdout_pipe[0]);
		// execve(cgiPass, argv, envp)
		std::vector<std::string> envStrings;
		std::vector<char*> env;
		envStrings.push_back("REQUEST_METHOD="+_method);
		envStrings.push_back("QUERY_STRING="+_query);
		envStrings.push_back("CONTENT_LENGTH="+std::to_string(_body.size()));
		envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");
		envStrings.push_back("SCRIPT_FILENAME="+_scriptPath);
		envStrings.push_back("PATH_INFO="+_scriptPath);
		envStrings.push_back("CONTENT_TYPE="+_contentType);
		envStrings.push_back("SERVER_NAME="+_serverName);
		for(auto& s : envStrings)
			env.push_back(const_cast<char *>(s.c_str()));
		env.push_back(NULL);
		std::vector<char*> argv;
		argv.push_back(const_cast<char *>(_cgiPass.c_str()));
		argv.push_back(const_cast<char *>(_scriptPath.c_str()));
		argv.push_back(NULL);
		execve(_cgiPass.c_str(), argv.data(), env.data()); //argv.data() returns: char** (pointer to first element of vector)
		std::cerr << "[CGI] execve failed: " << strerror(errno) << std::endl;
		_exit(42);
	}
	else{
		close(stdout_pipe[1]);
		close(stdin_pipe[0]);
		//Write POST body (GET writes nothing)
		if(_method=="POST" && !_body.empty()){
			if (write(stdin_pipe[1], _body.c_str(), _body.size()) < 0)
				std::cerr << "[CGI] write() failed"<<std::endl;
		}
		close(stdin_pipe[1]);
		//Read CGI output from stdout_pipe[0]
		char buffer[4096];
		std::string output;
		ssize_t bytes; //should use ssize_t, If read returns -1, this becomes a large integer
		while((bytes = read(stdout_pipe[0], buffer, sizeof(buffer))) > 0) //one read can not get everything.
			output.append(buffer, bytes);
		close(stdout_pipe[0]);
		//Wait for the child process
		int status;
		waitpid(pid, &status, 0);
		if(WIFEXITED(status) && WEXITSTATUS(status) == 42){
			std::cerr << "[CGI] child process failed"<<std::endl;
			return "";
		}
		return output;
	}
}
#include "CGI.hpp"
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
    std::string root = lc.root;
    if (root.ends_with("/"))
        root.erase(root.size() - 1);
    std::string raw = req.getPath();
    size_t pos = raw.find('?');
    std::string locationUri = lc.path;
    if(root.find(locationUri) != std::string::npos)
        root = "./sites/cgi" ;

    if(pos != std::string::npos){
        _scriptPath = root + raw.substr(0, pos);
        _query = raw.substr(pos + 1);
    }
    else{
        _scriptPath = root + raw;
        _query = "";
    }
    //count() and at() come from std::map
    if (_header.count("content-type")) //count can check if Content-Type exits in map
        _contentType = _header.at("content-type"); //retrieves the value for the Content-type
    if (_header.count("host"))
        _serverName = _header.at("host");
}

bool CGI::isAllowedCgi()const
{
	if (_cgiPass.empty())
		return false;
	if (_method != "GET" && _method != "POST")
		return false;
	//like: _scriptPath = "./sites/cgi/test.py"; _cgiExt = ".py";
	if (!_scriptPath.ends_with(_cgiExt))
		return false;
	//exist and readable?
	//MAYBE NEED TO CHECK THIS SOMEWHERE ELSE 17/12 Lucio's note
	// if(access(_scriptPath.c_str(), R_OK) != 0)
	// 	return false;
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
		
		// Close all inherited file descriptors (listening sockets, epoll, etc.)
		// This prevents the child from holding ports/connections
		for(int fd = 3; fd < 1024; fd++){
			if(fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO)
				close(fd);
		}
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
		envStrings.push_back("REDIRECT_STATUS=200");  // Required for PHP-CGI security
		for(auto& s : envStrings)
			env.push_back(const_cast<char *>(s.c_str()));
		env.push_back(NULL);
		std::vector<char*> argv;
		argv.push_back(const_cast<char *>(_cgiPass.c_str()));
		argv.push_back(const_cast<char *>(_scriptPath.c_str()));
		argv.push_back(NULL);
		execve(_cgiPass.c_str(), argv.data(), env.data());
		std::cerr << "[CGI] execve failed: " << strerror(errno) << std::endl;
		_exit(42);
	}
	else{
		close(stdout_pipe[1]);
		close(stdin_pipe[0]);
		//Write POST body (GET writes nothing)
		if(_method=="POST" && !_body.empty()){
			size_t total = 0;
			size_t len = _body.size();
			while(total < len)
			{
				ssize_t written = write(stdin_pipe[1], _body.c_str() + total, len - total);
				if(written < 0)
				{
					std::cerr << "[CGI] write() failed\n";
					break;
				}
				total += written;
			}
		}
		close(stdin_pipe[1]);
		//Read CGI output from stdout_pipe[0]
		char buffer[4096];
		std::string output;
		ssize_t bytes;
		while(true)
		{
			bytes = read(stdout_pipe[0], buffer, sizeof(buffer));
			if(bytes > 0)
				output.append(buffer, bytes);
			else if(bytes == 0)
				break; //EOF
			else
			{
				std::cerr << "[CGI] read() failed\n";
				break;
			}
		}
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

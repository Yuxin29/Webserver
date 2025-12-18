#include "CGI.hpp"
#include <unistd.h>
#include <string>
#include <sys/wait.h>
#include <cstring>
#include "HttpRequest.hpp"
#include <iostream>

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
    if (_header.count("content-type"))
        _contentType = _header.at("content-type");
    if (_header.count("host"))
        _serverName = _header.at("host");
}

bool CGI::isAllowedCgi()const
{
	if (_cgiPass.empty())
		return false;
	if (_method != "GET" && _method != "POST")
		return false;
	if (!_scriptPath.ends_with(_cgiExt))
		return false;
	return true;
}

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
		close(stdin_pipe[1]);
		close(stdout_pipe[0]);
		for(int fd = 3; fd < 1024; fd++){
			if(fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO)
				close(fd);
		}
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
		envStrings.push_back("REDIRECT_STATUS=200");
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
		char buffer[4096];
		std::string output;
		ssize_t bytes;
		while(true)
		{
			bytes = read(stdout_pipe[0], buffer, sizeof(buffer));
			if(bytes > 0)
				output.append(buffer, bytes);
			else if (bytes == 0)
				break;
			else
			{
				std::cerr << "[CGI] read() failed\n";
				break;
			}
		}
		close(stdout_pipe[0]);
		int status;
		waitpid(pid, &status, 0);
		if(WIFEXITED(status) && WEXITSTATUS(status) == 42){
			std::cerr << "[CGI] child process failed"<<std::endl;
			return "";
		}
		return output;
	}
}

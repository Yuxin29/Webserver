#pragma once
#include "Server.hpp"
#include "Http.hpp"
#include "Request.hpp"
#include "CGI.hpp"
#include "Config.hpp"
#include <poll.h>
#include <signal.h>
#include <iostream>

// General exit codes
#define SUCCESS 0
#define FAILURE 1

// macros for error codes 
enum error_macros{
		WRONG_ARGUMENTS = 1,
		FAILED_TO_START_SERVER
};

int exitWithError(int errorCode);

class Webserver {
	
	private:
		std::vector<Server> servers;

	public:
		int startServers();
		void run();
		void stopServers();
};


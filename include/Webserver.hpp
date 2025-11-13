#pragma once
#include "Server.hpp"
#include "Http.hpp"
#include "Request.hpp"
#include "CGI.hpp"
#include "Config.hpp"
#include "utils.hpp"
#include <poll.h>
#include <signal.h>
#include <iostream>
#include <vector>

class Webserver {
	
	private:
		std::vector<Server> _servers;

		void handleSignal();

	public:
		explicit Webserver();
		Webserver(const Webserver& other) = delete;
		Webserver& operator=(const Webserver& other) = delete;
		~Webserver();

		int createServers(const Configuration& file);
		int runServers();
		void stopServers();
};


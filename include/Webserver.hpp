#pragma once
#include "Server.hpp"
#include "Config.hpp"
#include "utils.hpp"
#include <map>
#include <poll.h>
#include <signal.h>
#include <iostream>
#include <vector>

class Webserver {
	
	private:
		std::vector<pollfd>		_pollFds;
		std::vector<Server> 	_servers;
		std::map<int, size_t> 	_listenFdToServerIndex;

		void handleSignal();

	public:
		explicit Webserver();
		Webserver(const Webserver& other) = delete;
		Webserver& operator=(const Webserver& other) = delete;
		~Webserver();

		int createServers(const Configuration& config);
		int runWebserver();
		void stopWebserver();
};


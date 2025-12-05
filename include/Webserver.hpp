#pragma once
#include "Server.hpp"
#include "ConfigBuilder.hpp"
#include "utils.hpp"
#include <map>
#include <sys/epoll.h>
#include <csignal>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

class Webserver {
	private:
	    static constexpr int CONNECTION_TIMEOUT = 10;

		int 					_epollFd;
		std::vector<Server> 	_servers;
		std::map<int, size_t> 	_listenFdToServerIndex;
		std::map<int, size_t>	_clientFdToServerIndex;
		bool					_running;
		std::map<int, time_t>	_lastActivity;

		bool isListeningSocket(int fd) const;
		void handleNewConnection(int listenFd);
		void handleClientRequest(int clientFd);
		void checkIdleConnections();
		void addClientToPoll(int clientFd, size_t serverIndex);
		void removeFdFromPoll(int fd);
		void removeClientFd(int clientFd);
		void closeAllClients(void);
		bool hasError(const epoll_event& event) const;

	public:
		explicit Webserver();
		Webserver(const Webserver& other) = delete;
		Webserver& operator=(const Webserver& other) = delete;
		~Webserver();

		int  createServers(const std::vector<config::ServerConfig>& config);
		int  runWebserver(void);
		void stopWebserver(void);
};

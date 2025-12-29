#pragma once
#include "Server.hpp"
#include "ConfigBuilder.hpp"
#include "utils.hpp"
#include <sys/epoll.h>
#include <csignal>
#include <vector>
#include <cstring>



/**
 * @brief 
 * 
 * 
 * 
 * 
 */
class Webserver {
	private:
	    static constexpr int CONNECTION_TIMEOUT = 60;

		int 					_epollFd;
		std::vector<Server> 	_servers;
		std::map<int, size_t> 	_listenFdToServerIndex;
		std::map<int, size_t>	_clientFdToServerIndex;
		bool					_running;
		std::map<int, time_t>	_lastActivity;

		bool isListeningSocket(int fd) const;
		void handleNewConnection(int listenFd);
		void handleClientRequest(int clientFd);
		void handleClientWrite(int clientFd);
		void modifyClientEvents(int clientFd, uint32_t events);
		void checkIdleConnections();
		void addClientToPoll(int clientFd, size_t serverIndex);
		void removeFdFromPoll(int fd);
		void removeClientFd(int clientFd);
		void closeAllClients(void);
		bool hasError(const epoll_event& event) const;
		void sendTimeoutResponse(int clientFd);
		bool isFdWriting(int clientFd) const;

	public:
		explicit Webserver();
		Webserver(const Webserver& other) = delete;
		Webserver& operator=(const Webserver& other) = delete;
		~Webserver();

		int  createServers(const std::vector<config::ServerConfig>& config);
		int  runWebserver(void);
		void stopWebserver(void);
};

#pragma once

#include "Server.hpp"
#include "utils.hpp"

#include <sys/epoll.h>
#include <csignal>

// using namespace config;
using namespace utils;

/**
 * @class Webserver
 * @brief Central event-driven web server controller.
 *
 * The Webserver class is responsible for managing the global
 * event loop using epoll, coordinating multiple Server instances,
 * handling client connections, routing events, and enforcing
 * connection timeouts.
 *
 * It acts as the top-level orchestrator of the application:
 * - Initializes and runs multiple servers
 * - Dispatches epoll events
 * - Manages client lifecycles
 * - Handles idle connection cleanup
 */
class Webserver {
	private:
	    static constexpr int CONNECTION_TIMEOUT = 60;		//Maximum allowed idle time for a client connection (in seconds).

		int 					_epollFd;					// epoll instance file descriptor 
		bool					_running;					//
		std::vector<Server> 	_servers;					// All running Server instances 
		std::map<int, size_t> 	_listenFdToServerIndex;		// Maps listening socket fd to its Server index.
		std::map<int, size_t>	_clientFdToServerIndex;		// Maps client socket fd to its Server index.
		std::map<int, time_t>	_lastActivity;				// Tracks the last activity time for each client fd.

		//epoll and event handleing
		bool isListeningSocket(int fd) const;
		bool hasError(const epoll_event& event) const;
		bool isFdWriting(int clientFd) const;

		// new contectiong
		void handleNewConnection(int listenFd);

		// client reading and writing
		void handleClientRequest(int clientFd);
		void handleClientWrite(int clientFd);

		//epoll event mofifying
		void modifyClientEvents(int clientFd, uint32_t events);

		//timeout
		void checkIdleConnections();
		void sendTimeoutResponse(int clientFd);

		//fd management and cleanign up
		void addClientToPoll(int clientFd, size_t serverIndex);
		void removeFdFromPoll(int fd);
		void removeClientFd(int clientFd);
		void closeAllClients(void);
	

	public:
		//constructors
		explicit Webserver();
		Webserver(const Webserver& other) = delete;
		Webserver& operator=(const Webserver& other) = delete;
		~Webserver();

		// create server instances, runs main event loop, stops the websier and exits
		int  createServers(const std::vector<config::ServerConfig>& config);
		int  runWebserver(void);
		void stopWebserver(void);
};

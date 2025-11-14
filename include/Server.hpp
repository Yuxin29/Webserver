#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "Config.hpp"
#include "Request.hpp"
#include "Http.hpp"

// Server can only be constructed by passing a configuration
class Server{

	public:
		enum StartResult{
			START_SUCCESS,
			START_SOCKET_ERROR,
			START_BIND_ERROR,
			START_LISTEN_ERROR
		};

	private:
		int							_listenFd;
		Configuration::ServerBlock	_servConfig;
		struct sockaddr_in			_addr;

	public:
		Server() = delete;
		explicit Server(const Configuration::ServerBlock& serverBlock);
		Server(const Server& other);
		Server& operator=(const Server& other) = delete;
		~Server();

		// Setup and cleanup
		StartResult start();
		void shutdown();

		// Return the new client fd, or -1 on error
		int acceptConnection();

		// Process a client request
		void handleClient(int fd);

		// Getters
		int getListenFd() const;
		int getPort() const;
};
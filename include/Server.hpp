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

class Server {
	public:
		enum StartResult{
			START_SUCCESS,
			START_SOCKET_ERROR,
			START_BIND_ERROR,
			START_LISTEN_ERROR
		};

	private:
		std::string _host;
		int			_listenFd;
		int  		_port;
		std::vector<Configuration::ServerBlock>	_virtualHosts;
		sockaddr_in	_addr;
		Http		_httpHandler;

		const Configuration::ServerBlock* matchVirtualHost(const std::string& host);
		std::string extractHostHeader(const std::string& rawRequest) const;
		const Configuration::ServerBlock::LocationBlock*
			findLocation(const Configuration::ServerBlock& server,
						const std::string& path) const;
		bool shouldKeepAlive(const std::string& rawRequest) const;
		
	public:
		Server() = delete;
		explicit Server(const std::string& host, int port, const std::vector<Configuration::ServerBlock>& serverBlocks);
		Server(const Server& other) = delete;
		Server& operator=(const Server& other) = delete;
		~Server();

		StartResult start(void);
		void shutdown(void);
		int  acceptConnection(void);
		void handleClient(int fd);
		int  getListenFd(void) const;
		int  getPort(void) const;
};
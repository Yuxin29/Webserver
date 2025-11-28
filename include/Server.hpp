#pragma once
#include <vector>
#include <map>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "ConfigBuilder.hpp"
#include "HttpResponseHandler.hpp"

class Server {
	public:
		enum ClientStatus {
			CLIENT_INCOMPLETE,
			CLIENT_KEEP_ALIVE,
			CLIENT_COMPLETE,
			CLIENT_ERROR
		};

		enum StartResult {
			START_SUCCESS,
			START_SOCKET_ERROR,
			START_BIND_ERROR,
			START_LISTEN_ERROR
		};

	private:
		std::string 				_host;
		int							_listenFd;
		int  						_port;
		std::vector<config::ServerConfig>	_virtualHosts;
		sockaddr_in					_addr;
		//Http						_httpHandler;
		HttpResponseHandler		_httpHandler;
		std::map<int, std::string> _partialRequests;

		const config::ServerConfig* matchVirtualHost(const std::string& hostHeader);
		std::string extractHostHeader(const std::string& rawRequest) const;
		const config::LocationConfig* findLocation(const config::ServerConfig& server, const std::string& path) const;

	public:
		Server() = delete;
		explicit Server(const std::string& host, int port, const std::vector<config::ServerConfig>& serverBlocks);
		Server(const Server& other);
		Server& operator=(const Server& other) = delete;
		~Server();

		StartResult start(void);
		void shutdown(void);
		int  acceptConnection(void);
		ClientStatus handleClient(int clientFd);
		int  getListenFd(void) const;
		int  getPort(void) const;
};
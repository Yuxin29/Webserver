#pragma once
#include <vector>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>

#include "ConfigBuilder.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponseHandler.hpp"

class Server {
	public:
		enum ClientStatus {
			CLIENT_INCOMPLETE,
			CLIENT_KEEP_ALIVE,
			CLIENT_COMPLETE,
			CLIENT_ERROR,
			CLIENT_WRITING
		};

		enum StartResult {
			START_SUCCESS,
			START_SOCKET_ERROR,
			START_BIND_ERROR,
			START_LISTEN_ERROR
		};

		struct WriteBuffer {
			std::string data;
			size_t sent = 0;
			bool keepAlive = false;

			bool isComplete() const;
			size_t remainingToSend() const;
		};

	private:
		static constexpr int MAX_REQUESTS = 20;
		static constexpr int NOT_VALID_FD = -1;

		std::string 				_host;
		int							_listenFd;
		int  						_port;
		std::vector<config::ServerConfig>	_virtualHosts;
		sockaddr_in					_addr;
		std::map<int, int>			_requestCount;
		std::map<int, HttpParser>	_parsers;
		std::map<int, WriteBuffer>	_writeBuffers;
		HttpResponseHandler			_httpHandler;

		const config::ServerConfig* matchVirtualHost(const std::string& hostHeader);
		
	public:
		Server() = delete;
		explicit Server(const std::string& host, int port, const std::vector<config::ServerConfig>& serverBlocks);
		Server(Server&& other) noexcept;
		Server(const Server& other) = delete;
		Server& operator=(const Server& other) = delete;
		~Server();

		StartResult start(void);
		void shutdown(void);
		int  acceptConnection(void);
		ClientStatus handleClient(int clientFd);
		ClientStatus handleClientWrite(int clientFd);
		int  getListenFd(void) const;
		int  getPort(void) const;
		bool hasWriteBuffer(int clientFd) const;
		void cleanMaps(int clientFd);
};

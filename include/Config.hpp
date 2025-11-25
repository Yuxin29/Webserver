#pragma once

#include <map>
#include <vector>
#include <string>
#include <iostream>

//Placeholder class to be obtained from parsed config file
class Configuration{
	
	public:
	// Data Structure for ONE parsed server block
		struct ServerBlock {
			int 						port;
			std::string 				host;
			std::vector<std::string> 	serverNames;
			std::string 				root;
			std::string					index;
			size_t						clientMaxBodySize;
			std::map<int, std::string>	errorPages;

			// Location Block
			struct LocationBlock {
				std::string 			 path;
				std::vector<std::string> allowedMethods;
				bool					 autoindex;
				std::string				 root;
				std::string				 cgiPass;
				std::string				 cgiExit;
			};
			// Vector to save multiple locations in ONE server block
			std::vector<LocationBlock> locations;
		};

	private:
		std::vector<ServerBlock> _serverBlocks;

	public:
		uint16_t getNumberOfServers() const {
			return _serverBlocks.size();
		}

		const ServerBlock& getServerBlock(size_t index) const {
			return _serverBlocks[index];
		}

		void addServerBlock(const ServerBlock& block) {
			_serverBlocks.push_back(block);
		}

		bool load(const std::string& path){
			(void)path;
			
			// Create a minimal test server block
			ServerBlock testServer;
			testServer.port = 8080;
			testServer.host = "0.0.0.0";
			testServer.serverNames.push_back("localhost");
			testServer.serverNames.push_back("127.0.0.1");
			testServer.root = "/var/www/html";
			testServer.index = "index.html";
			testServer.clientMaxBodySize = 1048576; // 1MB
			
			// Add a root location
			ServerBlock::LocationBlock rootLocation;
			rootLocation.path = "/";
			rootLocation.allowedMethods.push_back("GET");
			rootLocation.allowedMethods.push_back("POST");
			rootLocation.autoindex = false;
			rootLocation.root = "/var/www/html";
			testServer.locations.push_back(rootLocation);
			
			_serverBlocks.push_back(testServer);
			
			return true;
		}
};

#pragma once
#include <map>
#include <iostream>

//Placeholder class to be obtained from parsed config file
class Configuration{
	
	public:
	// Data Structure for ONE parsed server block
		struct ServerBlock {
			int 						port;
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
		int getNumberOfServers() const {
			return _serverBlocks.size();
		}

		const ServerBlock& getServerBlock(size_t index) const {
			return _serverBlocks[index];
		}

		void addServerBlock(const ServerBlock& block) {
			_serverBlocks.push_back(block);
		}
};

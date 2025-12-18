#ifndef CONFIGBUILDER_HPP
#define CONFIGBUILDER_HPP

#include "ConfigParser.hpp"
#include <string>
#include <vector>
#include <map>

namespace config
{
	struct LocationConfig
	{
		std::string 			path;
		std::string 			root;
		std::string 			redirect;
		std::vector<std::string>index;
		std::string 			cgiPass;
		std::string 			cgiExt;
		std::string 			upload_dir;
		unsigned long 			clientMaxBodySize;
		bool 					autoindex;
		std::vector<std::string>methods;
	};

	struct ServerConfig
	{
		std::string 				host;
		int 						port;
		std::vector<std::string> 	serverNames;
		std::map<int, std::string> 	errorPages;
		long 						clientMaxBodySize;
		std::string 				root;
		std::vector<std::string> 	index;
		std::vector<LocationConfig> locations;
	};

	class ConfigBuilder
	{
	public:
		static std::vector<ServerConfig> build(const std::vector<ServerNode>& ast);
	private:
		static ServerConfig buildServerConfig(const ServerNode& node);
		static LocationConfig buildLocationConfig(const LocationNode& node, const ServerConfig& parent);
		static std::vector<std::string> defaultIndex();
		static std::vector<std::string> defaultMethods();
		static long defaultClientMaxBodySize();
		static long parseSizeLiteral(const std::string& size);
		static std::map<int, std::string> defaultErrorPages();
	};
}

#endif

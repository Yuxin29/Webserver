#pragma once

#include "ConfigParser.hpp"

#include <sys/stat.h>
#include <unistd.h>

/**
 * @brief 
 * 
 * 
 */
namespace config
{
	/**
	 * @struct LocationNode
	 * @brief Represents a location block in the configuration file.
	 *
	 * Contains all possible settings for a location including root,
	 * redirect, CGI configuration, upload directory, allowed methods, etc.
	 */
	struct LocationConfig
	{
		std::string 				path;
		std::string 				root;
		std::string 				redirect;
		std::vector<std::string>	index;
		std::string 				cgiPass;
		std::string 				cgiExt;
		std::string 				upload_dir;
		unsigned long 				clientMaxBodySize;
		bool 						autoindex;
		std::vector<std::string>	methods;
	};

	/**
	 * @struct ServerNode
	 * @brief Represents a server block in the configuration file.
	 *
	 * Contains server-level settings like server names, listen address,
	 * error pages, client body size, root, index, and associated locations.
	 */
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
	private:
		static ServerConfig					buildServerConfig(const ServerNode& node);
		static LocationConfig				buildLocationConfig(const LocationNode& node, const ServerConfig& parent);
		static std::vector<std::string>		defaultIndex();
		static std::vector<std::string>		defaultMethods();
		static long							defaultClientMaxBodySize();
		static long							parseSizeLiteral(const std::string& size);
		static std::map<int, std::string>	defaultErrorPages();

	public:
		static std::vector<ServerConfig>	build(const std::vector<ServerNode>& ast);
	};
}	

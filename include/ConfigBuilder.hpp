#pragma once

#include "ConfigParser.hpp"

#include <sys/stat.h>
#include <unistd.h>

/**
 * @namespace config
 * @brief Configuration building layer for the web server.
 *
 * This namespace contains runtime-ready configuration structures
 *and utilities that transform parsed configuration AST nodes
 * into validated, fully-resolved server configurations.
 *
 * The builder applies default values, performs type conversion,
 * handles inheritance between server and location blocks,
 * and prepares data for direct use by the web server.
 */
namespace config
{
	/**
	 * @struct LocationNode
	 * @brief Represents a location block in the configuration file.
	 *
	 * Contains all possible settings for a location including root,redirect, CGI configuration, upload directory, allowed methods, etc.
	 * This structure is built from a LocationNode and may inherit settings from its parent ServerConfig.
	 */
	struct LocationConfig
	{
		std::string 				path;					///< Location path(e.g. "/images", "/cgi-bin")
		std::string 				root;					///< Root directory for this location	
		std::string 				redirect;				///< Redirect URL for this location  (empty if not set)
		std::vector<std::string>	index;					//< Default pages for this location
		std::string 				cgiPass;				///< CGI executable path
		std::string 				cgiExt;					///< CGI file extension
		std::string 				upload_dir;				///< Upload directory for this location
		unsigned long 				clientMaxBodySize;		///< Max body size for this location
		bool 						autoindex;				///< Directory listing  enabled/disabled	
		std::vector<std::string>	methods;				///< Allowed HTTP methods for this location
	};

	/**
	 * @struct ServerNode
	 * @brief Represents a server block in the configuration file.
	 *
	 * Contains server-level settings like server names, listen address, error pages, client body size, root, index, and associated locations.
	* This structure is used directly by the server at runtime.
	 */
	struct ServerConfig
	{
		std::string 				host;				///< IP address to listen on
		int 						port;				///< Port number to listen on
		std::vector<std::string> 	serverNames;		///< Server names
		std::map<int, std::string> 	errorPages;			///< Custom error pages
		long 						clientMaxBodySize;	///< Max body size for this server
		std::string 				root;				///< Root directory for this server
		std::vector<std::string> 	index;				///< Default pages for this server	
		std::vector<LocationConfig> locations;			///< Location blocks within this server
	};

	class ConfigBuilder
	{
	private:
		static long							defaultClientMaxBodySize();
		static long							parseSizeLiteral(const std::string& size);
		static std::map<int, std::string>	defaultErrorPages();
		static std::vector<std::string>		defaultMethods();

		static ServerConfig					buildServerConfig(const ServerNode& node);
		static LocationConfig				buildLocationConfig(const LocationNode& node, const ServerConfig& parent);
	public:
		static std::vector<ServerConfig>	build(const std::vector<ServerNode>& ast);
	};
}	

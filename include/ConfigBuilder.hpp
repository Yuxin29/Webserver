/*
Why cannot use ServerNode / LocationNode directly (AST is not enough):
AST nodes (ServerNode, LocationNode) represent syntax, not semantics.
in AST:
They do not include inherited values
They do not include default values
They do not validate semantic correctness

AST = “What the config text looks like”
Runtime config = “How the server should behave”
*/

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
		std::string path;
		std::string root; //inherit
		std::string redirect;
		std::vector<std::string> index;//inherit
		std::string cgi_pass;
		std::string cgi_ext;
		std::string upload_dir;
		long client_max_body_size;//inherit
		bool autoindex;
		std::vector<std::string> methods; //cannot be empty
	};

	struct ServerConfig
	{
		std::string host;
		int port;
		std::vector<std::string> server_names;
		std::map<int, std::string> error_pages;
		long client_max_body_size; //bytes
		std::string root;//cannot be empty
		std::vector<std::string> index;//cannot be empty
		std::vector<LocationConfig> locations;
	};

	class ConfigBuilder
	{
	//static means it does NOT belong to an object, it belongs to the class
	public:
	//convert AST → final runtime configs
		static std::vector<ServerConfig> build(const std::vector<ServerNode>& ast);
	private:
	//Build one server from server AST
		static ServerConfig buildServerConfig(const ServerNode& node);
	//Build one location with inheritance
		static LocationConfig buildLocationConfig(const LocationNode& node, const ServerConfig& parent);
	//helper
		static std::vector<std::string> defaultIndex();
		static std::vector<std::string> defaultMethods();
		static long defaultClientMaxBodySize();
		static long parseSizeLiteral(const std::string& size);
		/*This function must:
		Read the last char → check if it's M/m/K/k
		Parse the number part
		Convert to bytes*/
	};
}




#endif

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
		std::string root;
		std::string redirect;
		std::vector<std::string> index;//
		std::string cgi_pass;
		std::string cgi_ext;
		std::string upload_dir;
		long client_max_body_size;
		bool autoindex;
		std::vector<std::string> methods;
	};

	struct ServerConfig
	{
		std::string host;
		int port;
		std::vector<std::string> server_names;
		std::map<int, std::string> error_pages;
		long client_max_body_size; //bytes
		std::string root;
		std::vector<std::string> index;
		std::vector<LocationConfig> locations;
	};

	class ConfigBuilder
	{
	public:
		std::vector<ServerConfig> build(const std::vector<ServerConfig>& ast);
	private:
		ServerConfig buildServerConfig(const ServerNode& node);
		LocationConfig buildLocationConfig(const LocationNode& node);

	};
}




#endif

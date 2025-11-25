#include "ConfigBuilder.hpp"

namespace config{
	std::vector<std::string> ConfigBuilder::defaultIndex()
	{

	}

	long ConfigBuilder::defaultClientMaxBodySize()
	{

	}

	long ConfigBuilder::parseSizeLiteral(const std::string& size)
	{

	}

	std::vector<std::string> ConfigBuilder::defaultMethods()
	{

	}

	LocationConfig ConfigBuilder::buildLocationConfig(const LocationNode& node, const ServerConfig& parent)
	{
		LocationConfig lc;
		lc.path = node.path;
		lc.root = node.root.empty() ? parent.root : node.root;
		lc.index = node.index.empty() ? parent.index : node.index;
		lc.client_max_body_size = node.client_max_body_size.empty()
									? parent.client_max_body_size
									: parseSizeLiteral(node.client_max_body_size);
		lc.redirect = node.redirect;
		lc.cgi_pass = node.cgi_pass;
		lc.cgi_ext = node.cgi_ext;
		lc.upload_dir = node.upload_dir;
		lc.autoindex = node.autoindex;
		lc.methods = node.methods.empty() ? defaultMethods() : node.methods;
		return lc;
	}

	ServerConfig ConfigBuilder::buildServerConfig(const ServerNode& node)
	{
		ServerConfig cfg;
		cfg.host = node.listen.first;
		cfg.port = node.listen.second;
		cfg.server_names = node.server_names;
		cfg.error_pages = node.error_pages;
		cfg.root = node.root.empty() ? "." : node.root;
		cfg.index = node.index.empty() ? defaultIndex() : node.index;
		cfg.client_max_body_size = node.client_max_body_size.empty()
									? defaultClientMaxBodySize()
									: parseSizeLiteral(node.client_max_body_size);
		for(size_t i = 0; i < node.locations.size(); i++){
			LocationConfig lc = buildLocationConfig(node.locations[i], cfg);
			cfg.locations.push_back(lc);
		}
		return cfg;
	}

	std::vector<ServerConfig> ConfigBuilder::build(const std::vector<ServerNode>& servers)
	{
		std::vector<ServerConfig> cfgs;
		for(size_t i = 0; i < servers.size(); i++)
		{
			ServerConfig cfg = buildServerConfig(servers[i]);
			cfgs.push_back(cfg);
		}
		return cfgs;
	}
}

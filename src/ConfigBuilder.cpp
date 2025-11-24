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

	LocationConfig ConfigBuilder::buildLocationConfig(const LocationNode& node, const ServerConfig& parent)
	{
		LocationConfig lc;
		lc.path = node.path;
		if(node.root.empty())
			lc.root = parent.root;
		//...
		return lc;
	}

	ServerConfig ConfigBuilder::buildServerConfig(const ServerNode& node)
	{
		ServerConfig cfg;
		cfg.host = node.listen.first;
		cfg.port = node.listen.second;
		if (node.server_names.empty())
			cfg.server_names = {};
		else
			cfg.server_names = node.server_names;
		if (node.error_pages.empty())
			cfg.error_pages = {};
		else
			cfg.error_pages = node.error_pages;
		if (node.root.empty())
			cfg.root = ".";
		else
			cfg.root = node.root;
		if (node.index.empty())
			cfg.index = defaultIndex();
		else
			cfg.index = node.index;
		if (node.client_max_body_size.empty())
			cfg.client_max_body_size = defaultClientMaxBodySize();
		else
			cfg.client_max_body_size = parseSizeLiteral(node.client_max_body_size);
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

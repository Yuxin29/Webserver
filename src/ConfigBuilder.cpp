#include "ConfigBuilder.hpp"
#include <iostream>

namespace config{
	std::vector<std::string> ConfigBuilder::defaultIndex()
	{
		std::vector<std::string> result;
		result.push_back("index.html");
		return result;
	}

	long ConfigBuilder::defaultClientMaxBodySize()
	{
		long size = 1*1024*1024;
		return size;
	}

	long ConfigBuilder::parseSizeLiteral(const std::string& sizeStr)
	{
		std::string numberStr;
		long num ;
		size_t last = sizeStr.size() - 1;
		char c = sizeStr[last];
		if(isdigit(c))
			num = std::stol(sizeStr);
		else if(c == 'M' || c == 'm'){
			numberStr = sizeStr.substr(0, last);
			num = std::stol(numberStr) * 1024 * 1024;
		}
		else if(c == 'K'||c == 'k'){
			numberStr = sizeStr.substr(0, last);
			num = std::stol(numberStr) * 1024;
		}
		else
			throw std::runtime_error("Invalid size in clientMaxBodySize");
		return num;
	}

	std::map<int, std::string> ConfigBuilder::defaultErrorPages()
	{
		std::map<int, std::string> pages;
		pages[400] = "/errors/400.html";
		pages[403] = "/errors/403.html";
		pages[404] = "/errors/404.html";
		pages[500] = "/errors/500.html";
		pages[502] = "/errors/502.html";
		pages[503] = "/errors/503.html";
		pages[504] = "/errors/504.html";
		return pages;
	}

	std::vector<std::string> ConfigBuilder::defaultMethods()
	{
		std::vector<std::string> methods;
		methods.push_back("GET");
		methods.push_back("POST");
		methods.push_back("DELETE");
		return methods;
	}

	LocationConfig ConfigBuilder::buildLocationConfig(const LocationNode& node, const ServerConfig& parent)
	{
		LocationConfig lc;
		lc.path = node.path;
		lc.root = node.root.empty() ? parent.root : node.root;
		lc.index = node.index.empty() ? parent.index : node.index;
		lc.clientMaxBodySize = node.clientMaxBodySize.empty()
									? parent.clientMaxBodySize
									: parseSizeLiteral(node.clientMaxBodySize);
		lc.redirect = node.redirect;
		lc.cgiPass = node.cgiPass;
		lc.cgiExt = node.cgiExt;
		lc.upload_dir = node.uploadDir;
		lc.autoindex = node.autoindex;
		lc.methods = node.methods.empty() ? defaultMethods() : node.methods;
		return lc;
	}

	ServerConfig ConfigBuilder::buildServerConfig(const ServerNode& node)
	{
		ServerConfig cfg;
		cfg.host = node.listen.first;
		cfg.port = node.listen.second;
		cfg.serverNames = node.serverNames;
		cfg.errorPages = node.errorPages.empty() ? defaultErrorPages() : node.errorPages;
		cfg.root = node.root.empty() ? "." : node.root;
		cfg.index = node.index.empty() ? defaultIndex() : node.index;
		cfg.clientMaxBodySize = node.clientMaxBodySize.empty()
									? defaultClientMaxBodySize()
									: parseSizeLiteral(node.clientMaxBodySize);
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

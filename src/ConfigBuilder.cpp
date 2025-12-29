#include "ConfigBuilder.hpp"
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

/**
 * @brief 
 * 
 * 
 * 
 */
namespace config{
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
		pages[301] = "sites/static/errors/301.html";
		pages[400] = "sites/static/errors/400.html";
		pages[403] = "sites/static/errors/403.html";
		pages[404] = "sites/static/errors/404.html";
		pages[405] = "sites/static/errors/405.html";
		pages[408] = "sites/static/errors/408.html";
		pages[413] = "sites/static/errors/413.html";
		pages[414] = "sites/static/errors/414.html";
		pages[431] = "sites/static/errors/431.html";
		pages[500] = "sites/static/errors/500.html";
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
		lc.path = node.path.empty() ? "/" : node.path;
		lc.root = node.root.empty() ? parent.root : node.root;
		lc.index = node.index.empty() ? parent.index : node.index;
		lc.clientMaxBodySize = node.clientMaxBodySize.empty()
									? parent.clientMaxBodySize
									: parseSizeLiteral(node.clientMaxBodySize);
		lc.redirect = node.redirect;
		lc.cgiPass = node.cgiPass;
		lc.cgiExt = node.cgiExt;
		lc.upload_dir = node.uploadDir.empty() ? "./sites/static/uploads":node.uploadDir;
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
		cfg.index = node.index;
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

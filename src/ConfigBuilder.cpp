#include "ConfigBuilder.hpp"
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

static void validateRoot(const std::string& root)
{
	struct stat st;
	if (stat(root.c_str(), &st) < 0 || !S_ISDIR(st.st_mode))
		throw std::runtime_error("Invalid root: '" + root + "' must be an existing directory");
}

// static void validatePath(const std::string& path)
// {
// 	if(path.empty() || path[0] != '/')
// 		throw std::runtime_error("Location path must start with '/' : "+path);
// }

static void validateindex(const std::vector<std::string>& index)
{
	for(size_t i = 0; i < index.size(); i++){
		if(index[i][0] == '/' || index[i].empty())
			throw std::runtime_error("Invalidated index file : " + index[i]);
	}
}

static void validateCgiext(const std::string& ext)
{
	if(!ext.empty()){
		if(ext[0] != '.')
			throw std::runtime_error("cgi_ext should start with '.'");
	}
}

static void validateCgipass(const std::string& pass)
{
	if(!pass.empty()){
		if(access(pass.c_str(), X_OK) != 0)
			throw std::runtime_error("cgi_pass is not executable : "+ pass);
		}
}

static void validateUploaddir(const std::string& upload)
{
	struct stat st;
	if (stat(upload.c_str(), &st) < 0 || !S_ISDIR(st.st_mode) || access(upload.c_str(), W_OK) != 0)
		throw std::runtime_error("Invalid upload_dir : "+upload);
}


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
		pages[301] = "/errors/301.html";
		pages[400] = "/errors/400.html";
		pages[403] = "/errors/403.html";
		pages[404] = "/errors/404.html";
		pages[405] = "/errors/405.html";
		pages[408] = "/errors/408.html";
		pages[413] = "/errors/413.html";
		pages[414] = "/errors/414.html";
		pages[431] = "/errors/431.html";
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
		lc.path = node.path.empty() ? "/" : node.path;
		//validatePath(lc.path);
		lc.root = node.root.empty() ? parent.root : node.root;
		validateRoot(lc.root);
		lc.index = node.index.empty() ? parent.index : node.index;
		validateindex(lc.index);
		lc.clientMaxBodySize = node.clientMaxBodySize.empty()
									? parent.clientMaxBodySize
									: parseSizeLiteral(node.clientMaxBodySize);
		lc.redirect = node.redirect;
		lc.cgiPass = node.cgiPass;
		validateCgipass(lc.cgiPass);
		lc.cgiExt = node.cgiExt;
		validateCgiext(lc.cgiExt);
		lc.upload_dir = node.uploadDir.empty() ? "./sites/static/uploads":node.uploadDir;
		validateUploaddir(lc.upload_dir);
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
		validateRoot(cfg.root);
		cfg.index = node.index.empty() ? defaultIndex() : node.index;
		validateindex(cfg.index);
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

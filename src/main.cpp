#include "ConfigParser.hpp"
#include "ConfigTokenizer.hpp"
#include <iostream>
using namespace config;

int main() {
	try {
		// point to config file
		config::Parser parser("configuration/webserv.conf");

		// parse it into vector of ServerNode
		std::vector<ServerNode> servers = parser.parse();

		//  print results
		for (std::size_t i = 0; i < servers.size(); ++i) {
			const ServerNode& server = servers[i];
			std::cout << "==== Server " << i + 1 << " ====\n";
			std::cout << "Listen: " << server.listen.first << ":" << server.listen.second << "\n";
			std::cout << "Root: " << server.root << "\n";

			std::cout << "Server Names:\n";
			for (const std::string& name : server.server_names)
				std::cout << "  - " << name << "\n";

			std::cout << "Error Pages:\n";
			for (const auto& [code, path] : server.error_pages)
				std::cout << "  " << code << " => " << path << "\n";

			std::cout << "Client Max Body Size: " << server.client_max_body_size << "\n";

			std::cout << "Locations:\n";
			for (const LocationNode& loc : server.locations) {
				std::cout << "  Path: " << loc.path << "\n";
				std::cout << "  Root: " << loc.root << "\n";
				std::cout << "  Index: " << loc.index << "\n";
				std::cout << "  Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
				std::cout << "  Redirect: " << loc.redirect << "\n";
				std::cout << "  CGI Path: " << loc.cgi_path << "\n";
				std::cout << "  Upload Dir: " << loc.upload_dir << "\n";
				std::cout << "  Methods:\n";
				for (const std::string& method : loc.methods)
					std::cout << "    - " << method << "\n";
				std::cout << "\n";
			}
			std::cout << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error while parsing config: " << e.what() << std::endl;
		return 1;
	}
}


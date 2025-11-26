#include "ConfigBuilder.hpp"
#include <iostream>

using namespace config;

static void printVector(const std::vector<std::string>& v) {
    for (size_t i = 0; i < v.size(); i++)
        std::cout << "    - " << v[i] << "\n";
}

static void printErrorPages(const std::map<int, std::string>& m) {
    for (std::map<int, std::string>::const_iterator it = m.begin();
         it != m.end(); it++)
    {
        std::cout << "    " << it->first << " => " << it->second << "\n";
    }
}
int main() {
    try {
        // load config file
        Parser parser("configuration/webserv.conf");

        // parse into AST
        std::vector<ServerNode> servers = parser.parse();
		//convert AST to config
		std::vector<ServerConfig> cfgs = ConfigBuilder::build(servers);

        // print results
         for (size_t i = 0; i < cfgs.size(); i++) {
            const ServerConfig& s = cfgs[i];
            std::cout << "=============================\n";
            std::cout << "        SERVER " << i + 1 << "\n";
            std::cout << "=============================\n";

            std::cout << "Host: " << s.host << "\n";
            std::cout << "Port: " << s.port << "\n";

            std::cout << "Root: " << s.root << "\n";

            std::cout << "Index:\n";
            printVector(s.index);

            std::cout << "Client Max Body Size: " << s.clientMaxBodySize<< " bytes\n";

            std::cout << "Server Names:\n";
            printVector(s.serverNames);

            std::cout << "Error Pages:\n";
            printErrorPages(s.errorPages);

            std::cout << "\nLocations:\n";
            for (size_t j = 0; j < s.locations.size(); j++) {
                const LocationConfig& loc = s.locations[j];
                std::cout << "\n  --- Location " << j + 1 << " ---\n";
                std::cout << "  Path: " << loc.path << "\n";
                std::cout << "  Root: " << loc.root << "\n";

                std::cout << "  Index:\n";
                printVector(loc.index);

                std::cout << "  Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
                std::cout << "  Redirect: " << loc.redirect << "\n";
                std::cout << "  CGI Pass: " << loc.cgiPass << "\n";
                std::cout << "  CGI Ext: " << loc.cgiExt << "\n";
                std::cout << "  Upload Dir: " << loc.upload_dir << "\n";

                std::cout << "  Client Max Body Size: "
                          << loc.clientMaxBodySize << " bytes\n";

                std::cout << "  Methods:\n";
                printVector(loc.methods);
            }

            std::cout << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Error while parsing config: " << e.what() << "\n";
        return 1;
    }
}


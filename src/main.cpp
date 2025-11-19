#include <fstream>
#include <iostream>
#include "ConfigParser.hpp"
#include "ConfigTokenizer.hpp"
#include <sstream>

int main() {
	try {
		// std::ifstream file("new.conf");
		// if (!file)
		// 	throw std::runtime_error("Failed to open config file");

		// std::stringstream buffer;
		// buffer << file.rdbuf();
		// std::string content = buffer.str();

		// Tokenizer tokenizer(content);
		// std::vector<Token> tokens = tokenizer.tokenize();

		Parser parser("../configuration/simple.conf");
		LocationNode loc = parser.parseLocationBlock();

		std::cout << "Location path: " << loc.path << "\n";
		std::cout << "Root: " << loc.root << "\n";
		std::cout << "Redirect: " << loc.redirect << "\n";
		std::cout << "Index: " << loc.index << "\n";
		std::cout << "CGI Path: " << loc.cgi_path << "\n";
		std::cout << "Upload Dir: " << loc.upload_dir << "\n";
		std::cout << "Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";

		std::cout << "Methods: ";
		for (const std::string& m : loc.methods)
			std::cout << m << " ";
		std::cout << "\n";

	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
	}
}

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <vector>
#include <map>
#include "ConfigTokenizer.hpp"

namespace config{
	struct LocationNode
	{
		std::string path;
		std::string root;
		std::string redirect;
		std::string index;
		std::string cgi_path;
		std::string upload_dir;
		bool autoindex;
		std::vector<std::string> methods;
	};

	struct ServerNode
	{
		std::vector<std::string> server_names;
		std::pair<std::string, int> listen; //might 127.0.0.1:8080 or just 8080; 127.0.0.1 is string
		std::map<int, std::string> error_pages; //int: error code; string:path to html file
		int client_max_body_size;
		std::string root;
		std::vector<LocationNode> locations;
	};

	class Parser
	{
	public:
		Parser(const std::string& filename);
		std::vector<ServerNode> parse();
		//LocationNode parseLocationBlock();

	private:
		std::vector<Token> _tokens;
		std::size_t _pos;

		Token peek() const;
		Token get();
		bool eof();
		bool match(TokenType type);
		void expect(TokenType type, const std::string& msg);
		ServerNode parseServerBlock();
		LocationNode parseLocationBlock();
		std::string parseSimpleDirective();
	};
}

#endif

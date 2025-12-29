#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <vector>
#include <map>
#include "ConfigTokenizer.hpp"

/**
 * @brief 
 * 
 * 
 * 
 * 
 */
namespace config{
	struct LocationNode
	{
		std::string 			path;
		std::string 			root;
		std::string 			redirect;
		std::vector<std::string> index;
		std::string 			cgiPass;
		std::string 			cgiExt;
		std::string 			uploadDir;
		std::string 			clientMaxBodySize;
		bool 					autoindex;
		std::vector<std::string> methods;
	};

	struct ServerNode
	{
		std::vector<std::string> 	serverNames;
		std::pair<std::string, int> listen;
		std::map<int, std::string> 	errorPages;
		std::string 				clientMaxBodySize;
		std::string 				root;
		std::vector<std::string> 	index;
		std::vector<LocationNode> 	locations;
	};

	class Parser
	{
	public:
		Parser(const std::string& filename);
		std::vector<ServerNode> parse();

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
		std::string parseSimpleDirective(const std::string& str);
		std::vector<std::string> parseVectorStringDirective(const std::string& str);
		bool isKeyword(std::string& s);
	};
}

#endif

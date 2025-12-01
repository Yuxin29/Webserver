#include "ConfigParser.hpp"
#include "utils.hpp"
#include <fstream>
#include <sstream> //std::stringstream
#include <iostream>
#include <filesystem>

namespace config{
	Parser::Parser(const std::string& filename)
	:_pos(0)
	{
		//does the file exist?
		if(!std::filesystem::exists(filename))
			throw std::runtime_error("Config file does not exist: " + filename);
		// Is the path a directory?
		if(std::filesystem::is_directory(filename))
			throw std::runtime_error("Config path is a directory.");
		//validate .conf extension
		if(std::filesystem::path(filename).extension() != ".conf")
			throw std::runtime_error("Config file should end with '.conf'.");

		std::ifstream infile(filename);
		if (!infile)
			throw std::runtime_error("Failed to open config file " + filename);
		std::stringstream buffer;
		buffer << infile.rdbuf();//.rdbuf() gives access to its underlying stream buffer.
		std::string content = buffer.str();
		//if really empty(noting, no new line)
		if (content.empty())
			throw std::runtime_error("Empty conf file: " + filename);
		Tokenizer tokenizer(content);
		_tokens = tokenizer.tokenize();
		//has new line or comment
		if(_tokens.size() == 1 && _tokens[0].type == TK_EOF)
			throw std::runtime_error("Empty conf file : " + filename);
		//debug, should delete later
		for (auto &tk : _tokens)
		{
			std::cout << "[" << tk.value << "] "
				<< tk.type
				<< " line " << tk.line
				<< " col " << tk.col << "\n";
		} //for print the token list, debug
	}

	Token Parser::peek() const
	{
		if(_pos >= _tokens.size())
			return Token{TK_EOF, "", 0, 0};
		return _tokens[_pos];
	}

	Token Parser::get()
	{
		if (_pos >= _tokens.size())
			return Token{TK_EOF, "", 0, 0};
		return _tokens[_pos++];
	}

	bool Parser::eof()
	{
		return _pos >= _tokens.size();
	}

	bool Parser::match(TokenType type)
	{
		if (peek().type == type)
		{
			get();
			return true;
		}
		return false;
	}

	void Parser::expect(TokenType type, const std::string& msg)
	{
		Token cur_token = get();
		if (cur_token.type != type)
			throw std::runtime_error(makeError(msg, cur_token.line, cur_token.col));
	}

	std::string Parser::parseSimpleDirective(const std::string& str)
	{
		get();
		Token valuetoken = get();
		if (valuetoken.type != TK_IDENTIFIER && valuetoken.type != TK_STRING)
			throw std::runtime_error("Expect value after " + str);
		expect(TK_SEMICOLON, "Expected ';'");
		return valuetoken.value;
	}

	std::vector<std::string> Parser::parseVectorStringDirective(const std::string& str)
	{
		std::vector<std::string> results;
		while(true)
		{
			if (eof()) {
				throw std::runtime_error("Unexpected EOF while parsing index value");
			}
			Token tok = get();
			if(isKeyword(tok.value))
				throw std::runtime_error("Missing value after " + str);
			if(tok.type != TK_IDENTIFIER && tok.type != TK_STRING)
				throw std::runtime_error(makeError("Expect value", tok.line, tok.col));
			results.push_back(tok.value);
			Token next = peek();
			if(next.type == TK_SEMICOLON)
			{
				get(); //eat
				break;
			}
			if (next.type == TK_IDENTIFIER && isKeyword(next.value)) {
				throw std::runtime_error(makeError("Expected ';'", tok.line, tok.col));
			}
			if (next.type != TK_IDENTIFIER){
				throw std::runtime_error(makeError("Expected ';'", tok.line, tok.col));
			}
		}
		return results;
	}

	bool Parser::isKeyword(std::string& s)
	{
		return s == "path"
		|| s == "redirect"
		|| s == "server_name"
		|| s == "root"
		|| s == "index"
		|| s == "autoindex"
		|| s == "cgi_pass"
		|| s == "cgi_ext"
		|| s == "upload_dir"
		|| s == "client_max_body_size"
		|| s == "allowed_methods"
		|| s == "error_pages" ;
	}

	ServerNode Parser::parseServerBlock()
	{
		Token token = get();
		//std::cout<< token.value << std::endl;
		if (token.value != "server")
			throw std::runtime_error(makeError("Expect server ", token.line, token.col));
		expect(TK_LBRACE, "Expected '{' after location path");

		ServerNode server;
		while(true)
		{
			Token token = peek();
			if (token.type == TK_RBRACE){
				get();
				break;
			}
			if (token.type == TK_IDENTIFIER && token.value == "listen")
			{
				get();
				Token listenToken = get();
				if(listenToken.type != TK_IDENTIFIER && listenToken.type != TK_NUMBER)
					throw std::runtime_error(makeError("Expect port ", listenToken.line, listenToken.col));
				std::string host;
				int port;
				std::string listenValue = listenToken.value;
				size_t colon = listenValue.find(':');
				if(colon == std::string::npos)
				{
					host = "0.0.0.0";
					port = std::stoi(listenValue);
				}
				else{
					host = listenValue.substr(0, colon);
					port = std::stoi(listenValue.substr(colon+1));
				}
				server.listen = std::make_pair(host, port);
				expect(TK_SEMICOLON, "Expected ';' after port ");
			}
			else if (token.type == TK_IDENTIFIER && token.value == "server_name")
			{
				get(); // eat server_name
				server.serverNames = parseVectorStringDirective("server_name");
			}
			else if(token.type == TK_IDENTIFIER && token.value == "error_page")
			{
				get();//eat error_page
				Token codeTok = get();
				if (codeTok.type != TK_NUMBER)
					throw std::runtime_error(makeError("Expect error code ", codeTok.line, codeTok.col));
				Token pathTok = get();
				if (pathTok.type != TK_IDENTIFIER)
					throw std::runtime_error(makeError("Expect path ", codeTok.line, codeTok.col));
				server.errorPages[std::stoi(codeTok.value)] = pathTok.value;
				expect(TK_SEMICOLON, "Expected ';' after error path");
			}
			else if(token.type == TK_IDENTIFIER && token.value == "client_max_body_size")
				server.clientMaxBodySize = parseSimpleDirective("client_max_body_size");
			else if (token.type == TK_IDENTIFIER && token.value == "root")
				server.root = parseSimpleDirective("root");
			else if(token.type==TK_IDENTIFIER && token.value == "index")
			{
				get(); //eat index
				server.index = parseVectorStringDirective("index");
			}
			else if (token.type == TK_IDENTIFIER && token.value == "location")
				server.locations.push_back(parseLocationBlock());
			else
				throw std::runtime_error(makeError("Unknown keyword in server block ", token.line, token.col));
		}
		return server;
	}

	LocationNode Parser::parseLocationBlock()
	{

		Token token = get();
		if (token.value != "location")
			throw std::runtime_error(makeError("Expect location ", token.line, token.col));
		Token path = get();
		if (path.type != TK_IDENTIFIER)
			throw std::runtime_error(makeError("Expected path after 'location '", path.line, path.col));
		expect(TK_LBRACE, "Expected '{' after location path");

		LocationNode location;
		location.path = path.value;
		location.autoindex = false;
		while(true)
		{
			Token token = peek();
			if (token.type == TK_RBRACE){
				get();
				break;
			}
			if (token.type == TK_IDENTIFIER && token.value == "root")
				location.root = parseSimpleDirective("root");
			else if(token.type == TK_IDENTIFIER && token.value == "redirect")
				location.redirect = parseSimpleDirective("redirect");
			else if(token.type==TK_IDENTIFIER && token.value == "cgi_pass")
				location.cgiPass = parseSimpleDirective("cgi_pass");
			else if(token.type==TK_IDENTIFIER && token.value == "cgi_ext")
				location.cgiExt = parseSimpleDirective("cgi_ext");
			else if(token.type==TK_IDENTIFIER && token.value == "upload_dir")
				location.uploadDir = parseSimpleDirective("upload_dir");
			else if(token.type == TK_IDENTIFIER && token.value == "client_max_body_size")
				location.clientMaxBodySize = parseSimpleDirective("client_max_body_size");
			else if(token.type==TK_IDENTIFIER && token.value == "index")
			{
				get(); //eat index
				location.index = parseVectorStringDirective("index");
			}
			else if(token.type==TK_IDENTIFIER && token.value == "autoindex")
			{
				get();
				Token autoindex = get();
				if (autoindex.value == "on")
					location.autoindex = true;
				else if(autoindex.value == "off")
					location.autoindex = false;
				else
					throw std::runtime_error("Expect on/off after autoindex.");
				expect(TK_SEMICOLON, "Expected ';' after location path");
			}
			else if(token.type==TK_IDENTIFIER && (token.value == "methods" || token.value == "allowed_methods"))
			{
				get();
				location.methods = parseVectorStringDirective("allowed_methods");
			}
			else
				throw std::runtime_error(makeError("Unknown keyword in location block ", token.line, token.col));
		}
		return location;
	}

	std::vector<ServerNode> Parser::parse()
	{
		std::vector<ServerNode> servers;
		while(peek().type != TK_EOF)
		{
			Token t = peek();
			if (t.type == TK_IDENTIFIER && t.value == "server")
				servers.push_back(parseServerBlock());
			else
				throw std::runtime_error(makeError("Expected 'server' block ", t.line, t.col));
		}
		return servers;
	}
}

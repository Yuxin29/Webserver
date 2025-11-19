#include "ConfigParser.hpp"
#include <fstream>
#include <sstream> //std::stringstream
#include <format>
#include <iostream>

Parser::Parser(const std::string& filename)
:_pos(0)
{
	std::ifstream infile(filename);
	if (!infile)
		throw std::runtime_error("Failed to open config file" + filename);
	std::stringstream buffer;
	buffer << infile.rdbuf();//.rdbuf() gives access to its underlying stream buffer.
	std::string content = buffer.str();
	Tokenizer tokenizer(content);
	_tokens = tokenizer.tokenize();
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

bool Parser::match(TokenType type)
{
	if (peek().type == type)
	{
		get();
		return true;
	}
	return false;
}

void Parser::expect(TokenType type, const std::string& msg) //msg?
{
	Token cur_token = get();
	if (cur_token.type != type)
		throw std::runtime_error(msg);
}

ServerNode Parser::parseServerBlock()
{


}

std::string Parser::parseSimpleDirective(const std::string& name)
{
	get();
	Token valuetoken = get();
	if (valuetoken.type != TK_IDENTIFIER)
		throw std::runtime_error(std::format("Expcept {} value at line {}, col {}", name, valuetoken.line, valuetoken.col));
	expect(TK_SEMICOLON, std::format("Expected ';' after {}", name));
	return valuetoken.value;
}

LocationNode Parser::parseLocationBlock()
{
	expect(TK_IDENTIFIER, "Expect 'Location");
	Token path = get();
	if (path.type != TK_IDENTIFIER)
		throw std::runtime_error(std::format("Expected path after 'location' at line {}, col {}", path.line, path.col));
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
		else if(token.type==TK_IDENTIFIER && token.value == "index")
			location.index = parseSimpleDirective("redirect");
		else if(token.type==TK_IDENTIFIER && token.value == "cgi_path")
			location.cgi_path = parseSimpleDirective("cgi_path");
		else if(token.type==TK_IDENTIFIER && token.value == "upload_dir")
			location.upload_dir = parseSimpleDirective("upload_dir");
		else if(token.type==TK_IDENTIFIER && token.value == "autoindex")
		{
			get();
			Token autoindex = get();
			if (autoindex.value == "on")
				location.autoindex = true;
			else if(autoindex.value == "off")
				location.autoindex = false;
			else
				throw std::runtime_error(std::format("Expcept on/off after autoindex at line {}, col {}", autoindex.line, autoindex.col));
			expect(TK_SEMICOLON, "Expected ';' after location path");
		}
		else if(token.type==TK_IDENTIFIER && token.value == "methods")
		{
			get();
			while(true)
			{
				Token methodValue = get();
				if(methodValue.value == "GET" || methodValue.value == "POST" || methodValue.value == "DELETE")
					location.methods.push_back(methodValue.value);
				else
					throw std::runtime_error(std::format("Expcept GET/POST/DELETE after methods at line {}, col {}", methodValue.line, methodValue.col));
				if(peek().type == TK_SEMICOLON){
					get();
					break;
				}
			}
		}
		else
			throw std::runtime_error(std::format("Unknown directive {} in location block at line {}, col {}", token.value, token.line, token.col));
	}
	return location;
}

std::vector<ServerNode> Parser::parse()
{

}

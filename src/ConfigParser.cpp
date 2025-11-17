#include "ConfigParser.hpp"
#include <fstream>
#include <sstream> //std::stringstream

Parser::Parser(std::string& filename)
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
		throw std::runtime_error("Error at line " + std::to_string(cur_token.line) + ", col " + std::to_string(cur_token.col) + " : " + msg);
}

ServerNode Parser::parseServerBlock()
{

	
}

LocationNode Parser::parseLocationBlock()
{

}

std::vector<ServerNode> Parser::parse()
{

}

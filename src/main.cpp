#include <iostream>
#include <vector>
#include "ConfigParser.hpp"

std::string tokenTypeToString(TokenType type) {
	switch (type) {
		case TK_IDENTIFIER: return "IDENTIFIER";
		case TK_NUMBER:     return "NUMBER";
		case TK_STRING:     return "STRING";
		case TK_LBRACE:     return "LBRACE";
		case TK_RBRACE:     return "RBRACE";
		case TK_SEMICOLON:  return "SEMICOLON";
		case TK_EOF:        return "EOF";
		default:            return "UNKNOWN";
	}
}

int main() {
	std::string config = R"(
		#comment
		server {
			listen 8080;
			server_name localhost;

			error_page 404 /errors/404.html;
			error_page 403 /errors/403.html;
			error_page 500 /errors/500.html;

			client_max_body_size 1M;

			# Location: root page with autoindex off
			location / {
				root /home/linliu/42_github/rank5/webserver/sites/static/index.html;
				index index.html;
				autoindex off;
				methods GET;
			}
	)";

	Tokenizer tokenizer(config);
	std::vector<Token> tokens = tokenizer.tokenize();

	for (size_t i = 0; i < tokens.size(); ++i) {
		const Token& tok = tokens[i];

		std::cout << "Line " << tok.line
		          << ", Col " << tok.col
		          << " | Type: " << tokenTypeToString(tok.type)
		          << " | Value: \"" << tok.value << "\""
		          << std::endl;
	}
}

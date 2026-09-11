#pragma once
#include "token.hpp"
#include <string>
#include <vector>

namespace ac {

class Lexer {
public:
  Lexer(std::string source, std::string file = "<stdin>");
  std::vector<Token> lex();
private:
  char peek(std::size_t n = 0) const;
  char take();
  bool eof() const;
  void skipWhitespaceAndComments();
  Token make(TokenKind kind, std::size_t start, SourceLocation loc);
  Token lexIdentifier();
  Token lexNumber();
  Token lexString();

  std::string source_;
  std::string file_;
  std::size_t index_ = 0, line_ = 1, column_ = 1;
};

} // namespace ac

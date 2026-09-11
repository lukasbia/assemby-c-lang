#pragma once
#include "ast.hpp"
#include "token.hpp"
#include <vector>

namespace ac {

class Parser {
public:
  explicit Parser(const std::vector<Token>& tokens):tokens_(tokens){}
  SourceFile parse();
private:
  const Token& peek(std::size_t n=0) const;
  bool match(TokenKind k);
  Token expect(TokenKind k, const char* message);
  std::string expectIdentifier(const char* message);
  std::string parseTypeName();
  FunctionDecl parseFunction();
  std::vector<StmtPtr> parseBlock();
  StmtPtr parseStatement();
  ExprPtr parseExpression(int minPrec=0);
  ExprPtr parsePrimary();
  int precedence(TokenKind k) const;
  const std::vector<Token>& tokens_;
  std::size_t pos_=0;
};

} // namespace ac

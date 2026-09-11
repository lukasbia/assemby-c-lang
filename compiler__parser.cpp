#include "parser.hpp"
#include <cstdlib>
#include <stdexcept>

namespace ac {
const Token& Parser::peek(std::size_t n) const { return tokens_.at(pos_+n); }
bool Parser::match(TokenKind k) { if (peek().kind==k) {++pos_; return true;} return false; }
Token Parser::expect(TokenKind k,const char* msg){ if(!match(k)) throw std::runtime_error(msg); return tokens_[pos_-1]; }
std::string Parser::expectIdentifier(const char* msg){ return expect(TokenKind::Identifier,msg).text; }

std::string Parser::parseTypeName() {
  auto n = expectIdentifier("expected type name");
  if (match(TokenKind::Question)) n += "?";
  return n;
}

SourceFile Parser::parse() {
  SourceFile f;
  if (match(TokenKind::KwModule)) f.moduleName=expectIdentifier("expected module name");
  while (match(TokenKind::KwImport)) f.imports.push_back(expectIdentifier("expected imported module"));
  while (peek().kind != TokenKind::Eof) {
    if (peek().kind==TokenKind::KwFunc) f.functions.push_back(parseFunction());
    else throw std::runtime_error("expected declaration");
  }
  return f;
}

FunctionDecl Parser::parseFunction() {
  expect(TokenKind::KwFunc,"expected func");
  FunctionDecl f; f.name=expectIdentifier("expected function name");
  expect(TokenKind::LeftParen,"expected '('");
  if(!match(TokenKind::RightParen)) {
    do {
      auto n=expectIdentifier("expected parameter name");
      expect(TokenKind::Colon,"expected ':'");
      auto t=parseTypeName();
      f.params.push_back({n,t});
    } while(match(TokenKind::Comma));
    expect(TokenKind::RightParen,"expected ')'");
  }
  if(match(TokenKind::Arrow)) f.returnType=parseTypeName();
  else f.returnType="Void";
  f.body=parseBlock();
  return f;
}

std::vector<StmtPtr> Parser::parseBlock() {
  expect(TokenKind::LeftBrace,"expected '{'");
  std::vector<StmtPtr> body;
  while(peek().kind!=TokenKind::RightBrace && peek().kind!=TokenKind::Eof)
    body.push_back(parseStatement());
  expect(TokenKind::RightBrace,"expected '}'");
  return body;
}

StmtPtr Parser::parseStatement() {
  if(match(TokenKind::KwLet)||match(TokenKind::KwVar)) {
    bool mut=tokens_[pos_-1].kind==TokenKind::KwVar;
    auto n=expectIdentifier("expected variable name");
    std::string t;
    if(match(TokenKind::Colon)) t=parseTypeName();
    expect(TokenKind::Equal,"expected '=' in declaration");
    auto e=parseExpression();
    match(TokenKind::Semicolon);
    return std::make_unique<VarDeclStmt>(mut,n,t,std::move(e));
  }
  if(match(TokenKind::KwReturn)) {
    ExprPtr e;
    if(peek().kind!=TokenKind::RightBrace && peek().kind!=TokenKind::Semicolon) e=parseExpression();
    match(TokenKind::Semicolon);
    return std::make_unique<ReturnStmt>(std::move(e));
  }
  if(peek().kind==TokenKind::Identifier && peek(1).kind==TokenKind::Equal) {
    auto n=peek().text; pos_+=2;
    auto e=parseExpression(); match(TokenKind::Semicolon);
    return std::make_unique<AssignStmt>(n,std::move(e));
  }
  auto e=parseExpression(); match(TokenKind::Semicolon);
  return std::make_unique<ExprStmt>(std::move(e));
}

int Parser::precedence(TokenKind k) const {
  switch(k){
    case TokenKind::OrOr:return 1;
    case TokenKind::AndAnd:return 2;
    case TokenKind::EqualEqual: case TokenKind::NotEqual:return 3;
    case TokenKind::Less: case TokenKind::LessEqual: case TokenKind::Greater: case TokenKind::GreaterEqual:return 4;
    case TokenKind::Plus: case TokenKind::Minus:return 5;
    case TokenKind::Star: case TokenKind::Slash: case TokenKind::Percent:return 6;
    default:return -1;
  }
}

ExprPtr Parser::parseExpression(int minPrec) {
  auto lhs=parsePrimary();
  while(true){
    int p=precedence(peek().kind);
    if(p<minPrec) break;
    auto op=peek().kind; ++pos_;
    auto rhs=parseExpression(p+1);
    lhs=std::make_unique<BinaryExpr>(op,std::move(lhs),std::move(rhs));
  }
  return lhs;
}

ExprPtr Parser::parsePrimary() {
  auto t=peek();
  if(match(TokenKind::Integer)) return std::make_unique<IntegerExpr>(std::stoll(t.text));
  if(match(TokenKind::Float)) return std::make_unique<FloatExpr>(std::stod(t.text));
  if(match(TokenKind::String)) return std::make_unique<StringExpr>(t.text.substr(1,t.text.size()-2));
  if(match(TokenKind::KwTrue)) return std::make_unique<BoolExpr>(true);
  if(match(TokenKind::KwFalse)) return std::make_unique<BoolExpr>(false);
  if(match(TokenKind::KwNil)) return std::make_unique<NilExpr>();
  if(match(TokenKind::Identifier)) {
    ExprPtr e=std::make_unique<NameExpr>(t.text);
    if(match(TokenKind::LeftParen)){
      std::vector<ExprPtr> args;
      if(!match(TokenKind::RightParen)){
        do { args.push_back(parseExpression()); } while(match(TokenKind::Comma));
        expect(TokenKind::RightParen,"expected ')'");
      }
      e=std::make_unique<CallExpr>(std::move(e),std::move(args));
    }
    return e;
  }
  if(match(TokenKind::LeftParen)){
    auto e=parseExpression(); expect(TokenKind::RightParen,"expected ')'");
    return e;
  }
  throw std::runtime_error("expected expression");
}
} // namespace ac

#include "lexer.hpp"
#include <cctype>
#include <stdexcept>

namespace ac {

static std::unordered_map<std::string, TokenKind> makeKeywords() {
  std::unordered_map<std::string, TokenKind> m;
#define AC_KEYWORD(name, spelling) m.emplace(spelling, TokenKind::name);
#include "keywords.def"
#undef AC_KEYWORD
  return m;
}
const std::unordered_map<std::string, TokenKind>& keywordMap() {
  static const auto m = makeKeywords();
  return m;
}

const char* tokenName(TokenKind k) {
  switch (k) {
    case TokenKind::Eof: return "end of file";
    case TokenKind::Identifier: return "identifier";
    case TokenKind::Integer: return "integer";
    case TokenKind::Float: return "float";
    case TokenKind::String: return "string";
    case TokenKind::Plus: return "+";
    case TokenKind::Minus: return "-";
    case TokenKind::Star: return "*";
    case TokenKind::Slash: return "/";
    case TokenKind::Percent: return "%";
    case TokenKind::Equal: return "=";
    case TokenKind::EqualEqual: return "==";
    case TokenKind::NotEqual: return "!=";
    case TokenKind::Less: return "<";
    case TokenKind::LessEqual: return "<=";
    case TokenKind::Greater: return ">";
    case TokenKind::GreaterEqual: return ">=";
    case TokenKind::AndAnd: return "&&";
    case TokenKind::OrOr: return "||";
    case TokenKind::Bang: return "!";
    case TokenKind::Colon: return ":";
    case TokenKind::Comma: return ",";
    case TokenKind::Dot: return ".";
    case TokenKind::Arrow: return "->";
    case TokenKind::Question: return "?";
    case TokenKind::LeftParen: return "(";
    case TokenKind::RightParen: return ")";
    case TokenKind::LeftBrace: return "{";
    case TokenKind::RightBrace: return "}";
    case TokenKind::LeftBracket: return "[";
    case TokenKind::RightBracket: return "]";
    case TokenKind::Semicolon: return ";";
#define AC_KEYWORD(name, spelling) case TokenKind::name: return spelling;
#include "keywords.def"
#undef AC_KEYWORD
  }
  return "?";
}

Lexer::Lexer(std::string source, std::string file) : source_(std::move(source)), file_(std::move(file)) {}

char Lexer::peek(std::size_t n) const {
  return index_ + n < source_.size() ? source_[index_ + n] : '\0';
}
char Lexer::take() {
  char c = peek();
  if (c) {
    ++index_;
    if (c == '\n') { ++line_; column_ = 1; }
    else ++column_;
  }
  return c;
}
bool Lexer::eof() const { return index_ >= source_.size(); }

Token Lexer::make(TokenKind kind, std::size_t start, SourceLocation loc) {
  return Token{kind, source_.substr(start, index_ - start), std::move(loc)};
}

void Lexer::skipWhitespaceAndComments() {
  for (;;) {
    while (!eof() && std::isspace(static_cast<unsigned char>(peek()))) take();
    if (peek() == '/' && peek(1) == '/') {
      while (!eof() && take() != '\n') {}
      continue;
    }
    if (peek() == '/' && peek(1) == '*') {
      take(); take();
      while (!eof() && !(peek() == '*' && peek(1) == '/')) take();
      if (!eof()) { take(); take(); }
      continue;
    }
    break;
  }
}

Token Lexer::lexIdentifier() {
  auto loc = SourceLocation{file_, index_, line_, column_};
  auto start = index_;
  while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') take();
  auto text = source_.substr(start, index_ - start);
  auto it = keywordMap().find(text);
  return make(it == keywordMap().end() ? TokenKind::Identifier : it->second, start, std::move(loc));
}

Token Lexer::lexNumber() {
  auto loc = SourceLocation{file_, index_, line_, column_};
  auto start = index_;
  bool dot = false;
  while (std::isdigit(static_cast<unsigned char>(peek()))) take();
  if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek(1)))) {
    dot = true; take();
    while (std::isdigit(static_cast<unsigned char>(peek()))) take();
  }
  return make(dot ? TokenKind::Float : TokenKind::Integer, start, std::move(loc));
}

Token Lexer::lexString() {
  auto loc = SourceLocation{file_, index_, line_, column_};
  auto start = index_;
  take();
  while (!eof() && peek() != '"') {
    if (peek() == '\\') { take(); if (!eof()) take(); }
    else take();
  }
  if (eof()) throw std::runtime_error("unterminated string literal");
  take();
  return make(TokenKind::String, start, std::move(loc));
}

std::vector<Token> Lexer::lex() {
  std::vector<Token> out;
  while (!eof()) {
    skipWhitespaceAndComments();
    if (eof()) break;
    if (std::isalpha(static_cast<unsigned char>(peek())) || peek() == '_') {
      out.push_back(lexIdentifier()); continue;
    }
    if (std::isdigit(static_cast<unsigned char>(peek()))) {
      out.push_back(lexNumber()); continue;
    }
    if (peek() == '"') { out.push_back(lexString()); continue; }

    auto loc = SourceLocation{file_, index_, line_, column_};
    auto start = index_;
    TokenKind kind;
    switch (take()) {
      case '+': kind = TokenKind::Plus; break;
      case '-': if (peek() == '>') { take(); kind = TokenKind::Arrow; } else kind = TokenKind::Minus; break;
      case '*': kind = TokenKind::Star; break;
      case '/': kind = TokenKind::Slash; break;
      case '%': kind = TokenKind::Percent; break;
      case '=': if (peek() == '=') { take(); kind = TokenKind::EqualEqual; } else kind = TokenKind::Equal; break;
      case '!': if (peek() == '=') { take(); kind = TokenKind::NotEqual; } else kind = TokenKind::Bang; break;
      case '<': if (peek() == '=') { take(); kind = TokenKind::LessEqual; } else kind = TokenKind::Less; break;
      case '>': if (peek() == '=') { take(); kind = TokenKind::GreaterEqual; } else kind = TokenKind::Greater; break;
      case '&': if (peek() == '&') { take(); kind = TokenKind::AndAnd; } else throw std::runtime_error("expected '&'");
        break;
      case '|': if (peek() == '|') { take(); kind = TokenKind::OrOr; } else throw std::runtime_error("expected '|'");
        break;
      case ':': kind = TokenKind::Colon; break;
      case ',': kind = TokenKind::Comma; break;
      case '.': kind = TokenKind::Dot; break;
      case '?': kind = TokenKind::Question; break;
      case '(': kind = TokenKind::LeftParen; break;
      case ')': kind = TokenKind::RightParen; break;
      case '{': kind = TokenKind::LeftBrace; break;
      case '}': kind = TokenKind::RightBrace; break;
      case '[': kind = TokenKind::LeftBracket; break;
      case ']': kind = TokenKind::RightBracket; break;
      case ';': kind = TokenKind::Semicolon; break;
      default: throw std::runtime_error("unexpected character in source");
    }
    out.push_back(make(kind, start, std::move(loc)));
  }
  out.push_back(Token{TokenKind::Eof, "", SourceLocation{file_, index_, line_, column_}});
  return out;
}

} // namespace ac

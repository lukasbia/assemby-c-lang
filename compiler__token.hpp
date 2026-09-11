#pragma once
#include <cstddef>
#include <string>
#include <unordered_map>

namespace ac {

enum class TokenKind {
  Eof, Identifier, Integer, Float, String,
  Plus, Minus, Star, Slash, Percent,
  Equal, EqualEqual, NotEqual, Less, LessEqual, Greater, GreaterEqual,
  AndAnd, OrOr, Bang,
  Colon, Comma, Dot, Arrow, Question,
  LeftParen, RightParen, LeftBrace, RightBrace, LeftBracket, RightBracket,
  Semicolon,
#define AC_KEYWORD(name, spelling) name,
#include "keywords.def"
#undef AC_KEYWORD
};

struct SourceLocation {
  std::string file;
  std::size_t offset = 0;
  std::size_t line = 1;
  std::size_t column = 1;
};

struct Token {
  TokenKind kind;
  std::string text;
  SourceLocation location;
};

const char* tokenName(TokenKind kind);
const std::unordered_map<std::string, TokenKind>& keywordMap();

} // namespace ac

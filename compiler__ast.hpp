#pragma once
#include "token.hpp"
#include <memory>
#include <string>
#include <vector>

namespace ac {

struct Expr { virtual ~Expr() = default; };
using ExprPtr = std::unique_ptr<Expr>;

struct IntegerExpr : Expr { long long value; explicit IntegerExpr(long long v):value(v){} };
struct FloatExpr : Expr { double value; explicit FloatExpr(double v):value(v){} };
struct StringExpr : Expr { std::string value; explicit StringExpr(std::string v):value(std::move(v)){} };
struct BoolExpr : Expr { bool value; explicit BoolExpr(bool v):value(v){} };
struct NilExpr : Expr {};
struct NameExpr : Expr { std::string name; explicit NameExpr(std::string n):name(std::move(n)){} };

struct BinaryExpr : Expr {
  TokenKind op; ExprPtr lhs, rhs;
  BinaryExpr(TokenKind o, ExprPtr a, ExprPtr b):op(o),lhs(std::move(a)),rhs(std::move(b)){}
};

struct CallExpr : Expr {
  ExprPtr callee; std::vector<ExprPtr> args;
  CallExpr(ExprPtr c, std::vector<ExprPtr> a):callee(std::move(c)),args(std::move(a)){}
};

struct Stmt { virtual ~Stmt() = default; };
using StmtPtr = std::unique_ptr<Stmt>;

struct ExprStmt : Stmt { ExprPtr expr; explicit ExprStmt(ExprPtr e):expr(std::move(e)){} };
struct ReturnStmt : Stmt { ExprPtr expr; explicit ReturnStmt(ExprPtr e):expr(std::move(e)){} };

struct VarDeclStmt : Stmt {
  bool mutableValue; std::string name, typeName; ExprPtr initializer;
  VarDeclStmt(bool m, std::string n, std::string t, ExprPtr i)
    :mutableValue(m),name(std::move(n)),typeName(std::move(t)),initializer(std::move(i)){}
};

struct AssignStmt : Stmt {
  std::string name; ExprPtr value;
  AssignStmt(std::string n, ExprPtr v):name(std::move(n)),value(std::move(v)){}
};

struct IfStmt : Stmt {
  ExprPtr condition; std::vector<StmtPtr> thenBody, elseBody;
};

struct FunctionDecl {
  std::string name, returnType;
  std::vector<std::pair<std::string,std::string>> params;
  std::vector<StmtPtr> body;
};

struct SourceFile {
  std::string moduleName;
  std::vector<std::string> imports;
  std::vector<FunctionDecl> functions;
};

} // namespace ac

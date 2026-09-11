#pragma once
#include "ast.hpp"
#include <string>
#include <vector>

namespace ac {

struct ACILInstruction {
  std::string result;
  std::string opcode;
  std::vector<std::string> operands;
};

struct ACILFunction {
  std::string name;
  std::string returnType;
  std::vector<std::string> params;
  std::vector<ACILInstruction> instructions;
};

struct ACILModule {
  std::string name;
  std::vector<ACILFunction> functions;
};

class ACILGenerator {
public:
  ACILModule generate(const SourceFile& source);
private:
  int temp_=0;
  std::string emitExpr(const Expr*, ACILFunction&, std::vector<std::pair<std::string,std::string>>& env);
};

class ACILVerifier {
public:
  bool verify(const ACILModule&, std::string& error) const;
};

class ACILOptimizer {
public:
  void optimize(ACILModule&) const;
};

std::string printACIL(const ACILModule&);

} // namespace ac

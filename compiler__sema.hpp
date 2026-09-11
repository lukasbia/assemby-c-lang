#pragma once
#include "ast.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace ac {

struct Diagnostic {
  std::string message;
};

class SemanticAnalyzer {
public:
  std::vector<Diagnostic> analyze(const SourceFile& file);
private:
  std::string typeOf(const Expr* e, const std::unordered_map<std::string,std::string>& env,
                    const SourceFile& file);
  std::vector<Diagnostic> diagnostics_;
};

} // namespace ac

#include "sema.hpp"
#include <unordered_set>

namespace ac {

static bool numeric(const std::string& t) {
  return t=="Int8"||t=="Int16"||t=="Int32"||t=="Int64"||t=="UInt8"||t=="UInt16"||t=="UInt32"||t=="UInt64"||t=="Int"||t=="UInt"||t=="Float32"||t=="Float64";
}

std::string SemanticAnalyzer::typeOf(const Expr* e,const std::unordered_map<std::string,std::string>& env,const SourceFile& file) {
  if(dynamic_cast<const IntegerExpr*>(e)) return "Int32";
  if(dynamic_cast<const FloatExpr*>(e)) return "Float64";
  if(dynamic_cast<const StringExpr*>(e)) return "String";
  if(dynamic_cast<const BoolExpr*>(e)) return "Bool";
  if(dynamic_cast<const NilExpr*>(e)) return "Nil";
  if(auto n=dynamic_cast<const NameExpr*>(e)){
    auto it=env.find(n->name);
    if(it==env.end()) { diagnostics_.push_back({"unknown name '"+n->name+"'"}); return "<error>"; }
    return it->second;
  }
  if(auto b=dynamic_cast<const BinaryExpr*>(e)){
    auto a=typeOf(b->lhs.get(),env,file), c=typeOf(b->rhs.get(),env,file);
    if(a=="<error>"||c=="<error>") return "<error>";
    if(numeric(a)&&numeric(c)) return (a==c)?a:"Int32";
    if(a==c && (b->op==TokenKind::EqualEqual||b->op==TokenKind::NotEqual)) return "Bool";
    diagnostics_.push_back({"invalid operands for binary expression: "+a+" and "+c});
    return "<error>";
  }
  if(auto call=dynamic_cast<const CallExpr*>(e)){
    if(auto n=dynamic_cast<const NameExpr*>(call->callee.get())){
      for(auto& fn:file.functions) if(fn.name==n->name) return fn.returnType;
      diagnostics_.push_back({"unknown function '"+n->name+"'"});
      return "<error>";
    }
    diagnostics_.push_back({"callee is not callable"});
    return "<error>";
  }
  return "<error>";
}

std::vector<Diagnostic> SemanticAnalyzer::analyze(const SourceFile& file) {
  diagnostics_.clear();
  std::unordered_set<std::string> names;
  for(const auto& fn:file.functions){
    if(!names.insert(fn.name).second) diagnostics_.push_back({"duplicate function '"+fn.name+"'"});
    std::unordered_map<std::string,std::string> env;
    for(auto& p:fn.params) env[p.first]=p.second;
    for(const auto& s:fn.body){
      if(auto v=dynamic_cast<const VarDeclStmt*>(s.get())){
        auto actual=typeOf(v->initializer.get(),env,file);
        if(!v->typeName.empty() && actual!="<error>" && v->typeName!=actual)
          diagnostics_.push_back({"initializer for '"+v->name+"' has type '"+actual+"', expected '"+v->typeName+"'"});
        env[v->name]=v->typeName.empty()?actual:v->typeName;
      } else if(auto a=dynamic_cast<const AssignStmt*>(s.get())){
        auto it=env.find(a->name);
        if(it==env.end()) diagnostics_.push_back({"assignment to unknown name '"+a->name+"'"});
        else {
          auto actual=typeOf(a->value.get(),env,file);
          if(actual!="<error>" && actual!=it->second) diagnostics_.push_back({"assignment type mismatch for '"+a->name+"'"});
        }
      } else if(auto r=dynamic_cast<const ReturnStmt*>(s.get())){
        auto actual=r->expr?typeOf(r->expr.get(),env,file):"Void";
        if(actual!=fn.returnType) diagnostics_.push_back({"return type '"+actual+"' does not match '"+fn.returnType+"'"});
      } else if(auto e=dynamic_cast<const ExprStmt*>(s.get())) {
        typeOf(e->expr.get(),env,file);
      }
    }
  }
  return diagnostics_;
}

} // namespace ac

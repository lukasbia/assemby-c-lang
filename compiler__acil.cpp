#include "acil.hpp"
#include <sstream>
#include <unordered_map>

namespace ac {

static std::string llvmType(const std::string& t) {
  if(t=="Int8") return "i8"; if(t=="Int16") return "i16"; if(t=="Int32"||t=="Bool") return "i32";
  if(t=="Int64") return "i64"; if(t=="Float32") return "float"; if(t=="Float64") return "double";
  return "ptr";
}

std::string ACILGenerator::emitExpr(const Expr* e, ACILFunction& f, std::vector<std::pair<std::string,std::string>>& env) {
  if(auto i=dynamic_cast<const IntegerExpr*>(e)){
    std::string r="%"+std::to_string(temp_++); f.instructions.push_back({r,"integer_literal",{std::to_string(i->value)}}); return r;
  }
  if(auto n=dynamic_cast<const NameExpr*>(e)){
    for(auto it=env.rbegin();it!=env.rend();++it) if(it->first==n->name) return it->second;
    return "%unknown";
  }
  if(auto b=dynamic_cast<const BinaryExpr*>(e)){
    auto a=emitExpr(b->lhs.get(),f,env), c=emitExpr(b->rhs.get(),f,env);
    std::string op;
    switch(b->op){case TokenKind::Plus:op="add";break;case TokenKind::Minus:op="sub";break;case TokenKind::Star:op="mul";break;case TokenKind::Slash:op="sdiv";break;case TokenKind::Percent:op="srem";break;default:op="compare";break;}
    std::string r="%"+std::to_string(temp_++);
    f.instructions.push_back({r,op,{a,c}}); return r;
  }
  if(auto s=dynamic_cast<const StringExpr*>(e)){
    std::string r="%"+std::to_string(temp_++); f.instructions.push_back({r,"string_literal",{s->value}}); return r;
  }
  if(auto call=dynamic_cast<const CallExpr*>(e)){
    std::vector<std::string> args;
    for(auto& a:call->args) args.push_back(emitExpr(a.get(),f,env));
    std::string callee="unknown";
    if(auto n=dynamic_cast<const NameExpr*>(call->callee.get())) callee=n->name;
    std::string r="%"+std::to_string(temp_++);
    f.instructions.push_back({r,"apply",{callee}}); f.instructions.back().operands.insert(f.instructions.back().operands.end(),args.begin(),args.end());
    return r;
  }
  return "%undef";
}

ACILModule ACILGenerator::generate(const SourceFile& source) {
  ACILModule m; m.name=source.moduleName.empty()?"Main":source.moduleName;
  for(const auto& src:source.functions){
    ACILFunction f; f.name=src.name; f.returnType=src.returnType;
    std::vector<std::pair<std::string,std::string>> env;
    for(auto& p:src.params){ auto v="%"+p.first; f.params.push_back(v); env.push_back({p.first,v}); }
    f.instructions.push_back({"","entry",{}});
    for(const auto& s:src.body){
      if(auto v=dynamic_cast<const VarDeclStmt*>(s.get())){
        auto val=emitExpr(v->initializer.get(),f,env);
        auto local="%"+v->name; f.instructions.push_back({local,"bind",{val}}); env.push_back({v->name,local});
      } else if(auto a=dynamic_cast<const AssignStmt*>(s.get())){
        auto val=emitExpr(a->value.get(),f,env);
        std::string local;
        for(auto it=env.rbegin();it!=env.rend();++it) if(it->first==a->name){local=it->second;break;}
        f.instructions.push_back({local,"assign",{val}});
      } else if(auto r=dynamic_cast<const ReturnStmt*>(s.get())){
        auto val=r->expr?emitExpr(r->expr.get(),f,env):"";
        f.instructions.push_back({"","return",val.empty()?std::vector<std::string>{}:std::vector<std::string>{val}});
      } else if(auto e=dynamic_cast<const ExprStmt*>(s.get())) emitExpr(e->expr.get(),f,env);
    }
    m.functions.push_back(std::move(f));
  }
  return m;
}

bool ACILVerifier::verify(const ACILModule& m,std::string& error) const {
  for(const auto& f:m.functions){
    bool returned=f.returnType=="Void";
    for(const auto& i:f.instructions) if(i.opcode=="return") returned=true;
    if(!returned){ error="function '"+f.name+"' has no return"; return false; }
  }
  return true;
}

void ACILOptimizer::optimize(ACILModule& m) const {
  for(auto& f:m.functions){
    std::vector<ACILInstruction> out;
    for(auto& i:f.instructions){
      if(i.opcode=="bind" && i.operands.size()==1 && i.result==i.operands[0]) continue;
      out.push_back(std::move(i));
    }
    f.instructions=std::move(out);
  }
}

std::string printACIL(const ACILModule& m) {
  std::ostringstream o;
  o<<"acil_module "<<m.name<<"\n";
  for(const auto& f:m.functions){
    o<<"acil_function "<<f.name<<" -> "<<f.returnType<<" {\n";
    for(const auto& i:f.instructions){
      o<<"  "; if(!i.result.empty()) o<<i.result<<" = "; o<<i.opcode;
      for(auto& x:i.operands) o<<" "<<x;
      o<<"\n";
    }
    o<<"}\n";
  }
  return o.str();
}

} // namespace ac

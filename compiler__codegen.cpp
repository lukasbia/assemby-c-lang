#include "codegen.hpp"
#include <sstream>
#include <unordered_map>

namespace ac {
static std::string esc(const std::string& s) {
  std::string out;
  for(char c:s){ if(c=='"') out+="\\22"; else if(c=='\n') out+="\\0A"; else out+=c; }
  return out;
}

std::string emitLLVM(const ACILModule& m) {
  std::ostringstream o;
  o<<"; ModuleID = '"<<m.name<<"'\n";
  o<<"source_filename = \""<<m.name<<".ac\"\n\n";
  for(const auto& f:m.functions){
    std::string ret=f.returnType=="Void"?"void":"i32";
    o<<"define "<<ret<<" @"<<f.name<<"(";
    for(std::size_t i=0;i<f.params.size();++i){ if(i) o<<", "; o<<"i32 "<<f.params[i]; }
    o<<") {\nentry:\n";
    std::unordered_map<std::string,std::string> value;
    for(auto& p:f.params) value[p]=p;
    for(const auto& i:f.instructions){
      if(i.opcode=="entry") continue;
      if(i.opcode=="integer_literal"){ o<<"  "<<i.result<<" = add i32 0, "<<i.operands[0]<<"\n"; value[i.result]=i.result; }
      else if(i.opcode=="add"||i.opcode=="sub"||i.opcode=="mul"||i.opcode=="sdiv"||i.opcode=="srem"){
        o<<"  "<<i.result<<" = "<<i.opcode<<" i32 "<<i.operands[0]<<", "<<i.operands[1]<<"\n"; value[i.result]=i.result;
      } else if(i.opcode=="bind"||i.opcode=="assign") {
        if(!i.operands.empty()) value[i.result]=i.operands[0];
      } else if(i.opcode=="return") {
        if(ret=="void") o<<"  ret void\n";
        else if(i.operands.empty()) o<<"  ret i32 0\n";
        else o<<"  ret i32 "<<i.operands[0]<<"\n";
      } else if(i.opcode=="string_literal") {
        o<<"  "<<i.result<<" = getelementptr inbounds ([1 x i8], ptr null, i32 0, i32 0) ; string literal: "<<esc(i.operands[0])<<"\n";
      } else if(i.opcode=="apply") {
        o<<"  "<<i.result<<" = call i32 @"<<i.operands[0]<<"()\n";
      }
    }
    o<<"}\n\n";
  }
  return o.str();
}
} // namespace ac

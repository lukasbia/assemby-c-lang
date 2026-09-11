#include "lexer.hpp"
#include "parser.hpp"
#include "sema.hpp"
#include "acil.hpp"
#include "codegen.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static std::string readFile(const std::string& path){
  std::ifstream in(path);
  if(!in) throw std::runtime_error("cannot open '"+path+"'");
  std::ostringstream s; s<<in.rdbuf(); return s.str();
}

int main(int argc,char** argv){
  if(argc<2){
    std::cerr<<"usage: assenblyc <file.ac> [--check|--emit-acil|--emit-llvm]\n";
    return 2;
  }
  std::string input=argv[1], mode="--emit-llvm";
  for(int i=2;i<argc;++i) mode=argv[i];
  try{
    ac::Lexer lexer(readFile(input),input);
    auto tokens=lexer.lex();
    ac::Parser parser(tokens);
    auto source=parser.parse();
    ac::SemanticAnalyzer sema;
    auto diagnostics=sema.analyze(source);
    if(!diagnostics.empty()){
      for(auto& d:diagnostics) std::cerr<<"error: "<<d.message<<"\n";
      return 1;
    }
    ac::ACILGenerator gen;
    auto acil=gen.generate(source);
    std::string error;
    ac::ACILVerifier verifier;
    if(!verifier.verify(acil,error)){ std::cerr<<"error: ACIL verification failed: "<<error<<"\n"; return 1; }
    ac::ACILOptimizer().optimize(acil);
    if(mode=="--check"){ std::cout<<"OK\n"; return 0; }
    if(mode=="--emit-acil"){ std::cout<<ac::printACIL(acil); return 0; }
    if(mode=="--emit-llvm"){ std::cout<<ac::emitLLVM(acil); return 0; }
    std::cerr<<"unknown mode: "<<mode<<"\n"; return 2;
  }catch(const std::exception& e){
    std::cerr<<"error: "<<e.what()<<"\n";
    return 1;
  }
}

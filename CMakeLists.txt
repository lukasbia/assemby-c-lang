cmake_minimum_required(VERSION 3.20)
project(assenblyc LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ASSENBLYC_ENABLE_LLVM "Enable LLVM backend when LLVM is installed" OFF)

add_executable(assenblyc
  compiler/driver/main.cpp
  compiler/frontend/lexer/lexer.cpp
  compiler/frontend/parser/parser.cpp
  compiler/frontend/sema/sema.cpp
  compiler/middleend/acil/acil.cpp
  compiler/backend/codegen/codegen.cpp
)
target_include_directories(assenblyc PRIVATE compiler)

if(ASSENBLYC_ENABLE_LLVM)
  find_package(LLVM CONFIG REQUIRED)
  target_include_directories(assenblyc PRIVATE ${LLVM_INCLUDE_DIRS})
  target_compile_definitions(assenblyc PRIVATE ASSENBLYC_HAS_LLVM=1)
  target_link_libraries(assenblyc PRIVATE LLVM)
endif()

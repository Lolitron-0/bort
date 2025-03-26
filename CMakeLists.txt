cmake_minimum_required(VERSION 3.22)
project(bort)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
    src/CLI/Main.cpp
    src/CLI/IO.cpp
    src/Frontend/SourceFile.cpp
    src/Frontend/FrontendInstance.cpp
    src/Frontend/Symbol.cpp
    src/Frontend/Type.cpp
    src/Lex/Lexer.cpp
    src/Lex/Preprocessor.cpp
    src/Lex/Token.cpp
    src/Lex/Utils.cpp
    src/Parse/Parser.cpp
    src/AST/ASTNode.cpp
    src/AST/BinOpExpr.cpp
    src/AST/UnaryOpExpr.cpp
    src/AST/Block.cpp
    src/AST/ExpressionNode.cpp
    src/AST/FunctionDecl.cpp
    src/AST/NumberExpr.cpp
    src/AST/VarDecl.cpp
    src/AST/VariableExpr.cpp
    src/AST/Visitors/ASTVisitor.cpp
    src/AST/Visitors/SymbolResolutionVisitor.cpp
    src/AST/Visitors/TypePropagationVisitor.cpp
    src/AST/Visitors/NodeSubstitutionVisitor.cpp
    src/AST/Visitors/ASTPrinter.cpp
    src/IR/MiddleEndInstance.cpp
    src/IR/Value.cpp
    src/IR/VariableUse.cpp
    src/IR/Instruction.cpp
    src/IR/IRCodegen.cpp
    src/IR/IRPrinter.cpp
    src/IR/BasicBlock.cpp
    src/Codegen/RISCVCodegen.cpp
    src/Codegen/RISCVPrinter.cpp
    src/Codegen/InstructionVisitor.cpp
    src/Codegen/Utils.cpp
    src/Codegen/ValueLoc.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

# Dependencies
include(FetchContent)

# cxxopts
message(STATUS "Fetching dependency: cxxopts")
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts
  GIT_TAG 3bf268481da8208d171d8908e6491459de3651d7) # 3.2.0
FetchContent_MakeAvailable(cxxopts)
target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts)

# fmt
message(STATUS "Fetching dependency: fmt")
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 0c9fce2ffefecfdce794e1859584e25877b7b592) # 11.0.2
FetchContent_MakeAvailable(fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

# cul
message(STATUS "Fetching dependency: cul")
FetchContent_Declare(
  cul
  GIT_REPOSITORY https://github.com/Lolitron-0/cul.git
  GIT_TAG c316615e9a5fb3535293082fb187856da01ea492)
FetchContent_MakeAvailable(cul)
target_link_libraries(${PROJECT_NAME} PRIVATE cul)

# frozen
message(STATUS "Fetching dependency: frozen")
FetchContent_Declare(
  frozen
  GIT_REPOSITORY https://github.com/serge-sans-paille/frozen
  GIT_TAG 06bee5321cecd9a9ffaceb164b5adb2ffafae781)
FetchContent_MakeAvailable(frozen)
target_link_libraries(${PROJECT_NAME} PRIVATE frozen)

# boost
message(STATUS "Fetching dependency: boost (may take a while)")
add_compile_definitions(BOOST_ENABLE_ASSERT_DEBUG_HANDLER)

set(BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES functional;assert;stacktrace;range)
FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.tar.xz
      # DOWNLOAD_EXTRACT_TIMESTAMP ON
)

FetchContent_MakeAvailable(Boost)

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::functional Boost::assert
                                              Boost::stacktrace Boost::range)

add_subdirectory(tests)

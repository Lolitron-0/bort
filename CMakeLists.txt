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
    src/AST/Block.cpp
    src/AST/ExpressionNode.cpp
    src/AST/FunctionDecl.cpp
    src/AST/NumberExpr.cpp
    src/AST/VarDecl.cpp
    src/AST/VariableExpr.cpp
    src/AST/Visitors/ASTVisitor.cpp
    src/AST/Visitors/SymbolResolutionVisitor.cpp
    src/AST/Visitors/ASTPrinter.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

# Dependencies
include(FetchContent)

# cxxopts
add_subdirectory(thirdparty/cxxopts)
target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts)

# fmt
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 0c9fce2ffefecfdce794e1859584e25877b7b592) # 11.0.2
FetchContent_MakeAvailable(fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

# cul
FetchContent_Declare(
  cul
  GIT_REPOSITORY https://github.com/Lolitron-0/cul.git
  GIT_TAG c316615e9a5fb3535293082fb187856da01ea492)
FetchContent_MakeAvailable(cul)
target_link_libraries(${PROJECT_NAME} PRIVATE cul)

# frozen
FetchContent_Declare(
  frozen
  GIT_REPOSITORY https://github.com/serge-sans-paille/frozen
  GIT_TAG 06bee5321cecd9a9ffaceb164b5adb2ffafae781)
FetchContent_MakeAvailable(frozen)
target_link_libraries(${PROJECT_NAME} PRIVATE frozen)

# boost
message(STATUS "Fetching Boost (may take a while)")
add_compile_definitions(BOOST_ENABLE_ASSERT_DEBUG_HANDLER
                        BOOST_STACKTRACE_USE_ADDR2LINE)

if(${CMAKE_MINOR_VERSION} LESS 28)
  message(WARNING "CMake version less than
3.28 (${CMAKE_VERSION}), building entire Boost")
  FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    URL_MD5 893b5203b862eb9bbd08553e24ff146a
    DOWNLOAD_EXTRACT_TIMESTAMP ON)
else()
  FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    URL_MD5 893b5203b862eb9bbd08553e24ff146a
    DOWNLOAD_EXTRACT_TIMESTAMP ON EXCLUDE_FROM_ALL)

endif()

set(BOOST_INCLUDE_LIBRARIES functional;assert;stacktrace;range)
FetchContent_MakeAvailable(Boost)

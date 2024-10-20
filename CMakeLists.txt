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
  src/Lex/Lexer.cpp
  src/Lex/Preprocessor.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# Dependencies
include(FetchContent)

#cxxopts
add_subdirectory(thirdparty/cxxopts)
target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts)

# fmt
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        0c9fce2ffefecfdce794e1859584e25877b7b592) # 11.0.2
FetchContent_MakeAvailable(fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

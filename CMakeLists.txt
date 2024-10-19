cmake_minimum_required(VERSION 3.22)
project(bort)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES 
  src/CLI/Main.cpp
  src/CLI/IO.cpp
  src/Frontend/InputFile.cpp
  src/Frontend/FrontendInstance.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# Dependencies
add_subdirectory(thirdparty/cxxopts)
target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts)
target_compile_options(cxxopts INTERFACE -Wno-everything) # little workaround

# for header only
target_include_directories(${PROJECT_NAME} PRIVATE thirdparty)

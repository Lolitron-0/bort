cmake_minimum_required(VERSION 3.22)
project(bort)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-Wall)
endif()

include_directories(include)

set(SOURCES 
  src/CLI/Main.cpp
  src/CLI/IO.cpp
  src/Frontend/InputFile.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Dependencies
add_subdirectory(thirdparty/cxxopts)
target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts)

# for header only
target_include_directories(${PROJECT_NAME} PRIVATE thirdparty)

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
  src/Frontend/File.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Dependencies

find_package(Hana REQUIRED)
target_link_libraries(${PROJECT_NAME} hana)


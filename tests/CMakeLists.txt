set(PARENT_PROJECT_NAME ${PROJECT_NAME})
project(${PARENT_PROJECT_NAME}_tests)

include(FetchContent)

enable_testing()

# # Unit tests FetchContent_Declare( googletest URL
# https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
# DOWNLOAD_EXTRACT_TIMESTAMP ON) FetchContent_MakeAvailable(googletest)
#
# set(UNIT_PROJECT_NAME ${PROJECT_NAME}_unit)
# add_executable(${UNIT_PROJECT_NAME} gtest.cpp)
# target_link_libraries(${UNIT_PROJECT_NAME} PRIVATE ${PARENT_PROJECT_NAME} cul)
# target_link_libraries(${UNIT_PROJECT_NAME} PRIVATE GTest::gtest_main
#
# include(GoogleTest) gtest_discover_tests(${UNIT_PROJECT_NAME})

find_package(Python REQUIRED COMPONENTS Interpreter)

set(SMOKE_PROJECT_NAME ${PROJECT_NAME}_smoke)
add_test(NAME ${SMOKE_PROJECT_NAME}
         COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/smoke.py
                 ${CMAKE_BINARY_DIR})

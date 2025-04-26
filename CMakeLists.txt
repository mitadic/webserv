# NOTE: The main `webserv` executable is built using the Makefile.
# CMake is only used for building and running tests.

cmake_minimum_required(VERSION 3.10)
project(WebservTest)

# Enable testing
enable_testing()

# Add Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add libcurl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

add_executable(test_http_GET test/test_http_GET.cpp)
target_link_libraries(test_http_GET ${GTEST_LIBRARIES} gtest_main pthread curl)

add_executable(test_http_POST test/test_http_POST.cpp)
target_link_libraries(test_http_POST ${GTEST_LIBRARIES} gtest_main pthread curl)

add_executable(test_http_DELETE test/test_http_DELETE.cpp)
target_link_libraries(test_http_DELETE ${GTEST_LIBRARIES} gtest_main pthread curl)

# Register the test targets with CTest
add_test(NAME test_http_GET COMMAND test_http_GET)
add_test(NAME test_http_POST COMMAND test_http_POST)
add_test(NAME test_http_DELETE COMMAND test_http_DELETE)

set_tests_properties(test_http_GET PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_tests_properties(test_http_POST PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_tests_properties(test_http_DELETE PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
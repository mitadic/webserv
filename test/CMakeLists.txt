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

# Add the test executable
add_executable(test_utils
    test/test_utils.cpp
    src/Utils.cpp
    src/ServerBlock.cpp
    src/Request.cpp
    src/Location.cpp
    src/Log.cpp
    src/Config.cpp
    src/ConfigUtils.cpp
    src/RequestParser.cpp
    src/RequestUtils.cpp
    src/Exceptions.cpp
    src/ContentTypes.cpp
    src/CgiHandler.cpp
    src/StatusCodes.cpp
    src/HttpHeaders.cpp
)
target_include_directories(test_utils PRIVATE ${CMAKE_SOURCE_DIR}/incl) # Add incl/ for test_utils
target_link_libraries(test_utils gtest gtest_main pthread)

add_executable(test_http_GET test/test_http_GET.cpp)
target_link_libraries(test_http_GET ${GTEST_LIBRARIES} gtest_main pthread curl)

add_executable(test_http_POST test/test_http_POST.cpp)
target_link_libraries(test_http_POST ${GTEST_LIBRARIES} gtest_main pthread curl)

add_executable(test_http_DELETE test/test_http_DELETE.cpp)
target_link_libraries(test_http_DELETE ${GTEST_LIBRARIES} gtest_main pthread curl)

# Register the test targets with CTest
add_test(NAME test_utils COMMAND test_utils)
add_test(NAME test_http_GET COMMAND test_http_GET)
add_test(NAME test_http_POST COMMAND test_http_POST)
add_test(NAME test_http_DELETE COMMAND test_http_DELETE)

set_tests_properties(test_http_GET PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_tests_properties(test_http_POST PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_tests_properties(test_http_DELETE PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
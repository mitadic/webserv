# NOTE: The main `webserv` executable is built using the Makefile.
# CMake is only used for building and running tests.

cmake_minimum_required(VERSION 3.10)
project(WebservTest)

# Enable testing
enable_testing()

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

# Register the test targets with CTest
add_test(NAME test_utils COMMAND test_utils)
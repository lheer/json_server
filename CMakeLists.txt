cmake_minimum_required(VERSION 3.18)
project(json_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(fmt REQUIRED)

option(WITH_TESTS "Build tests" OFF)
option(SOCKPP_BUILD_SHARED "" OFF)
option(SOCKPP_BUILD_STATIC "" ON)
add_subdirectory(externals/sockpp)

add_library(${PROJECT_NAME} STATIC
    json_server.cpp
    json_server.hpp
    json_client.cpp
    json_client.hpp
    exceptions.hpp
    details.hpp
    externals/nlohmann/json.hpp
    externals/nlohmann/json_fwd.hpp
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
target_link_options(${PROJECT_NAME} PRIVATE)
target_include_directories(${PROJECT_NAME} PRIVATE externals externals/sockpp/include)
target_link_libraries(${PROJECT_NAME} PRIVATE sockpp-static fmt::fmt)

if (WITH_TESTS)
add_subdirectory(test)
endif(WITH_TESTS)

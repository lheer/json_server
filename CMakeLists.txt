cmake_minimum_required(VERSION 3.18)
project(main LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(fmt REQUIRED)

option(SOCKPP_BUILD_SHARED "" OFF)
option(SOCKPP_BUILD_STATIC "" ON)
add_subdirectory(externals/sockpp)

add_executable(${PROJECT_NAME}
    main.cpp
    json_server.cpp
    json_server.hpp
    json_client.cpp
    json_client.hpp
    exceptions.hpp
    details.hpp
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
target_link_options(${PROJECT_NAME} PRIVATE)
target_include_directories(${PROJECT_NAME} PRIVATE externals externals/sockpp/include)
target_link_libraries(${PROJECT_NAME} PRIVATE sockpp-static fmt::fmt)

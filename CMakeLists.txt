cmake_minimum_required(VERSION 3.18)

set(namespace "lh")

project(json_server
    LANGUAGES CXX
    VERSION 0.2.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

option(WITH_TESTS "Build tests" OFF)
option(WITH_GPROF "Build with gprof enabled" OFF)


add_library(${PROJECT_NAME} STATIC)
add_library(lh::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
    src/json_server.cpp
    src/json_client.cpp
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${json_server_VERSION}
    SOVERSION ${json_server_VERSION_MAJOR}
)

FetchContent_Declare(sockpp
    GIT_REPOSITORY https://github.com/fpagliughi/sockpp.git
    GIT_TAG v0.8.1
)
FetchContent_MakeAvailable(sockpp)

FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.1.1
)
# Exclude fmt from install target
FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
  FetchContent_Populate(fmt)
  add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/externals
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(${PROJECT_NAME} PRIVATE sockpp fmt)

# Public headers to install (used in Install.cmake)
set(public_headers
    include/json_server.hpp
    include/json_client.hpp
    include/details.hpp
    include/exceptions.hpp
    externals/nlohmann/json_fwd.hpp
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)

# where to find our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Install)

if (WITH_GPROF)
    target_compile_options(${PROJECT_NAME} PRIVATE -pg)
    target_link_options(${PROJECT_NAME} PRIVATE -pg)
endif(WITH_GPROF)

if (WITH_TESTS)
    set(TEST_EXEC_NAME json_server_test)
    add_executable(${TEST_EXEC_NAME} test/test.cpp)
    target_link_libraries(${TEST_EXEC_NAME} PRIVATE ${PROJECT_NAME} sockpp fmt)
    target_include_directories(${TEST_EXEC_NAME} PRIVATE include externals)

    add_custom_command(TARGET ${TEST_EXEC_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/test/test_data.json $<TARGET_FILE_DIR:json_server>)

endif(WITH_TESTS)

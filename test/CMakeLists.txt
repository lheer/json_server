
add_executable(test test.cpp)
target_include_directories(test PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/externals
    ${CMAKE_SOURCE_DIR}/externals/sockpp/include
    ${CMAKE_SOURCE_DIR}/externals/utest
)
target_link_libraries(test json_server)

if (WITH_GPROF)
    target_compile_options(test PRIVATE -pg)
    target_link_options(test PRIVATE -pg)
endif(WITH_GPROF)

add_custom_command(TARGET test POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/test/test_data.json $<TARGET_FILE_DIR:json_server>
)

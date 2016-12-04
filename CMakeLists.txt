cmake_minimum_required (VERSION 2.6)
project (json_parse_test C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
endif()

add_library(json_parse JsonParser.c)
add_executable(json_parse_test test.c)
target_link_libraries(json_parse_test json_parse)

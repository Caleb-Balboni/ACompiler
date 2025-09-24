cmake_minimum_required(VERSION 3.16)
project(ACompiler)

add_library(tokenizer src/tokenizer/tokenizer.c)
add_library(utils     src/utils/hashtable.c)

target_include_directories(tokenizer PUBLIC ${CMAKE_SOURCE_DIR}/include) 
target_include_directories(utils     PUBLIC ${CMAKE_SOURCE_DIR}/include) 
add_executable(ACompiler src/main.c)
target_link_libraries(ACompiler PRIVATE tokenizer utils)

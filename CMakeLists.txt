cmake_minimum_required(VERSION 3.16)
project(ACompiler)
add_executable(ACompiler src/main.c)

add_library(tokenizer src/tokenizer/tokenizer.c)
target_include_directories(tokenizer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/tokenizer)
target_link_libraries(ACompiler PRIVATE tokenizer)


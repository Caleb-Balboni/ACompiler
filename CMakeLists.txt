cmake_minimum_required(VERSION 3.16)
project(ACompiler)
include(CTest)
add_library(tokenizer src/tokenizer/tokenizer.c)
add_library(parser src/parser/parser.c)
add_library(errors src/errors/errors.c)
add_library(utils     src/utils/hashtable.c src/utils/arraylist.c src/utils/stack.c)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS, "${CMAKE_C_FLAGS} -g")
target_include_directories(tokenizer PUBLIC ${CMAKE_SOURCE_DIR}/include) 
target_include_directories(utils     PUBLIC ${CMAKE_SOURCE_DIR}/include) 
target_include_directories(parser PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(errors PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_executable(ACompiler src/main.c)
target_link_libraries(ACompiler PRIVATE tokenizer utils parser errors)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CRITERION REQUIRED criterion)

add_executable(test_all
  test/test_utils.c
  # add more test .c files here
)

target_include_directories(test_all PRIVATE
  ${CRITERION_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_all PRIVATE
  ${CRITERION_LIBRARIES}
  tokenizer utils   # if your tests use them
)

add_test(NAME all_tests COMMAND test_all)

cmake_minimum_required(VERSION 3.16)
project(ACompiler)
include(CTest)
add_library(tokenizer src/tokenizer/tokenizer.c)
add_library(parser src/parser/parser.c)
add_library(errors src/errors/errors.c)
add_library(asm src/assembler/assembler.c src/assembler/emitter.c)
add_library(utils     src/utils/hashtable.c src/utils/arraylist.c src/utils/stack.c)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS, "${CMAKE_C_FLAGS} -g")
target_include_directories(tokenizer PUBLIC ${CMAKE_SOURCE_DIR}/include) 
target_include_directories(utils     PUBLIC ${CMAKE_SOURCE_DIR}/include) 
target_include_directories(parser PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(errors PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(asm PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_executable(ACompiler src/main.c)
target_link_libraries(ACompiler PRIVATE tokenizer utils parser errors asm)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CRITERION REQUIRED criterion)

add_executable(test_all
  test/test_utils.c
  test/test_utils_extended.c
  test/test_tokenizer.c
  test/test_tokenizer_extended.c
  test/test_emitter.c
  test/test_emitter_extended.c
  test/test_parser.c
  test/test_integration.c
)

target_include_directories(test_all PRIVATE
  ${CRITERION_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_all PRIVATE
  ${CRITERION_LIBRARIES}
  tokenizer utils asm parser errors
)

add_test(NAME all_tests COMMAND test_all)

# Documentation target
find_program(DOXYGEN_EXECUTABLE doxygen)
if(DOXYGEN_EXECUTABLE)
  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
endif()

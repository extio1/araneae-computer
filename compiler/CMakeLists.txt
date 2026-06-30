cmake_minimum_required(VERSION 3.28)
project(AraneaeCompiler VERSION 0.0.1 DESCRIPTION "Yet Another Compiler")

set(ARANEAE_COMPILER_LIB_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(ARANEAE_COMPILER_SOURCE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(ARANEAE_COMPILER_BINARY_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")

file(MAKE_DIRECTORY ${ARANEAE_COMPILER_BINARY_INCLUDE_DIR}/araneae)

add_subdirectory(external)
add_subdirectory(lib)
add_subdirectory(tools)

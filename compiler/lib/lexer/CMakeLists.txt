set(TARGET AraneaeLexer)

find_package(FLEX REQUIRED)

set(LEXER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l)
set(LEXER_OUT_C ${CMAKE_CURRENT_BINARY_DIR}/lexer.out.c)
add_custom_command(
  OUTPUT
    ${LEXER_OUT_C}
  COMMAND 
    ${FLEX_EXECUTABLE} -o ${LEXER_OUT_C} ${LEXER_SRC}
  DEPENDS 
    ${LEXER_SRC}
  COMMENT 
    "Generating lexer with Flex"
)

add_library(${TARGET} STATIC ${LEXER_OUT_C})
target_include_directories(${TARGET} 
  PRIVATE 
    ${ARANEAE_COMPILER_BINARY_INCLUDE_DIR}
    ${ARANEAE_COMPILER_SOURCE_INCLUDE_DIR}
)

set(TARGET AraneaeParser)
set(TARGET_GEN AraneaeParserGen)

find_package(BISON REQUIRED)

set(BISON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/parser.y)
set(PARSER_OUT_H_DIR ${ARANEAE_COMPILER_BINARY_INCLUDE_DIR}/araneae/parser)
set(PARSER_OUT_H ${PARSER_OUT_H_DIR}/parser.out.h)
set(PARSER_OUT_C ${CMAKE_CURRENT_BINARY_DIR}/parser.out.c)

file(MAKE_DIRECTORY ${PARSER_OUT_H_DIR})
add_custom_command(
  OUTPUT
    ${PARSER_OUT_C} 
    ${PARSER_OUT_H}
  COMMAND 
    ${BISON_EXECUTABLE} --header=${PARSER_OUT_H} -o ${PARSER_OUT_C} ${BISON_SRC}
  DEPENDS 
    ${BISON_SRC}
  COMMENT 
    "Generating parser with Bison"
)

add_library(${TARGET} STATIC ${PARSER_OUT_C} ${PARSER_OUT_H})
target_include_directories(${TARGET} 
  PRIVATE
    ${ARANEAE_COMPILER_SOURCE_INCLUDE_DIR} 
    ${PARSER_OUT_H_DIR}
)
target_link_libraries(${TARGET}
  AraneaeAst
)

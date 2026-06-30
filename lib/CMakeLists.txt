add_subdirectory(aux)
add_subdirectory(error)

add_subdirectory(ast)
add_subdirectory(cfg)

add_subdirectory(parser)
add_subdirectory(lexer)
add_dependencies(AraneaeLexer AraneaeParser)

add_subdirectory(codegen)

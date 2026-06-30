set(TARGET araneae-compiler)

set(ARANEAE_SOURSES main.c)

add_executable(${TARGET} ${ARANEAE_SOURSES})
target_include_directories(${TARGET} 
  PRIVATE 
  ${ARANEAE_COMPILER_SOURCE_INCLUDE_DIR}
)
target_link_libraries(${TARGET} 
  PRIVATE
    AraneaeErrorCollector
    AraneaeAst
    AraneaeCfg
    AraneaeCodegen
)
target_compile_options(${TARGET} PRIVATE "-Wall")

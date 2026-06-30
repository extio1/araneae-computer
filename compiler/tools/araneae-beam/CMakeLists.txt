set(TARGET araneae-beam)

add_executable(${TARGET} main.c)
target_include_directories(${TARGET} PRIVATE ${ARANEAE_COMPILER_SOURCE_INCLUDE_DIR})
target_link_libraries(${TARGET} PRIVATE
    AraneaeErrorCollector
    AraneaeAst
    AraneaeCfg
    AraneaeCodegen
)
target_compile_options(${TARGET} PRIVATE "-Wall")

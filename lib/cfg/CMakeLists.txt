set(TARGET AraneaeCfg)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
add_library(${TARGET} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${TARGET} 
  PRIVATE
    ${ARANEAE_COMPILER_SOURCE_INCLUDE_DIR}
)

target_link_libraries(${TARGET}
  PRIVATE
    AraneaeAux
    AraneaeErrorCollector
    Containers
)

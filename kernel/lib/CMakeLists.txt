set(TARGET araneae-kernel)

add_custom_target(
  ${TARGET}
)

add_dependencies(
  ${TARGET}
  araneae-compiler
)

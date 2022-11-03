set(LLVM_LINK_COMPONENTS
  Support
)

add_library(graph-builder graph-builder.h)

target_include_directories(graph-builder 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(graph-builder
  PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  clangASTMatchers
  utils
  ast
  icfg
  call
  data
)

install(TARGETS graph-builder DESTINATION bin)
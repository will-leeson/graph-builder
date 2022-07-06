set(LLVM_LINK_COMPONENTS
  Support
)

add_executable(c-graph-builder app.cpp)

target_include_directories(c-graph-builder 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(c-graph-builder
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

install(TARGETS c-graph-builder DESTINATION bin)
set(LLVM_LINK_COMPONENTS
  Support
)

add_executable(app app.cpp)

target_include_directories(app 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(app
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
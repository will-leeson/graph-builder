set(LLVM_LINK_COMPONENTS
  Support
)

add_library(ast ASTBuilder.cpp)

target_include_directories(ast 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
   

target_link_libraries(ast
  PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  clangASTMatchers
  utils
)
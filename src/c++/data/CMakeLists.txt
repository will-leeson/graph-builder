set(LLVM_LINK_COMPONENTS
  Support
)

add_library(data DataFlowBuilder.cpp)

target_include_directories(data 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..
)   

target_link_libraries(icfg
  PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  clangASTMatchers
  utils
)
set(LLVM_LINK_COMPONENTS
  Support
)


add_library(icfg ICFGBuilder.cpp)

target_include_directories(icfg 
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
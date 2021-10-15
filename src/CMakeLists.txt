set(LLVM_LINK_COMPONENTS
  Support
)
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_executable(graph-builder GraphBuilder.cpp)

target_link_libraries(graph-builder
  PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  clangASTMatchers
)
set(LLVM_LINK_COMPONENTS
  Support
)

add_library(utils utils.cpp)

target_include_directories(utils 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(utils
  PRIVATE
  clangAST
)
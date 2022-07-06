CMAKE_MINIMUM_REQUIRED(VERSION 3.19)

project(graph-builder)

set (CMAKE_CXX_STANDARD 17)

# LLVM.
find_package(LLVM 11.1 REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs Support Core Passes)

#Clang
find_package(Clang REQUIRED)
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${CLANG_DEFINITIONS})

foreach(dir ${llvm_libs})
  message(STATUS "dir='${dir}'")
endforeach()

add_subdirectory(src)
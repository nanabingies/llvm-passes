cmake_minimum_required(VERSION 3.13)
project(MyLLVMPass)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(CMAKE_BUILD_TYPE Debug)

# Set LLVM include and link flags
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Create executable
add_executable(my_pass_runner main.cpp pass.cpp)

# Link against LLVM libraries
llvm_map_components_to_libnames(llvm_libs
  Core
  IRReader
  Support
  Passes
)

target_link_libraries(my_pass_runner PRIVATE ${llvm_libs})

cmake_minimum_required(VERSION 2.8.3)
project(inc_packer)
add_compile_options(-std=c++11)

find_package(Qt5 REQUIRED COMPONENTS Core)

include_directories(
  include
  src
)

add_executable(inc_packer
  test.cpp
  tie_packer.cpp)
target_link_libraries(inc_packer
  Qt5::Core
)
cmake_minimum_required(VERSION 2.8.3)
project(file_packer)
add_compile_options(-std=c++11)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g2 -ggdb")

find_package(Qt5 REQUIRED COMPONENTS Core)

include_directories(
  include
  src
)

add_executable(tiec
  src/libtie/libtiec.cpp
  src/example/tiec.cpp)
target_link_libraries(tiec
  Qt5::Core
)

add_executable(tiex
  src/libtie/libtiex.cpp
  src/example/tiex.cpp)
target_link_libraries(tiex
  Qt5::Core
)

add_executable(tiex_file
  src/libtie/libtiex.cpp
  src/example/tiex_file.cpp)
target_link_libraries(tiex_file
  Qt5::Core
)


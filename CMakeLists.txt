cmake_minimum_required(VERSION 3.12)

project(raycast)
add_subdirectory(raylib)
include_directories(raylib/src)

add_executable(raycast main.cpp)
set_property(TARGET raycast PROPERTY CXX_STANDARD 20)

target_link_libraries(raycast raylib)

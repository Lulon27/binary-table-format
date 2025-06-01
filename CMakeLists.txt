cmake_minimum_required(VERSION 3.5)
project(BinaryTableTest)
add_executable(BinaryTableTest "test/main.cpp")

add_library(BinaryTableFormat INTERFACE)
target_include_directories(BinaryTableFormat INTERFACE include)
target_link_libraries(BinaryTableTest PUBLIC BinaryTableFormat)
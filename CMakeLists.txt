cmake_minimum_required(VERSION 3.5)
project(BinaryTableTest)

enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)
include(googletest)

add_executable(BinaryTableTest "test/main.cpp")

add_library(BinaryTableFormat INTERFACE)
target_include_directories(BinaryTableFormat INTERFACE include)
target_link_libraries(BinaryTableTest PUBLIC BinaryTableFormat GTest::gtest_main)

gtest_discover_tests(BinaryTableTest)
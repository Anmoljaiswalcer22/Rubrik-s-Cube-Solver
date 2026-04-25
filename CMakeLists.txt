cmake_minimum_required(VERSION 3.14)
project(RubiksCubeSolver CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_library(cube STATIC
  src/RubiksCube.cpp
  src/RubiksCube3dArray.cpp
  src/RubiksCube1dArray.cpp
  src/RubiksCubeBitboard.cpp
  src/CornerPatternDatabase.cpp
  src/CornerDBMaker.cpp)
target_include_directories(cube PUBLIC include)

add_executable(rubiks src/main.cpp)
target_link_libraries(rubiks cube)

add_executable(run_tests tests/test_main.cpp)
target_link_libraries(run_tests cube)

cmake_minimum_required(VERSION 3.3)
project(lazy_string)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES home_assessment_2_smoke_test.cpp)

include_directories(.)

add_executable(lazy_string ${SOURCE_FILES})

add_subdirectory(test)


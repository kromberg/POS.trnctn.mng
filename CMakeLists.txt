cmake_minimum_required(VERSION 2.6.0)

project(exchange.rate C CXX)

# set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -g -Wall -Werror")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

list(APPEND SRC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)
list(APPEND TEST_SRC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/tests)

foreach (_dir ${SRC_DIRS})
    file(GLOB_RECURSE _src ${_dir}/*.cpp)
    list(APPEND SRC_ALL ${_src})
endforeach()

foreach (_dir ${TEST_SRC_DIRS})
    file(GLOB_RECURSE _src ${_dir}/*.cpp)
    list(APPEND TEST_SRC_ALL ${_src})
endforeach()

# main app
add_executable(${PROJECT_NAME} ${SRC_ALL})
# tests
add_executable(${PROJECT_NAME}.test ${TEST_SRC_ALL})
cmake_minimum_required(VERSION 2.6.0)

project(exchange.rate C CXX)

# set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -g -Wall -Werror")
if(COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fPIC")
endif()

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

if(COVERAGE)
    set(COVERAGE_FILE coverage.info)
    set(COVERAGE_DIR coverage)

    add_custom_command(OUTPUT coverage.info
        COMMAND bash -c "lcov --capture --directory . --output-file ${COVERAGE_FILE}")
    add_custom_target(coverage
        COMMAND bash -c "genhtml ${COVERAGE_FILE} --output-directory ${COVERAGE_DIR}"
        DEPENDS coverage.info)

    add_custom_target(clean-coverage 
        #COMMAND bash -c "find . -name \"*.gcda\" -print0 | xargs -0 rm"
        COMMAND ${CMAKE_COMMAND} remove_directory ${COVERAGE_DIR}
        COMMAND ${CMAKE_COMMAND} remove ${COVERAGE_FILE})
endif()
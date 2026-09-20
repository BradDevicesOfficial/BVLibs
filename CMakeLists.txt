cmake_minimum_required(VERSION 3.16)
project(BVLibs VERSION 0.2.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(bvlib_reference STATIC src/reference.c)
target_include_directories(bvlib_reference PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(BVLIBS_BUILD_TESTS "Build the BVML/BVN test programs" ON)

if(BVLIBS_BUILD_TESTS)
    enable_testing()
    foreach(test_name bvml bvn)
        add_executable(test_${test_name} tests/test_${test_name}.c)
        target_link_libraries(test_${test_name} PRIVATE bvlib_reference)
        target_link_libraries(test_${test_name} PRIVATE m)
        add_test(NAME test_${test_name} COMMAND test_${test_name})
    endforeach()
endif()
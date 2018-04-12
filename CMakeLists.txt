cmake_minimum_required(VERSION 3.5)
project(cipells VERSION 0.0.1 LANGUAGES CXX)

add_subdirectory(external/pybind11)
add_subdirectory(external/fmt EXCLUDE_FROM_ALL)

add_library(cipells
    SHARED
    src/XYTuple.cc
    src/Interval.cc
)
target_link_libraries(cipells PRIVATE fmt::fmt-header-only)
target_include_directories(cipells
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
set_target_properties(cipells
    PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

configure_file(cipells/__init__.py cipells/__init__.py COPYONLY)

pybind11_add_module(continuous1d src/experiments/continuous1d.cc)
target_include_directories(continuous1d
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
set_target_properties(continuous1d
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY cipells/experiments
)


pybind11_add_module(_cipells
    src/python/module.cc
    src/python/XYTuple.cc
    src/python/Interval.cc
)
target_include_directories(_cipells
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
set_target_properties(_cipells
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY cipells
)
target_link_libraries(_cipells
    PRIVATE
        cipells
)


enable_testing()

configure_file(tests/continuous1d.py test_continuous1d.py COPYONLY)
add_test(NAME test_continuous1d
    COMMAND
        ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/test_continuous1d.py
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file(tests/XYTuple.py test_XYTuple.py COPYONLY)
add_test(NAME test_XYTuple
    COMMAND
        ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/test_XYTuple.py
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(test_XYTuple
    PROPERTIES
        ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}
)


configure_file(tests/Interval.py test_Interval.py COPYONLY)
add_test(NAME test_Interval
    COMMAND
        ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/test_Interval.py
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(test_Interval
    PROPERTIES
        ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}
)

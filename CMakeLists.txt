cmake_minimum_required(VERSION 3.5)
project(cipells VERSION 0.0.1 LANGUAGES CXX)

add_subdirectory(pybind11)

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

enable_testing()

configure_file(tests/continuous1d.py test_continuous1d.py COPYONLY)

add_test(NAME test_continuous1d
    COMMAND
        ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/test_continuous1d.py
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

cmake_minimum_required(VERSION 3.5)
project(cipells VERSION 0.0.1 LANGUAGES CXX)

add_subdirectory(external/pybind11)
add_subdirectory(external/fmt EXCLUDE_FROM_ALL)

add_library(cipells
    SHARED
    src/formatting.cc
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
        fmt::fmt-header-only
)

enable_testing()
configure_file(cipells/tests/__init__.py cipells/tests/__init__.py COPYONLY)

function(add_python_test TEST_NAME)
    configure_file(cipells/tests/${TEST_NAME}.py cipells/tests/${TEST_NAME}.py COPYONLY)
    add_test(NAME test_${TEST_NAME}
        COMMAND
            ${PYTHON_EXECUTABLE} -m unittest cipells.tests.${TEST_NAME}
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(test_${TEST_NAME}
        PROPERTIES
            ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction(add_python_test)

add_python_test(continuous1d)
add_python_test(XYTuple)
add_python_test(Interval)

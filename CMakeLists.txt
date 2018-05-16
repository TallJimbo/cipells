cmake_minimum_required(VERSION 3.5)
project(cipells VERSION 0.0.1 LANGUAGES CXX)

add_subdirectory(external/pybind11)
add_subdirectory(external/fmt EXCLUDE_FROM_ALL)

add_library(cipells
    SHARED
    src/formatting.cc
    src/XYTuple.cc
    src/Interval.cc
    src/Box.cc
    src/transforms.cc
    src/profiles.cc
    src/Image.cc
    src/Interpolant.cc
    src/Kernel.cc
    src/profiles.cc
)
target_link_libraries(cipells PRIVATE fmt::fmt-header-only)
target_include_directories(cipells
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:external/eigen>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/eigen>
)
set_target_properties(cipells
    PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

function(cipells_add_python MODULE_NAME)
    configure_file(cipells/${MODULE_NAME}.py cipells/${MODULE_NAME}.py COPYONLY)
endfunction(cipells_add_python)

cipells_add_python(__init__)

cipells_add_python(experiments/__init__)
pybind11_add_module(continuous1d src/experiments/continuous1d.cc)
target_include_directories(continuous1d
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/eigen
)
set_target_properties(continuous1d
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY cipells/experiments
)

pybind11_add_module(_cipells
    src/python/module.cc
    src/python/XYTuple.cc
    src/python/Interval.cc
    src/python/Box.cc
    src/python/transforms.cc
    src/python/Image.cc
    src/python/Interpolant.cc
    src/python/Kernel.cc
    src/python/profiles.cc
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


pybind11_add_module(_tests
    src/python/tests/module.cc
)
target_include_directories(_tests
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
set_target_properties(_tests
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY cipells/tests
)
target_link_libraries(_tests
    PRIVATE
        cipells
        fmt::fmt-header-only
)


enable_testing()
cipells_add_python(tests/__init__)

function(cipells_add_test TEST_NAME)
    cipells_add_python(tests/${TEST_NAME})
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
endfunction(cipells_add_test)

cipells_add_test(continuous1d)
cipells_add_test(XYTuple)
cipells_add_test(Interval)
cipells_add_test(Box)
cipells_add_test(transforms)
cipells_add_test(Image)
cipells_add_test(Interpolant)

# zvmone: Fast Zond Virtual Machine implementation
# Copyright 2019 The evmone Authors.
# SPDX-License-Identifier: Apache-2.0

find_package(intx CONFIG REQUIRED)

add_library(testutils INTERFACE)
add_library(zvmone::testutils ALIAS testutils)
target_link_libraries(testutils INTERFACE zvmc::zvmc_cpp)
target_include_directories(testutils INTERFACE ${PROJECT_SOURCE_DIR} ${zvmone_private_include_dir})

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.19)
    target_sources(
        testutils PRIVATE
        bytecode.hpp
        stdx/utility.hpp
        utils.hpp
    )
endif()

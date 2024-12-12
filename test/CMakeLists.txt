# zvmone: Fast Zond Virtual Machine implementation
# Copyright 2019-2020 The evmone Authors.
# SPDX-License-Identifier: Apache-2.0

include(GoogleTest)
include(${PROJECT_SOURCE_DIR}/zvmc/cmake/ZVMC.cmake)

set(zvmone_private_include_dir ${PROJECT_SOURCE_DIR}/lib)

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

hunter_add_package(benchmark)
find_package(benchmark CONFIG REQUIRED)

add_subdirectory(utils)
add_subdirectory(bench)
add_subdirectory(integration)
add_subdirectory(internal_benchmarks)
add_subdirectory(state)
add_subdirectory(statetest)
add_subdirectory(t8n)
add_subdirectory(unittests)

set(targets zvmone-bench zvmone-bench-internal zvmone-state zvmone-statetest zvmone-t8n zvmone-unittests)

if(ZVMONE_FUZZING)
    add_subdirectory(fuzzer)
    list(APPEND targets zvmone-fuzzer)
endif()

set_target_properties(
    ${targets} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
)

get_target_property(type zvmone TYPE)
if(type STREQUAL SHARED_LIBRARY)
    zvmc_add_vm_test(NAME ${PROJECT_NAME}/zvmc-vmtester TARGET zvmone)

    if(TARGET zvm-test)
        add_test(NAME ${PROJECT_NAME}/zvm-test COMMAND zvm-test $<TARGET_FILE:zvmone>)
    endif()
endif()

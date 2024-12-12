# zvmone-fuzzer: LibFuzzer based testing tool for ZVMC-compatible ZVM implementations.
# Copyright 2019 The evmone Authors.
# SPDX-License-Identifier: Apache-2.0
include(ExternalProject)

get_target_property(type zvmone TYPE)
if(NOT type STREQUAL STATIC_LIBRARY)
    message(FATAL_ERROR "The zvmone must be built as static library")
endif()

if(fuzzing_coverage)
    set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=fuzzer")
else()
    string(REPLACE fuzzer-no-link fuzzer CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
endif()

add_executable(zvmone-fuzzer fuzzer.cpp)
target_link_libraries(zvmone-fuzzer PRIVATE zvmone zvmone::testutils zvmc::mocked_host)

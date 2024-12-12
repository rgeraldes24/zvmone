# zvmone: Fast Zond Virtual Machine implementation
# Copyright 2019-2020 The evmone Authors.
# SPDX-License-Identifier: Apache-2.0

add_executable(
    zvmone-bench-internal
    find_jumpdest_bench.cpp
    memory_allocation.cpp
)

target_link_libraries(zvmone-bench-internal PRIVATE benchmark::benchmark)

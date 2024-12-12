# zvmone: Fast Zond Virtual Machine implementation
# Copyright 2019 The evmone Authors.
# SPDX-License-Identifier: Apache-2.0

add_executable(zvmone-bench)
target_include_directories(zvmone-bench PRIVATE ${zvmone_private_include_dir})
target_link_libraries(zvmone-bench PRIVATE zvmone zvmone::testutils zvmone::statetestutils zvmc::loader benchmark::benchmark)
target_sources(
    zvmone-bench PRIVATE
    bench.cpp
    helpers.hpp
    synthetic_benchmarks.cpp synthetic_benchmarks.hpp
)

# Tests

set(PREFIX zvmone/bench)
set(BENCHMARK_SUITE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../zvm-benchmarks/benchmarks)

# Check if DIR argument works.
add_test(NAME ${PREFIX}/dir COMMAND zvmone-bench ${BENCHMARK_SUITE_DIR} --benchmark_list_tests)
set_tests_properties(${PREFIX}/dir PROPERTIES PASS_REGULAR_EXPRESSION "total/synth")

# Omitting DIR is fine.
add_test(NAME ${PREFIX}/no_dir COMMAND zvmone-bench --benchmark_list_tests)
set_tests_properties(${PREFIX}/no_dir PROPERTIES PASS_REGULAR_EXPRESSION "total/synth")

# Empty DIR name should list only built-in benchmarks.
add_test(NAME ${PREFIX}/dirname_empty COMMAND zvmone-bench "" --benchmark_list_tests)
set_tests_properties(${PREFIX}/dirname_empty PROPERTIES PASS_REGULAR_EXPRESSION "total/synth")

# Run all benchmark cases split into groups to check if none of them crashes.
add_test(NAME ${PREFIX}/synth COMMAND zvmone-bench --benchmark_min_time=0 --benchmark_filter=synth)
add_test(NAME ${PREFIX}/micro COMMAND zvmone-bench --benchmark_min_time=0 --benchmark_filter=micro ${BENCHMARK_SUITE_DIR})
add_test(NAME ${PREFIX}/main/b COMMAND zvmone-bench --benchmark_min_time=0 --benchmark_filter=main/[b] ${BENCHMARK_SUITE_DIR})
add_test(NAME ${PREFIX}/main/s COMMAND zvmone-bench --benchmark_min_time=0 --benchmark_filter=main/[s] ${BENCHMARK_SUITE_DIR})
add_test(NAME ${PREFIX}/main/w COMMAND zvmone-bench --benchmark_min_time=0 --benchmark_filter=main/[w] ${BENCHMARK_SUITE_DIR})
add_test(NAME ${PREFIX}/main/_ COMMAND zvmone-bench --benchmark_min_time=0 --benchmark_filter=main/[^bsw] ${BENCHMARK_SUITE_DIR})

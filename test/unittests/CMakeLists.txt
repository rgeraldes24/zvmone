# zvmone: Fast Zond Virtual Machine implementation
# Copyright 2018-2020 The evmone Authors.
# SPDX-License-Identifier: Apache-2.0

# The internal zvmone unit tests. The generic ZVM ones are also built in.
add_executable(zvmone-unittests)
target_sources(
    zvmone-unittests PRIVATE
    analysis_test.cpp
    bytecode_test.cpp
    zvm_fixture.cpp
    zvm_fixture.hpp
    zvm_test.cpp
    zvm_calls_test.cpp
    zvm_control_flow_test.cpp
    zvm_eip2929_test.cpp
    zvm_eip3198_basefee_test.cpp
    zvm_eip3855_push0_test.cpp
    zvm_eip3860_initcode_test.cpp
    zvm_memory_test.cpp
    zvm_state_test.cpp
    zvm_storage_test.cpp
    zvm_other_test.cpp
    zvm_benchmark_test.cpp
    zvmone_test.cpp
    execution_state_test.cpp
    instructions_test.cpp
    state_bloom_filter_test.cpp
    state_mpt_hash_test.cpp
    state_mpt_test.cpp
    state_new_account_address_test.cpp
    state_rlp_test.cpp
    state_transition.hpp
    state_transition.cpp
    state_transition_block_test.cpp
    state_transition_create_test.cpp
    statetest_loader_block_info_test.cpp
    statetest_loader_test.cpp
    statetest_loader_tx_test.cpp
    statetest_logs_hash_test.cpp
    tracing_test.cpp
)
target_link_libraries(zvmone-unittests PRIVATE zvmone zvmone::state zvmone::statetestutils testutils zvmc::instructions GTest::gtest GTest::gtest_main)
target_include_directories(zvmone-unittests PRIVATE ${zvmone_private_include_dir})

gtest_discover_tests(zvmone-unittests TEST_PREFIX ${PROJECT_NAME}/unittests/)

option(ZVMONE_ZVM_TEST_TOOL "Enable ZVM unit testing tool for ZVMC implementations (not maintained)" OFF)
if(ZVMONE_ZVM_TEST_TOOL)
    # The zvm-test tool that contains the all zvm-unittests and loads VMs as ZVMC modules.
    add_executable(zvm-test main.cpp)
    target_link_libraries(zvm-test PRIVATE zvm-unittests testutils zvmc::zvmc zvmc::loader GTest::gtest)
endif()

# Provide the project version to selected source files.
set_source_files_properties(
    zvmone_test.cpp
    main.cpp
    PROPERTIES COMPILE_DEFINITIONS PROJECT_VERSION="${PROJECT_VERSION}"
)

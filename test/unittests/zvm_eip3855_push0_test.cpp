// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2022 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

/// This file contains ZVM unit tests for EIP-3855 "PUSH0 instruction"
/// https://eips.ethereum.org/EIPS/eip-3855

#include "zvm_fixture.hpp"

using namespace zvmc::literals;
using zvmone::test::zvm;

TEST_P(zvm, push0)
{
    rev = ZVMC_SHANGHAI;
    execute(OP_PUSH0 + ret_top());
    EXPECT_GAS_USED(ZVMC_SUCCESS, 17);
    EXPECT_OUTPUT_INT(0);
}

TEST_P(zvm, push0_return_empty)
{
    rev = ZVMC_SHANGHAI;
    execute(bytecode{} + OP_PUSH0 + OP_PUSH0 + OP_RETURN);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 4);
    EXPECT_EQ(result.output_size, 0);
}

TEST_P(zvm, push0_full_stack)
{
    rev = ZVMC_SHANGHAI;
    execute(1024 * bytecode{OP_PUSH0});
    EXPECT_GAS_USED(ZVMC_SUCCESS, 1024 * 2);
}

TEST_P(zvm, push0_stack_overflow)
{
    rev = ZVMC_SHANGHAI;
    execute(1025 * bytecode{OP_PUSH0});
    EXPECT_STATUS(ZVMC_STACK_OVERFLOW);
}

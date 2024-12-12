// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2021 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

/// This file contains ZVM unit tests for EIP-3198 "BASEFEE opcode"
/// https://eips.ethereum.org/EIPS/eip-3198

#include "zvm_fixture.hpp"

using namespace zvmc::literals;
using zvmone::test::zvm;

TEST_P(zvm, basefee_nominal_case)
{
    // https://eips.ethereum.org/EIPS/eip-3198#nominal-case
    rev = ZVMC_SHANGHAI;
    host.tx_context.block_base_fee = zvmc::bytes32{7};

    execute(bytecode{} + OP_BASEFEE + OP_STOP);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 2);

    execute(bytecode{} + OP_BASEFEE + ret_top());
    EXPECT_GAS_USED(ZVMC_SUCCESS, 17);
    EXPECT_OUTPUT_INT(7);
}

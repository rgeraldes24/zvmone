// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2022 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

/// This file contains ZVM unit tests for EIP-3860 "Limit and meter initcode"
/// https://eips.ethereum.org/EIPS/eip-3860

#include "zvm_fixture.hpp"

using namespace zvmc::literals;
using zvmone::test::zvm;

inline constexpr size_t initcode_size_limit = 0xc000;

TEST_P(zvm, create_initcode_limit)
{
    host.call_result.create_address = "Z02"_address;
    for (const auto& c : {create().input(0, calldataload(0)) + ret_top(),
             create2().input(0, calldataload(0)) + ret_top()})
    {
        for (const auto r : {ZVMC_SHANGHAI})
        {
            rev = r;
            for (const auto s : {initcode_size_limit, initcode_size_limit + 1})
            {
                execute(c, zvmc::uint256be{s});
                if (rev >= ZVMC_SHANGHAI && s > initcode_size_limit)
                {
                    EXPECT_STATUS(ZVMC_OUT_OF_GAS);
                }
                else
                {
                    EXPECT_OUTPUT_INT(2);
                }
            }
        }
    }
}

TEST_P(zvm, create_initcode_gas_cost)
{
    rev = ZVMC_SHANGHAI;
    const auto code = create().input(0, calldataload(0));
    execute(44300, code, zvmc::uint256be{initcode_size_limit});
    EXPECT_GAS_USED(ZVMC_SUCCESS, 44300);
    execute(44299, code, zvmc::uint256be{initcode_size_limit});
    EXPECT_STATUS(ZVMC_OUT_OF_GAS);
}

TEST_P(zvm, create2_initcode_gas_cost)
{
    rev = ZVMC_SHANGHAI;
    const auto code = create2().input(0, calldataload(0));
    execute(53519, code, zvmc::uint256be{initcode_size_limit});
    EXPECT_GAS_USED(ZVMC_SUCCESS, 53519);
    execute(53518, code, zvmc::uint256be{initcode_size_limit});
    EXPECT_STATUS(ZVMC_OUT_OF_GAS);
}

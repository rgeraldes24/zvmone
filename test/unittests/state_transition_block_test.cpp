// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2023 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "state_transition.hpp"

using namespace zvmc::literals;
using namespace zvmone::test;

TEST_F(state_transition, block_apply_withdrawal)
{
    static constexpr auto withdrawal_address = "Z8888"_address;

    block.withdrawals = {{withdrawal_address, 3}};
    tx.to = To;
    expect.post[withdrawal_address].balance = intx::uint256{3} * 1'000'000'000;
}

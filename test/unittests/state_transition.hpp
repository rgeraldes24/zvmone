// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2023 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <gtest/gtest.h>
#include <test/state/host.hpp>
#include <zvmone/zvmone.h>

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

namespace zvmone::test
{
using namespace zvmone;
using namespace zvmone::state;

/// Fixture to defining test cases in form similar to JSON State Tests.
///
/// It takes the "pre" state and produces "post" state by applying the defined "tx" transaction.
/// Then expectations declared in "except" are checked in the "post" state.
class state_transition : public testing::Test
{
protected:
    /// The default sender address of the test transaction.
    /// Private key: 0x2b1263d2b.
    static constexpr auto Sender = "Zcb5a46208433371c566fcba08c07d071d769c8e0c6874f3d"_address;

    /// The default destination address of the test transaction.
    static constexpr auto To = "Zc0de"_address;

    static constexpr auto Coinbase = "Zc014bace"_address;

    static inline zvmc::VM vm{zvmc_create_zvmone()};

    struct ExpectedAccount
    {
        bool exists = true;
        std::optional<uint64_t> nonce;
        std::optional<intx::uint256> balance;
        std::optional<bytes> code;
        std::unordered_map<bytes32, bytes32> storage;
    };

    struct Expectation
    {
        zvmc_status_code status = ZVMC_SUCCESS;
        std::optional<int64_t> gas_used;

        std::unordered_map<address, ExpectedAccount> post;
    };


    zvmc_revision rev = ZVMC_SHANGHAI;
    BlockInfo block{
        .gas_limit = 1'000'000,
        .coinbase = Coinbase,
        .base_fee = 999,
    };
    Transaction tx{
        .gas_limit = block.gas_limit,
        .max_gas_price = block.base_fee + 1,
        .max_priority_gas_price = 1,
        .sender = Sender,
    };
    State pre;
    Expectation expect;

    void SetUp() override;

    /// The test runner.
    void TearDown() override;
};

}  // namespace zvmone::test

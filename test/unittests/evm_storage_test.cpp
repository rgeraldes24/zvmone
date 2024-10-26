// evmone: Fast Ethereum Virtual Machine implementation
// Copyright 2019 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

/// This file contains EVM unit tests that access or modify the contract storage.
// TODO(now.youtrack.cloud/issue/TE-13)

#include "evm_fixture.hpp"
#include <array>

using namespace evmc::literals;
using evmone::test::evm;

TEST_P(evm, storage)
{
    const auto code = sstore(0xee, 0xff) + sload(0xee) + mstore8(0) + ret(0, 1);
    execute(100000, code);
    EXPECT_GAS_USED(EVMC_SUCCESS, 22224);
    EXPECT_EQ(bytes_view(result.output_data, result.output_size), bytes{0xff});
}

TEST_P(evm, sstore_pop_stack)
{
    execute(100000, sstore(1, dup1(0)) + mstore8(0) + ret(0, 1));
    EXPECT_GAS_USED(EVMC_SUCCESS, 2224);
    EXPECT_EQ(bytes_view(result.output_data, result.output_size), bytes{0x00});
    EXPECT_EQ(
        host.accounts[msg.recipient].storage.find(0x01_bytes32)->second.current, 0x00_bytes32);
}

TEST_P(evm, sload_cost)
{
    rev = EVMC_SHANGHAI;
    execute(2106, sload(dup1(0)));
    EXPECT_GAS_USED(EVMC_SUCCESS, 2106);
    EXPECT_EQ(host.accounts[msg.recipient].storage.size(), 1);
}

TEST_P(evm, sstore_out_of_block_gas)
{
    const auto code = push(0) + sstore(0, 1) + OP_POP;

    // Barely enough gas to execute successfully.
    host.accounts[msg.recipient] = {};  // Reset contract account.
    execute(22111, code);
    EXPECT_GAS_USED(EVMC_SUCCESS, 22111);

    // Out of block gas - 1 too low.
    host.accounts[msg.recipient] = {};  // Reset contract account.
    execute(22110, code);
    EXPECT_STATUS(EVMC_OUT_OF_GAS);

    // Out of block gas - 2 too low.
    host.accounts[msg.recipient] = {};  // Reset contract account.
    execute(22109, code);
    EXPECT_STATUS(EVMC_OUT_OF_GAS);

    // SSTORE instructions out of gas.
    host.accounts[msg.recipient] = {};  // Reset contract account.
    execute(22108, code);
    EXPECT_STATUS(EVMC_OUT_OF_GAS);
}

TEST_P(evm, sstore_cost)
{
    auto& storage = host.accounts[msg.recipient].storage;

    constexpr auto v1 = 0x01_bytes32;

    for (auto r : {EVMC_SHANGHAI})
    {
        rev = r;

        // Added:
        storage.clear();
        execute(22106, sstore(1, 1));
        EXPECT_EQ(result.status_code, EVMC_SUCCESS);
        storage.clear();
        execute(22105, sstore(1, 1));
        EXPECT_EQ(result.status_code, EVMC_OUT_OF_GAS);

        // Deleted:
        storage.clear();
        storage[v1] = v1;
        execute(5006, sstore(1, 0));
        EXPECT_EQ(result.status_code, EVMC_SUCCESS);
        storage[v1] = v1;
        execute(5005, sstore(1, 0));
        EXPECT_EQ(result.status_code, EVMC_OUT_OF_GAS);

        // Modified:
        storage.clear();
        storage[v1] = v1;
        execute(5006, sstore(1, 2));
        EXPECT_EQ(result.status_code, EVMC_SUCCESS);
        storage[v1] = v1;
        execute(5005, sstore(1, 2));
        EXPECT_EQ(result.status_code, EVMC_OUT_OF_GAS);

        // Unchanged:
        storage.clear();
        storage[v1] = v1;
        execute(sstore(1, 1));
        EXPECT_EQ(result.status_code, EVMC_SUCCESS);
        EXPECT_EQ(gas_used, 2206);
        execute(205, sstore(1, 1));
        EXPECT_EQ(result.status_code, EVMC_OUT_OF_GAS);

        // Added & unchanged:
        storage.clear();
        execute(sstore(1, 1) + sstore(1, 1));
        EXPECT_EQ(result.status_code, EVMC_SUCCESS);
        EXPECT_EQ(gas_used, 22212);

        // Modified again:
        storage.clear();
        storage[v1] = {v1, 0x00_bytes32};
        execute(sstore(1, 2));
        EXPECT_EQ(result.status_code, EVMC_SUCCESS);
        EXPECT_EQ(gas_used, 2206);

        // Added & modified again:
        storage.clear();
        execute(sstore(1, 1) + sstore(1, 2));
        EXPECT_EQ(result.status_code, EVMC_SUCCESS);
        EXPECT_EQ(gas_used, 22212);

        // Modified & modified again:
        storage.clear();
        storage[v1] = v1;
        execute(sstore(1, 2) + sstore(1, 3));
        EXPECT_EQ(result.status_code, EVMC_SUCCESS);
        EXPECT_EQ(gas_used, 5112);

        // Modified & modified again back to original:
        storage.clear();
        storage[v1] = v1;
        execute(sstore(1, 2) + sstore(1, 1));
        EXPECT_EQ(result.status_code, EVMC_SUCCESS);
        EXPECT_EQ(gas_used, 5112);
    }
}

TEST_P(evm, sstore_cost_net_gas_metering)
{
    // Follow the table on https://evmc.ethereum.org/storagestatus.html

    static constexpr auto O = 0x000000000000000000_bytes32;
    static constexpr auto X = 0x00ffffffffffffffff_bytes32;
    static constexpr auto Y = 0x010000000000000000_bytes32;
    static constexpr auto Z = 0x010000000000000001_bytes32;
    static constexpr auto key = 0xde_bytes32;

    static constexpr int64_t b = 6;  // Cost of other instructions.

    struct CostConstants
    {
        int64_t warm_access = -1;
        int64_t set = -1;
        int64_t reset = -1;
        int64_t clear = -1;
    };

    const auto test = [this](const evmc::bytes32& original, const evmc::bytes32& current,
                          const evmc::bytes32& value, int64_t expected_gas_used,
                          int64_t expected_gas_refund) {
        auto& storage_entry = host.accounts[msg.recipient].storage[key];
        storage_entry.original = original;
        storage_entry.current = current;
        storage_entry.access_status = EVMC_ACCESS_WARM;
        execute(sstore(key, value));
        EXPECT_EQ(storage_entry.current, value);
        EXPECT_GAS_USED(EVMC_SUCCESS, expected_gas_used);
        EXPECT_EQ(result.gas_refund, expected_gas_refund);
    };

    std::array<CostConstants, EVMC_MAX_REVISION + 1> cost_constants{};
    cost_constants[EVMC_SHANGHAI] = {100, 20000, 2900, 4800};

    for (const auto r : {EVMC_SHANGHAI})
    {
        rev = r;
        const auto& c = cost_constants.at(static_cast<size_t>(r));

        test(O, O, O, b + c.warm_access, 0);  // assigned
        test(X, O, O, b + c.warm_access, 0);
        test(O, Y, Y, b + c.warm_access, 0);
        test(X, Y, Y, b + c.warm_access, 0);
        test(Y, Y, Y, b + c.warm_access, 0);
        test(O, Y, Z, b + c.warm_access, 0);
        test(X, Y, Z, b + c.warm_access, 0);

        test(O, O, Z, b + c.set, 0);                                          // added
        test(X, X, O, b + c.reset, c.clear);                                  // deleted
        test(X, X, Z, b + c.reset, 0);                                        // modified
        test(X, O, Z, b + c.warm_access, -c.clear);                           // deleted added
        test(X, Y, O, b + c.warm_access, c.clear);                            // modified deleted
        test(X, O, X, b + c.warm_access, c.reset - c.warm_access - c.clear);  // deleted restored
        test(O, Y, O, b + c.warm_access, c.set - c.warm_access);              // added deleted
        test(X, Y, X, b + c.warm_access, c.reset - c.warm_access);            // modified restored
    }
}

TEST_P(evm, sstore_below_stipend)
{
    const auto code = sstore(0, 0);

    rev = EVMC_SHANGHAI;
    execute(2306, code);
    EXPECT_EQ(result.status_code, EVMC_OUT_OF_GAS);

    execute(2307, code);
    EXPECT_EQ(result.status_code, EVMC_SUCCESS);
}

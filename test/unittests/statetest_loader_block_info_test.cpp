// evmone: Fast Ethereum Virtual Machine implementation
// Copyright 2023 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <test/statetest/statetest.hpp>

using namespace evmone;

TEST(statetest_loader, block_info)
{
    constexpr std::string_view input = R"({
            "currentCoinbase": "Z1111111111111111111111111111111111111111",
            "currentGasLimit": "0x0",
            "currentNumber": "0",
            "currentTimestamp": "0",
            "currentBaseFee": "7",
            "currentRandom": "0x00",
            "withdrawals": []
        })";

    const auto bi = test::from_json<state::BlockInfo>(json::json::parse(input));
    EXPECT_EQ(bi.coinbase, "Z1111111111111111111111111111111111111111"_address);
    EXPECT_EQ(bi.prev_randao, 0x00_bytes32);
    EXPECT_EQ(bi.gas_limit, 0x0);
    EXPECT_EQ(bi.base_fee, 7);
    EXPECT_EQ(bi.timestamp, 0);
    EXPECT_EQ(bi.number, 0);
    EXPECT_EQ(bi.withdrawals.size(), 0);
}

TEST(statetest_loader, block_info_hex)
{
    constexpr std::string_view input = R"({
        "currentCoinbase": "Z2adc25665018aa1fe0e6bc666dac8fc2697ff9ba",
        "currentGasLimit": "0x16345785D8A0000",
        "currentNumber": "1",
        "currentTimestamp": "0x3E8",
        "currentRandom": "0x00",
        "parentBaseFee": "7",
        "parentGasUsed": "0",
        "parentGasLimit": "0x16345785D8A0000",
        "parentTimstamp": "0",
        "blockHashes": {
            "0": "0xc305d826e3784046a7e9d31128ef98d3e96133fe454c16ef630574d967dfdb1a"
        },
        "withdrawals": []
    })";

    const auto bi = test::from_json<state::BlockInfo>(json::json::parse(input));
    EXPECT_EQ(bi.coinbase, "Z2adc25665018aa1fe0e6bc666dac8fc2697ff9ba"_address);
    EXPECT_EQ(bi.prev_randao, 0x00_bytes32);
    EXPECT_EQ(bi.gas_limit, 100000000000000000);
    EXPECT_EQ(bi.base_fee, 7);
    EXPECT_EQ(bi.timestamp, 1000);
    EXPECT_EQ(bi.number, 1);
}

TEST(statetest_loader, block_info_dec)
{
    constexpr std::string_view input = R"({
        "currentCoinbase": "Z2adc25665018aa1fe0e6bc666dac8fc2697ff9ba",
        "currentGasLimit": "100000000000000000",
        "currentNumber": "1",
        "currentTimestamp": "1000",
        "currentRandom": "0x00",
        "parentBaseFee": "7",
        "parentGasUsed": "0",
        "parentGasLimit": "100000000000000000",
        "parentTimstamp": "0",
        "blockHashes": {
            "0": "0xc305d826e3784046a7e9d31128ef98d3e96133fe454c16ef630574d967dfdb1a"
        },
        "withdrawals": []
    })";

    const auto bi = test::from_json<state::BlockInfo>(json::json::parse(input));
    EXPECT_EQ(bi.coinbase, "Z2adc25665018aa1fe0e6bc666dac8fc2697ff9ba"_address);
    EXPECT_EQ(bi.prev_randao, 0x00_bytes32);
    EXPECT_EQ(bi.gas_limit, 100000000000000000);
    EXPECT_EQ(bi.base_fee, 7);
    EXPECT_EQ(bi.timestamp, 1000);
    EXPECT_EQ(bi.number, 1);
}

TEST(statetest_loader, block_info_0_random)
{
    constexpr std::string_view input = R"({
            "currentCoinbase": "Z1111111111111111111111111111111111111111",
            "currentGasLimit": "0x0",
            "currentNumber": "0",
            "currentTimestamp": "0",
            "currentBaseFee": "7",
            "currentRandom": "0",
            "withdrawals": []
        })";

    const auto bi = test::from_json<state::BlockInfo>(json::json::parse(input));
    EXPECT_EQ(bi.coinbase, "Z1111111111111111111111111111111111111111"_address);
    EXPECT_EQ(bi.prev_randao, 0x00_bytes32);
    EXPECT_EQ(bi.gas_limit, 0x0);
    EXPECT_EQ(bi.base_fee, 7);
    EXPECT_EQ(bi.timestamp, 0);
    EXPECT_EQ(bi.number, 0);
}

TEST(statetest_loader, block_info_withdrawals)
{
    constexpr std::string_view input = R"({
            "currentCoinbase": "Z1111111111111111111111111111111111111111",
            "currentGasLimit": "0x0",
            "currentNumber": "0",
            "currentTimestamp": "0",
            "currentBaseFee": "7",
            "currentRandom": "0x00",
            "withdrawals": [
                {
                    "index": "0x0",
                    "validatorIndex": "0x0",
                    "address": "Z0000000000000000000000000000000000000100",
                    "amount": "0x800000000"
                },
                {
                    "index": "0x1",
                    "validatorIndex": "0x1",
                    "address": "Z0000000000000000000000000000000000000200",
                    "amount": "0xffffffffffffffff"
                }
            ]
        })";

    const auto bi = test::from_json<state::BlockInfo>(json::json::parse(input));
    EXPECT_EQ(bi.coinbase, "Z1111111111111111111111111111111111111111"_address);
    EXPECT_EQ(bi.prev_randao, 0x00_bytes32);
    EXPECT_EQ(bi.gas_limit, 0x0);
    EXPECT_EQ(bi.base_fee, 7);
    EXPECT_EQ(bi.timestamp, 0);
    EXPECT_EQ(bi.number, 0);
    EXPECT_EQ(bi.withdrawals.size(), 2);
    EXPECT_EQ(bi.withdrawals[0].recipient, "Z0000000000000000000000000000000000000100"_address);
    EXPECT_EQ(bi.withdrawals[0].get_amount(), intx::uint256{0x800000000} * 1'000'000'000);
    EXPECT_EQ(bi.withdrawals[1].recipient, "Z0000000000000000000000000000000000000200"_address);
    EXPECT_EQ(bi.withdrawals[1].get_amount(), intx::uint256{0xffffffffffffffff} * 1'000'000'000);
}

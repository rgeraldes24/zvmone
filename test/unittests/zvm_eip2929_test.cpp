// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2021 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

/// This file contains ZVM unit tests for EIP-2929 "Gas cost increases for state access opcodes"
/// https://eips.ethereum.org/EIPS/eip-2929

#include "zvm_fixture.hpp"

using namespace zvmc::literals;
using zvmone::test::zvm;

TEST_P(zvm, eip2929_case1)
{
    // https://gist.github.com/holiman/174548cad102096858583c6fbbb0649a#case-1
    rev = ZVMC_SHANGHAI;
    msg.sender = "Z000000000000000000000000000000000000000000000000"_address;
    msg.recipient = "Z00000000000000000000000000000000636F6E7472616374"_address;
    const bytecode code =
        "0x60013f5060023b506003315060f13f5060f23b5060f3315060f23f5060f33b5060f1315032315030315000";

    execute(code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 8653);
    EXPECT_EQ(result.output_size, 0);

    const auto& r = host.recorded_account_accesses;
    ASSERT_EQ(r.size(), 24);
    EXPECT_EQ(r[0], msg.sender);
    EXPECT_EQ(r[1], msg.recipient);
    EXPECT_EQ(r[2], "Z000000000000000000000000000000000000000000000001"_address);
    EXPECT_EQ(r[3], "Z000000000000000000000000000000000000000000000001"_address);
    EXPECT_EQ(r[4], "Z000000000000000000000000000000000000000000000002"_address);
    EXPECT_EQ(r[5], "Z000000000000000000000000000000000000000000000002"_address);
    EXPECT_EQ(r[6], "Z000000000000000000000000000000000000000000000003"_address);
    EXPECT_EQ(r[7], "Z000000000000000000000000000000000000000000000003"_address);
    EXPECT_EQ(r[8], "Z0000000000000000000000000000000000000000000000f1"_address);
    EXPECT_EQ(r[9], "Z0000000000000000000000000000000000000000000000f1"_address);
    EXPECT_EQ(r[10], "Z0000000000000000000000000000000000000000000000f2"_address);
    EXPECT_EQ(r[11], "Z0000000000000000000000000000000000000000000000f2"_address);
    EXPECT_EQ(r[12], "Z0000000000000000000000000000000000000000000000f3"_address);
    EXPECT_EQ(r[13], "Z0000000000000000000000000000000000000000000000f3"_address);
    EXPECT_EQ(r[14], "Z0000000000000000000000000000000000000000000000f2"_address);
    EXPECT_EQ(r[15], "Z0000000000000000000000000000000000000000000000f2"_address);
    EXPECT_EQ(r[16], "Z0000000000000000000000000000000000000000000000f3"_address);
    EXPECT_EQ(r[17], "Z0000000000000000000000000000000000000000000000f3"_address);
    EXPECT_EQ(r[18], "Z0000000000000000000000000000000000000000000000f1"_address);
    EXPECT_EQ(r[19], "Z0000000000000000000000000000000000000000000000f1"_address);
    EXPECT_EQ(r[20], "Z000000000000000000000000000000000000000000000000"_address);
    EXPECT_EQ(r[21], "Z000000000000000000000000000000000000000000000000"_address);
    EXPECT_EQ(r[22], msg.recipient);
    EXPECT_EQ(r[23], msg.recipient);
}

TEST_P(zvm, eip2929_case2)
{
    // https://gist.github.com/holiman/174548cad102096858583c6fbbb0649a#case-2
    rev = ZVMC_SHANGHAI;
    msg.sender = "Z000000000000000000000000000000000000000000000000"_address;
    msg.recipient = "Z00000000000000000000000000000000636F6E7472616374"_address;
    const bytecode code = "0x60006000600060ff3c60006000600060ff3c600060006000303c00";

    execute(code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 2835);
    EXPECT_EQ(result.output_size, 0);

    const auto& r = host.recorded_account_accesses;
    ASSERT_EQ(r.size(), 5);
    EXPECT_EQ(r[0], msg.sender);
    EXPECT_EQ(r[1], msg.recipient);
    EXPECT_EQ(r[2], "Z0000000000000000000000000000000000000000000000ff"_address);
    EXPECT_EQ(r[3], "Z0000000000000000000000000000000000000000000000ff"_address);
    EXPECT_EQ(r[4], msg.recipient);
}

TEST_P(zvm, eip2929_case3)
{
    // https://gist.github.com/holiman/174548cad102096858583c6fbbb0649a#case-3
    rev = ZVMC_SHANGHAI;
    msg.sender = "Z000000000000000000000000000000000000000000000000"_address;
    msg.recipient = "Z00000000000000000000000000000000636F6E7472616374"_address;
    const bytecode code = "0x60015450601160015560116002556011600255600254600154";

    execute(code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 44529);
    EXPECT_EQ(result.output_size, 0);
}

TEST_P(zvm, eip2929_case4)
{
    // https://gist.github.com/holiman/174548cad102096858583c6fbbb0649a#case-4
    rev = ZVMC_SHANGHAI;
    msg.sender = "Z000000000000000000000000000000000000000000000000"_address;
    msg.recipient = "Z00000000000000000000000000000000636F6E7472616374"_address;
    const bytecode code =
        "0x60008080808060046000f15060008080808060ff6000f15060008080808060ff6000fa50";

    execute(code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 2869);
    EXPECT_EQ(result.output_size, 0);
}

TEST_P(zvm, eip2929_balance_oog)
{
    rev = ZVMC_SHANGHAI;
    const auto code = push(0x0a) + OP_BALANCE;

    execute(2603, code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 2603);

    host.recorded_account_accesses.clear();
    execute(2602, code);
    EXPECT_GAS_USED(ZVMC_OUT_OF_GAS, 2602);
}

TEST_P(zvm, eip2929_extcodesize_oog)
{
    rev = ZVMC_SHANGHAI;
    const auto code = push(0x0a) + OP_EXTCODESIZE;

    execute(2603, code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 2603);

    host.recorded_account_accesses.clear();
    execute(2602, code);
    EXPECT_GAS_USED(ZVMC_OUT_OF_GAS, 2602);
}

TEST_P(zvm, eip2929_extcodecopy_oog)
{
    rev = ZVMC_SHANGHAI;
    const auto code = push(0) + OP_DUP1 + OP_DUP1 + push(0x0a) + OP_EXTCODECOPY;

    execute(2612, code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 2612);

    host.recorded_account_accesses.clear();
    execute(2611, code);
    EXPECT_GAS_USED(ZVMC_OUT_OF_GAS, 2611);
}

TEST_P(zvm, eip2929_extcodehash_oog)
{
    rev = ZVMC_SHANGHAI;
    const auto code = push(0x0a) + OP_EXTCODEHASH;

    execute(2603, code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 2603);

    host.recorded_account_accesses.clear();
    execute(2602, code);
    EXPECT_GAS_USED(ZVMC_OUT_OF_GAS, 2602);
}

TEST_P(zvm, eip2929_sload_cold)
{
    rev = ZVMC_SHANGHAI;
    const auto code = push(1) + OP_SLOAD;

    const zvmc::bytes32 key{1};
    host.accounts[msg.recipient].storage[key] = zvmc::bytes32{2};
    ASSERT_EQ(host.accounts[msg.recipient].storage[key].access_status, ZVMC_ACCESS_COLD);
    execute(2103, code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 2103);
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].access_status, ZVMC_ACCESS_WARM);

    host.accounts[msg.recipient].storage[key].access_status = ZVMC_ACCESS_COLD;
    execute(2102, code);
    EXPECT_GAS_USED(ZVMC_OUT_OF_GAS, 2102);
}

TEST_P(zvm, eip2929_sload_two_slots)
{
    rev = ZVMC_SHANGHAI;
    const zvmc::bytes32 key0{0};
    const zvmc::bytes32 key1{1};
    const auto code = push(key0) + OP_SLOAD + OP_POP + push(key1) + OP_SLOAD + OP_POP;

    execute(30000, code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 4210);
    EXPECT_EQ(host.accounts[msg.recipient].storage[key0].access_status, ZVMC_ACCESS_WARM);
    EXPECT_EQ(host.accounts[msg.recipient].storage[key1].access_status, ZVMC_ACCESS_WARM);
}

TEST_P(zvm, eip2929_sload_warm)
{
    rev = ZVMC_SHANGHAI;
    const auto code = push(1) + OP_SLOAD;

    const zvmc::bytes32 key{1};
    host.accounts[msg.recipient].storage[key] = {zvmc::bytes32{2}, ZVMC_ACCESS_WARM};
    ASSERT_EQ(host.accounts[msg.recipient].storage[key].access_status, ZVMC_ACCESS_WARM);
    execute(103, code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 103);
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].access_status, ZVMC_ACCESS_WARM);

    execute(102, code);
    EXPECT_GAS_USED(ZVMC_OUT_OF_GAS, 102);
}

TEST_P(zvm, eip2929_sstore_modify_cold)
{
    rev = ZVMC_SHANGHAI;
    const auto code = sstore(1, 3);

    const zvmc::bytes32 key{1};
    host.accounts[msg.recipient].storage[key] = zvmc::bytes32{2};
    execute(5006, code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 5006);
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].current, zvmc::bytes32{3});
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].access_status, ZVMC_ACCESS_WARM);

    host.accounts[msg.recipient].storage[key] = zvmc::bytes32{2};
    execute(5005, code);
    EXPECT_GAS_USED(ZVMC_OUT_OF_GAS, 5005);
    // The storage will be modified anyway, because the cost is checked after.
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].current, zvmc::bytes32{3});
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].access_status, ZVMC_ACCESS_WARM);
}

TEST_P(zvm, eip2929_delegatecall_cold)
{
    rev = ZVMC_SHANGHAI;
    const auto code = delegatecall(0xde);
    auto& r = host.recorded_account_accesses;

    execute(2618, code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 2618);
    ASSERT_EQ(r.size(), 4);
    EXPECT_EQ(r[0], msg.sender);
    EXPECT_EQ(r[1], msg.recipient);
    EXPECT_EQ(r[2], "Z0000000000000000000000000000000000000000000000de"_address);
    EXPECT_EQ(r[3], msg.sender);

    r.clear();
    execute(2617, code);
    EXPECT_GAS_USED(ZVMC_OUT_OF_GAS, 2617);
    ASSERT_EQ(r.size(), 3);
    EXPECT_EQ(r[0], msg.sender);
    EXPECT_EQ(r[1], msg.recipient);
    EXPECT_EQ(r[2], "Z0000000000000000000000000000000000000000000000de"_address);
}

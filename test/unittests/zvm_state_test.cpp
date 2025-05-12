// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2019 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

/// This file contains ZVM unit tests that access or modify information
/// about accounts, without storage.
// TODO(now.youtrack.cloud/issue/TE-13)

#include "zvm_fixture.hpp"
#include <zvmone/instructions_traits.hpp>

using namespace zvmc::literals;
using zvmone::test::zvm;

TEST_P(zvm, code)
{
    // CODESIZE 2 0 CODECOPY RETURN(0,9)
    const auto s = bytecode{"38600260003960096000f3"};
    execute(s);
    EXPECT_EQ(gas_used, 23);
    ASSERT_EQ(result.output_size, 9);
    EXPECT_EQ(bytes_view(&result.output_data[0], 9), bytes_view(&s[2], 9));
}

TEST_P(zvm, codecopy_combinations)
{
    // The CODECOPY arguments are provided in calldata: first byte is index, second byte is size.
    // The whole copied code is returned.
    const auto code = dup1(byte(calldataload(0), 1)) + byte(calldataload(0), 0) + push(0) +
                      OP_CODECOPY + ret(0, {});
    EXPECT_EQ(code.size(), 0x13);

    execute(code, "0013"_hex);
    EXPECT_EQ(output, code);

    execute(code, "0012"_hex);
    EXPECT_EQ(output, code.substr(0, 0x12));

    execute(code, "0014"_hex);
    EXPECT_EQ(output, code + "00");

    execute(code, "1300"_hex);
    EXPECT_EQ(output, bytes_view{});

    execute(code, "1400"_hex);
    EXPECT_EQ(output, bytes_view{});

    execute(code, "1200"_hex);
    EXPECT_EQ(output, bytes_view{});

    execute(code, "1301"_hex);
    EXPECT_EQ(output, "00"_hex);

    execute(code, "1401"_hex);
    EXPECT_EQ(output, "00"_hex);

    execute(code, "1201"_hex);
    EXPECT_EQ(output, code.substr(0x12, 1));
}

TEST_P(zvm, tx_context)
{
    rev = ZVMC_SHANGHAI;

    host.tx_context.block_timestamp = 0xdd;
    host.tx_context.block_number = 0x1100;
    host.tx_context.block_gas_limit = 0x990000;
    host.tx_context.chain_id.bytes[28] = 0xaa;
    host.tx_context.block_coinbase.bytes[1] = 0xcc;
    host.tx_context.tx_origin.bytes[2] = 0x55;
    host.tx_context.block_prev_randao.bytes[1] = 0xdd;
    host.tx_context.tx_gas_price.bytes[2] = 0x66;

    auto const code = bytecode{} + OP_TIMESTAMP + OP_COINBASE + OP_OR + OP_GASPRICE + OP_OR +
                      OP_NUMBER + OP_OR + OP_PREVRANDAO + OP_OR + OP_GASLIMIT + OP_OR + OP_ORIGIN +
                      OP_OR + OP_CHAINID + OP_OR + ret_top();
    execute(52, code);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(result.gas_left, 0);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[31], 0xdd);
    EXPECT_EQ(result.output_data[30], 0x11);
    EXPECT_EQ(result.output_data[29], 0x99);
    EXPECT_EQ(result.output_data[28], 0xaa);
    EXPECT_EQ(result.output_data[14], 0x55);
    EXPECT_EQ(result.output_data[13], 0xcc);
    EXPECT_EQ(result.output_data[2], 0x66);
    EXPECT_EQ(result.output_data[1], 0xdd);
}

TEST_P(zvm, balance)
{
    host.accounts[msg.recipient].set_balance(0x0504030201);
    auto code = bytecode{} + OP_ADDRESS + OP_BALANCE + mstore(0) + ret(32 - 6, 6);
    execute(417, code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 117);
    ASSERT_EQ(result.output_size, 6);
    EXPECT_EQ(result.output_data[0], 0);
    EXPECT_EQ(result.output_data[1], 0x05);
    EXPECT_EQ(result.output_data[2], 0x04);
    EXPECT_EQ(result.output_data[3], 0x03);
    EXPECT_EQ(result.output_data[4], 0x02);
    EXPECT_EQ(result.output_data[5], 0x01);
}

TEST_P(zvm, account_info_homestead)
{
    rev = ZVMC_SHANGHAI;
    host.accounts[msg.recipient].set_balance(1);
    host.accounts[msg.recipient].code = bytes{1};

    execute(bytecode{} + OP_ADDRESS + OP_BALANCE + ret_top());
    EXPECT_GAS_USED(ZVMC_SUCCESS, 117);
    EXPECT_OUTPUT_INT(1);

    execute(bytecode{} + OP_ADDRESS + OP_EXTCODESIZE + ret_top());
    EXPECT_GAS_USED(ZVMC_SUCCESS, 117);
    EXPECT_OUTPUT_INT(1);

    execute(bytecode{} + push(1) + push(0) + push(0) + OP_ADDRESS + OP_EXTCODECOPY + ret(0, 1));
    EXPECT_GAS_USED(ZVMC_SUCCESS, 123);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 1);
}

TEST_P(zvm, selfbalance)
{
    host.accounts[msg.recipient].set_balance(0x0504030201);
    // NOTE: adding push here to balance out the stack pre-Istanbul (needed to get undefined
    // instruction as a result)
    auto code = bytecode{} + push(1) + OP_SELFBALANCE + mstore(0) + ret(32 - 6, 6);

    rev = ZVMC_SHANGHAI;
    execute(code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 23);
    ASSERT_EQ(result.output_size, 6);
    EXPECT_EQ(result.output_data[0], 0);
    EXPECT_EQ(result.output_data[1], 0x05);
    EXPECT_EQ(result.output_data[2], 0x04);
    EXPECT_EQ(result.output_data[3], 0x03);
    EXPECT_EQ(result.output_data[4], 0x02);
    EXPECT_EQ(result.output_data[5], 0x01);
}

TEST_P(zvm, log)
{
    for (auto op : {OP_LOG0, OP_LOG1, OP_LOG2, OP_LOG3, OP_LOG4})
    {
        const auto n = op - OP_LOG0;
        const auto code =
            push(1) + push(2) + push(3) + push(4) + mstore8(2, 0x77) + push(2) + push(2) + op;
        host.recorded_logs.clear();
        execute(code);
        EXPECT_GAS_USED(ZVMC_SUCCESS, 421 + n * 375);
        ASSERT_EQ(host.recorded_logs.size(), 1);
        const auto& last_log = host.recorded_logs.back();
        ASSERT_EQ(last_log.data.size(), 2);
        EXPECT_EQ(last_log.data[0], 0x77);
        EXPECT_EQ(last_log.data[1], 0);
        ASSERT_EQ(last_log.topics.size(), n);
        for (size_t i = 0; i < static_cast<size_t>(n); ++i)
        {
            EXPECT_EQ(last_log.topics[i].bytes[31], 4 - i);
        }
    }
}

TEST_P(zvm, log0_empty)
{
    auto code = push(0) + OP_DUP1 + OP_LOG0;
    execute(code);
    ASSERT_EQ(host.recorded_logs.size(), 1);
    const auto& last_log = host.recorded_logs.back();
    EXPECT_EQ(last_log.topics.size(), 0);
    EXPECT_EQ(last_log.data.size(), 0);
}

TEST_P(zvm, log_data_cost)
{
    for (auto op : {OP_LOG0, OP_LOG1, OP_LOG2, OP_LOG3, OP_LOG4})
    {
        auto num_topics = op - OP_LOG0;
        auto code = push(0) + (4 * OP_DUP1) + push(1) + push(0) + op;
        auto cost = 407 + num_topics * 375;
        EXPECT_EQ(host.recorded_logs.size(), 0);
        execute(cost, code);
        EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
        EXPECT_EQ(host.recorded_logs.size(), 1);
        host.recorded_logs.clear();

        EXPECT_EQ(host.recorded_logs.size(), 0);
        execute(cost - 1, code);
        EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);
        EXPECT_EQ(host.recorded_logs.size(), 0) << zvmone::instr::traits[op].name;
        host.recorded_logs.clear();
    }
}

TEST_P(zvm, blockhash)
{
    host.block_hash.bytes[13] = 0x13;

    host.tx_context.block_number = 0;
    const auto code = push(0) + OP_BLOCKHASH + ret_top();
    execute(code);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(gas_used, 38);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[13], 0);
    EXPECT_EQ(host.recorded_blockhashes.size(), 0);

    host.tx_context.block_number = 257;
    execute(code);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(gas_used, 38);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[13], 0);
    EXPECT_EQ(host.recorded_blockhashes.size(), 0);

    host.tx_context.block_number = 256;
    execute(code);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(gas_used, 38);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[13], 0x13);
    ASSERT_EQ(host.recorded_blockhashes.size(), 1);
    EXPECT_EQ(host.recorded_blockhashes.back(), 0);
}

TEST_P(zvm, extcode)
{
    constexpr auto addr = "Zfffffffffffffffffffffffffffffffffffffffffffffffe"_address;
    host.accounts[addr].code = {'a', 'b', 'c', 'd'};

    bytecode code;
    code += "6002600003803b60019003";  // S = EXTCODESIZE(-2) - 1
    code += "90600080913c";            // EXTCODECOPY(-2, 0, 0, S)
    code += "60046000f3";              // RETURN(0, 4)

    execute(code);
    EXPECT_EQ(gas_used, 2745);
    ASSERT_EQ(result.output_size, 4);
    EXPECT_EQ(bytes_view(result.output_data, 3), bytes_view(host.accounts[addr].code.data(), 3));
    EXPECT_EQ(result.output_data[3], 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 6);
    EXPECT_EQ(host.recorded_account_accesses[2], addr);
    EXPECT_EQ(host.recorded_account_accesses[3], addr);
}

TEST_P(zvm, extcodesize)
{
    constexpr auto addr = "Z000000000000000000000000000000000000000000000002"_address;
    host.accounts[addr].code = {'\0'};
    execute(push(2) + OP_EXTCODESIZE + ret_top());
    EXPECT_OUTPUT_INT(1);
}

TEST_P(zvm, extcodecopy_big_index)
{
    constexpr auto index = uint64_t{std::numeric_limits<uint32_t>::max()} + 1;
    const auto code = dup1(1) + push(index) + dup1(0) + OP_EXTCODECOPY + ret(0, {});
    execute(code);
    EXPECT_EQ(output, "00"_hex);
}

TEST_P(zvm, extcodehash)
{
    auto& hash = host.accounts[{}].codehash;
    std::fill(std::begin(hash.bytes), std::end(hash.bytes), uint8_t{0xee});

    const auto code = push(0) + OP_EXTCODEHASH + ret_top();

    rev = ZVMC_SHANGHAI;
    execute(code);
    EXPECT_EQ(gas_used, 118);
    ASSERT_EQ(result.output_size, 32);
    auto expected_hash = bytes(32, 0xee);
    EXPECT_EQ(bytes_view(result.output_data, result.output_size),
        bytes_view(std::begin(hash.bytes), std::size(hash.bytes)));
}

TEST_P(zvm, codecopy_empty)
{
    execute(push(0) + 2 * OP_DUP1 + OP_CODECOPY + OP_MSIZE + ret_top());
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(std::count(result.output_data, result.output_data + result.output_size, 0), 32);
}

TEST_P(zvm, extcodecopy_empty)
{
    execute(push(0) + 3 * OP_DUP1 + OP_EXTCODECOPY + OP_MSIZE + ret_top());
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(std::count(result.output_data, result.output_data + result.output_size, 0), 32);
}

TEST_P(zvm, codecopy_memory_cost)
{
    auto code = push(1) + push(0) + push(0) + OP_CODECOPY;
    execute(18, code);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    execute(17, code);
    EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);
}

TEST_P(zvm, extcodecopy_memory_cost)
{
    auto code = push(1) + push(0) + 2 * OP_DUP1 + OP_EXTCODECOPY;
    execute(118, code);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    execute(117, code);
    EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);
}

TEST_P(zvm, extcodecopy_nonzero_index)
{
    constexpr auto addr = "Z00000000000000000000000000000000000000000000000a"_address;
    constexpr auto index = 15;

    auto& extcode = host.accounts[addr].code;
    extcode.assign(16, 0x00);
    extcode[index] = 0xc0;
    auto code = push(2) + push(index) + push(0) + push(0xa) + OP_EXTCODECOPY + ret(0, 2);
    EXPECT_EQ(code.length() + 1, index);
    execute(code);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 2);
    EXPECT_EQ(result.output_data[0], 0xc0);
    EXPECT_EQ(result.output_data[1], 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 4);
    EXPECT_EQ(host.recorded_account_accesses.back().bytes[19], 0xa);
}

TEST_P(zvm, extcodecopy_fill_tail)
{
    auto addr = zvmc_address{};
    addr.bytes[19] = 0xa;

    auto& extcode = host.accounts[addr].code;
    extcode = {0xff, 0xfe};
    extcode.resize(1);
    auto code = push(2) + push(0) + push(0) + push(0xa) + OP_EXTCODECOPY + ret(0, 2);
    execute(code);
    ASSERT_EQ(host.recorded_account_accesses.size(), 4);
    EXPECT_EQ(host.recorded_account_accesses.back().bytes[19], 0xa);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 2);
    EXPECT_EQ(result.output_data[0], 0xff);
    EXPECT_EQ(result.output_data[1], 0);
}

TEST_P(zvm, extcodecopy_buffer_overflow)
{
    const auto code = bytecode{} + OP_NUMBER + OP_TIMESTAMP + calldatasize() + OP_ADDRESS +
                      OP_EXTCODECOPY + ret(calldatasize(), OP_NUMBER);

    host.accounts[msg.recipient].code = code;

    const auto s = static_cast<int>(code.size());
    const auto values = {0, 1, s - 1, s, s + 1, 5000};
    for (auto offset : values)
    {
        for (auto size : values)
        {
            host.tx_context.block_timestamp = offset;
            host.tx_context.block_number = size;

            execute(code);
            EXPECT_STATUS(ZVMC_SUCCESS);
            EXPECT_EQ(result.output_size, size);
        }
    }
}

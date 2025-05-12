// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2019-2020 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

/// This file contains ZVM unit tests that perform any kind of calls.
// TODO(now.youtrack.cloud/issue/TE-13)

#include "zvm_fixture.hpp"

using namespace zvmc::literals;
using zvmone::test::zvm;

// Expected equality of these values:
//   output
//     Which is: { '\0', '\0', '\0', '\0', '\n' (10, 0xA), '\v' (11, 0xB), '\f' (12, 0xC), '\0' }
//   (bytes{0xff, 0xff, 0xff, 0xff, 0xa, 0xb, 0xc, 0xff})
//     Which is: { '\xFF' (255), '\xFF' (255), '\xFF' (255), '\xFF' (255), '\n' (10, 0xA), '\v' (11,
//     0xB), '\f' (12, 0xC), '\xFF' (255) }
TEST_P(zvm, delegatecall)
{
    auto code = bytecode{};
    code += "6001600003600052";              // m[0] = 0xffffff...
    code += "600560046003600260016103e8f4";  // DELEGATECALL(1000, 0x01, ...)
    code += "60086000f3";

    auto call_output = bytes{0xa, 0xb, 0xc};
    host.call_result.output_data = call_output.data();
    host.call_result.output_size = call_output.size();
    host.call_result.gas_left = 1;

    msg.value.bytes[17] = 0xfe;

    execute(1700, code);

    EXPECT_EQ(gas_used, 1141);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);

    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.gas, 1000);
    EXPECT_EQ(call_msg.input_size, 3);
    EXPECT_EQ(call_msg.value.bytes[17], 0xfe);

    ASSERT_EQ(result.output_size, 8);
    EXPECT_EQ(output, (bytes{0xff, 0xff, 0xff, 0xff, 0xa, 0xb, 0xc, 0xff}));
}

TEST_P(zvm, delegatecall_static)
{
    // Checks if DELEGATECALL forwards the "static" flag.
    msg.flags = ZVMC_STATIC;
    execute(bytecode{} + delegatecall(0).gas(1));
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.gas, 1);
    EXPECT_EQ(call_msg.flags, uint32_t{ZVMC_STATIC});
    EXPECT_GAS_USED(ZVMC_SUCCESS, 119);
}

TEST_P(zvm, delegatecall_oog_depth_limit)
{
    rev = ZVMC_SHANGHAI;
    msg.depth = 1024;
    const auto code = bytecode{} + delegatecall(0).gas(16) + ret_top();

    execute(code);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 133);
    EXPECT_OUTPUT_INT(0);

    execute(73, code);
    EXPECT_STATUS(ZVMC_OUT_OF_GAS);
}

TEST_P(zvm, create)
{
    auto& account = host.accounts[msg.recipient];
    account.set_balance(1);

    auto call_output = bytes{0xa, 0xb, 0xc};
    host.call_result.output_data = call_output.data();
    host.call_result.output_size = call_output.size();
    host.call_result.create_address = "Zcc01020304050607080901020304050607080901020304ce"_address;
    host.call_result.gas_left = 200000;
    execute(300000, sstore(1, create().value(1).input(0, 0x20)));

    EXPECT_GAS_USED(ZVMC_SUCCESS, 117916);

    EXPECT_EQ(account.storage[0x01_bytes32].current,
        0x000000000000000000000000cc010203040506070809010203040506070809ce_bytes32);

    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.kind, ZVMC_CREATE);
    EXPECT_EQ(call_msg.gas, 263799);
    EXPECT_EQ(call_msg.value, 0x01_bytes32);
    EXPECT_EQ(call_msg.input_size, 0x20);
    EXPECT_EQ(call_msg.create2_salt, 0x00_bytes32);
}

TEST_P(zvm, create_gas)
{
    size_t c = 0;
    for (auto r : {ZVMC_SHANGHAI})
    {
        ++c;
        rev = r;
        execute(50000, create());
        EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
        EXPECT_EQ(gas_used, 49719) << rev;
        ASSERT_EQ(host.recorded_calls.size(), c);
        EXPECT_EQ(host.recorded_calls.back().gas, 17710) << rev;
    }
}

TEST_P(zvm, create2)
{
    rev = ZVMC_SHANGHAI;
    auto& account = host.accounts[msg.recipient];
    account.set_balance(1);

    const bytes call_output{0xa, 0xb, 0xc};
    host.call_result.output_data = call_output.data();
    host.call_result.output_size = call_output.size();
    host.call_result.create_address = "Zc201020304050607080901020304050607080901020304ce"_address;
    host.call_result.gas_left = 200000;
    execute(300000, sstore(1, create2().value(1).input(0, 0x41).salt(0x5a)));
    EXPECT_GAS_USED(ZVMC_SUCCESS, 117917);

    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.kind, ZVMC_CREATE2);
    EXPECT_EQ(call_msg.gas, 263769);
    EXPECT_EQ(call_msg.value, 0x01_bytes32);
    EXPECT_EQ(call_msg.input_size, 0x41);
    EXPECT_EQ(call_msg.create2_salt, 0x5a_bytes32);

    EXPECT_EQ(account.storage[0x01_bytes32].current,
        0x000000000000000000000000c2010203040506070809010203040506070809ce_bytes32);
}

TEST_P(zvm, create2_salt_cost)
{
    rev = ZVMC_SHANGHAI;
    const auto code = create2().input(0, 0x20);

    execute(34128, code);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    EXPECT_EQ(host.recorded_calls.back().kind, ZVMC_CREATE2);
    EXPECT_EQ(host.recorded_calls.back().depth, 1);

    execute(32021 - 1, code);
    EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);
    EXPECT_EQ(result.gas_left, 0);
    EXPECT_EQ(host.recorded_calls.size(), 1);  // No more CREATE2 recorded.
}

TEST_P(zvm, create_balance_too_low)
{
    rev = ZVMC_SHANGHAI;
    host.accounts[msg.recipient].set_balance(1);
    for (auto op : {OP_CREATE, OP_CREATE2})
    {
        execute(push(2) + (3 * OP_DUP1) + hex(op) + ret_top());
        EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
        EXPECT_EQ(std::count(result.output_data, result.output_data + result.output_size, 0), 32);
        EXPECT_EQ(host.recorded_calls.size(), 0);
    }
}

TEST_P(zvm, create_failure)
{
    host.call_result.create_address = "Z0000000000000000000000000000000000000000000000ce"_address;
    const auto create_address =
        bytes_view{host.call_result.create_address.bytes, sizeof(host.call_result.create_address)};
    rev = ZVMC_SHANGHAI;
    for (auto op : {OP_CREATE, OP_CREATE2})
    {
        const auto code = push(0) + (3 * OP_DUP1) + op + ret_top();

        host.call_result.status_code = ZVMC_SUCCESS;
        execute(code);
        EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
        ASSERT_EQ(result.output_size, 32);
        EXPECT_EQ((bytes_view{result.output_data + 12, 20}), create_address);
        ASSERT_EQ(host.recorded_calls.size(), 1);
        EXPECT_EQ(host.recorded_calls.back().kind, op == OP_CREATE ? ZVMC_CREATE : ZVMC_CREATE2);
        host.recorded_calls.clear();

        host.call_result.status_code = ZVMC_REVERT;
        execute(code);
        EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
        EXPECT_OUTPUT_INT(0);
        ASSERT_EQ(host.recorded_calls.size(), 1);
        EXPECT_EQ(host.recorded_calls.back().kind, op == OP_CREATE ? ZVMC_CREATE : ZVMC_CREATE2);
        host.recorded_calls.clear();

        host.call_result.status_code = ZVMC_FAILURE;
        execute(code);
        EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
        EXPECT_OUTPUT_INT(0);
        ASSERT_EQ(host.recorded_calls.size(), 1);
        EXPECT_EQ(host.recorded_calls.back().kind, op == OP_CREATE ? ZVMC_CREATE : ZVMC_CREATE2);
        host.recorded_calls.clear();
    }
}

TEST_P(zvm, call_failing_with_value)
{
    host.accounts["Z0000000000000000000000000000000000000000000000aa"_address] = {};
    for (auto op : {OP_CALL})
    {
        const auto code = push(0xff) + push(0) + OP_DUP2 + OP_DUP2 + push(1) + push(0xaa) +
                          push(0x8000) + op + OP_POP;

        // Fails on balance check.
        execute(12000, code);
        EXPECT_GAS_USED(ZVMC_SUCCESS, 9347);
        EXPECT_EQ(host.recorded_calls.size(), 0);  // There was no call().

        // Fails on value transfer additional cost - minimum gas limit that triggers this condition.
        execute(747, code);
        EXPECT_STATUS(ZVMC_OUT_OF_GAS);
        EXPECT_EQ(host.recorded_calls.size(), 0);  // There was no call().

        // Fails on value transfer additional cost - maximum gas limit that triggers this condition.
        execute(744 + 9000, code);
        // EXPECT_STATUS(ZVMC_OUT_OF_GAS);
        // EXPECT_EQ(host.recorded_calls.size(), 0);  // There was no call().
    }
}

TEST_P(zvm, call_with_value)
{
    constexpr auto code = "60ff600060ff6000600160aa618000f150";

    constexpr auto call_sender = "Z5e4d0000000000000000000000000000000000000000d4e5"_address;
    constexpr auto call_dst = "Z0000000000000000000000000000000000000000000000aa"_address;

    msg.recipient = call_sender;
    host.accounts[msg.recipient].set_balance(1);
    host.accounts[call_dst] = {};
    host.call_result.gas_left = 1;

    execute(40000, code);
    EXPECT_EQ(gas_used, 39558);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.kind, ZVMC_CALL);
    EXPECT_EQ(call_msg.depth, 1);
    EXPECT_EQ(call_msg.gas, 30212);
    EXPECT_EQ(call_msg.recipient, call_dst);
    EXPECT_EQ(call_msg.sender, call_sender);
}

TEST_P(zvm, call_with_value_depth_limit)
{
    auto call_dst = zvmc_address{};
    call_dst.bytes[19] = 0xaa;
    host.accounts[call_dst] = {};

    msg.depth = 1024;
    execute(bytecode{"60ff600060ff6000600160aa618000f150"});
    EXPECT_EQ(gas_used, 9347);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    EXPECT_EQ(host.recorded_calls.size(), 0);
}

TEST_P(zvm, call_depth_limit)
{
    rev = ZVMC_SHANGHAI;
    msg.depth = 1024;

    for (auto op : {OP_CALL, OP_DELEGATECALL, OP_STATICCALL, OP_CREATE, OP_CREATE2})
    {
        const auto code = push(0) + 6 * OP_DUP1 + op + ret_top() + OP_INVALID;
        execute(code);
        EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
        EXPECT_EQ(host.recorded_calls.size(), 0);
        EXPECT_OUTPUT_INT(0);
    }
}

TEST_P(zvm, call_output)
{
    static bool result_is_correct = false;
    static uint8_t call_output[] = {0xa, 0xb};

    host.accounts[{}].set_balance(1);
    host.call_result.output_data = call_output;
    host.call_result.output_size = sizeof(call_output);
    host.call_result.release = [](const zvmc_result* r) {
        result_is_correct = r->output_size == sizeof(call_output) && r->output_data == call_output;
    };

    auto code_prefix_output_1 = push(1) + 6 * OP_DUP1 + push("7fffffffffffffff");
    auto code_prefix_output_0 = push(0) + 6 * OP_DUP1 + push("7fffffffffffffff");
    auto code_suffix = ret(0, 3);

    for (auto op : {OP_CALL, OP_DELEGATECALL, OP_STATICCALL})
    {
        result_is_correct = false;
        execute(code_prefix_output_1 + hex(op) + code_suffix);
        EXPECT_TRUE(result_is_correct);
        EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
        ASSERT_EQ(result.output_size, 3);
        EXPECT_EQ(result.output_data[0], 0);
        EXPECT_EQ(result.output_data[1], 0xa);
        EXPECT_EQ(result.output_data[2], 0);


        result_is_correct = false;
        execute(code_prefix_output_0 + hex(op) + code_suffix);
        EXPECT_TRUE(result_is_correct);
        EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
        ASSERT_EQ(result.output_size, 3);
        EXPECT_EQ(result.output_data[0], 0);
        EXPECT_EQ(result.output_data[1], 0);
        EXPECT_EQ(result.output_data[2], 0);
    }
}

TEST_P(zvm, call_high_gas)
{
    rev = ZVMC_SHANGHAI;
    host.accounts["Zaa"_address] = {};

    for (auto call_opcode : {OP_CALL, OP_DELEGATECALL})
    {
        execute(50, 5 * push(0) + push(0xaa) + push(0x134c) + call_opcode);
        EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);
    }
}

TEST_P(zvm, call_value_zero_to_nonexistent_account)
{
    constexpr auto call_gas = 6000;
    host.call_result.gas_left = 1000;

    const auto code = push(0x40) + push(0) + push(0x40) + push(0) + push(0) + push(0xaa) +
                      push(call_gas) + OP_CALL + OP_POP;

    execute(9000, code);
    EXPECT_EQ(gas_used, 7629);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.kind, ZVMC_CALL);
    EXPECT_EQ(call_msg.depth, 1);
    EXPECT_EQ(call_msg.gas, 6000);
    EXPECT_EQ(call_msg.input_size, 64);
    EXPECT_EQ(call_msg.recipient, "Z0000000000000000000000000000000000000000000000aa"_address);
    EXPECT_EQ(call_msg.value.bytes[31], 0);
}

TEST_P(zvm, call_new_account_creation_cost)
{
    constexpr auto call_dst = "Z0000000000000000000000000000000000000000000000ad"_address;
    constexpr auto msg_dst = "Z000000000000000000000000000000000000000000000003"_address;
    const auto code =
        4 * push(0) + calldataload(0) + push(call_dst) + push(0) + OP_CALL + ret_top();
    msg.recipient = msg_dst;

    rev = ZVMC_SHANGHAI;
    host.accounts[msg.recipient].set_balance(0);
    execute(code, "00"_hex);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 2639);
    EXPECT_OUTPUT_INT(1);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    EXPECT_EQ(host.recorded_calls.back().recipient, call_dst);
    EXPECT_EQ(host.recorded_calls.back().gas, 0);
    EXPECT_EQ(host.recorded_calls.back().sender, msg_dst);
    EXPECT_EQ(host.recorded_calls.back().value.bytes[31], 0);
    EXPECT_EQ(host.recorded_calls.back().input_size, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 4);
    host.recorded_account_accesses.clear();
    host.recorded_calls.clear();

    rev = ZVMC_SHANGHAI;
    host.accounts[msg.recipient].set_balance(1);
    execute(code, "0000000000000000000000000000000000000000000000000000000000000001"_hex);
    EXPECT_GAS_USED(ZVMC_SUCCESS, 36639);
    EXPECT_OUTPUT_INT(1);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    EXPECT_EQ(host.recorded_calls.back().recipient, call_dst);
    EXPECT_EQ(host.recorded_calls.back().gas, 2300);
    EXPECT_EQ(host.recorded_calls.back().sender, msg_dst);
    EXPECT_EQ(host.recorded_calls.back().value.bytes[31], 1);
    EXPECT_EQ(host.recorded_calls.back().input_size, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 6);
    EXPECT_EQ(host.recorded_account_accesses[1], msg.recipient);  // Balance.
    EXPECT_EQ(host.recorded_account_accesses[2], call_dst);       // Call.
    host.recorded_account_accesses.clear();
    host.recorded_calls.clear();
}


TEST_P(zvm, call_then_oog)
{
    // Performs a CALL then OOG in the same code block.
    auto call_dst = zvmc_address{};
    call_dst.bytes[19] = 0xaa;
    host.accounts[call_dst] = {};
    host.call_result.status_code = ZVMC_FAILURE;
    host.call_result.gas_left = 0;

    const auto code =
        call(0xaa).gas(254).value(0).input(0, 0x40).output(0, 0x40) + 4 * add(OP_DUP1) + OP_POP;

    execute(1000, code);
    EXPECT_EQ(gas_used, 1000);
    ASSERT_EQ(host.recorded_calls.size(), 0);
    EXPECT_EQ(result.gas_left, 0);
    EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);
}

TEST_P(zvm, delegatecall_then_oog)
{
    // Performs a CALL then OOG in the same code block.
    auto call_dst = zvmc_address{};
    call_dst.bytes[19] = 0xaa;
    host.accounts[call_dst] = {};
    host.call_result.status_code = ZVMC_FAILURE;
    host.call_result.gas_left = 0;

    const auto code =
        delegatecall(0xaa).gas(254).input(0, 0x40).output(0, 0x40) + 4 * add(OP_DUP1) + OP_POP;

    execute(1000, code);
    EXPECT_EQ(gas_used, 1000);
    ASSERT_EQ(host.recorded_calls.size(), 0);
    EXPECT_EQ(result.gas_left, 0);
    EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);
}

TEST_P(zvm, staticcall_then_oog)
{
    // Performs a STATICCALL then OOG in the same code block.
    auto call_dst = zvmc_address{};
    call_dst.bytes[19] = 0xaa;
    host.accounts[call_dst] = {};
    host.call_result.status_code = ZVMC_FAILURE;
    host.call_result.gas_left = 0;

    const auto code =
        staticcall(0xaa).gas(254).input(0, 0x40).output(0, 0x40) + 4 * add(OP_DUP1) + OP_POP;

    execute(1000, code);
    EXPECT_EQ(gas_used, 1000);
    ASSERT_EQ(host.recorded_calls.size(), 0);
    EXPECT_EQ(result.gas_left, 0);
    EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);
}

TEST_P(zvm, staticcall_input)
{
    const auto code = mstore(3, 0x010203) + staticcall(0).gas(0xee).input(32, 3);
    execute(code);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.gas, 0xee);
    EXPECT_EQ(call_msg.input_size, 3);
    EXPECT_EQ(hex(bytes_view(call_msg.input_data, call_msg.input_size)), "010203");
}

TEST_P(zvm, call_with_value_low_gas)
{
    // Create the call destination account.
    host.accounts["Z000000000000000000000000000000000000000000000000"_address] = {};
    for (auto call_op : {OP_CALL})
    {
        auto code = 4 * push(0) + push(1) + 2 * push(0) + call_op + OP_POP;
        execute(9721, code);
        EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
        EXPECT_EQ(result.gas_left, 2898);
    }
}

/*
TEST_P(zvm, call_oog_after_depth_check)
{
    // Create the call recipient account.
    host.accounts["Z000000000000000000000000000000000000000000000000"_address] = {};
    msg.depth = 1024;

    for (auto op : {OP_CALL, OP_CALLCODE})
    {
        const auto code = 4 * push(0) + push(1) + 2 * push(0) + op + OP_SELFDESTRUCT;
        execute(12420, code);
        EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);
    }

    rev = ZVMC_TANGERINE_WHISTLE;
    const auto code = 7 * push(0) + OP_CALL + OP_SELFDESTRUCT;
    execute(721, code);
    EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);

    execute(721 + 5000 - 1, code);
    EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);
}
*/

TEST_P(zvm, call_recipient_and_code_address)
{
    constexpr auto origin = "Z990000000000000000000000000000000000000000000099"_address;
    constexpr auto executor = "Zee00000000000000000000000000000000000000000000ee"_address;
    constexpr auto recipient = "Z440000000000000000000000000000000000000000000044"_address;

    msg.sender = origin;
    msg.recipient = executor;

    for (auto op : {OP_CALL, OP_DELEGATECALL, OP_STATICCALL})
    {
        const auto code = 5 * push(0) + push(recipient) + push(0) + op;
        execute(100000, code);
        EXPECT_GAS_USED(ZVMC_SUCCESS, (op == OP_CALL) ? 2621 : 121);
        ASSERT_EQ(host.recorded_calls.size(), 1);
        const auto& call = host.recorded_calls[0];
        EXPECT_EQ(call.recipient, (op == OP_CALL || op == OP_STATICCALL) ? recipient : executor);
        EXPECT_EQ(call.code_address, recipient);
        EXPECT_EQ(call.sender, (op == OP_DELEGATECALL) ? origin : executor);
        host.recorded_calls.clear();
    }
}

TEST_P(zvm, call_value)
{
    constexpr auto origin = "Z990000000000000000000000000000000000000000000099"_address;
    constexpr auto executor = "Zee00000000000000000000000000000000000000000000ee"_address;
    constexpr auto recipient = "Z440000000000000000000000000000000000000000000044"_address;

    constexpr auto passed_value = 3;
    constexpr auto origin_value = 8;

    msg.sender = origin;
    msg.recipient = executor;
    msg.value.bytes[31] = origin_value;
    host.accounts[executor].set_balance(passed_value);
    host.accounts[recipient] = {};  // Create the call recipient account.

    for (auto op : {OP_CALL, OP_DELEGATECALL, OP_STATICCALL})
    {
        const auto has_value_arg = (op == OP_CALL);
        // const auto value_cost = has_value_arg ? 9000 : 0;
        const auto expected_value = has_value_arg           ? passed_value :
                                    (op == OP_DELEGATECALL) ? origin_value :
                                                              0;

        const auto code =
            4 * push(0) + push(has_value_arg ? passed_value : 0) + push(recipient) + push(0) + op;
        execute(100000, code);
        EXPECT_GAS_USED(ZVMC_SUCCESS, (op == OP_CALL) ? 11621 : 121);
        ASSERT_EQ(host.recorded_calls.size(), 1);
        const auto& call = host.recorded_calls[0];
        EXPECT_EQ(call.value.bytes[31], expected_value) << op;
        host.recorded_calls.clear();
    }
}

// TODO(now.youtrack.cloud/issue/TE-13)
/*
TEST_P(zvm, create_oog_after)
{
    rev = ZVMC_CONSTANTINOPLE;
    for (auto op : {OP_CREATE, OP_CREATE2})
    {
        auto code = 4 * push(0) + op + OP_SELFDESTRUCT;
        execute(39000, code);
        EXPECT_STATUS(ZVMC_OUT_OF_GAS);
    }
}
*/

TEST_P(zvm, returndatasize_before_call)
{
    execute(bytecode{"3d60005360016000f3"});
    EXPECT_EQ(gas_used, 17);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0);
}

TEST_P(zvm, returndatasize)
{
    uint8_t call_output[13];
    host.call_result.output_size = std::size(call_output);
    host.call_result.output_data = std::begin(call_output);

    const auto code =
        push(0) + 5 * OP_DUP1 + OP_DELEGATECALL + mstore8(0, OP_RETURNDATASIZE) + ret(0, 1);
    execute(code);
    EXPECT_EQ(gas_used, 135);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], std::size(call_output));

    host.call_result.output_size = 1;
    host.call_result.status_code = ZVMC_FAILURE;
    execute(code);
    EXPECT_EQ(gas_used, 135);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 1);

    host.call_result.output_size = 0;
    host.call_result.status_code = ZVMC_INTERNAL_ERROR;
    execute(code);
    EXPECT_EQ(gas_used, 135);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0);
}

TEST_P(zvm, returndatacopy)
{
    uint8_t call_output[32] = {1, 2, 3, 4, 5, 6, 7};
    host.call_result.output_size = std::size(call_output);
    host.call_result.output_data = std::begin(call_output);

    const bytecode code = "600080808060aa60fff4506020600060003e60206000f3";
    execute(code);
    EXPECT_EQ(gas_used, 2899);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[0], 1);
    EXPECT_EQ(result.output_data[1], 2);
    EXPECT_EQ(result.output_data[2], 3);
    EXPECT_EQ(result.output_data[6], 7);
    EXPECT_EQ(result.output_data[7], 0);
}

TEST_P(zvm, returndatacopy_empty)
{
    const bytecode code = "600080808060aa60fff4600080803e60016000f3";
    execute(code);
    EXPECT_EQ(gas_used, 2894);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0);
}

TEST_P(zvm, returndatacopy_cost)
{
    auto call_output = uint8_t{};
    host.call_result.output_data = &call_output;
    host.call_result.output_size = sizeof(call_output);
    auto code = "60008080808080fa6001600060003e";
    execute(136, code);
    EXPECT_EQ(result.status_code, ZVMC_SUCCESS);
    execute(135, code);
    EXPECT_EQ(result.status_code, ZVMC_OUT_OF_GAS);
}

TEST_P(zvm, returndatacopy_outofrange)
{
    auto call_output = uint8_t{};
    host.call_result.output_data = &call_output;
    host.call_result.output_size = sizeof(call_output);
    execute(735, "60008080808080fa6002600060003e");
    EXPECT_EQ(result.status_code, ZVMC_INVALID_MEMORY_ACCESS);

    execute(735, "60008080808080fa6001600160003e");
    EXPECT_EQ(result.status_code, ZVMC_INVALID_MEMORY_ACCESS);

    execute(735, "60008080808080fa6000600260003e");
    EXPECT_EQ(result.status_code, ZVMC_INVALID_MEMORY_ACCESS);
}

TEST_P(zvm, call_gas_refund_propagation)
{
    rev = ZVMC_SHANGHAI;
    host.accounts[msg.recipient].set_balance(1);
    host.call_result.status_code = ZVMC_SUCCESS;
    host.call_result.gas_refund = 1;

    const auto code_prolog = 7 * push(1);
    for (const auto op : {OP_CALL, OP_DELEGATECALL, OP_STATICCALL, OP_CREATE, OP_CREATE2})
    {
        execute(code_prolog + op);
        EXPECT_STATUS(ZVMC_SUCCESS);
        EXPECT_EQ(result.gas_refund, 1);
    }
}

TEST_P(zvm, call_gas_refund_aggregation_different_calls)
{
    rev = ZVMC_SHANGHAI;
    host.accounts[msg.recipient].set_balance(1);
    host.call_result.status_code = ZVMC_SUCCESS;
    host.call_result.gas_refund = 1;

    const auto a = "Zaa"_address;
    const auto code = call(a) + delegatecall(a) + staticcall(a) + create() + create2();
    execute(code);
    EXPECT_STATUS(ZVMC_SUCCESS);
    EXPECT_EQ(result.gas_refund, 5);
}

TEST_P(zvm, call_gas_refund_aggregation_same_calls)
{
    rev = ZVMC_SHANGHAI;
    host.accounts[msg.recipient].set_balance(2);
    host.call_result.status_code = ZVMC_SUCCESS;
    host.call_result.gas_refund = 1;

    const auto code_prolog = 14 * push(1);
    for (const auto op : {OP_CALL, OP_DELEGATECALL, OP_STATICCALL, OP_CREATE, OP_CREATE2})
    {
        execute(code_prolog + 2 * op);
        EXPECT_STATUS(ZVMC_SUCCESS);
        EXPECT_EQ(result.gas_refund, 2);
    }
}

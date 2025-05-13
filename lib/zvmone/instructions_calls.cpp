// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2019 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "instructions.hpp"
#include <iostream>

namespace zvmone::instr::core
{
template <Opcode Op>
Result call_impl(StackTop stack, int64_t gas_left, ExecutionState& state) noexcept
{
    static_assert(Op == OP_CALL || Op == OP_DELEGATECALL || Op == OP_STATICCALL);

    const auto gas = stack.pop();
    const auto dst = intx::be::trunc<zvmc::address>(stack.pop());
    const auto value = (Op == OP_STATICCALL || Op == OP_DELEGATECALL) ? 0 : stack.pop();
    const auto has_value = value != 0;
    const auto input_offset_u256 = stack.pop();
    const auto input_size_u256 = stack.pop();
    const auto output_offset_u256 = stack.pop();
    const auto output_size_u256 = stack.pop();

    stack.push(0);  // Assume failure.
    state.return_data.clear();

    if (state.host.access_account(dst) == ZVMC_ACCESS_COLD)
    {
        if ((gas_left -= instr::additional_cold_account_access_cost) < 0)
            return {ZVMC_OUT_OF_GAS, gas_left};
    }

    if (!check_memory(gas_left, state.memory, input_offset_u256, input_size_u256))
        return {ZVMC_OUT_OF_GAS, gas_left};

    if (!check_memory(gas_left, state.memory, output_offset_u256, output_size_u256))
        return {ZVMC_OUT_OF_GAS, gas_left};

    const auto input_offset = static_cast<size_t>(input_offset_u256);
    const auto input_size = static_cast<size_t>(input_size_u256);
    const auto output_offset = static_cast<size_t>(output_offset_u256);
    const auto output_size = static_cast<size_t>(output_size_u256);

    auto msg = zvmc_message{};
    msg.kind = (Op == OP_DELEGATECALL) ? ZVMC_DELEGATECALL : ZVMC_CALL;
    msg.flags = (Op == OP_STATICCALL) ? uint32_t{ZVMC_STATIC} : state.msg->flags;
    msg.depth = state.msg->depth + 1;
    msg.recipient = (Op == OP_CALL || Op == OP_STATICCALL) ? dst : state.msg->recipient;
    msg.code_address = dst;
    msg.sender = (Op == OP_DELEGATECALL) ? state.msg->sender : state.msg->recipient;
    msg.value =
        (Op == OP_DELEGATECALL) ? state.msg->value : intx::be::store<zvmc::uint256be>(value);

    if (input_size > 0)
    {
        // input_offset may be garbage if input_size == 0.
        msg.input_data = &state.memory[input_offset];
        msg.input_size = input_size;
    }

    auto cost = has_value ? 9000 : 0;

    if constexpr (Op == OP_CALL)
    {
        if (has_value && state.in_static_mode())
            return {ZVMC_STATIC_MODE_VIOLATION, gas_left};

        if ((has_value) && !state.host.account_exists(dst))
            cost += 25000;
    }

    if ((gas_left -= cost) < 0)
        return {ZVMC_OUT_OF_GAS, gas_left};

    msg.gas = std::numeric_limits<int64_t>::max();
    if (gas < msg.gas)
        msg.gas = static_cast<int64_t>(gas);

    msg.gas = std::min(msg.gas, gas_left - gas_left / 64);

    if (has_value)
    {
        msg.gas += 2300;  // Add stipend.
        gas_left += 2300;
    }

    if (state.msg->depth >= 1024)
        return {ZVMC_SUCCESS, gas_left};  // "Light" failure.

    if (has_value && intx::be::load<uint256>(state.host.get_balance(state.msg->recipient)) < value)
        return {ZVMC_SUCCESS, gas_left};  // "Light" failure.

    const auto result = state.host.call(msg);
    state.return_data.assign(result.output_data, result.output_size);
    stack.top() = result.status_code == ZVMC_SUCCESS;

    if (const auto copy_size = std::min(output_size, result.output_size); copy_size > 0)
        std::memcpy(&state.memory[output_offset], result.output_data, copy_size);

    const auto gas_used = msg.gas - result.gas_left;
    gas_left -= gas_used;
    state.gas_refund += result.gas_refund;
    return {ZVMC_SUCCESS, gas_left};
}

template Result call_impl<OP_CALL>(
    StackTop stack, int64_t gas_left, ExecutionState& state) noexcept;
template Result call_impl<OP_STATICCALL>(
    StackTop stack, int64_t gas_left, ExecutionState& state) noexcept;
template Result call_impl<OP_DELEGATECALL>(
    StackTop stack, int64_t gas_left, ExecutionState& state) noexcept;


template <Opcode Op>
Result create_impl(StackTop stack, int64_t gas_left, ExecutionState& state) noexcept
{
    static_assert(Op == OP_CREATE || Op == OP_CREATE2);

    if (state.in_static_mode())
        return {ZVMC_STATIC_MODE_VIOLATION, gas_left};

    const auto endowment = stack.pop();
    const auto init_code_offset_u256 = stack.pop();
    const auto init_code_size_u256 = stack.pop();
    const auto salt = (Op == OP_CREATE2) ? stack.pop() : uint256{};

    stack.push(0);  // Assume failure.
    state.return_data.clear();

    if (!check_memory(gas_left, state.memory, init_code_offset_u256, init_code_size_u256))
        return {ZVMC_OUT_OF_GAS, gas_left};

    const auto init_code_offset = static_cast<size_t>(init_code_offset_u256);
    const auto init_code_size = static_cast<size_t>(init_code_size_u256);

    if (init_code_size > 0xC000)
        return {ZVMC_OUT_OF_GAS, gas_left};

    const auto init_code_word_cost = 6 * (Op == OP_CREATE2) + 2;
    const auto init_code_cost = num_words(init_code_size) * init_code_word_cost;
    if ((gas_left -= init_code_cost) < 0)
        return {ZVMC_OUT_OF_GAS, gas_left};

    if (state.msg->depth >= 1024)
        return {ZVMC_SUCCESS, gas_left};  // "Light" failure.

    if (endowment != 0 &&
        intx::be::load<uint256>(state.host.get_balance(state.msg->recipient)) < endowment)
        return {ZVMC_SUCCESS, gas_left};  // "Light" failure.

    auto msg = zvmc_message{};
    msg.gas = gas_left;
    msg.gas = msg.gas - msg.gas / 64;

    msg.kind = (Op == OP_CREATE) ? ZVMC_CREATE : ZVMC_CREATE2;
    if (init_code_size > 0)
    {
        // init_code_offset may be garbage if init_code_size == 0.
        msg.input_data = &state.memory[init_code_offset];
        msg.input_size = init_code_size;
    }
    msg.sender = state.msg->recipient;
    msg.depth = state.msg->depth + 1;
    msg.create2_salt = intx::be::store<zvmc::bytes32>(salt);
    msg.value = intx::be::store<zvmc::uint256be>(endowment);

    const auto result = state.host.call(msg);
    gas_left -= msg.gas - result.gas_left;
    state.gas_refund += result.gas_refund;

    state.return_data.assign(result.output_data, result.output_size);
    if (result.status_code == ZVMC_SUCCESS)
        stack.top() = intx::be::load<uint256>(result.create_address);

    return {ZVMC_SUCCESS, gas_left};
}

template Result create_impl<OP_CREATE>(
    StackTop stack, int64_t gas_left, ExecutionState& state) noexcept;
template Result create_impl<OP_CREATE2>(
    StackTop stack, int64_t gas_left, ExecutionState& state) noexcept;
}  // namespace zvmone::instr::core

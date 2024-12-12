// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2020 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <zvmone/advanced_analysis.hpp>
#include <zvmone/execution_state.hpp>
#include <type_traits>

static_assert(std::is_default_constructible_v<zvmone::ExecutionState>);
static_assert(!std::is_move_constructible_v<zvmone::ExecutionState>);
static_assert(!std::is_copy_constructible_v<zvmone::ExecutionState>);
static_assert(!std::is_move_assignable_v<zvmone::ExecutionState>);
static_assert(!std::is_copy_assignable_v<zvmone::ExecutionState>);

static_assert(std::is_default_constructible_v<zvmone::advanced::AdvancedExecutionState>);
static_assert(!std::is_move_constructible_v<zvmone::advanced::AdvancedExecutionState>);
static_assert(!std::is_copy_constructible_v<zvmone::advanced::AdvancedExecutionState>);
static_assert(!std::is_move_assignable_v<zvmone::advanced::AdvancedExecutionState>);
static_assert(!std::is_copy_assignable_v<zvmone::advanced::AdvancedExecutionState>);

TEST(execution_state, construct)
{
    zvmc_message msg{};
    msg.gas = -1;
    const zvmc_host_interface host_interface{};
    const uint8_t code[]{0x0f};
    const zvmone::ExecutionState st{
        msg, ZVMC_MAX_REVISION, host_interface, nullptr, {code, std::size(code)}};

    EXPECT_EQ(st.memory.size(), 0);
    EXPECT_EQ(st.msg, &msg);
    EXPECT_EQ(st.rev, ZVMC_MAX_REVISION);
    EXPECT_EQ(st.return_data.size(), 0);
    EXPECT_EQ(st.status, ZVMC_SUCCESS);
    EXPECT_EQ(st.output_offset, 0);
    EXPECT_EQ(st.output_size, 0);
}

TEST(execution_state, default_construct)
{
    const zvmone::ExecutionState st;

    EXPECT_EQ(st.memory.size(), 0);
    EXPECT_EQ(st.msg, nullptr);
    EXPECT_EQ(st.rev, ZVMC_SHANGHAI);
    EXPECT_EQ(st.return_data.size(), 0);
    EXPECT_EQ(st.status, ZVMC_SUCCESS);
    EXPECT_EQ(st.output_offset, 0);
    EXPECT_EQ(st.output_size, 0);
}

TEST(execution_state, default_construct_advanced)
{
    const zvmone::advanced::AdvancedExecutionState st;

    EXPECT_EQ(st.gas_left, 0);
    EXPECT_EQ(st.stack.size(), 0);
    EXPECT_EQ(st.memory.size(), 0);
    EXPECT_EQ(st.msg, nullptr);
    EXPECT_EQ(st.rev, ZVMC_SHANGHAI);
    EXPECT_EQ(st.return_data.size(), 0);
    EXPECT_EQ(st.status, ZVMC_SUCCESS);
    EXPECT_EQ(st.output_offset, 0);
    EXPECT_EQ(st.output_size, 0);

    EXPECT_EQ(st.current_block_cost, 0u);
    EXPECT_EQ(st.analysis.advanced, nullptr);
}

TEST(execution_state, reset_advanced)
{
    const zvmc_message msg{};
    const zvmone::advanced::AdvancedCodeAnalysis analysis;

    zvmone::advanced::AdvancedExecutionState st;
    st.gas_left = 1;
    st.gas_refund = 2;
    st.stack.push({});
    st.memory.grow(64);
    st.msg = &msg;
    st.rev = ZVMC_SHANGHAI;
    st.return_data.push_back('0');
    st.status = ZVMC_FAILURE;
    st.output_offset = 3;
    st.output_size = 4;
    st.current_block_cost = 5;
    st.analysis.advanced = &analysis;

    EXPECT_EQ(st.gas_left, 1);
    EXPECT_EQ(st.gas_refund, 2);
    EXPECT_EQ(st.stack.size(), 1);
    EXPECT_EQ(st.memory.size(), 64);
    EXPECT_EQ(st.msg, &msg);
    EXPECT_EQ(st.rev, ZVMC_SHANGHAI);
    EXPECT_EQ(st.return_data.size(), 1);
    EXPECT_EQ(st.status, ZVMC_FAILURE);
    EXPECT_EQ(st.output_offset, 3);
    EXPECT_EQ(st.output_size, 4u);
    EXPECT_EQ(st.current_block_cost, 5u);
    EXPECT_EQ(st.analysis.advanced, &analysis);

    {
        zvmc_message msg2{};
        msg2.gas = 13;
        const zvmc_host_interface host_interface2{};
        const uint8_t code2[]{0x80, 0x81};

        st.reset(msg2, ZVMC_SHANGHAI, host_interface2, nullptr, {code2, std::size(code2)});

        // TODO: We are not able to test HostContext with current API. It may require an execution
        //       test.
        EXPECT_EQ(st.gas_left, 13);
        EXPECT_EQ(st.gas_refund, 0);
        EXPECT_EQ(st.stack.size(), 0);
        EXPECT_EQ(st.memory.size(), 0);
        EXPECT_EQ(st.msg, &msg2);
        EXPECT_EQ(st.rev, ZVMC_SHANGHAI);
        EXPECT_EQ(st.return_data.size(), 0);
        EXPECT_EQ(st.status, ZVMC_SUCCESS);
        EXPECT_EQ(st.output_offset, 0);
        EXPECT_EQ(st.output_size, 0);
        EXPECT_EQ(st.current_block_cost, 0u);
        EXPECT_EQ(st.analysis.advanced, nullptr);
    }
}

TEST(execution_state, stack_reset)
{
    zvmone::StackSpace stack_space;
    zvmone::advanced::Stack stack{stack_space.bottom()};
    EXPECT_EQ(stack.size(), 0);

    stack.push({});
    EXPECT_EQ(stack.size(), 1);

    stack.reset(stack_space.bottom());
    EXPECT_EQ(stack.size(), 0);

    stack.reset(stack_space.bottom());
    EXPECT_EQ(stack.size(), 0);
}

TEST(execution_state, const_stack)
{
    zvmone::StackSpace stack_space;
    zvmone::advanced::Stack stack{stack_space.bottom()};
    stack.push(1);
    stack.push(2);

    const auto& cstack = stack;

    EXPECT_EQ(cstack[0], 2);
    EXPECT_EQ(cstack[1], 1);
}

TEST(execution_state, memory_view)
{
    zvmone::Memory memory;
    memory.grow(32);

    const zvmone::bytes_view view{memory.data(), memory.size()};
    ASSERT_EQ(view.size(), 32);
    EXPECT_EQ(view[0], 0x00);
    EXPECT_EQ(view[1], 0x00);
    EXPECT_EQ(view[2], 0x00);

    memory[0] = 0xc0;
    memory[2] = 0xc2;
    ASSERT_EQ(view.size(), 32);
    EXPECT_EQ(view[0], 0xc0);
    EXPECT_EQ(view[1], 0x00);
    EXPECT_EQ(view[2], 0xc2);
}

// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2019 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <test/utils/bytecode.hpp>
#include <zvmone/advanced_analysis.hpp>
#include <zvmone/instructions_traits.hpp>

namespace
{
// Temporarily include ZVMC instructions in an inline namespace so that zvmc_opcode enum
// doesn't name clash with zvmone::Opcode but the zvmc_ functions are accessible.
#include <zvmc/instructions.h>
}  // namespace

using namespace zvmone;

namespace zvmone::test
{
namespace
{
constexpr int unspecified = -1000000;

constexpr int get_revision_defined_in(size_t op) noexcept
{
    for (size_t r = ZVMC_SHANGHAI; r <= ZVMC_MAX_REVISION; ++r)
    {
        if (instr::gas_costs[r][op] != instr::undefined)
            return static_cast<int>(r);
    }
    return unspecified;
}

constexpr bool is_terminating(Opcode op) noexcept
{
    switch (op)
    {
    case OP_STOP:
    case OP_RETURN:
    case OP_REVERT:
    case OP_INVALID:
        return true;
    default:
        return false;
    }
}

template <Opcode Op>
constexpr void validate_traits_of() noexcept
{
    constexpr auto tr = instr::traits[Op];

    // immediate_size
    if constexpr (Op >= OP_PUSH1 && Op <= OP_PUSH32)
        static_assert(tr.immediate_size == Op - OP_PUSH1 + 1);
    else
        static_assert(tr.immediate_size == 0);

    // is_terminating
    static_assert(tr.is_terminating == is_terminating(Op));

    // since
    constexpr auto expected_rev = get_revision_defined_in(Op);
    static_assert(tr.since.has_value() ? *tr.since == expected_rev : expected_rev == unspecified);
}

template <std::size_t... Ops>
constexpr bool validate_traits(std::index_sequence<Ops...>)
{
    // Instantiate validate_traits_of for each opcode.
    // Validation errors are going to be reported via static_asserts.
    (validate_traits_of<static_cast<Opcode>(Ops)>(), ...);
    return true;
}
static_assert(validate_traits(std::make_index_sequence<256>{}));


// Check some cases for has_const_gas_cost().
static_assert(instr::has_const_gas_cost(OP_STOP));
static_assert(instr::has_const_gas_cost(OP_ADD));
static_assert(instr::has_const_gas_cost(OP_PUSH1));
}  // namespace

}  // namespace zvmone::test

TEST(instructions, compare_with_zvmc_instruction_tables)
{
    for (int r = ZVMC_SHANGHAI; r <= ZVMC_MAX_REVISION; ++r)
    {
        const auto rev = static_cast<zvmc_revision>(r);
        const auto& instr_tbl = instr::gas_costs[rev];
        const auto& zvmone_tbl = advanced::get_op_table(rev);
        const auto* zvmc_tbl = zvmc_get_instruction_metrics_table(rev);

        for (size_t i = 0; i < zvmone_tbl.size(); ++i)
        {
            const auto gas_cost = (instr_tbl[i] != instr::undefined) ? instr_tbl[i] : 0;
            const auto& metrics = zvmone_tbl[i];
            const auto& ref_metrics = zvmc_tbl[i];

            const auto case_descr = [rev](size_t opcode) {
                auto case_descr_str = std::ostringstream{};
                case_descr_str << "opcode " << instr::traits[opcode].name;
                case_descr_str << " on revision " << rev;
                return case_descr_str.str();
            };

            EXPECT_EQ(gas_cost, ref_metrics.gas_cost) << case_descr(i);
            EXPECT_EQ(metrics.gas_cost, ref_metrics.gas_cost) << case_descr(i);
            EXPECT_EQ(metrics.stack_req, ref_metrics.stack_height_required) << case_descr(i);
            EXPECT_EQ(metrics.stack_change, ref_metrics.stack_height_change) << case_descr(i);
        }
    }
}

TEST(instructions, compare_undefined_instructions)
{
    for (int r = ZVMC_SHANGHAI; r <= ZVMC_MAX_REVISION; ++r)
    {
        const auto rev = static_cast<zvmc_revision>(r);
        const auto& instr_tbl = instr::gas_costs[rev];
        const auto* zvmc_names_tbl = zvmc_get_instruction_names_table(rev);

        for (size_t i = 0; i < instr_tbl.size(); ++i)
        {
            EXPECT_EQ(instr_tbl[i] == instr::undefined, zvmc_names_tbl[i] == nullptr) << i;
        }
    }
}

TEST(instructions, compare_with_zvmc_instruction_names)
{
    const auto* zvmc_tbl = zvmc_get_instruction_names_table(ZVMC_MAX_REVISION);
    for (size_t i = 0; i < instr::traits.size(); ++i)
    {
        EXPECT_STREQ(instr::traits[i].name, zvmc_tbl[i]);
    }
}

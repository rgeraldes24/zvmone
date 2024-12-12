// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2020 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "baseline_instruction_table.hpp"
#include "instructions_traits.hpp"

namespace zvmone::baseline
{
namespace
{
constexpr auto common_cost_tables = []() noexcept {
    std::array<CostTable, ZVMC_MAX_REVISION + 1> tables{};
    for (size_t r = ZVMC_SHANGHAI; r <= ZVMC_MAX_REVISION; ++r)
    {
        auto& table = tables[r];
        for (size_t i = 0; i < table.size(); ++i)
        {
            table[i] = instr::gas_costs[r][i];  // Include instr::undefined in the table.
        }
    }
    return tables;
}();

}  // namespace

const CostTable& get_baseline_cost_table(zvmc_revision rev) noexcept
{
    const auto& tables = common_cost_tables;
    return tables[rev];
}
}  // namespace zvmone::baseline

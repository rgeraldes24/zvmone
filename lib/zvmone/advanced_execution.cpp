// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2019 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "advanced_execution.hpp"
#include "advanced_analysis.hpp"
#include <memory>

namespace zvmone::advanced
{
zvmc_result execute(AdvancedExecutionState& state, const AdvancedCodeAnalysis& analysis) noexcept
{
    state.analysis.advanced = &analysis;  // Allow accessing the analysis by instructions.

    const auto* instr = state.analysis.advanced->instrs.data();  // Get the first instruction.
    while (instr != nullptr)
        instr = instr->fn(instr, state);

    const auto gas_left =
        (state.status == ZVMC_SUCCESS || state.status == ZVMC_REVERT) ? state.gas_left : 0;
    const auto gas_refund = (state.status == ZVMC_SUCCESS) ? state.gas_refund : 0;

    assert(state.output_size != 0 || state.output_offset == 0);
    return zvmc::make_result(state.status, gas_left, gas_refund,
        state.memory.data() + state.output_offset, state.output_size);
}

zvmc_result execute(zvmc_vm* /*unused*/, const zvmc_host_interface* host, zvmc_host_context* ctx,
    zvmc_revision rev, const zvmc_message* msg, const uint8_t* code, size_t code_size) noexcept
{
    AdvancedCodeAnalysis analysis;
    const bytes_view container = {code, code_size};
    analysis = analyze(rev, container);
    auto state = std::make_unique<AdvancedExecutionState>(*msg, rev, *host, ctx, container);
    return execute(*state, analysis);
}
}  // namespace zvmone::advanced

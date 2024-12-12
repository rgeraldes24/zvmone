// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2018 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <zvmc/utils.h>
#include <zvmc/zvmc.h>

namespace zvmone::advanced
{
struct AdvancedExecutionState;
struct AdvancedCodeAnalysis;

/// Execute the already analyzed code using the provided execution state.
ZVMC_EXPORT zvmc_result execute(
    AdvancedExecutionState& state, const AdvancedCodeAnalysis& analysis) noexcept;

/// ZVMC-compatible execute() function.
zvmc_result execute(zvmc_vm* vm, const zvmc_host_interface* host, zvmc_host_context* ctx,
    zvmc_revision rev, const zvmc_message* msg, const uint8_t* code, size_t code_size) noexcept;
}  // namespace zvmone::advanced

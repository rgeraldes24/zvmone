// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2020 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <zvmc/utils.h>
#include <zvmc/zvmc.h>
#include <memory>
#include <string_view>
#include <vector>

namespace zvmone
{
using bytes_view = std::basic_string_view<uint8_t>;

class ExecutionState;
class VM;

namespace baseline
{
class CodeAnalysis
{
public:
    using JumpdestMap = std::vector<bool>;

    bytes_view executable_code;  ///< Executable code section.
    JumpdestMap jumpdest_map;    ///< Map of valid jump destinations.

private:
    /// Padded code for faster legacy code execution.
    /// If not nullptr the executable_code must point to it.
    std::unique_ptr<uint8_t[]> m_padded_code;

public:
    CodeAnalysis(std::unique_ptr<uint8_t[]> padded_code, size_t code_size, JumpdestMap map)
      : executable_code{padded_code.get(), code_size},
        jumpdest_map{std::move(map)},
        m_padded_code{std::move(padded_code)}
    {}
};
static_assert(std::is_move_constructible_v<CodeAnalysis>);
static_assert(std::is_move_assignable_v<CodeAnalysis>);
static_assert(!std::is_copy_constructible_v<CodeAnalysis>);
static_assert(!std::is_copy_assignable_v<CodeAnalysis>);

/// Analyze the code to build the bitmap of valid JUMPDEST locations.
ZVMC_EXPORT CodeAnalysis analyze(zvmc_revision /*rev*/, bytes_view code);

/// Executes in Baseline interpreter using ZVMC-compatible parameters.
zvmc_result execute(zvmc_vm* vm, const zvmc_host_interface* host, zvmc_host_context* ctx,
    zvmc_revision rev, const zvmc_message* msg, const uint8_t* code, size_t code_size) noexcept;

/// Executes in Baseline interpreter on the given external and initialized state.
ZVMC_EXPORT zvmc_result execute(
    const VM&, int64_t gas_limit, ExecutionState& state, const CodeAnalysis& analysis) noexcept;

}  // namespace baseline
}  // namespace zvmone

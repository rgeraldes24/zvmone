// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2022 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "../utils/stdx/utility.hpp"
#include <zvmc/zvmc.hpp>
#include <optional>

namespace zvmone::state
{
/// The total number of known precompiles ids, including 0.
inline constexpr std::size_t NumPrecompiles = 10;

enum class PrecompileId : uint8_t
{
    depositroot = 0x01,
    sha256 = 0x02,
    identity = 0x04,
    expmod = 0x05,
    ecadd = 0x06,
    ecmul = 0x07,
    ecpairing = 0x08,
};

struct ExecutionResult
{
    zvmc_status_code status_code;
    size_t output_size;
};

std::optional<zvmc::Result> call_precompile(zvmc_revision rev, const zvmc_message& msg) noexcept;
}  // namespace zvmone::state

// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2020 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <zvmc/zvmc.h>
#include <array>

namespace zvmone::baseline
{
using CostTable = std::array<int16_t, 256>;

const CostTable& get_baseline_cost_table(zvmc_revision rev) noexcept;
}  // namespace zvmone::baseline

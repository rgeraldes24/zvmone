// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2022 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "hash_utils.hpp"
#include "precompiles.hpp"
#include <zvmc/zvmc.hpp>
#include <array>
#include <cstdint>
#include <optional>
#include <unordered_map>

namespace zvmone::state
{
using zvmc::bytes;
using zvmc::bytes_view;

class Cache
{
    std::array<std::unordered_map<hash256, std::optional<bytes>>, NumPrecompiles> m_cache;

public:
    Cache() noexcept;
    ~Cache() noexcept;

    /// Lookups the precompiles cache.
    ///
    /// @param id        The precompile ID.
    /// @param input     The input for precompile execution.
    /// @param gas_left  The amount of gas left _after_ execution,
    ///                  used for constructing the result for successful execution.
    /// @return          The cached execution result
    ///                  or std::nullopt if the matching cache entry is not found.
    std::optional<zvmc::Result> find(PrecompileId id, bytes_view input, int64_t gas_left) const;

    /// Inserts new precompiles cache entry.
    void insert(PrecompileId id, bytes_view input, const zvmc::Result& result);
};
}  // namespace zvmone::state

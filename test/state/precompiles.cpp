// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2022 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "precompiles.hpp"
#include "precompiles_cache.hpp"
#include <intx/intx.hpp>
#include <bit>
#include <cassert>
#include <iostream>
#include <limits>
#include <unordered_map>

namespace zvmone::state
{
using namespace zvmc::literals;

namespace
{
constexpr auto GasCostMax = std::numeric_limits<int64_t>::max();

struct PrecompileAnalysis
{
    int64_t gas_cost;
    size_t max_output_size;
};

inline constexpr int64_t num_words(size_t size_in_bytes) noexcept
{
    return static_cast<int64_t>((size_in_bytes + 31) / 32);
}

template <int BaseCost, int WordCost>
inline constexpr int64_t cost_per_input_word(size_t input_size) noexcept
{
    return BaseCost + WordCost * num_words(input_size);
}

PrecompileAnalysis depositroot_analyze(bytes_view /*input*/, zvmc_revision /*rev*/) noexcept
{
    return {19992, 32};
}

PrecompileAnalysis sha256_analyze(bytes_view input, zvmc_revision /*rev*/) noexcept
{
    return {cost_per_input_word<60, 12>(input.size()), 32};
}

PrecompileAnalysis identity_analyze(bytes_view input, zvmc_revision /*rev*/) noexcept
{
    return {cost_per_input_word<15, 3>(input.size()), input.size()};
}

PrecompileAnalysis ecadd_analyze(bytes_view /*input*/, zvmc_revision /*rev*/) noexcept
{
    return {150, 64};
}

PrecompileAnalysis ecmul_analyze(bytes_view /*input*/, zvmc_revision /*rev*/) noexcept
{
    return {6000, 64};
}

PrecompileAnalysis ecpairing_analyze(bytes_view input, zvmc_revision /*rev*/) noexcept
{
    const auto base_cost = 45000;
    const auto element_cost = 34000;
    const auto num_elements = static_cast<int64_t>(input.size() / 192);
    return {base_cost + num_elements * element_cost, 32};
}

PrecompileAnalysis expmod_analyze(bytes_view input, zvmc_revision /*rev*/) noexcept
{
    using namespace intx;

    static constexpr size_t input_header_required_size = 3 * sizeof(uint256);
    const int64_t min_gas = 200;

    uint8_t input_header[input_header_required_size]{};
    std::copy_n(input.data(), std::min(input.size(), input_header_required_size), input_header);

    const auto base_len = be::unsafe::load<uint256>(&input_header[0]);
    const auto exp_len = be::unsafe::load<uint256>(&input_header[32]);
    const auto mod_len = be::unsafe::load<uint256>(&input_header[64]);

    if (base_len == 0 && mod_len == 0)
        return {min_gas, 0};

    static constexpr auto len_limit = std::numeric_limits<size_t>::max();
    if (base_len > len_limit || exp_len > len_limit || mod_len > len_limit)
        return {GasCostMax, 0};

    auto adjusted_len = [input](size_t offset, size_t len) {
        const auto head_len = std::min(len, size_t{32});
        const auto head_explicit_len =
            std::max(std::min(offset + head_len, input.size()), offset) - offset;
        const bytes_view head_explicit_bytes(&input[offset], head_explicit_len);
        const auto top_byte_index = head_explicit_bytes.find_first_not_of(uint8_t{0});
        const size_t exp_bit_width =
            (top_byte_index != bytes_view::npos) ?
                (head_len - top_byte_index - 1) * 8 +
                    static_cast<size_t>(std::bit_width(head_explicit_bytes[top_byte_index])) :
                0;

        return std::max(
            8 * (std::max(len, size_t{32}) - 32) + (std::max(exp_bit_width, size_t{1}) - 1),
            size_t{1});
    };

    static constexpr auto mult_complexity_eip2565 = [](const uint256& x) noexcept {
        const auto w = (x + 7) >> 3;
        return w * w;
    };

    const auto max_len = std::max(mod_len, base_len);
    const auto adjusted_exp_len = adjusted_len(
        sizeof(input_header) + static_cast<size_t>(base_len), static_cast<size_t>(exp_len));
    const auto gas = mult_complexity_eip2565(max_len) * adjusted_exp_len / 3;
    return {std::max(min_gas, static_cast<int64_t>(std::min(gas, intx::uint256{GasCostMax}))),
        static_cast<size_t>(mod_len)};
}

ExecutionResult identity_execute(const uint8_t* input, size_t input_size, uint8_t* output,
    [[maybe_unused]] size_t output_size) noexcept
{
    assert(output_size == input_size);
    std::copy_n(input, input_size, output);
    return {ZVMC_SUCCESS, input_size};
}

struct PrecompileTraits
{
    decltype(identity_analyze)* analyze = nullptr;
    decltype(identity_execute)* execute = nullptr;
};

template <PrecompileId Id>
ExecutionResult dummy_execute(const uint8_t*, size_t, uint8_t*, size_t) noexcept
{
    std::cerr << "Precompile " << static_cast<int>(Id) << " not implemented!\n";
    return ExecutionResult{ZVMC_INTERNAL_ERROR, 0};
}

inline constexpr auto traits = []() noexcept {
    std::array<PrecompileTraits, NumPrecompiles> tbl{{
        {},  // undefined for 0
        {depositroot_analyze, dummy_execute<PrecompileId::depositroot>},
        {sha256_analyze, dummy_execute<PrecompileId::sha256>},
        {identity_analyze, identity_execute},
        {expmod_analyze, dummy_execute<PrecompileId::expmod>},
        {ecadd_analyze, dummy_execute<PrecompileId::ecadd>},
        {ecmul_analyze, dummy_execute<PrecompileId::ecmul>},
        {ecpairing_analyze, dummy_execute<PrecompileId::ecpairing>},
    }};
    return tbl;
}();
}  // namespace

std::optional<zvmc::Result> call_precompile(zvmc_revision rev, const zvmc_message& msg) noexcept
{
    // Define compile-time constant,
    // TODO: workaround for Clang Analyzer bug https://github.com/llvm/llvm-project/issues/59493.
    static constexpr zvmc::address address_boundary{NumPrecompiles};

    if (zvmc::is_zero(msg.code_address) || msg.code_address >= address_boundary)
        return {};

    const auto id = msg.code_address.bytes[23];

    assert(id > 0);
    assert(msg.gas >= 0);

    const auto [analyze, execute] = traits[id];

    const bytes_view input{msg.input_data, msg.input_size};
    const auto [gas_cost, max_output_size] = analyze(input, rev);
    const auto gas_left = msg.gas - gas_cost;
    if (gas_left < 0)
        return zvmc::Result{ZVMC_OUT_OF_GAS};

    static Cache cache;
    if (auto r = cache.find(static_cast<PrecompileId>(id), input, gas_left); r.has_value())
        return r;

    uint8_t output_buf[256];  // Big enough to handle all "expmod" tests.
    assert(std::size(output_buf) >= max_output_size);

    const auto [status_code, output_size] =
        execute(msg.input_data, msg.input_size, output_buf, max_output_size);

    zvmc::Result result{
        status_code, status_code == ZVMC_SUCCESS ? gas_left : 0, 0, output_buf, output_size};

    cache.insert(static_cast<PrecompileId>(id), input, result);

    return result;
}
}  // namespace zvmone::state

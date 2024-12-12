// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2019 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "instructions.hpp"

namespace zvmone::instr::core
{
namespace
{
/// The gas cost specification for storage instructions.
struct StorageCostSpec
{
    int16_t warm_access;  ///< Storage warm access cost, YP: G_{warmaccess}
    int16_t set;          ///< Storage addition cost, YP: G_{sset}
    int16_t reset;        ///< Storage modification cost, YP: G_{sreset}
    int16_t clear;        ///< Storage deletion refund, YP: R_{sclear}
};

/// Table of gas cost specification for storage instructions per ZVM revision.
/// TODO: This can be moved to instruction traits and be used in other places: e.g.
///       SLOAD cost, replacement for warm_storage_read_cost.
constexpr auto storage_cost_spec = []() noexcept {
    std::array<StorageCostSpec, ZVMC_MAX_REVISION + 1> tbl{};

    // Net cost schedule.
    tbl[ZVMC_SHANGHAI] = {
        instr::warm_storage_read_cost, 20000, 5000 - instr::cold_sload_cost, 4800};
    return tbl;
}();


struct StorageStoreCost
{
    int16_t gas_cost;
    int16_t gas_refund;
};

// The lookup table of SSTORE costs by the storage update status.
constexpr auto sstore_costs = []() noexcept {
    std::array<std::array<StorageStoreCost, ZVMC_STORAGE_MODIFIED_RESTORED + 1>,
        ZVMC_MAX_REVISION + 1>
        tbl{};

    for (size_t rev = ZVMC_SHANGHAI; rev <= ZVMC_MAX_REVISION; ++rev)
    {
        auto& e = tbl[rev];
        const auto c = storage_cost_spec[rev];
        e[ZVMC_STORAGE_ASSIGNED] = {c.warm_access, 0};
        e[ZVMC_STORAGE_ADDED] = {c.set, 0};
        e[ZVMC_STORAGE_DELETED] = {c.reset, c.clear};
        e[ZVMC_STORAGE_MODIFIED] = {c.reset, 0};
        e[ZVMC_STORAGE_DELETED_ADDED] = {c.warm_access, static_cast<int16_t>(-c.clear)};
        e[ZVMC_STORAGE_MODIFIED_DELETED] = {c.warm_access, c.clear};
        e[ZVMC_STORAGE_DELETED_RESTORED] = {
            c.warm_access, static_cast<int16_t>(c.reset - c.warm_access - c.clear)};
        e[ZVMC_STORAGE_ADDED_DELETED] = {
            c.warm_access, static_cast<int16_t>(c.set - c.warm_access)};
        e[ZVMC_STORAGE_MODIFIED_RESTORED] = {
            c.warm_access, static_cast<int16_t>(c.reset - c.warm_access)};
    }

    return tbl;
}();
}  // namespace

Result sload(StackTop stack, int64_t gas_left, ExecutionState& state) noexcept
{
    auto& x = stack.top();
    const auto key = intx::be::store<zvmc::bytes32>(x);

    if (state.host.access_storage(state.msg->recipient, key) == ZVMC_ACCESS_COLD)
    {
        // The warm storage access cost is already applied (from the cost table).
        // Here we need to apply additional cold storage access cost.
        constexpr auto additional_cold_sload_cost =
            instr::cold_sload_cost - instr::warm_storage_read_cost;
        if ((gas_left -= additional_cold_sload_cost) < 0)
            return {ZVMC_OUT_OF_GAS, gas_left};
    }

    x = intx::be::load<uint256>(state.host.get_storage(state.msg->recipient, key));

    return {ZVMC_SUCCESS, gas_left};
}

Result sstore(StackTop stack, int64_t gas_left, ExecutionState& state) noexcept
{
    if (state.in_static_mode())
        return {ZVMC_STATIC_MODE_VIOLATION, gas_left};

    if (gas_left <= 2300)
        return {ZVMC_OUT_OF_GAS, gas_left};

    const auto key = intx::be::store<zvmc::bytes32>(stack.pop());
    const auto value = intx::be::store<zvmc::bytes32>(stack.pop());

    const auto gas_cost_cold =
        (state.host.access_storage(state.msg->recipient, key) == ZVMC_ACCESS_COLD) ?
            instr::cold_sload_cost :
            0;
    const auto status = state.host.set_storage(state.msg->recipient, key, value);

    const auto [gas_cost_warm, gas_refund] = sstore_costs[state.rev][status];
    const auto gas_cost = gas_cost_warm + gas_cost_cold;
    if ((gas_left -= gas_cost) < 0)
        return {ZVMC_OUT_OF_GAS, gas_left};
    state.gas_refund += gas_refund;
    return {ZVMC_SUCCESS, gas_left};
}
}  // namespace zvmone::instr::core

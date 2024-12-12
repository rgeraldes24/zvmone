// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2018 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

/// @file
/// ZVMC instance (class VM) and entry point of zvmone is defined here.

#include "vm.hpp"
#include "advanced_execution.hpp"
#include "baseline.hpp"
#include <zvmone/zvmone.h>
#include <cassert>
#include <iostream>

namespace zvmone
{
namespace
{
void destroy(zvmc_vm* vm) noexcept
{
    assert(vm != nullptr);
    delete static_cast<VM*>(vm);
}

constexpr zvmc_capabilities_flagset get_capabilities(zvmc_vm* /*vm*/) noexcept
{
    return ZVMC_CAPABILITY_ZVM1;
}

zvmc_set_option_result set_option(zvmc_vm* c_vm, char const* c_name, char const* c_value) noexcept
{
    const auto name = (c_name != nullptr) ? std::string_view{c_name} : std::string_view{};
    const auto value = (c_value != nullptr) ? std::string_view{c_value} : std::string_view{};
    auto& vm = *static_cast<VM*>(c_vm);

    if (name == "advanced")
    {
        c_vm->execute = zvmone::advanced::execute;
        return ZVMC_SET_OPTION_SUCCESS;
    }
    else if (name == "cgoto")
    {
#if ZVMONE_CGOTO_SUPPORTED
        if (value == "no")
        {
            vm.cgoto = false;
            return ZVMC_SET_OPTION_SUCCESS;
        }
        return ZVMC_SET_OPTION_INVALID_VALUE;
#else
        return ZVMC_SET_OPTION_INVALID_NAME;
#endif
    }
    else if (name == "trace")
    {
        vm.add_tracer(create_instruction_tracer(std::cerr));
        return ZVMC_SET_OPTION_SUCCESS;
    }
    else if (name == "histogram")
    {
        vm.add_tracer(create_histogram_tracer(std::cerr));
        return ZVMC_SET_OPTION_SUCCESS;
    }
    return ZVMC_SET_OPTION_INVALID_NAME;
}

}  // namespace


inline constexpr VM::VM() noexcept
  : zvmc_vm{
        ZVMC_ABI_VERSION,
        "zvmone",
        PROJECT_VERSION,
        zvmone::destroy,
        zvmone::baseline::execute,
        zvmone::get_capabilities,
        zvmone::set_option,
    }
{}

}  // namespace zvmone

extern "C" {
ZVMC_EXPORT zvmc_vm* zvmc_create_zvmone() noexcept
{
    return new zvmone::VM{};
}
}

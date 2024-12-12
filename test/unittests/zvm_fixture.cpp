// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2019-2020 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "zvm_fixture.hpp"
#include <zvmone/zvmone.h>

namespace zvmone::test
{
namespace
{
zvmc::VM advanced_vm{zvmc_create_zvmone(), {{"advanced", ""}}};
zvmc::VM baseline_vm{zvmc_create_zvmone()};
zvmc::VM bnocgoto_vm{zvmc_create_zvmone(), {{"cgoto", "no"}}};

const char* print_vm_name(const testing::TestParamInfo<zvmc::VM*>& info) noexcept
{
    if (info.param == &advanced_vm)
        return "advanced";
    if (info.param == &baseline_vm)
        return "baseline";
    if (info.param == &bnocgoto_vm)
        return "bnocgoto";
    return "unknown";
}
}  // namespace

INSTANTIATE_TEST_SUITE_P(
    zvmone, zvm, testing::Values(&advanced_vm, &baseline_vm, &bnocgoto_vm), print_vm_name);

bool zvm::is_advanced() noexcept
{
    return GetParam() == &advanced_vm;
}
}  // namespace zvmone::test

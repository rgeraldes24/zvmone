// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2019-2020 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <zvmc/zvmc.hpp>
#include <zvmone/vm.hpp>
#include <zvmone/zvmone.h>

TEST(zvmone, info)
{
    auto vm = zvmc::VM{zvmc_create_zvmone()};
    EXPECT_STREQ(vm.name(), "zvmone");
    EXPECT_STREQ(vm.version(), PROJECT_VERSION);
    EXPECT_TRUE(vm.is_abi_compatible());
}

TEST(zvmone, capabilities)
{
    auto vm = zvmc_create_zvmone();
    EXPECT_EQ(vm->get_capabilities(vm), zvmc_capabilities_flagset{ZVMC_CAPABILITY_ZVM1});
    vm->destroy(vm);
}

TEST(zvmone, set_option_invalid)
{
    auto vm = zvmc_create_zvmone();
    ASSERT_NE(vm->set_option, nullptr);
    EXPECT_EQ(vm->set_option(vm, "", ""), ZVMC_SET_OPTION_INVALID_NAME);
    EXPECT_EQ(vm->set_option(vm, "o", ""), ZVMC_SET_OPTION_INVALID_NAME);
    EXPECT_EQ(vm->set_option(vm, "0", ""), ZVMC_SET_OPTION_INVALID_NAME);
    vm->destroy(vm);
}

TEST(zvmone, set_option_advanced)
{
    auto vm = zvmc::VM{zvmc_create_zvmone()};
    EXPECT_EQ(vm.set_option("advanced", ""), ZVMC_SET_OPTION_SUCCESS);

    // This will also enable Advanced.
    EXPECT_EQ(vm.set_option("advanced", "no"), ZVMC_SET_OPTION_SUCCESS);
}

TEST(zvmone, set_option_cgoto)
{
    zvmc::VM vm{zvmc_create_zvmone()};

#if ZVMONE_CGOTO_SUPPORTED
    EXPECT_EQ(vm.set_option("cgoto", ""), ZVMC_SET_OPTION_INVALID_VALUE);
    EXPECT_EQ(vm.set_option("cgoto", "yes"), ZVMC_SET_OPTION_INVALID_VALUE);
    EXPECT_EQ(vm.set_option("cgoto", "no"), ZVMC_SET_OPTION_SUCCESS);
#else
    EXPECT_EQ(vm.set_option("cgoto", "no"), ZVMC_SET_OPTION_INVALID_NAME);
#endif
}

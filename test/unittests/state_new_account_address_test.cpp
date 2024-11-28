// evmone: Fast Ethereum Virtual Machine implementation
// Copyright 2023 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <test/state/host.hpp>

using namespace evmc;
using namespace evmc::literals;
inline constexpr auto addr = evmone::state::compute_new_account_address;

inline constexpr uint64_t nonces[] = {0, 1, 0x80, 0xffffffffffffffff};
inline constexpr address senders[] = {
    "Z00"_address, "Z01"_address, "Z8000000000000000000000000000000000000000"_address};
inline const bytes init_codes[] = {bytes{}, bytes{0xFE}};
inline constexpr bytes32 salts[] = {
    0x00_bytes32, 0xe75fb554e433e03763a1560646ee22dcb74e5274b34c5ad644e7c0f619a7e1d0_bytes32};

TEST(state_new_account_address, create)
{
    for (const auto& ic : init_codes)  // Init-code doesn't affect CREATE.
    {
        auto s = senders[0];
        EXPECT_EQ(addr(s, nonces[0], {}, ic), "Zbd770416a3345f91e4b34576cb804a576fa48eb1"_address);
        EXPECT_EQ(addr(s, nonces[3], {}, ic), "Z1262d73ea59d3a661bf8751d16cf1a5377149e75"_address);

        s = senders[1];
        EXPECT_EQ(addr(s, nonces[0], {}, ic), "Z522b3294e6d06aa25ad0f1b8891242e335d3b459"_address);
        EXPECT_EQ(addr(s, nonces[1], {}, ic), "Z535b3d7a252fa034ed71f0c53ec0c6f784cb64e1"_address);
        EXPECT_EQ(addr(s, nonces[2], {}, ic), "Z09c1ef8f55c61b94e8b92a55d0891d408a991e18"_address);
        EXPECT_EQ(addr(s, nonces[3], {}, ic), "Z001567239734aeadea21023c2a7c0d9bb9ae4af9"_address);

        s = senders[2];
        EXPECT_EQ(addr(s, nonces[0], {}, ic), "Z3cb1045aee4a06f522ea2b69e4f3d21ed3c135d1"_address);
        EXPECT_EQ(addr(s, nonces[3], {}, ic), "Ze1aa03e4a7b6991d69aff8ece53ceafdf347082e"_address);

        const auto beacon_deposit_address =
            addr("Zb20a608c624Ca5003905aA834De7156C68b2E1d0"_address, 0, {}, ic);
        EXPECT_EQ(beacon_deposit_address, "Z00000000219ab540356cbb839cbe05303d7705fa"_address);
    }
}

TEST(state_new_account_address, create2)
{
    for (const auto n : nonces)  // Nonce doesn't affect CREATE2.
    {
        EXPECT_EQ(addr(senders[0], n, salts[0], init_codes[0]),
            "Ze33c0c7f7df4809055c3eba6c09cfe4baf1bd9e0"_address);

        EXPECT_EQ(addr(senders[2], n, salts[0], init_codes[1]),
            "Z3517dea701ed18fc4a99dc111c5946e1f1541dad"_address);

        EXPECT_EQ(addr(senders[1], n, salts[1], init_codes[0]),
            "Z7be1c1cb3b8298f21c56add66defce03e2d32604"_address);

        EXPECT_EQ(addr(senders[2], n, salts[1], init_codes[1]),
            "Z8f459e65c8f00a9c0c0493de7b0c61c3c27f7384"_address);
    }
}

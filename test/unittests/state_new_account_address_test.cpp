// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2023 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <test/state/host.hpp>

using namespace zvmc;
using namespace zvmc::literals;
inline constexpr auto addr = zvmone::state::compute_new_account_address;

inline constexpr uint64_t nonces[] = {0, 1, 0x80, 0xffffffffffffffff};
inline constexpr address senders[] = {
    "Z00"_address, "Z01"_address, "Z800000000000000000000000000000000000000000000000"_address};
inline const bytes init_codes[] = {bytes{}, bytes{0xFE}};
inline constexpr bytes32 salts[] = {
    0x00_bytes32, 0xe75fb554e433e03763a1560646ee22dcb74e5274b34c5ad644e7c0f619a7e1d0_bytes32};

TEST(state_new_account_address, create)
{
    for (const auto& ic : init_codes)  // Init-code doesn't affect CREATE.
    {
        auto s = senders[0];
        EXPECT_EQ(addr(s, nonces[0], {}, ic), "Zf7f2855b719fab117571ba72b464821b20b8a291213b5efc"_address);
        EXPECT_EQ(addr(s, nonces[3], {}, ic), "Z2519161e0d96846c460b2b523d05aac64c6b736ceb2031c2"_address);

        s = senders[1];
        EXPECT_EQ(addr(s, nonces[0], {}, ic), "Z0db73ba3ce32c2940c040c8ed45dc5b644a98407eb8d1c94"_address);
        EXPECT_EQ(addr(s, nonces[1], {}, ic), "Za5b07a9c31e152a61faac2780b510276f1157d34f593b673"_address);
        EXPECT_EQ(addr(s, nonces[2], {}, ic), "Z6b8a6ed311fe18c9d803887895f23ff0985dde9656bbbc8e"_address);
        EXPECT_EQ(addr(s, nonces[3], {}, ic), "Z6b30e290e38c7b9f977a4b4ab18eee76e10fc8fb14b25f44"_address);

        s = senders[2];
        EXPECT_EQ(addr(s, nonces[0], {}, ic), "Z470468759abfb31ffbf2a254d468c29831961d3345e7039f"_address);
        EXPECT_EQ(addr(s, nonces[3], {}, ic), "Zdf65ec1f119f0dcd531ca80e6a74ab0e0504bfbf9403a1b3"_address);

        const auto beacon_deposit_address =
            addr("Z856d2d2c496f09d828ea5a5c013b4315f5c039c381bb6eee"_address, 0, {}, ic);
        EXPECT_EQ(beacon_deposit_address, "Z493ac2255d8fd261b2ade5113ac327ae923133b2dfe17efa"_address);
    }
}

TEST(state_new_account_address, create2)
{
    for (const auto n : nonces)  // Nonce doesn't affect CREATE2.
    {
        EXPECT_EQ(addr(senders[0], n, salts[0], init_codes[0]),
            "Z64e269873ff7334e1c900fc48f4d100dd4a3db230315c50c"_address);

        EXPECT_EQ(addr(senders[2], n, salts[0], init_codes[1]),
            "Zc5c3232ebb5127119e264513f00d7e2410daaf3c90602175"_address);

        EXPECT_EQ(addr(senders[1], n, salts[1], init_codes[0]),
            "Z1d0f46d99b9fa2dfd64d3677f1c3833bfb0091b7de50ae24"_address);

        EXPECT_EQ(addr(senders[2], n, salts[1], init_codes[1]),
            "Z7ccb4f0aa7854099b5d0e48e9ae2ee2ca81e5eba036a1dc7"_address);
    }
}

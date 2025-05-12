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
        EXPECT_EQ(addr(s, nonces[0], {}, ic), "Z541e37d13b0b5d00b40a8f2c7c63bb721d9a60ba45ecc81c"_address);
        EXPECT_EQ(addr(s, nonces[3], {}, ic), "Ze84b1b3915a1dc1e05e47be9bbb9b4c1f7dd393ef749b6cd"_address);

        s = senders[1];
        EXPECT_EQ(addr(s, nonces[0], {}, ic), "Z6f1dd739caaab1da726313b79ebe720b36724636706d6428"_address);
        EXPECT_EQ(addr(s, nonces[1], {}, ic), "Z4191879a336f7140de1a53154955245c97d7550d195a98d4"_address);
        EXPECT_EQ(addr(s, nonces[2], {}, ic), "Z245f97a561198bfaff3f070b6f4cd5be3ccd232779a0a7da"_address);
        EXPECT_EQ(addr(s, nonces[3], {}, ic), "Zcaad888cef88630918fafc308ca1283866638709fd120736"_address);

        s = senders[2];
        EXPECT_EQ(addr(s, nonces[0], {}, ic), "Zf7c525f004ddf03cb1312b549636eab242ad552089c683cc"_address);
        EXPECT_EQ(addr(s, nonces[3], {}, ic), "Z76f79604fa72bc3a20f0feb471b17696f1e7c0789cfbaedb"_address);

        const auto beacon_deposit_address =
            addr("Z856d2d2c496f09d828ea5a5c013b4315f5c039c381bb6eee"_address, 0, {}, ic);
        EXPECT_EQ(beacon_deposit_address, "Ze63ca458d1401e0b2961fe49c39a31d30fcdfe9fa0462447"_address);
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

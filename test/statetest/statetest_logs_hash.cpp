// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2022 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "../state/rlp.hpp"
#include "statetest.hpp"

namespace zvmone::test
{
hash256 logs_hash(const std::vector<state::Log>& logs)
{
    return keccak256(rlp::encode(logs));
}
}  // namespace zvmone::test

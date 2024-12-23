// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2022 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "hash_utils.hpp"
#include <span>
#include <unordered_map>

namespace zvmone::state
{
struct Account;
struct Transaction;
struct TransactionReceipt;

/// Computes Merkle Patricia Trie root hash for the given collection of state accounts.
hash256 mpt_hash(const std::unordered_map<address, Account>& accounts);

/// Computes Merkle Patricia Trie root hash for the given collection of transactions.
hash256 mpt_hash(std::span<const Transaction> transactions);

/// Computes Merkle Patricia Trie root hash for the given collection of transactions receipts.
hash256 mpt_hash(std::span<const TransactionReceipt> receipts);

}  // namespace zvmone::state

// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2018 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <zvmc/hex.hpp>

using zvmc::bytes;
using zvmc::bytes_view;
using zvmc::from_hex;
using zvmc::from_spaced_hex;
using zvmc::hex;

/// Converts a string to bytes by casting individual characters.
inline bytes to_bytes(std::string_view s)
{
    return {s.begin(), s.end()};
}

/// Produces bytes out of string literal.
inline bytes operator""_b(const char* data, size_t size)
{
    return to_bytes({data, size});
}

inline bytes operator""_hex(const char* s, size_t size)
{
    return from_spaced_hex({s, size}).value();
}

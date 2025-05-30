# zvmone: Fast Zond Virtual Machine implementation
# Copyright 2022 The evmone Authors.
# SPDX-License-Identifier: Apache-2.0

add_library(zvmone-state STATIC)
add_library(zvmone::state ALIAS zvmone-state)
target_link_libraries(zvmone-state PUBLIC zvmc::zvmc_cpp PRIVATE zvmone ethash::keccak)
target_include_directories(zvmone-state PRIVATE ${zvmone_private_include_dir})
target_sources(
    zvmone-state PRIVATE
    account.hpp
    bloom_filter.hpp
    bloom_filter.cpp
    errors.hpp
    hash_utils.hpp
    hash_utils.cpp
    host.hpp
    host.cpp
    mpt.hpp
    mpt.cpp
    mpt_hash.hpp
    mpt_hash.cpp
    precompiles.hpp
    precompiles.cpp
    precompiles_cache.hpp
    precompiles_cache.cpp
    rlp.hpp
    state.hpp
    state.cpp
)

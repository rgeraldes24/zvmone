// zvmone: Fast Zond Virtual Machine implementation
// Copyright 2018-2019 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#ifndef ZVMONE_H
#define ZVMONE_H

#include <zvmc/utils.h>
#include <zvmc/zvmc.h>

#if __cplusplus
extern "C" {
#endif

ZVMC_EXPORT struct zvmc_vm* zvmc_create_zvmone(void) ZVMC_NOEXCEPT;

#if __cplusplus
}
#endif

#endif  // ZVMONE_H

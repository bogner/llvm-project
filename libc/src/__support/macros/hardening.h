//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// \file
// Hardning mode macros.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIBC_SRC___SUPPORT_MACROS_HARDENING_H
#define LLVM_LIBC_SRC___SUPPORT_MACROS_HARDENING_H

#define LIBC_HARDENING_MODE_NONE 0x0000000F
#define LIBC_HARDENING_MODE_FAST 0x000000F0
#define LIBC_HARDENING_MODE_EXTENSIVE 0x00000F00
#define LIBC_HARDENING_MODE_DEBUG 0x0000F000

#ifndef LIBC_COPT_HARDENING_MODE
#define LIBC_COPT_HARDENING_MODE LIBC_HARDENING_MODE_NONE
#endif

#if (LIBC_COPT_HARDENING_MODE != LIBC_HARDENING_MODE_NONE &&                   \
     LIBC_COPT_HARDENING_MODE != LIBC_HARDENING_MODE_FAST &&                   \
     LIBC_COPT_HARDENING_MODE != LIBC_HARDENING_MODE_EXTENSIVE &&              \
     LIBC_COPT_HARDENING_MODE != LIBC_HARDENING_MODE_DEBUG)
#error                                                                         \
    "LIBC_COPT_HARDENING_MODE must be defined with one of the following values: \
LIBC_HARDENING_MODE_NONE, LIBC_HARDENING_MODE_FAST, \
LIBC_HARDENING_MODE_EXTENSIVE, LIBC_HARDENING_MODE_DEBUG"
#endif

#endif // LLVM_LIBC_SRC___SUPPORT_MACROS_HARDENING_H

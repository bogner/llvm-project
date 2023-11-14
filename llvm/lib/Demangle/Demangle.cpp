//===-- Demangle.cpp - Common demangling functions ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file This file contains definitions of common demangling functions.
///
//===----------------------------------------------------------------------===//

#include "llvm/Demangle/Demangle.h"
#include "llvm/Demangle/StringViewExtras.h"
#include <cstdlib>
#include <string_view>

using llvm::itanium_demangle::starts_with;

std::string llvm::demangle(std::string_view MangledName) {
  std::string Result;

  if (nonMicrosoftDemangle(MangledName, Result))
    return Result;

  if (starts_with(MangledName, '_') &&
      nonMicrosoftDemangle(MangledName.substr(1), Result,
                           /*CanHaveLeadingDot=*/false))
    return Result;

  if (char *Demangled = microsoftDemangle(MangledName, nullptr, nullptr)) {
    Result = Demangled;
    std::free(Demangled);
  } else {
    Result = MangledName;
  }
  return Result;
}

static bool isItaniumEncoding(std::string_view S) {
  // Itanium encoding requires 1 or 3 leading underscores, followed by 'Z'.
  return starts_with(S, "_Z") || starts_with(S, "___Z");
}

static bool isRustEncoding(std::string_view S) { return starts_with(S, "_R"); }

static bool isDLangEncoding(std::string_view S) { return starts_with(S, "_D"); }

static bool isMSEncoding(std::string_view S) { return starts_with(S, "?"); }

bool llvm::nonMicrosoftDemangle(std::string_view MangledName,
                                std::string &Result, bool CanHaveLeadingDot) {
  DemangleStyle Style(DemangleStyle::Any() & ~DemangleStyle::Microsoft());
  if (std::optional<std::string> Str = demangleWithStyle(MangledName, Style)) {
    Result = *Str;
    return true;
  }
  return false;
}

std::optional<std::string>
llvm::demangleWithStyle(std::string_view MangledName,
                        llvm::DemangleStyle StyleFlags,
                        bool CanHaveLeadingDot) {
  std::string Result;
  char *Demangled = nullptr;

  // Do not consider the dot prefix as part of the demangled symbol name.
  if (CanHaveLeadingDot && MangledName.size() > 0 && MangledName[0] == '.') {
    MangledName.remove_prefix(1);
    Result = ".";
  }

  if ((StyleFlags & DemangleStyle::Itanium()) && isItaniumEncoding(MangledName))
    Demangled = itaniumDemangle(MangledName);
  else if ((StyleFlags & DemangleStyle::Rust()) && isRustEncoding(MangledName))
    Demangled = rustDemangle(MangledName);
  else if ((StyleFlags & DemangleStyle::D()) && isDLangEncoding(MangledName))
    Demangled = dlangDemangle(MangledName);
  else if ((StyleFlags & DemangleStyle::Microsoft()) &&
           isMSEncoding(MangledName))
    Demangled = microsoftDemangle(MangledName, nullptr, nullptr);

  if (!Demangled)
    return {};

  Result += Demangled;
  std::free(Demangled);
  return Result;


}

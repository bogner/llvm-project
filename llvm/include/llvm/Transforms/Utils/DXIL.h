//===- DXIL.h - Abstractions for DXIL constructs ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// \file This file defines various abstractions for transforming between DXIL's
// and LLVM's representations of shader metadata.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_DXIL_H
#define LLVM_TRANSFORMS_UTILS_DXIL_H

#include "llvm/Support/Error.h"
#include "llvm/TargetParser/Triple.h"

namespace llvm {
class Module;

namespace dxil {

class ShaderStage {
  Triple::EnvironmentType Stage = Triple::Library;

public:
  ShaderStage() = default;
  ShaderStage(Triple::EnvironmentType Stage) : Stage(Stage) {}

  static Expected<ShaderStage> fromTriple(Triple T);
  static Expected<ShaderStage> fromStageName(StringRef Name);
  static Expected<ShaderStage> fromShortName(StringRef Name);

  bool isLibrary() const { return Stage == Triple::Library; }

  /// Get the short name of the stage, suitable for DXIL metadata.
  StringRef getShortName() const;
  /// Get the name of the stage, suitable for an entry attribute.
  StringRef getEntryName() const {
    assert(!isLibrary() && "Cannot use Library shader as entry attribute");
    return Triple::getEnvironmentTypeName(Stage);
  }
  /// Get the environment type of the stage for representation in the Triple.
  Triple::EnvironmentType getTripleEnv() const { return Stage; }
};


class ShaderModel {
  ShaderStage Stage;
  unsigned Major = 0;
  unsigned Minor = 0;

public:
  ShaderModel() = default;
  ShaderModel(ShaderStage Stage, unsigned Major, unsigned Minor)
      : Stage(Stage), Major(Major), Minor(Minor) {}

  /// Get the ShaderModel for \c M
  static Expected<ShaderModel> get(Module &M);
  /// Read the shader model from the DXIL metadata in \c M
  static Expected<ShaderModel> readDXIL(Module &M);

  /// Returns true if no shader model is set
  bool empty() {
    return Stage.isLibrary() && Major == 0 && Minor == 0;
  }

  /// Remove any non-DXIL LLVM representations of the shader model from \c M.
  void strip(Module &M);
  /// Embed the LLVM representation of the shader model into \c M.
  void embed(Module &M);
  /// Remove any DXIL representation of the shader model from \c M.
  void stripDXIL(Module &M);
  /// Embed a DXIL representation of the shader model into \c M.
  void embedDXIL(Module &M);

  void print(raw_ostream &OS) const {
    // Format like dxc's target profile option.
    OS << Stage.getShortName() << "_" << Major << "_"
       << (Minor == 0xF ? Twine("x") : Twine(Minor));
  }
  LLVM_DUMP_METHOD void dump() const { print(errs()); }
};

inline raw_ostream &operator<<(raw_ostream &OS, const ShaderModel &SM) {
  SM.print(OS);
  return OS;
}


} // namespace dxil
} // namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_DXIL_H

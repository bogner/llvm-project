//===- DXILCBufferAccess.cpp - Translate CBuffer Loads --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "DXILCBufferAccess.h"
#include "DirectX.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#define DEBUG_TYPE "dxil-cbuffer-access"

#include "llvm/Frontend/HLSL/CBuffer.h"

using namespace llvm;

static bool replaceCBufferAccesses(Module &M) {
  return false;
}

PreservedAnalyses DXILCBufferAccess::run(Module &M, ModuleAnalysisManager &AM) {
  PreservedAnalyses PA;
  bool Changed = replaceCBufferAccesses(M);

  if (!Changed)
    return PreservedAnalyses::all();
  return PA;
}

namespace {
class DXILCBufferAccessLegacy : public ModulePass {
public:
  bool runOnModule(Module &M) override {
    return replaceCBufferAccesses(M);
  }
  StringRef getPassName() const override { return "DXIL CBuffer Access"; }
  DXILCBufferAccessLegacy() : ModulePass(ID) {}

  static char ID; // Pass identification.
};
char DXILCBufferAccessLegacy::ID = 0;
} // end anonymous namespace

INITIALIZE_PASS(DXILCBufferAccessLegacy, DEBUG_TYPE, "DXIL CBuffer Access",
                false, false)

ModulePass *llvm::createDXILCBufferAccessLegacyPass() {
  return new DXILCBufferAccessLegacy();
}

//===- DXILForwardHandleAccesses.cpp - Cleanup Handles --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "DXILForwardHandleAccesses.h"
#include "DXILShaderFlags.h"
#include "llvm/Analysis/DXILResource.h"
#include "DirectX.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#define DEBUG_TYPE "dxil-forward-handle-accesses"

using namespace llvm;

static bool forwardHandleAccesses(Module &M) {
  return false;
}

PreservedAnalyses DXILForwardHandleAccesses::run(Module &M, ModuleAnalysisManager &AM) {
  PreservedAnalyses PA;
  PA.preserve<DXILResourceTypeAnalysis>();
  PA.preserve<DXILResourceBindingAnalysis>();
  PA.preserve<DXILMetadataAnalysis>();
  PA.preserve<dxil::ShaderFlagsAnalysis>();

  bool Changed = forwardHandleAccesses(M);

  if (!Changed)
    return PreservedAnalyses::all();
  return PA;
}

namespace {
class DXILForwardHandleAccessesLegacy : public ModulePass {
public:
  bool runOnModule(Module &M) override { return forwardHandleAccesses(M); }
  StringRef getPassName() const override {
    return "DXIL Forward Handle Accesses";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DXILResourceTypeWrapperPass>();
    AU.addRequired<DXILResourceBindingWrapperPass>();
    AU.addPreserved<DXILResourceBindingWrapperPass>();
    AU.addPreserved<DXILMetadataAnalysisWrapperPass>();
    AU.addPreserved<dxil::ShaderFlagsAnalysisWrapper>();
  }

  DXILForwardHandleAccessesLegacy() : ModulePass(ID) {}

  static char ID; // Pass identification.
};
char DXILForwardHandleAccessesLegacy::ID = 0;
} // end anonymous namespace

INITIALIZE_PASS(DXILForwardHandleAccessesLegacy, DEBUG_TYPE, "DXIL Forward Handle Accesses",
                false, false)

ModulePass *llvm::createDXILForwardHandleAccessesLegacyPass() {
  return new DXILForwardHandleAccessesLegacy();
}

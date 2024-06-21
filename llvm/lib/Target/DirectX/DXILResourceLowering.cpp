//===- DXILResourceLowering.cpp - Lower resource intrinsics to DXIL Ops ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file Provide a pass to lower resource types, bindings, and intrinsics to
/// DXIL operations and metadata.
///
//===----------------------------------------------------------------------===//

#include "DirectX.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#define DEBUG_TYPE "dxil-resource-lowering"

using namespace llvm;

PreservedAnalyses DXILResourceLoweringPass::run(Module &M,
                                                ModuleAnalysisManager &) {
  return PreservedAnalyses::all();
}

namespace {
class DXILResourceLoweringLegacyPass : public ModulePass {
  DXILResourceLoweringPass Impl;

public:
  static char ID;

  DXILResourceLoweringLegacyPass() : ModulePass(ID) {
    initializeDXILResourceLoweringLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override { return "DXIL Resource Lowering"; }

  bool runOnModule(Module &M) override {
    ModuleAnalysisManager DummyMAM;
    PreservedAnalyses PA = Impl.run(M, DummyMAM);
    return !PA.areAllPreserved();
  }
};

char DXILResourceLoweringLegacyPass::ID = 0;
} // namespace

INITIALIZE_PASS(DXILResourceLoweringLegacyPass, DEBUG_TYPE,
                "DXIL Resource Lowering", false, false)

ModulePass *llvm::createDXILResourceLoweringLegacyPass() {
  return new DXILResourceLoweringLegacyPass();
}

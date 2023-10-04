//===- DXILUpgrade.cpp - Upgrade DXIL metadata to LLVM constructs ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/DXILUpgrade.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/DXIL.h"

using namespace llvm;

#define DEBUG_TYPE "dxil-upgrade"

static bool handleValVerMetadata(Module &M) {
  NamedMDNode *ValVer = M.getNamedMetadata("dx.valver");
  if (!ValVer)
    return false;

  LLVM_DEBUG({
    MDNode *N = ValVer->getOperand(0);
    auto X = mdconst::extract<ConstantInt>(N->getOperand(0))->getZExtValue();
    auto Y = mdconst::extract<ConstantInt>(N->getOperand(1))->getZExtValue();
    dbgs() << "DXIL: validation version: " << X << "." << Y << "\n";
  });
  // We don't need the validation version internally, so we drop it.
  ValVer->dropAllReferences();
  ValVer->eraseFromParent();
  return true;
}

static bool handleShaderModel(Module &M) {
  auto SM = dxil::ShaderModel::readDXIL(M);
  if (Error E = SM.takeError()) {
    report_fatal_error(std::move(E), /*gen_crash_diag=*/false);
  }
  if (SM->empty())
    return false;

  LLVM_DEBUG(dbgs() << "DXIL: Shader model " << *SM << "\n");
  SM->embed(M);
  SM->stripDXIL(M);
  return true;
}

PreservedAnalyses DXILUpgradePass::run(Module &M, ModuleAnalysisManager &AM) {
  PreservedAnalyses PA;
  // We never add, remove, or change functions here.
  PA.preserve<FunctionAnalysisManagerModuleProxy>();
  PA.preserveSet<AllAnalysesOn<Function>>();

  bool Changed = false;
  Changed |= handleValVerMetadata(M);
  Changed |= handleShaderModel(M);

  if (!Changed)
    return PreservedAnalyses::all();
  return PA;
}

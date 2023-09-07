//===- DXILTranslateMetadata.cpp - Pass to emit DXIL metadata ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
//===----------------------------------------------------------------------===//

#include "DXILMetadata.h"
#include "DXILResource.h"
#include "DXILResourceAnalysis.h"
#include "DXILShaderFlags.h"
#include "DirectX.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/Transforms/Utils/DXIL.h"

using namespace llvm;
using namespace llvm::dxil;

namespace {
class DXILTranslateMetadata : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  explicit DXILTranslateMetadata() : ModulePass(ID) {}

  StringRef getPassName() const override { return "DXIL Metadata Emit"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<DXILResourceWrapper>();
    AU.addRequired<ShaderFlagsAnalysisWrapper>();
  }

  bool runOnModule(Module &M) override;
};

} // namespace

bool DXILTranslateMetadata::runOnModule(Module &M) {

  dxil::ValidatorVersionMD ValVerMD(M);
  if (ValVerMD.isEmpty())
    ValVerMD.update(VersionTuple(1, 0));

  auto SM = dxil::ShaderModel::get(M);
  if (Error E = SM.takeError()) {
    // TODO: It would be better to have invariants such that we can just
    // llvm_unreachable/assert here
    report_fatal_error(std::move(E), /*gen_crash_diag=*/false);
  } else if (SM->empty())
    report_fatal_error("Cannot generate DXIL without a shader model",
                       /*gen_crash_diag=*/false);
  SM->embedDXIL(M);
  // TODO: We should probably call SM->strip(M) here, so that we're not leaving
  // LLVM style details in the emitted DXIL.

  const dxil::Resources &Res =
      getAnalysis<DXILResourceWrapper>().getDXILResource();
  Res.write(M);

  const uint64_t Flags =
      (uint64_t)(getAnalysis<ShaderFlagsAnalysisWrapper>().getShaderFlags());
  dxil::createEntryMD(M, Flags);

  return false;
}

char DXILTranslateMetadata::ID = 0;

ModulePass *llvm::createDXILTranslateMetadataPass() {
  return new DXILTranslateMetadata();
}

INITIALIZE_PASS_BEGIN(DXILTranslateMetadata, "dxil-metadata-emit",
                      "DXIL Metadata Emit", false, false)
INITIALIZE_PASS_DEPENDENCY(DXILResourceWrapper)
INITIALIZE_PASS_DEPENDENCY(ShaderFlagsAnalysisWrapper)
INITIALIZE_PASS_END(DXILTranslateMetadata, "dxil-metadata-emit",
                    "DXIL Metadata Emit", false, false)

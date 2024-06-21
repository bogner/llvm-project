//===- DirectXTargetMachine.h - DirectX Target Implementation ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIRECTX_DIRECTX_H
#define LLVM_LIB_TARGET_DIRECTX_DIRECTX_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class ModulePass;
class PassRegistry;
class raw_ostream;

/// Initializer for dxil writer pass
void initializeWriteDXILPassPass(PassRegistry &);

/// Initializer for dxil embedder pass
void initializeEmbedDXILPassPass(PassRegistry &);

/// Initializer for DXIL-prepare
void initializeDXILPrepareModulePass(PassRegistry &);

/// Pass to convert modules into DXIL-compatable modules
ModulePass *createDXILPrepareModulePass();

/// Initializer for DXIL Intrinsic Expansion
void initializeDXILIntrinsicExpansionLegacyPass(PassRegistry &);

/// Pass to expand intrinsic operations that lack DXIL opCodes
ModulePass *createDXILIntrinsicExpansionLegacyPass();

/// Initializer for DXILOpLowering
void initializeDXILOpLoweringLegacyPass(PassRegistry &);

/// Pass to lowering LLVM intrinsic call to DXIL op function call.
ModulePass *createDXILOpLoweringLegacyPass();

/// Transform resource types, bindings, and intrinsics to DXIL ops and metadata
class DXILResourceLoweringPass
    : public PassInfoMixin<DXILResourceLoweringPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
};
void initializeDXILResourceLoweringLegacyPassPass(PassRegistry &);
ModulePass *createDXILResourceLoweringLegacyPass();

/// Initializer for DXILTranslateMetadata.
void initializeDXILTranslateMetadataPass(PassRegistry &);

/// Pass to emit metadata for DXIL.
ModulePass *createDXILTranslateMetadataPass();

/// Initializer for DXILTranslateMetadata.
void initializeDXILResourceWrapperPass(PassRegistry &);

/// Pass to pretty print DXIL metadata.
ModulePass *createDXILPrettyPrinterPass(raw_ostream &OS);

/// Initializer for DXILPrettyPrinter.
void initializeDXILPrettyPrinterPass(PassRegistry &);

/// Initializer for dxil::ShaderFlagsAnalysisWrapper pass.
void initializeShaderFlagsAnalysisWrapperPass(PassRegistry &);

/// Initializer for DXContainerGlobals pass.
void initializeDXContainerGlobalsPass(PassRegistry &);

/// Pass for generating DXContainer part globals.
ModulePass *createDXContainerGlobalsPass();
} // namespace llvm

#endif // LLVM_LIB_TARGET_DIRECTX_DIRECTX_H

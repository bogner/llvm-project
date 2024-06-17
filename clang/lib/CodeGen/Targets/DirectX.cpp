//===- DirectX.cpp --------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ABIInfoImpl.h"
#include "TargetInfo.h"

using namespace clang;
using namespace clang::CodeGen;

namespace {
class DirectXTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  DirectXTargetCodeGenInfo(CodeGen::CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<DefaultABIInfo>(CGT)) {}

  llvm::Type *getHLSLType(CodeGenModule &CGM, const Type *T) const override;
};
} // End anonymous namespace.

llvm::Type *DirectXTargetCodeGenInfo::getHLSLType(CodeGenModule &CGM,
                                                  const Type *Ty) const {
  auto *BuiltinTy = dyn_cast<BuiltinType>(Ty);
  if (!BuiltinTy || BuiltinTy->getKind() != BuiltinType::HLSLResource)
    return nullptr;

  llvm::LLVMContext &Ctx = CGM.getLLVMContext();

  // TODO: Get all this from the attributes...
  llvm::Type *ElTy = llvm::FixedVectorType::get(llvm::Type::getFloatTy(Ctx), 4);
  bool IsWriteable = true;
  bool IsROV = false;

  return llvm::TargetExtType::get(Ctx, "dx.TypedBuffer", {ElTy},
                                  {IsWriteable, IsROV});
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createDirectXTargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<DirectXTargetCodeGenInfo>(CGM.getTypes());
}

//===- DXIL.cpp - Abstractions for DXIL constructs ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/DXIL.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

using namespace llvm;
using namespace llvm::dxil;

static llvm::Error errInvalid(const char *Msg) {
  return llvm::createStringError(std::errc::invalid_argument, Msg);
}

template <typename... Ts>
static llvm::Error errInvalid(const char *Fmt, const Ts &... Vals) {
  return llvm::createStringError(std::errc::invalid_argument, Fmt, Vals...);
}

Expected<ShaderStage> ShaderStage::fromTriple(Triple T) {
  if (!T.hasEnvironment())
    return ShaderStage(Triple::Library);
  if (!T.isShaderStageEnvironment())
    return errInvalid("Invalid shader stage '%s'",
                      T.getEnvironmentName().str().c_str());
  return ShaderStage(T.getEnvironment());
}

Expected<ShaderStage> ShaderStage::fromStageName(StringRef Name) {
  Triple T(/*Arch=*/"", /*Vendor=*/"", /*OS=*/"", Name);
  return ShaderStage::fromTriple(T);
}

Expected<ShaderStage> ShaderStage::fromShortName(StringRef Name) {
  auto Stage = StringSwitch<Triple::EnvironmentType>(Name)
                   .Case("ps", Triple::Pixel)
                   .Case("vs", Triple::Vertex)
                   .Case("gs", Triple::Geometry)
                   .Case("hs", Triple::Hull)
                   .Case("ds", Triple::Domain)
                   .Case("cs", Triple::Compute)
                   .Case("lib", Triple::Library)
                   .Case("ms", Triple::Mesh)
                   .Case("as", Triple::Amplification)
                   .Default(Triple::UnknownEnvironment);
  if (Stage == Triple::UnknownEnvironment)
    return errInvalid("Unknown short shader stage name: '%s'",
                      Name.str().c_str());
  return ShaderStage(Stage);
}

StringRef ShaderStage::getShortName() const {
  switch (Stage) {
  case Triple::Pixel:
    return "ps";
  case Triple::Vertex:
    return "vs";
  case Triple::Geometry:
    return "gs";
  case Triple::Hull:
    return "hs";
  case Triple::Domain:
    return "ds";
  case Triple::Compute:
    return "cs";
  case Triple::Library:
    return "lib";
  case Triple::Mesh:
    return "ms";
  case Triple::Amplification:
    return "as";
  default:
    llvm_unreachable("Invalid shader stage");
  }
}

llvm::Expected<ShaderModel> ShaderModel::get(Module &M) {
  Triple TT(M.getTargetTriple());

  if (!TT.isDXIL())
    return errInvalid("Cannot get DXIL shader model for arch '%s'",
                      TT.getArchName().str().c_str());

  // If the OS field is completely blank, treat this as an empty shadermodel to
  // match how an unversioned shadermodel behaves.
  if (TT.getOSName().empty())
    return ShaderModel();

  if (!TT.isShaderModelOS())
    return errInvalid("Invalid shader model '%s'",
                      TT.getOSName().str().c_str());
  VersionTuple Ver = TT.getOSVersion();

  auto TargetStage = ShaderStage::fromTriple(TT);
  if (Error E = TargetStage.takeError())
    return std::move(E);

  return ShaderModel(*TargetStage, Ver.getMajor(), Ver.getMinor().value_or(0));
}

llvm::Expected<ShaderModel> ShaderModel::readDXIL(Module &M) {
  NamedMDNode *ShaderModelMD = M.getNamedMetadata("dx.shaderModel");
  if (!ShaderModelMD)
    return ShaderModel();

  if (ShaderModelMD->getNumOperands() != 1)
    return errInvalid("dx.shaderModel must have one operand");

  MDNode *N = ShaderModelMD->getOperand(0);
  if (N->getNumOperands() != 3)
    return errInvalid("Shader model must have 3 components, not %d",
                      N->getNumOperands());

  const auto *StageOp = dyn_cast<MDString>(N->getOperand(0));
  const auto *MajorOp = mdconst::dyn_extract<ConstantInt>(N->getOperand(1));
  const auto *MinorOp = mdconst::dyn_extract<ConstantInt>(N->getOperand(2));
  if (!StageOp)
    return errInvalid("Shader model stage must be a string");
  if (!MajorOp)
    return errInvalid("Shader model major version must be an integer");
  if (!MinorOp)
    return errInvalid("Shader model minor version must be an integer");

  auto MDStage = ShaderStage::fromShortName(StageOp->getString());
  if (Error E = MDStage.takeError())
    return std::move(E);

  return ShaderModel(*MDStage, MajorOp->getZExtValue(),
                     MinorOp->getZExtValue());
}

void ShaderModel::strip(Module &M) {
  M.setTargetTriple("dxil-ms-dx");
}

void ShaderModel::embed(Module &M) {
  SmallString<64> Triple;
  raw_svector_ostream OS(Triple);
  OS << "dxil-unknown-shadermodel" << Major << "." << Minor << "-"
     << Triple::getEnvironmentTypeName(Stage.getTripleEnv());
  M.setTargetTriple(OS.str());
}

void ShaderModel::stripDXIL(Module &M) {
  if (NamedMDNode *SM = M.getNamedMetadata("dx.shaderModel")) {
    SM->dropAllReferences();
    SM->eraseFromParent();
  }
}

void ShaderModel::embedDXIL(Module &M) {
  LLVMContext &Ctx = M.getContext();
  IRBuilder<> B(Ctx);

  Metadata *Vals[3];
  Vals[0] = MDString::get(Ctx, Stage.getShortName());
  Vals[1] = ConstantAsMetadata::get(B.getInt32(Major));
  Vals[2] = ConstantAsMetadata::get(B.getInt32(Minor));
  MDNode *MD = MDNode::get(Ctx, Vals);

  NamedMDNode *SM = M.getOrInsertNamedMetadata("dx.shaderModel");
  if (SM->getNumOperands())
    SM->setOperand(0, MD);
  else
    SM->addOperand(MD);
}

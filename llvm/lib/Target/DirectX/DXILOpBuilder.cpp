//===- DXILOpBuilder.cpp - Helper class for build DIXLOp functions --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file This file contains class to help build DXIL op functions.
//===----------------------------------------------------------------------===//

#include "DXILOpBuilder.h"
#include "DXILConstants.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/DXILABI.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;
using namespace llvm::dxil;

constexpr StringLiteral DXILOpNamePrefix = "dx.op.";

static std::string getOverloadSuffix(Type &Ty) {
  if (auto *ST = dyn_cast<StructType>(&Ty))
    return ST->getStructName().str();
  if (Ty.isFloatTy())
    return "f32";
  if (Ty.isHalfTy())
    return "f16";
  if (Ty.isDoubleTy())
    return "f64";
  if (Ty.isIntegerTy(32))
    return "i32";
  if (Ty.isIntegerTy(1))
    return "i1";
  if (Ty.isIntegerTy(16))
    return "i16";
  if (Ty.isIntegerTy(64))
    return "i64";

  // If it isn't a struct or a basic HLSL type, just use the type printer.
  std::string Str;
  raw_string_ostream OS(Str);
  Ty.print(OS);
  return OS.str();
}

static Expected<Type *> getOverloadTy(LLVMContext &Context, dxil::OpCode OpCode,
                                      SmallVectorImpl<Value *> &Args) {
#define DXIL_GET_OP_TYPE_DEFINITIONS
#include "DXILOperation.inc"

  switch (OpCode) {
      default:
        // If the opcode isn't listed, the function isn't overloaded.
        return nullptr;
#define DXIL_OP_OVERLOAD_TYPES(Opcode, ParamIdx, ...)                          \
  case Opcode:                                                                 \
    if (ParamIdx >= Args.size())                                               \
      return createStringError(std::errc::invalid_argument,                    \
                               "not enough arguments for DXIL Op");            \
    for (const Type *T : {__VA_ARGS__})                                        \
      if (Args[ParamIdx] == T)                                                 \
        return T;                                                              \
    break;
#include "DXILOperation.inc"
  }
  return createStringError(std::errc::invalid_argument,
                           "Invalid overload for DXIL Op");
}

static FunctionType *DXILOpBuilder::getOpFunctionType(LLVMContext &Context,
                                                      dxil::OpCode OpCode,
                                                      Type *OverloadType) {
#define DXIL_GET_OP_TYPE_DEFINITIONS
#include "DXILOperation.inc"

  Type *OpTy = Type::getInt32Ty(Context);
  switch (OpCode) {
#define OP_OVERLOAD_TYPE OverloadType
#define DXIL_OP_OVERLOAD(Opcode, Result, ...)                                  \
  case Opcode:                                                                 \
    return FunctionType::get(Result, {OpTy, __VA_ARGS__}, /*isVarArg=*/false);
#include "DXILOperation.inc"
  }
  llvm_unreachable("Unhandled opcode");
}

namespace llvm {
namespace dxil {

CallInst *DXILOpBuilder::tryCreateOp(dxil::OpCode OpCode,
                                     SmallVectorImpl<Value *> &Args) {
  LLVMContext &Context;

  std::string Name("dx.op.");
  Name.append(getOpCodeClassName(OpCode));

  Expected<Type *> OverloadOrErr = getOverloadTy(Context, OpCode, Args);
  if (auto E = OverloadOrErr.takeError())
    return std::move(E);
  Type *OverloadTy = *OverloadOrErr;

  if (OverloadTy)
    Name.append(Twine(".") + getOverloadSuffix(OverloadTy));
  FunctionType *FTy = getOpFunctionType(Context, OpCode, OverloadTy);
  FunctionCallee Fn = M.getOrInsertFunction(Name, FTy);

  // We need to inject the opcode as the first argument.
  SmallVector<Value *> DXILArgs;
  DXILArgs.push_back(B.getInt32(llvm::to_underlying(OpCode)));
  DXILArgs.append(Args.begin(), Args.end());

  return B.CreateCall(Fn, DXILArgs);
}

CallInst *DXILOpBuilder::createOp(dxil::OpCode OpCode,
                                  SmallVectorImpl<Value *> &Args) {
  Expected<CallInst *> Result = tryCreateOp(OpCode, Args);
  assert(!Result.hasError() && "Invalid arguments for operation");
  return *Result;
}

const char *DXILOpBuilder::getOpCodeName(dxil::OpCode DXILOp) {
  return ::getOpCodeName(DXILOp);
}
} // namespace dxil
} // namespace llvm

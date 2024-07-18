//===- DXILOpBuilder.h - Helper class for build DIXLOp functions ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file This file contains class to help build DXIL op functions.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIRECTX_DXILOPBUILDER_H
#define LLVM_LIB_TARGET_DIRECTX_DXILOPBUILDER_H

#include "DXILConstants.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/DXILABI.h"

namespace llvm {
class Module;
class CallInst;
class Constant;
class FunctionType;
class IRBuilderBase;
class StructType;
class Type;
class Value;

namespace dxil {

class DXILOpBuilder {
public:
  DXILOpBuilder(Module &M, IRBuilderBase &Builder) : M(M), Builder(Builder) {}
  /// Create an instruction that calls DXIL Op with return type, specified
  /// opcode, and call arguments. \param OpCode Opcode of the DXIL Op call
  /// constructed \param ReturnTy Return type of the DXIL Op call constructed
  /// \param OverloadTy Overload type of the DXIL Op call constructed
  /// \return DXIL Op call constructed
  CallInst *createDXILOpCall(dxil::OpCode OpCode, Type *ReturnTy,
                             Type *OverloadTy, SmallVector<Value *> Args);
  Type *getOverloadTy(dxil::OpCode OpCode, FunctionType *FT);
  static const char *getOpCodeName(dxil::OpCode DXILOp);

  /// Get the `%dx.types.Handle` type.
  StructType *getHandleTy();
  /// Get the `%dx.types.ResBind` type.
  StructType *getResBindTy();
  /// Get a constant `%dx.types.ResBind` value.
  Constant *getResBind(uint32_t LowerBound, uint32_t UpperBound,
                       uint32_t SpaceID, dxil::ResourceClass RC);
  /// Get the `%dx.types.ResProps` type.
  StructType *getResPropsTy();
  /// Get a constant `%dx.types.ResourceProperties` value.
  Constant *getResProps(uint32_t Word0, uint32_t Word1);

  // TODO: These should come from DXIL.td rather than being hand coded.
  CallInst *createCreateHandleOp(dxil::ResourceClass RC, uint32_t RangeID,
                                Value *Index, Value *NonUniform);
  CallInst *createCreateHandleFromBindingOp(Constant *ResBind, Value *Index,
                                            Value *NonUniform);
  CallInst *createAnnotateHandle(Value *Handle, Constant *ResProps);

private:
  Module &M;
  IRBuilderBase &Builder;
};

} // namespace dxil
} // namespace llvm

#endif

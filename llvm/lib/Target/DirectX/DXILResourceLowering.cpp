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
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsDirectX.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/DXILABI.h"
#include "llvm/Transforms/Utils/DXILResource.h"

#define DEBUG_TYPE "dxil-resource-lowering"

using namespace llvm;

namespace {

class ResourceContext {
  Module &M;
  LLVMContext &Ctx;
  StructType *HandleTy = nullptr;
  StructType *ResBindTy = nullptr;
  StructType *ResPropsTy = nullptr;

public:
  ResourceContext(Module &M) : M(M), Ctx(M.getContext()) {}

  StructType *getHandleTy() {
    if (!HandleTy) {
      assert(!StructType::getTypeByName(Ctx, "dx.types.Handle") &&
             "dx.types.Handle created before DXIL resource lowering");
      // DXIL defines the handle type as `%dx.types.Handle = type { i8 * }`, so
      // we need `%dx.types.Handle = type { ptr }` here, which will turn into
      // `i8 *` during the DXIL bitcode writer.
      HandleTy =
          StructType::create({PointerType::getUnqual(Ctx)}, "dx.types.Handle");
    }
    return HandleTy;
  }

  StructType *getResBindTy() {
    if (!ResBindTy) {
      assert(!StructType::getTypeByName(Ctx, "dx.types.ResBind") &&
             "dx.types.ResBind created before DXIL resource lowering");
      Type *Int32Ty = Type::getInt32Ty(Ctx);
      Type *Int8Ty = Type::getInt8Ty(Ctx);
      ResBindTy = StructType::create({Int32Ty, Int32Ty, Int32Ty, Int8Ty},
                                     "dx.types.ResBind");
    }
    return ResBindTy;
  }

  StructType *getResPropsTy() {
    if (!ResPropsTy) {
      assert(!StructType::getTypeByName(Ctx, "dx.types.ResProps") &&
             "dx.types.ResProps created before DXIL resource lowering");
      Type *Int32Ty = Type::getInt32Ty(Ctx);
      ResPropsTy =
          StructType::create({Int32Ty, Int32Ty}, "dx.types.ResourceProperties");
    }
    return ResPropsTy;
  }

  CallInst *buildCreateHandleOp(IRBuilder<> &Builder, dxil::ResourceClass RC,
                                uint32_t RangeID, Value *Index,
                                Value *NonUniform) {
    // TODO: Use DXILOpBuilder here
    const uint32_t Opcode = 57;

    Type *Int32Ty = Type::getInt32Ty(Ctx);
    Type *Int8Ty = Type::getInt8Ty(Ctx);
    Type *Int1Ty = Type::getInt1Ty(Ctx);
    FunctionCallee Fn = M.getOrInsertFunction(
        "dx.op.createHandle",
        FunctionType::get(getHandleTy(),
                          {Int32Ty, Int8Ty, Int32Ty, Int32Ty, Int1Ty},
                          /*IsVarArg=*/false));
    return Builder.CreateCall(
        Fn, {ConstantInt::get(Int32Ty, Opcode),
             ConstantInt::get(Int8Ty, llvm::to_underlying(RC)),
             ConstantInt::get(Int32Ty, RangeID), Index, NonUniform});
  }

  CallInst *buildCreateHandleFromBindingOp(IRBuilder<> &Builder,
                                           Constant *ResBind,
                                           Value *Index, Value *NonUniform) {
    // TODO: Use DXILOpBuilder here
    const uint32_t Opcode = 217;

    Type *Int32Ty = Type::getInt32Ty(Ctx);
    Type *Int1Ty = Type::getInt1Ty(Ctx);
    FunctionCallee Fn = M.getOrInsertFunction(
        "dx.op.createHandleFromBinding",
        FunctionType::get(getHandleTy(),
                          {Int32Ty, getResBindTy(), Int32Ty, Int1Ty},
                          /*IsVarArg=*/false));
    assert(ResBind->getType() == getResBindTy() &&
           "Resource binding has wrong type");
    return Builder.CreateCall(
        Fn, {ConstantInt::get(Int32Ty, Opcode), ResBind, Index, NonUniform});
  }

  CallInst *buildCreateHandleFromBindingOp(IRBuilder<> &Builder,
                                           uint32_t LowerBound,
                                           uint32_t UpperBound,
                                           uint32_t SpaceID,
                                           dxil::ResourceClass RC, Value *Index,
                                           Value *NonUniform) {
    Type *Int32Ty = Type::getInt32Ty(Ctx);
    Type *Int8Ty = Type::getInt8Ty(Ctx);
    Constant *ResBind = ConstantStruct::get(
        getResBindTy(), {ConstantInt::get(Int32Ty, LowerBound),
                         ConstantInt::get(Int32Ty, UpperBound),
                         ConstantInt::get(Int32Ty, SpaceID),
                         ConstantInt::get(Int8Ty, llvm::to_underlying(RC))});
    return buildCreateHandleFromBindingOp(Builder, ResBind, Index, NonUniform);
  }

  CallInst *buildAnnotateHandle(IRBuilder<> &Builder, Value *Handle,
                                Constant *ResProps) {
    // TODO: Use DXILOpBuilder here
    const uint32_t Opcode = 216;

    Type *HandleTy = getHandleTy();
    Type *Int32Ty = Type::getInt32Ty(Ctx);
    FunctionCallee Fn = M.getOrInsertFunction(
        "dx.op.annotateHandle",
        FunctionType::get(HandleTy, {HandleTy, getResPropsTy()},
                          /*IsVarArg=*/false));
    assert(ResProps->getType() == getResPropsTy() &&
           "Resource properties has wrong type");
    return Builder.CreateCall(
        Fn, {ConstantInt::get(Int32Ty, Opcode), Handle, ResProps});
  }
};
}

static bool lowerHandlesFromBinding(Function &IntFn) {
  bool Changed = false;
  IRBuilder<> Builder(IntFn.getContext());

  for (User *U : make_early_inc_range(IntFn.users())) {
    CallInst *CI = cast<CallInst>(U);
    Builder.SetInsertPoint(CI);
  }

  return Changed;
}

static bool lowerResources(Module &M) {
  bool Changed = false;

  for (Function &F : M.functions()) {
    if (!F.isDeclaration())
      continue;
    switch (F.getIntrinsicID()) {
    default:
      continue;
    case Intrinsic::dx_handle_fromBinding:
      Changed |= lowerHandlesFromBinding(F);
      break;
    }
  }

  return Changed;
}

PreservedAnalyses DXILResourceLoweringPass::run(Module &M,
                                                ModuleAnalysisManager &) {
  if (lowerResources(M))
    return PreservedAnalyses::none();
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

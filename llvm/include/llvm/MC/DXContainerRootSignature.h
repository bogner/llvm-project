//===- llvm/MC/DXContainerRootSignature.h - RootSignature -*- C++ -*- ========//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_MC_DXCONTAINERROOTSIGNATURE_H
#define LLVM_MC_DXCONTAINERROOTSIGNATURE_H

#include "llvm/BinaryFormat/DXContainer.h"
#include "llvm/Support/Compiler.h"
#include <cstdint>
#include <limits>

namespace llvm {

class raw_ostream;
namespace mcdxbc {

struct RootParameterInfo {
  dxbc::RootParameterType Type;
  dxbc::ShaderVisibility Visibility;
  uint32_t Offset;
  size_t Location;

  RootParameterInfo() = default;

  RootParameterInfo(dxbc::RootParameterType Type,
                    dxbc::ShaderVisibility Visibility, uint32_t Offset,
                    size_t Location)
      : Type(Type), Visibility(Visibility), Offset(Offset), Location(Location) {
  }
};

struct DescriptorTable {
  SmallVector<dxbc::RTS0::v2::DescriptorRange> Ranges;
  SmallVector<dxbc::RTS0::v2::DescriptorRange>::const_iterator begin() const {
    return Ranges.begin();
  }
  SmallVector<dxbc::RTS0::v2::DescriptorRange>::const_iterator end() const {
    return Ranges.end();
  }
};

struct RootParametersContainer {
  SmallVector<RootParameterInfo> ParametersInfo;

  SmallVector<dxbc::RTS0::v1::RootConstants> Constants;
  SmallVector<dxbc::RTS0::v2::RootDescriptor> Descriptors;
  SmallVector<DescriptorTable> Tables;

  void addInfo(dxbc::RootParameterType Type, dxbc::ShaderVisibility Visibility,
               uint32_t Offset, size_t Location) {
    ParametersInfo.emplace_back(Type, Visibility, Offset, Location);
  }

  void addParameter(dxbc::RootParameterType Type,
                    dxbc::ShaderVisibility Visibility, uint32_t Offset,
                    dxbc::RTS0::v1::RootConstants Constant) {
    ParametersInfo.emplace_back(Type, Visibility, Offset, Constants.size());
    Constants.push_back(Constant);
  }

  void addParameter(dxbc::RootParameterType Type,
                    dxbc::ShaderVisibility Visibility, uint32_t Offset,
                    dxbc::RTS0::v2::RootDescriptor Descriptor) {
    ParametersInfo.emplace_back(Type, Visibility, Offset, Descriptors.size());
    Descriptors.push_back(Descriptor);
  }

  void addParameter(dxbc::RootParameterType Type,
                    dxbc::ShaderVisibility Visibility, uint32_t Offset,
                    DescriptorTable Table) {
    ParametersInfo.emplace_back(Type, Visibility, Offset, Tables.size());
    Tables.push_back(Table);
  }

  const RootParameterInfo &getInfo(uint32_t Location) const {
    return ParametersInfo[Location];
  }

  const dxbc::RTS0::v1::RootConstants &getConstant(size_t Index) const {
    return Constants[Index];
  }

  const dxbc::RTS0::v2::RootDescriptor &getRootDescriptor(size_t Index) const {
    return Descriptors[Index];
  }

  const DescriptorTable &getDescriptorTable(size_t Index) const {
    return Tables[Index];
  }

  size_t size() const { return ParametersInfo.size(); }

  SmallVector<RootParameterInfo>::const_iterator begin() const {
    return ParametersInfo.begin();
  }
  SmallVector<RootParameterInfo>::const_iterator end() const {
    return ParametersInfo.end();
  }
};
struct RootSignatureDesc {

  uint32_t Version = 2U;
  uint32_t Flags = 0U;
  uint32_t RootParameterOffset = 0U;
  uint32_t StaticSamplersOffset = 0u;
  uint32_t NumStaticSamplers = 0u;
  mcdxbc::RootParametersContainer ParametersContainer;
  SmallVector<dxbc::RTS0::v1::StaticSampler> StaticSamplers;

  LLVM_ABI void write(raw_ostream &OS) const;

  LLVM_ABI size_t getSize() const;
};
} // namespace mcdxbc
} // namespace llvm

#endif // LLVM_MC_DXCONTAINERROOTSIGNATURE_H

; RUN: opt -S -dxil-resource-lowering %s | FileCheck %s

define void @test_fromBinding() {
entry:
  ; RWBuffer<float4> Buf : register(u5, space3)
  %tb0 = call target("dx.TypedBuffer", float, 1, 0)
              @llvm.dx.handle.fromBinding.tdx.TypedBuffer_f32_1_0(
                  i32 3, i32 5, i32 1, i32 0, i1 false)

  ; RWBuffer<uint> Buf : register(u7, space2)
  %tb1 = call target("dx.TypedBuffer", i32, 1, 0)
              @llvm.dx.handle.fromBinding.tdx.TypedBuffer_i32_1_0t(
                  i32 2, i32 7, i32 1, i32 0, i1 false)

  ; Buffer<uint4> Buf[24] : register(t3, space5)
  %tb2 = call target("dx.TypedBuffer", i32, 0, 0)
              @llvm.dx.handle.fromBinding.tdx.TypedBuffer_i32_0_0t(
                  i32 2, i32 7, i32 24, i32 0, i1 false)

  ; struct S { float4 a; uint4 b; };
  ; StructuredBuffer<S> Buf : register(t2, space4)
  %rb0 = call target("dx.RawBuffer", {<4 x float>, <4 x i32>}, 0, 0)
              @llvm.dx.handle.fromBinding.tdx.RawBuffer_sl_v4f32v4i32s_0_0t(
                  i32 4, i32 2, i32 1, i32 0, i1 false)

  ; ByteAddressBuffer Buf : register(t8, space1)
  %rb1 = call target("dx.RawBuffer", i8, 0, 0)
              @llvm.dx.handle.fromBinding.tdx.RawBuffer_i8_0_0t(
                  i32 1, i32 8, i32 1, i32 0, i1 false)

  ; cbuffer cb0 {
  ;   float4 g_MaxThreadIter : packoffset(c0);
  ;   float4 g_Window : packoffset(c1);
  ; }
  %cb0 = call target("dx.CBuffer", 32)
              @llvm.dx.handle.fromBinding.tdx.CBuffer_32t(
                  i32 0, i32 0, i32 1, i32 0, i1 false)

  ; Texture2D<float4> ColorMapTexture : register(t3);
  %tx0 = call target("dx.Texture2D", <4 x float>, 0, 0)
              @llvm.dx.handle.fromBinding.tdx.Texture2D_v4f32_0_0t(
                  i32 0, i32 3, i32 1, i32 0, i1 false)

  ; Texture1D<float4> Buf[5] : register(t3);
  ; Texture1D<float4> B = Buf[NonUniformResourceIndex(i)];
  %i_tx1 = call i32 @opaque_int()
  %tx1 = call target("dx.Texture1D", <4 x float>, 0, 0)
              @llvm.dx.handle.fromBinding.tdx.Texture1D_v4f32_0_0t(
                  i32 0, i32 3, i32 5, i32 %i_tx1, i1 true)

  ; SamplerState ColorMapSampler : register(s0);
  %sm0 = call target("dx.Sampler", 0)
              @llvm.dx.handle.fromBinding.tdx.Sampler_0t(
                  i32 0, i32 0, i32 1, i32 0, i1 false)

  ret void
}

declare i32 @opaque_int()

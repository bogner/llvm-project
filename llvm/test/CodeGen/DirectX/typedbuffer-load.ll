; RUN: opt -S -dxil-op-lower %s | FileCheck %s

target triple = "dxil-pc-shadermodel6.0-compute"

define void @test_bufferload() {
  %float4 = call target("dx.TypedBuffer", <4 x float>, 0, 0, 0)
              @llvm.dx.handle.fromBinding.tdx.TypedBuffer_f32_1_0(
                  i32 0, i32 0, i32 1, i32 0, i1 false)

  %data = call <4 x float> @llvm.dx.typedBufferLoad(
            target("dx.TypedBuffer", <4 x float>, 0, 0, 0) %float4, i32 0)

  ret void
}

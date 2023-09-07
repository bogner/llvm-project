; RUN: opt -passes=dxil-upgrade -S < %s | FileCheck %s

; CHECK: target triple = "dxil-unknown-shadermodel6.7-amplification"
; CHECK-NOT: !dx.shaderModel =
; CHECK-NOT: !{!"as", i32 6, i32 7}

target triple = "dxil-ms-dx"

!dx.shaderModel = !{!0}
!0 = !{!"as", i32 6, i32 7}

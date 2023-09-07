; RUN: opt -passes=dxil-upgrade -S < %s | FileCheck %s

; CHECK: target triple = "dxil-unknown-shadermodel6.2-mesh"
; CHECK-NOT: !dx.shaderModel =
; CHECK-NOT: !{!"ms", i32 6, i32 2}

target triple = "dxil-ms-dx"

!dx.shaderModel = !{!0}
!0 = !{!"ms", i32 6, i32 2}

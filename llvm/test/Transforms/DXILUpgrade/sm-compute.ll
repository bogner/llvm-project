; RUN: opt -passes=dxil-upgrade -S < %s | FileCheck %s

; CHECK: target triple = "dxil-unknown-shadermodel6.2-compute"
; CHECK-NOT: !dx.shaderModel =
; CHECK-NOT: !{!"cs", i32 6, i32 2}

target triple = "dxil-ms-dx"

!dx.shaderModel = !{!0}
!0 = !{!"cs", i32 6, i32 2}

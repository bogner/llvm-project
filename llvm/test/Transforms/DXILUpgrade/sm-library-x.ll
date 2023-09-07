; RUN: opt -passes=dxil-upgrade -S < %s | FileCheck %s

; TODO: Should this be shadermodel6.x-library or something like that?
; CHECK: target triple = "dxil-unknown-shadermodel6.15-library"
; CHECK-NOT: !dx.shaderModel =
; CHECK-NOT: !{!"lib", i32 6, i32 15}

target triple = "dxil-ms-dx"

!dx.shaderModel = !{!0}
!0 = !{!"lib", i32 6, i32 15}

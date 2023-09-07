; RUN: not opt -S -mtriple x86_64-unknown-linux-gnu -dxil-metadata-emit %s 2>&1 | FileCheck --check-prefix=CHECK-BAD-ARCH %s
; RUN: not opt -S -mtriple x86_64--shadermodel6.2-library -dxil-metadata-emit %s 2>&1 | FileCheck --check-prefix=CHECK-BAD-ARCH %s
; RUN: not opt -S -mtriple dxil--shadermodel-library -dxil-metadata-emit %s 2>&1 | FileCheck --check-prefix=CHECK-NO-VERSION %s
; RUN: not opt -S -mtriple dxil -dxil-metadata-emit %s 2>&1 | FileCheck --check-prefix=CHECK-NO-VERSION %s
; RUN: not opt -S -mtriple dxil-ms-dx -dxil-metadata-emit %s 2>&1 | FileCheck --check-prefix=CHECK-BAD-SM %s
; RUN: not opt -S -mtriple dxil--shader6.7 -dxil-metadata-emit %s 2>&1 | FileCheck --check-prefix=CHECK-BAD-SM %s
; RUN: not opt -S -mtriple dxil--linux-library -dxil-metadata-emit %s 2>&1 | FileCheck --check-prefix=CHECK-BAD-SM %s
; RUN: not opt -S -mtriple dxil--shadermodel6.7-eabi -dxil-metadata-emit %s 2>&1 | FileCheck --check-prefix=CHECK-BAD-STAGE %s

; CHECK-BAD-ARCH: LLVM ERROR: Cannot get DXIL shader model for arch
; CHECK-NO-VERSION: Cannot generate DXIL without a shader model
; CHECK-BAD-SM: LLVM ERROR: Invalid shader model
; CHECK-BAD-STAGE: LLVM ERROR: Invalid shader stage

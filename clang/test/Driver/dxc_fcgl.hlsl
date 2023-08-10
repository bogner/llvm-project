// RUN: %clang_dxc -fcgl -T lib_6_7 -### %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-ASM %s
// RUN: %clang_dxc -fcgl -T lib_6_7 -Fc x.asm -### %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-ASM %s
// RUN: %clang_dxc -fcgl -T lib_6_7 -Fo x.obj -### %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-OBJ %s

// The -fcgl flag should disable llvm passes regardless of the output type
// CHECK: "-cc1"
// CHECK-ASM-SAME: -emit-llvm
// CHECK-OBJ-SAME: -emit-llvm-bc

// RUN: %clang_cc1 -triple dxil-pc-shadermodel6.0-compute -std=hlsl202x -x hlsl -ast-dump -o - %s -verify | FileCheck %s

typedef vector<float, 4> float4;

// CHECK: -TypeAliasDecl 0x{{[0-9a-f]+}} <line:[[# @LINE + 2]]:1, col:53>
// CHECK: -AttributedType 0x{{[0-9a-f]+}} '__hlsl_resource_t {{\[\[}}hlsl::contained_type(...){{]]}}' sugar
using ResourceIntAliasT = __hlsl_resource_t [[hlsl::contained_type(int)]];
ResourceIntAliasT h1;

// CHECK: -VarDecl 0x{{[0-9a-f]+}} <line:[[# @LINE + 1]]:1, col:52> col:52 h2 '__hlsl_resource_t {{\[\[}}hlsl::contained_type(...){{]]}}':'__hlsl_resource_t'
__hlsl_resource_t [[hlsl::contained_type(float4)]] h2;

// expected-error@+1{{'contained_type' attribute cannot be applied to a declaration}}
[[hlsl::contained_type(float4)]] __hlsl_resource_t h3;

// expected-error@+1{{expected a type}}
[[hlsl::contained_type(0)]] __hlsl_resource_t h4;

// expected-no-error
template <typename T> struct S {
  __hlsl_resource_t [[hlsl::contained_type(T)]] h;
};
S<float4> s1;

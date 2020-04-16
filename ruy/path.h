/* Copyright 2019 Google LLC. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef RUY_RUY_PATH_H_
#define RUY_RUY_PATH_H_

#include <cstdint>

#include "ruy/platform.h"
#include "ruy/size_util.h"

namespace ruy {

// A Path is an implementation path, typically corresponding to a SIMD
// instruction set being targetted. For example, on the ARM architecture,
// Path::kNeon means using NEON instructions, and Path::kNeonDotprod means
// also using the newer NEON dot-product instructions.
//
// Different Path enum values are defined on different CPU architectures,
// corresponding to different SIMD ISA extensions available there.
//
// There are two special Path's universally defined on all CPU architectures:
// kReference and kStandardCpp. From a user's perspective, they are similar
// in that both are slow, portable, standard-c++-only implementation paths.
// They differ in that kStandardCpp is structurally similar to the actual
// optimized Path's and exercises much of the same ruy code as they do, while
// kReference is a special path bypassing most of ruy's code and implementing
// the whole ruy::Mul as a very simple self-contained function.
//
// Path enum values are bits and may be OR-ed to form "sets of Paths".
// Ruy entry points such as ruy::Mul either implicitly use such a set of Paths,
// or allow passing an explicit one as a template parameter. The meaning of such
// an OR-ed Path combination is "compile all of
// these paths; which path is used will be determined at runtime". This is why
// for most users, it is enough to call ruy::Mul(...), which will compile a
// reasonable selection of paths for the target CPU architecture's various
// SIMD ISA extensions, and let ruy determine at runtime which one to use.
// Internally, after the actual path has been resolved, ruy's internal functions
// templatized on a Path tend to require that to be a single bit.
//
// An element of ruy's internal design was to allow for code compiled for
// multiple such paths to coexist without violating the C++ One Definition Rule
// (ODR). This is achieved by having all ruy internal functions, whose
// definition depends on a choice of Path, be templatized on a Path, so that
// each path-specific specialization is a separate symbol. There is never
// a need to compile ruy code with different compilation flags to enable
// different SIMD extensions and dispatch at runtime between them, as this is
// taken care of internally by ruy in an ODR-correct way.
enum class Path : std::uint8_t {
  // This is a special null value, representing the absence of any path.
  kNone = 0,
  // Reference multiplication code.
  // The main purpose of this path is to have a very simple standalone Mul
  // implementation to check against.
  // This path bypasses almost all of Ruy's internal implementation details.
  //
  // This is intended for testing/development.
  kReference = 0x1,
  // Standard C++ implementation of Ruy's architecture-specific parts.
  // Unlike Path::kReference, this path exercises most of Ruy's internal logic.
  //
  // This is intended for testing/development, and as a fallback for when
  // the SIMD ISA extensions required by other paths are unavailable at runtime.
  kStandardCpp = 0x2,

#if RUY_PLATFORM(ARM)
  // ARM architectures.
  //
  // Optimized path using a widely available subset of ARM NEON instructions.
  kNeon = 0x4,
  // Optimized path making use of ARM NEON dot product instructions that are
  // available on newer ARM cores.
  kNeonDotprod = 0x8,
#endif  // RUY_PLATFORM(ARM)

#if RUY_PLATFORM(X86)
  // x86 architectures.
  //
  // TODO(b/147376783): SSE 4.2 and AVX-VNNI support is incomplete /
  // placeholder.
  // Optimization is not finished. In particular the dimensions of the kernel
  // blocks can be changed as desired.
  //
  // Optimized for SSE 4.2.
  kSse42 = 0x4,
  // Optimized for AVX2.
  kAvx2 = 0x8,
  // Optimized for AVX-512.
  kAvx512 = 0x10,
  // TODO(b/147376783): SSE 4.2 and AVX-VNNI support is incomplete /
  // placeholder.
  // Optimization is not finished. In particular the dimensions of the kernel
  // blocks can be changed as desired.
  //
  // Optimized for AVX-VNNI.
  kAvxVnni = 0x20,
#endif  // RUY_PLATFORM(X86)
};

inline constexpr Path operator|(Path p, Path q) {
  return static_cast<Path>(static_cast<std::uint32_t>(p) |
                           static_cast<std::uint32_t>(q));
}

inline constexpr Path operator&(Path p, Path q) {
  return static_cast<Path>(static_cast<std::uint32_t>(p) &
                           static_cast<std::uint32_t>(q));
}

inline constexpr Path operator^(Path p, Path q) {
  return static_cast<Path>(static_cast<std::uint32_t>(p) ^
                           static_cast<std::uint32_t>(q));
}

inline constexpr Path operator~(Path p) {
  return static_cast<Path>(~static_cast<std::uint32_t>(p));
}

inline Path GetMostSignificantPath(Path path_mask) {
  return static_cast<Path>(round_down_pot(static_cast<int>(path_mask)));
}

// ruy::kAllPaths represents all Path's that make sense to on a given
// base architecture.
#ifdef __linux__
#if RUY_PLATFORM(NEON_64)
constexpr Path kAllPaths =
    Path::kReference | Path::kStandardCpp | Path::kNeon | Path::kNeonDotprod;
#elif RUY_PLATFORM(NEON_32)
constexpr Path kAllPaths = Path::kReference | Path::kStandardCpp | Path::kNeon;
#elif RUY_PLATFORM(X86)
constexpr Path kAllPaths = Path::kReference | Path::kStandardCpp |
                           Path::kSse42 | Path::kAvx2 | Path::kAvx512 |
                           Path::kAvxVnni;
#else
constexpr Path kAllPaths = Path::kReference | Path::kStandardCpp;
#endif
#else   // __linux__
// We don't know how to do runtime dotprod detection outside of linux for now.
#if RUY_PLATFORM(NEON)
constexpr Path kAllPaths = Path::kReference | Path::kStandardCpp | Path::kNeon;
#elif RUY_PLATFORM(X86)
constexpr Path kAllPaths = Path::kReference | Path::kStandardCpp |
                           Path::kSse42 | Path::kAvx2 | Path::kAvx512 |
                           Path::kAvxVnni;
#else
constexpr Path kAllPaths = Path::kReference | Path::kStandardCpp;
#endif
#endif  // __linux__

}  // namespace ruy

#endif  // RUY_RUY_PATH_H_
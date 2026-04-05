#ifndef ASMJIT_CORE_API_CONFIG_H_INCLUDED
#define ASMJIT_CORE_API_CONFIG_H_INCLUDED
#define ASMJIT_LIBRARY_MAKE_VERSION(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#define ASMJIT_LIBRARY_VERSION ASMJIT_LIBRARY_MAKE_VERSION(1, 21, 0)
#if !defined(ASMJIT_ABI_NAMESPACE)
  #define ASMJIT_ABI_NAMESPACE v1_21
#endif
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <initializer_list>
#include <limits>
#include <type_traits>
#include <utility>
#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
  #include <pthread.h>
#endif
#if defined(_DOXYGEN)
#define ASMJIT_EMBED
#undef ASMJIT_EMBED
#define ASMJIT_STATIC
#undef ASMJIT_STATIC
#define ASMJIT_BUILD_DEBUG
#undef ASMJIT_BUILD_DEBUG
#define ASMJIT_BUILD_RELEASE
#undef ASMJIT_BUILD_RELEASE
#define ASMJIT_NO_ABI_NAMESPACE
#undef ASMJIT_NO_ABI_NAMESPACE
#define ASMJIT_NO_X86
#undef ASMJIT_NO_X86
#define ASMJIT_NO_AARCH64
#undef ASMJIT_NO_AARCH64
#define ASMJIT_NO_SHM_OPEN
#undef ASMJIT_NO_SHM_OPEN
#define ASMJIT_NO_JIT
#undef ASMJIT_NO_JIT
#define ASMJIT_NO_LOGGING
#undef ASMJIT_NO_LOGGING
#define ASMJIT_NO_TEXT
#undef ASMJIT_NO_TEXT
#define ASMJIT_NO_INTROSPECTION
#undef ASMJIT_NO_INTROSPECTION
#define ASMJIT_NO_FOREIGN
#undef ASMJIT_NO_FOREIGN
#define ASMJIT_NO_BUILDER
#undef ASMJIT_NO_BUILDER
#define ASMJIT_NO_COMPILER
#undef ASMJIT_NO_COMPILER
#define ASMJIT_NO_UJIT
#undef ASMJIT_NO_UJIT
#if defined(ASMJIT_NO_BUILDER) && !defined(ASMJIT_NO_COMPILER)
  #define ASMJIT_NO_COMPILER
#endif
#if defined(ASMJIT_NO_COMPILER) && !defined(ASMJIT_NO_UJIT)
  #define ASMJIT_NO_UJIT
#endif
#if defined(ASMJIT_NO_TEXT) && !defined(ASMJIT_NO_LOGGING)
  #pragma message("'ASMJIT_NO_TEXT' can only be defined when 'ASMJIT_NO_LOGGING' is defined.")
  #undef ASMJIT_NO_TEXT
#endif
#if defined(ASMJIT_NO_INTROSPECTION) && !defined(ASMJIT_NO_COMPILER)
  #pragma message("'ASMJIT_NO_INTROSPECTION' can only be defined when 'ASMJIT_NO_COMPILER' is defined")
  #undef ASMJIT_NO_INTROSPECTION
#endif
#endif
#if !defined(ASMJIT_BUILD_DEBUG) && !defined(ASMJIT_BUILD_RELEASE)
  #if !defined(NDEBUG)
    #define ASMJIT_BUILD_DEBUG
  #else
    #define ASMJIT_BUILD_RELEASE
  #endif
#endif
#if defined(_DOXYGEN)
  #define ASMJIT_ARCH_X86 __detected_at_runtime__
  #define ASMJIT_ARCH_ARM __detected_at_runtime__
  #define ASMJIT_ARCH_MIPS __detected_at_runtime__
  #define ASMJIT_ARCH_RISCV __detected_at_runtime__
  #define ASMJIT_ARCH_LA __detected_at_runtime__
  #define ASMJIT_ARCH_BITS __detected_at_runtime__(32 | 64)
  #define ASMJIT_HAS_HOST_BACKEND __detected_at_runtime__
#else
  #if defined(_M_X64) || defined(__x86_64__)
    #define ASMJIT_ARCH_X86 64
  #elif defined(_M_IX86) || defined(__X86__) || defined(__i386__)
    #define ASMJIT_ARCH_X86 32
  #else
    #define ASMJIT_ARCH_X86 0
  #endif
  #if defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)
  # define ASMJIT_ARCH_ARM 64
  #elif defined(_M_ARM) || defined(_M_ARMT) || defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
    #define ASMJIT_ARCH_ARM 32
  #else
    #define ASMJIT_ARCH_ARM 0
  #endif
  #if defined(_MIPS_ARCH_MIPS64) || defined(__mips64)
    #define ASMJIT_ARCH_MIPS 64
  #elif defined(_MIPS_ARCH_MIPS32) || defined(_M_MRX000) || defined(__mips__)
    #define ASMJIT_ARCH_MIPS 32
  #else
    #define ASMJIT_ARCH_MIPS 0
  #endif
  #if (defined(__riscv) || defined(__riscv__)) && defined(__riscv_xlen)
    #define ASMJIT_ARCH_RISCV __riscv_xlen
  #else
    #define ASMJIT_ARCH_RISCV 0
  #endif
  #if defined(__loongarch__) && defined(__loongarch_grlen)
    #define ASMJIT_ARCH_LA __loongarch_grlen
  #else
    #define ASMJIT_ARCH_LA 0
  #endif
  #define ASMJIT_ARCH_BITS (ASMJIT_ARCH_X86 | ASMJIT_ARCH_ARM | ASMJIT_ARCH_MIPS | ASMJIT_ARCH_RISCV | ASMJIT_ARCH_LA)
  #if ASMJIT_ARCH_BITS == 0 && !defined(_DOXYGEN)
    #undef ASMJIT_ARCH_BITS
    #if defined(__LP64__) || defined(_LP64)
      #define ASMJIT_ARCH_BITS 64
    #else
      #define ASMJIT_ARCH_BITS 32
    #endif
  #endif
  #if defined(ASMJIT_NO_FOREIGN)
    #if !ASMJIT_ARCH_X86 && !defined(ASMJIT_NO_X86)
      #define ASMJIT_NO_X86
    #endif
    #if ASMJIT_ARCH_ARM != 64 && !defined(ASMJIT_NO_AARCH64)
      #define ASMJIT_NO_AARCH64
    #endif
  #endif
  #if ASMJIT_ARCH_X86 != 0 && !defined(ASMJIT_NO_X86)
    #define ASMJIT_HAS_HOST_BACKEND
  #endif
  #if ASMJIT_ARCH_ARM == 64 && !defined(ASMJIT_NO_AARCH64)
    #define ASMJIT_HAS_HOST_BACKEND
  #endif
  #if !defined(ASMJIT_NO_UJIT)
    #if !defined(ASMJIT_NO_X86) && ASMJIT_ARCH_X86 != 0
      #define ASMJIT_UJIT_X86
    #elif !defined(ASMJIT_NO_AARCH64) && ASMJIT_ARCH_ARM == 64
      #define ASMJIT_UJIT_AARCH64
    #else
      #define ASMJIT_NO_UJIT
    #endif
  #endif
#endif
#if defined(__GNUC__) && defined(__has_attribute)
  #define ASMJIT_CXX_HAS_ATTRIBUTE(NAME, CHECK) (__has_attribute(NAME))
#else
  #define ASMJIT_CXX_HAS_ATTRIBUTE(NAME, CHECK) (!(!(CHECK)))
#endif
#if defined(_DOXYGEN)
#define ASMJIT_API
#define ASMJIT_VIRTAPI
#define ASMJIT_INLINE inline
#define ASMJIT_INLINE_NODEBUG inline
#define ASMJIT_INLINE_CONSTEXPR inline constexpr
#define ASMJIT_NOINLINE
#define ASMJIT_CDECL
#define ASMJIT_STDCALL
#define ASMJIT_FASTCALL
#define ASMJIT_REGPARM(N)
#define ASMJIT_VECTORCALL
#define ASMJIT_MAY_ALIAS
#define ASMJIT_ASSUME(...)
#define ASMJIT_LIKELY(...)
#define ASMJIT_UNLIKELY(...)
#else
#if !defined(ASMJIT_STATIC)
  #if defined(_WIN32) && (defined(_MSC_VER) || defined(__MINGW32__))
    #ifdef ASMJIT_EXPORTS
      #define ASMJIT_API __declspec(dllexport)
    #else
      #define ASMJIT_API __declspec(dllimport)
    #endif
  #elif defined(_WIN32) && defined(__GNUC__)
    #ifdef ASMJIT_EXPORTS
      #define ASMJIT_API __attribute__((__dllexport__))
    #else
      #define ASMJIT_API __attribute__((__dllimport__))
    #endif
  #elif defined(__GNUC__)
    #define ASMJIT_API __attribute__((__visibility__("default")))
  #endif
#endif
#if !defined(ASMJIT_API)
  #define ASMJIT_API
#endif
#if !defined(ASMJIT_VARAPI)
  #define ASMJIT_VARAPI extern ASMJIT_API
#endif
#if defined(__GNUC__) && !defined(_WIN32)
  #define ASMJIT_VIRTAPI ASMJIT_API
#else
  #define ASMJIT_VIRTAPI
#endif
#if !defined(ASMJIT_BUILD_DEBUG) && defined(__GNUC__) && !defined(_DOXYGEN)
  #define ASMJIT_INLINE inline __attribute__((__always_inline__))
#elif !defined(ASMJIT_BUILD_DEBUG) && defined(_MSC_VER) && !defined(_DOXYGEN)
  #define ASMJIT_INLINE __forceinline
#else
  #define ASMJIT_INLINE inline
#endif
#if defined(__clang__) && !defined(_DOXYGEN)
  #define ASMJIT_INLINE_NODEBUG inline __attribute__((__always_inline__, __nodebug__))
#elif defined(__GNUC__) && !defined(_DOXYGEN)
  #define ASMJIT_INLINE_NODEBUG inline __attribute__((__always_inline__, __artificial__))
#else
  #define ASMJIT_INLINE_NODEBUG inline
#endif
#define ASMJIT_INLINE_CONSTEXPR constexpr ASMJIT_INLINE_NODEBUG
#if defined(__GNUC__)
  #define ASMJIT_NOINLINE __attribute__((__noinline__))
#elif defined(_MSC_VER)
  #define ASMJIT_NOINLINE __declspec(noinline)
#else
  #define ASMJIT_NOINLINE
#endif
#if ASMJIT_ARCH_X86 == 32 && defined(__GNUC__)
  #define ASMJIT_CDECL __attribute__((__cdecl__))
  #define ASMJIT_STDCALL __attribute__((__stdcall__))
  #define ASMJIT_FASTCALL __attribute__((__fastcall__))
  #define ASMJIT_REGPARM(N) __attribute__((__regparm__(N)))
#elif ASMJIT_ARCH_X86 == 32 && defined(_MSC_VER)
  #define ASMJIT_CDECL __cdecl
  #define ASMJIT_STDCALL __stdcall
  #define ASMJIT_FASTCALL __fastcall
  #define ASMJIT_REGPARM(N)
#else
  #define ASMJIT_CDECL
  #define ASMJIT_STDCALL
  #define ASMJIT_FASTCALL
  #define ASMJIT_REGPARM(N)
#endif
#if ASMJIT_ARCH_X86 && defined(_WIN32) && defined(_MSC_VER)
  #define ASMJIT_VECTORCALL __vectorcall
#elif ASMJIT_ARCH_X86 && defined(_WIN32)
  #define ASMJIT_VECTORCALL __attribute__((__vectorcall__))
#else
  #define ASMJIT_VECTORCALL
#endif
#if defined(__GNUC__)
  #define ASMJIT_ALIGNAS(ALIGNMENT) __attribute__((__aligned__(ALIGNMENT)))
#elif defined(_MSC_VER)
  #define ASMJIT_ALIGNAS(ALIGNMENT) __declspec(align(ALIGNMENT))
#else
  #define ASMJIT_ALIGNAS(ALIGNMENT) alignas(ALIGNMENT)
#endif
#if defined(__GNUC__)
  #define ASMJIT_MAY_ALIAS __attribute__((__may_alias__))
#else
  #define ASMJIT_MAY_ALIAS
#endif
#if defined(__clang__) && !defined(_DOXYGEN)
  #define ASMJIT_NONNULL(FUNCTION_ARGUMENT) FUNCTION_ARGUMENT __attribute__((__nonnull__))
#else
  #define ASMJIT_NONNULL(FUNCTION_ARGUMENT) FUNCTION_ARGUMENT
#endif
#if defined(__clang__)
  #define ASMJIT_ASSUME(...) __builtin_assume(__VA_ARGS__)
#elif defined(__GNUC__)
  #define ASMJIT_ASSUME(...) do { if (!(__VA_ARGS__)) __builtin_unreachable(); } while (0)
#elif defined(_MSC_VER)
  #define ASMJIT_ASSUME(...) __assume(__VA_ARGS__)
#else
  #define ASMJIT_ASSUME(...) (void)0
#endif
#if defined(__GNUC__)
  #define ASMJIT_LIKELY(...) __builtin_expect(!!(__VA_ARGS__), 1)
  #define ASMJIT_UNLIKELY(...) __builtin_expect(!!(__VA_ARGS__), 0)
#else
  #define ASMJIT_LIKELY(...) (__VA_ARGS__)
  #define ASMJIT_UNLIKELY(...) (__VA_ARGS__)
#endif
#define ASMJIT_OFFSET_OF(STRUCT, MEMBER) ((int)(intptr_t)((const char*)&((const STRUCT*)0x100)->MEMBER) - 0x100)
#define ASMJIT_ARRAY_SIZE(X) uint32_t(sizeof(X) / sizeof(X[0]))
#if ASMJIT_CXX_HAS_ATTRIBUTE(no_sanitize, 0)
  #define ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF __attribute__((__no_sanitize__("undefined")))
#elif defined(__GNUC__)
  #define ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF __attribute__((__no_sanitize_undefined__))
#else
  #define ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF
#endif
#endif
#if defined(_MSC_VER) && !defined(__clang__) && !defined(_DOXYGEN)
  #define ASMJIT_BEGIN_DIAGNOSTIC_SCOPE                                        \
    __pragma(warning(push))                                                    \
    __pragma(warning(disable: 4127))      \
    __pragma(warning(disable: 4201))
  #define ASMJIT_END_DIAGNOSTIC_SCOPE                                          \
    __pragma(warning(pop))
#else
  #define ASMJIT_BEGIN_DIAGNOSTIC_SCOPE
  #define ASMJIT_END_DIAGNOSTIC_SCOPE
#endif
#if !defined(ASMJIT_NO_ABI_NAMESPACE) && !defined(_DOXYGEN)
  #define ASMJIT_BEGIN_NAMESPACE                                               \
    ASMJIT_BEGIN_DIAGNOSTIC_SCOPE                                              \
    namespace asmjit {                                                         \
    inline namespace ASMJIT_ABI_NAMESPACE {
  #define ASMJIT_END_NAMESPACE                                                 \
    }}                                                                         \
    ASMJIT_END_DIAGNOSTIC_SCOPE
#else
  #define ASMJIT_BEGIN_NAMESPACE                                               \
    ASMJIT_BEGIN_DIAGNOSTIC_SCOPE                                              \
    namespace asmjit {
  #define ASMJIT_END_NAMESPACE                                                 \
    }                                                                          \
    ASMJIT_END_DIAGNOSTIC_SCOPE
#endif
#define ASMJIT_BEGIN_SUB_NAMESPACE(NAMESPACE) ASMJIT_BEGIN_NAMESPACE namespace NAMESPACE {
#define ASMJIT_END_SUB_NAMESPACE } ASMJIT_END_NAMESPACE
#define ASMJIT_NONCOPYABLE(Type)                                               \
    Type(const Type& other) = delete;                                          \
    Type& operator=(const Type& other) = delete;
#define ASMJIT_NONCONSTRUCTIBLE(Type)                                          \
    Type() = delete;                                                           \
    Type(const Type& other) = delete;                                          \
    Type& operator=(const Type& other) = delete;
#if defined(_DOXYGEN)
  #define ASMJIT_DEFINE_ENUM_FLAGS(T)
#else
  #define ASMJIT_DEFINE_ENUM_FLAGS(T)                                          \
    static ASMJIT_INLINE_CONSTEXPR T operator~(T a) noexcept {                 \
      return T(~std::underlying_type_t<T>(a));                                 \
    }                                                                          \
                                                                               \
    static ASMJIT_INLINE_CONSTEXPR T operator|(T a, T b) noexcept {            \
      return T(std::underlying_type_t<T>(a) | std::underlying_type_t<T>(b));   \
    }                                                                          \
    static ASMJIT_INLINE_CONSTEXPR T operator&(T a, T b) noexcept {            \
      return T(std::underlying_type_t<T>(a) & std::underlying_type_t<T>(b));   \
    }                                                                          \
    static ASMJIT_INLINE_CONSTEXPR T operator^(T a, T b) noexcept {            \
      return T(std::underlying_type_t<T>(a) ^ std::underlying_type_t<T>(b));   \
    }                                                                          \
                                                                               \
    static ASMJIT_INLINE_CONSTEXPR T& operator|=(T& a, T b) noexcept {         \
      a = T(std::underlying_type_t<T>(a) | std::underlying_type_t<T>(b));      \
      return a;                                                                \
    }                                                                          \
    static ASMJIT_INLINE_CONSTEXPR T& operator&=(T& a, T b) noexcept {         \
      a = T(std::underlying_type_t<T>(a) & std::underlying_type_t<T>(b));      \
      return a;                                                                \
    }                                                                          \
    static ASMJIT_INLINE_CONSTEXPR T& operator^=(T& a, T b) noexcept {         \
      a = T(std::underlying_type_t<T>(a) ^ std::underlying_type_t<T>(b));      \
      return a;                                                                \
    }
#endif
#if defined(_DOXYGEN)
  #define ASMJIT_DEFINE_ENUM_COMPARE(T)
#else
  #define ASMJIT_DEFINE_ENUM_COMPARE(T)                                        \
    static ASMJIT_INLINE_CONSTEXPR bool operator<(T a, T b) noexcept {         \
      return (std::underlying_type_t<T>)(a) < (std::underlying_type_t<T>)(b);  \
    }                                                                          \
    static ASMJIT_INLINE_CONSTEXPR bool operator<=(T a, T b) noexcept {        \
      return (std::underlying_type_t<T>)(a) <= (std::underlying_type_t<T>)(b); \
    }                                                                          \
    static ASMJIT_INLINE_CONSTEXPR bool operator>(T a, T b) noexcept {         \
      return (std::underlying_type_t<T>)(a) > (std::underlying_type_t<T>)(b);  \
    }                                                                          \
    static ASMJIT_INLINE_CONSTEXPR bool operator>=(T a, T b) noexcept {        \
      return (std::underlying_type_t<T>)(a) >= (std::underlying_type_t<T>)(b); \
    }
#endif
#endif
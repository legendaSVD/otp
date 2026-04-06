#ifndef ETHREAD_INLINE_H__
#define ETHREAD_INLINE_H__
#define ETHR_GCC_COMPILER_FALSE 0
#define ETHR_GCC_COMPILER_TRUE 1
#define ETHR_GCC_COMPILER_CLANG -1
#define ETHR_GCC_COMPILER_ICC -2
#if !defined(__GNUC__) && !defined(__GNUG__)
#  define ETHR_GCC_COMPILER ETHR_GCC_COMPILER_FALSE
#elif defined(__clang__)
#  define ETHR_GCC_COMPILER ETHR_GCC_COMPILER_CLANG
#elif defined(__ICC) || defined(__INTEL_COMPILER)
#  define ETHR_GCC_COMPILER ETHR_GCC_COMPILER_ICC
#else
#  define ETHR_GCC_COMPILER ETHR_GCC_COMPILER_TRUE
#endif
#if !defined(__GNUC__)
#  define ETHR_AT_LEAST_GCC_VSN__(MAJ, MIN, PL) 0
#elif !defined(__GNUC_MINOR__)
#  define ETHR_AT_LEAST_GCC_VSN__(MAJ, MIN, PL) \
  ((__GNUC__ << 24) >= (((MAJ) << 24) | ((MIN) << 12) | (PL)))
#elif !defined(__GNUC_PATCHLEVEL__)
#  define ETHR_AT_LEAST_GCC_VSN__(MAJ, MIN, PL) \
  (((__GNUC__ << 24) | (__GNUC_MINOR__ << 12)) >= (((MAJ) << 24) | ((MIN) << 12) | (PL)))
#else
#  define ETHR_AT_LEAST_GCC_VSN__(MAJ, MIN, PL) \
  (((__GNUC__ << 24) | (__GNUC_MINOR__ << 12) | __GNUC_PATCHLEVEL__) >= (((MAJ) << 24) | ((MIN) << 12) | (PL)))
#endif
#undef ETHR_INLINE
#if defined(__GNUC__)
#  define ETHR_INLINE __inline__
#  if ETHR_AT_LEAST_GCC_VSN__(3, 1, 1)
#    define ETHR_FORCE_INLINE __inline__ __attribute__((__always_inline__))
#    define ETHR_NOINLINE __attribute__((__noinline__))
#  else
#    define ETHR_FORCE_INLINE __inline__
#    define ETHR_NOINLINE
#  endif
#elif defined(__WIN32__)
#  define ETHR_INLINE __forceinline
#  define ETHR_FORCE_INLINE __forceinline
#  define ETHR_NOINLINE
#endif
#endif
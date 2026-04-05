#ifndef PCRE2_UTIL_H_IDEMPOTENT_GUARD
#define PCRE2_UTIL_H_IDEMPOTENT_GUARD
#ifdef PCRE2_DEBUG
#if defined(HAVE_ASSERT_H) && !defined(NDEBUG)
#include <assert.h>
#endif
#if defined(HAVE_ASSERT_H) && !defined(NDEBUG)
#define PCRE2_ASSERT(x) assert(x)
#else
#define PCRE2_ASSERT(x) do                                            \
{                                                                     \
  if (!(x))                                                           \
  {                                                                   \
  fprintf(stderr, "Assertion failed at " __FILE__ ":%d\n", __LINE__); \
  abort();                                                            \
  }                                                                   \
} while(0)
#endif
#if defined(HAVE_ASSERT_H) && !defined(NDEBUG)
#define PCRE2_UNREACHABLE()                                         \
assert(((void)"Execution reached unexpected point", 0))
#else
#define PCRE2_UNREACHABLE() do                                      \
{                                                                   \
fprintf(stderr, "Execution reached unexpected point at " __FILE__   \
                ":%d\n", __LINE__);                                 \
abort();                                                            \
} while(0)
#endif
#define PCRE2_DEBUG_UNREACHABLE() PCRE2_UNREACHABLE()
#endif
#ifndef PCRE2_ASSERT
#define PCRE2_ASSERT(x) do {} while(0)
#endif
#ifndef PCRE2_DEBUG_UNREACHABLE
#define PCRE2_DEBUG_UNREACHABLE() do {} while(0)
#endif
#ifndef PCRE2_UNREACHABLE
#ifdef HAVE_BUILTIN_UNREACHABLE
#define PCRE2_UNREACHABLE() __builtin_unreachable()
#elif defined(HAVE_BUILTIN_ASSUME)
#define PCRE2_UNREACHABLE() __assume(0)
#else
#define PCRE2_UNREACHABLE() do {} while(0)
#endif
#endif
#ifndef PCRE2_FALLTHROUGH
#if defined(__cplusplus) && __cplusplus >= 202002L && \
    defined(__has_cpp_attribute)
#if __has_cpp_attribute(fallthrough)
#define PCRE2_FALLTHROUGH [[fallthrough]];
#endif
#elif !defined(__cplusplus) && \
      defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L && \
      defined(__has_c_attribute)
#if __has_c_attribute(fallthrough)
#define PCRE2_FALLTHROUGH [[fallthrough]];
#endif
#elif ((defined(__clang__) && __clang_major__ >= 10) || \
       (defined(__GNUC__) && __GNUC__ >= 7)) && \
      defined(__has_attribute)
#if __has_attribute(fallthrough)
#define PCRE2_FALLTHROUGH __attribute__((fallthrough));
#endif
#endif
#endif
#ifndef PCRE2_FALLTHROUGH
#define PCRE2_FALLTHROUGH
#endif
#endif
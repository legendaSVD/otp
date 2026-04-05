#ifndef ASMJIT_CORE_API_BUILD_P_H_INCLUDED
#define ASMJIT_CORE_API_BUILD_P_H_INCLUDED
#define ASMJIT_EXPORTS
#ifdef _MSC_VER
  #ifndef _CRT_SECURE_NO_DEPRECATE
    #define _CRT_SECURE_NO_DEPRECATE
  #endif
  #ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
  #endif
#endif
#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
#else
  #if !defined(_WIN32) && !defined(_LARGEFILE64_SOURCE)
    #define _LARGEFILE64_SOURCE 1
  #endif
  #if defined(__APPLE__    ) || \
      defined(__HAIKU__    ) || \
      defined(__bsdi__     ) || \
      defined(__DragonFly__) || \
      defined(__FreeBSD__  ) || \
      defined(__NetBSD__   ) || \
      defined(__OpenBSD__  )
    #define ASMJIT_FILE64_API(NAME) NAME
  #else
    #define ASMJIT_FILE64_API(NAME) NAME##64
  #endif
#endif
#include <asmjit/core/api-config.h>
#if !defined(ASMJIT_BUILD_DEBUG) && defined(__GNUC__) && !defined(__clang__)
  #define ASMJIT_FAVOR_SIZE  __attribute__((__optimize__("Os")))
  #define ASMJIT_FAVOR_SPEED __attribute__((__optimize__("O3")))
#elif ASMJIT_CXX_HAS_ATTRIBUTE(__minsize__, 0)
  #define ASMJIT_FAVOR_SIZE __attribute__((__minsize__))
  #define ASMJIT_FAVOR_SPEED
#else
  #define ASMJIT_FAVOR_SIZE
  #define ASMJIT_FAVOR_SPEED
#endif
#if !defined(ASMJIT_TEST) && defined(__INTELLISENSE__)
  #define ASMJIT_TEST
#endif
#if defined(ASMJIT_TEST)
  #include <asmjit-testing/tests/broken.h>
#endif
#endif
#ifndef ASMJIT_CORE_OSUTILS_H_INCLUDED
#define ASMJIT_CORE_OSUTILS_H_INCLUDED
#include <asmjit/core/globals.h>
ASMJIT_BEGIN_NAMESPACE
class Lock {
public:
  ASMJIT_NONCOPYABLE(Lock)
#if defined(_WIN32)
#pragma pack(push, 8)
  struct ASMJIT_MAY_ALIAS Handle {
    void* DebugInfo;
    long LockCount;
    long RecursionCount;
    void* OwningThread;
    void* LockSemaphore;
    unsigned long* SpinCount;
  };
  Handle _handle;
#pragma pack(pop)
#elif !defined(__EMSCRIPTEN__)
  using Handle = pthread_mutex_t;
  Handle _handle;
#endif
  ASMJIT_INLINE_NODEBUG Lock() noexcept;
  ASMJIT_INLINE_NODEBUG ~Lock() noexcept;
  ASMJIT_INLINE_NODEBUG void lock() noexcept;
  ASMJIT_INLINE_NODEBUG void unlock() noexcept;
};
ASMJIT_END_NAMESPACE
#endif
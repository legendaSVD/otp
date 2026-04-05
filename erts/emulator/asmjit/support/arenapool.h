#ifndef ASMJIT_SUPPORT_ARENAPOOL_H_INCLUDED
#define ASMJIT_SUPPORT_ARENAPOOL_H_INCLUDED
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
template<typename T, size_t Size = sizeof(T)>
class ArenaPool {
public:
  ASMJIT_NONCOPYABLE(ArenaPool)
  struct Link { Link* next; };
  Link* _data {};
  ASMJIT_INLINE_NODEBUG ArenaPool() noexcept = default;
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _data = nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE T* alloc(Arena& arena) noexcept {
    Link* p = _data;
    if (ASMJIT_UNLIKELY(p == nullptr)) {
      return arena.alloc_oneshot<T>(Arena::aligned_size(Size));
    }
    _data = p->next;
    return static_cast<T*>(static_cast<void*>(p));
  }
  ASMJIT_INLINE void release(T* ptr) noexcept {
    ASMJIT_ASSERT(ptr != nullptr);
    Link* p = reinterpret_cast<Link*>(ptr);
    p->next = _data;
    _data = p;
  }
  ASMJIT_INLINE size_t pooled_item_count() const noexcept {
    size_t n = 0;
    Link* p = _data;
    while (p) {
      n++;
      p = p->next;
    }
    return n;
  }
};
ASMJIT_END_NAMESPACE
#endif
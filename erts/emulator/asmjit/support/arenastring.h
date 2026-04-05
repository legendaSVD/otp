#ifndef ASMJIT_SUPPORT_ARENASTRING_H_INCLUDED
#define ASMJIT_SUPPORT_ARENASTRING_H_INCLUDED
#include <asmjit/core/globals.h>
#include <asmjit/support/arena.h>
ASMJIT_BEGIN_NAMESPACE
struct ArenaStringBase {
  union {
    struct {
      uint32_t _size;
      char _embedded[sizeof(void*) * 2 - 4];
    };
    struct {
      void* _dummy;
      char* _external;
    };
  };
  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _dummy = nullptr;
    _external = nullptr;
  }
  Error set_data(Arena& arena, uint32_t max_embedded_size, const char* str, size_t size) noexcept {
    if (size == SIZE_MAX)
      size = strlen(str);
    if (size <= max_embedded_size) {
      memcpy(_embedded, str, size);
      _embedded[size] = '\0';
    }
    else {
      char* external = static_cast<char*>(arena.dup(str, size, true));
      if (ASMJIT_UNLIKELY(!external))
        return make_error(Error::kOutOfMemory);
      _external = external;
    }
    _size = uint32_t(size);
    return Error::kOk;
  }
};
template<size_t N>
class ArenaString {
public:
  static inline constexpr uint32_t kWholeSize = (N > sizeof(ArenaStringBase)) ? uint32_t(N) : uint32_t(sizeof(ArenaStringBase));
  static inline constexpr uint32_t kMaxEmbeddedSize = kWholeSize - 5;
  union {
    ArenaStringBase _base;
    char _whole_data[kWholeSize];
  };
  ASMJIT_INLINE_NODEBUG ArenaString() noexcept { reset(); }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _base.reset(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _base._size == 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* data() const noexcept { return _base._size <= kMaxEmbeddedSize ? _base._embedded : _base._external; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t size() const noexcept { return _base._size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_embedded() const noexcept { return _base._size <= kMaxEmbeddedSize; }
  ASMJIT_INLINE_NODEBUG Error set_data(Arena& arena, const char* data, size_t size) noexcept {
    return _base.set_data(arena, kMaxEmbeddedSize, data, size);
  }
};
ASMJIT_END_NAMESPACE
#endif
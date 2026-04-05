#ifndef ASMJIT_SUPPORT_ARENA_H_INCLUDED
#define ASMJIT_SUPPORT_ARENA_H_INCLUDED
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
struct ArenaStatistics {
  size_t _block_count;
  size_t _used_size;
  size_t _reserved_size;
  size_t _overhead_size;
  size_t _pooled_size;
  ASMJIT_INLINE_NODEBUG size_t block_count() const noexcept { return _block_count; }
  ASMJIT_INLINE_NODEBUG size_t used_size() const noexcept { return _used_size; }
  ASMJIT_INLINE_NODEBUG size_t reserved_size() const noexcept { return _reserved_size; }
  ASMJIT_INLINE_NODEBUG size_t overhead_size() const noexcept { return _overhead_size; }
  ASMJIT_INLINE_NODEBUG size_t pooled_size() const noexcept { return _pooled_size; }
  ASMJIT_INLINE void aggregate(const ArenaStatistics& other) noexcept {
    _block_count += other._block_count;
    _used_size += other._used_size;
    _reserved_size += other._reserved_size;
    _overhead_size += other._overhead_size;
    _pooled_size += other._pooled_size;
  }
  ASMJIT_INLINE ArenaStatistics& operator+=(const ArenaStatistics& other) noexcept {
    aggregate(other);
    return *this;
  }
};
class Arena {
public:
  ASMJIT_NONCOPYABLE(Arena)
  static inline constexpr size_t kAlignment = 8u;
  static inline constexpr size_t kMinManagedBlockSize = 1024;
  static inline constexpr size_t kMaxManagedBlockSize = size_t(1) << (sizeof(size_t) * 8 - 2);
  static inline constexpr size_t kReusableSlotCount = 8;
  static inline constexpr size_t kMinReusableSlotSize = 16;
  static inline constexpr size_t kMaxReusableSlotSize = kMinReusableSlotSize << (kReusableSlotCount - 1u);
  struct alignas(kAlignment) ManagedBlock {
    ManagedBlock* next;
    size_t size;
    ASMJIT_INLINE_NODEBUG uint8_t* data() const noexcept {
      return const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(*this));
    }
    ASMJIT_INLINE_NODEBUG uint8_t* end() const noexcept {
      return data() + size;
    }
  };
  struct ReusableSlot {
    ReusableSlot* next;
  };
  struct DynamicBlock {
    DynamicBlock* prev;
    DynamicBlock* next;
  };
  [[nodiscard]]
  static ASMJIT_INLINE bool _get_reusable_slot_index(size_t size, Out<size_t> slot) noexcept {
    slot = Support::bit_size_of<size_t> - 4u - Support::clz((size - 1u) | 0xF);
    return *slot < kReusableSlotCount;
  }
  [[nodiscard]]
  static ASMJIT_INLINE bool _get_reusable_slot_index(size_t size, Out<size_t> slot, Out<size_t> allocated_size) noexcept {
    slot = Support::bit_size_of<size_t> - 4u - Support::clz((size - 1u) | 0xF);
    allocated_size = kMinReusableSlotSize << *slot;
    return *slot < kReusableSlotCount;
  }
  template<typename T>
  static ASMJIT_INLINE_CONSTEXPR size_t aligned_size_of() noexcept { return Support::align_up(sizeof(T), kAlignment); }
  static ASMJIT_INLINE_CONSTEXPR size_t aligned_size(size_t size) noexcept { return Support::align_up(size, kAlignment); }
  uint8_t* _ptr {};
  uint8_t* _end {};
  ManagedBlock* _current_block {};
  ManagedBlock* _first_block {};
  uint8_t _current_block_size_shift {};
  uint8_t _min_block_size_shift {};
  uint8_t _max_block_size_shift {};
  uint8_t _has_static_block {};
  uint32_t _unused_byte_count {};
  ReusableSlot* _reusable_slots[kReusableSlotCount] {};
  DynamicBlock* _dynamic_blocks {};
  ASMJIT_INLINE_NODEBUG explicit Arena(size_t min_block_size) noexcept {
    _init(min_block_size, Span<uint8_t>{});
  }
  ASMJIT_INLINE_NODEBUG Arena(size_t min_block_size, Span<uint8_t> static_arena_memory) noexcept {
    _init(min_block_size, static_arena_memory);
  }
  ASMJIT_INLINE_NODEBUG ~Arena() noexcept { reset(ResetPolicy::kHard); }
  ASMJIT_API void _init(size_t min_block_size, Span<uint8_t> static_arena_memory) noexcept;
  ASMJIT_API void reset(ResetPolicy reset_policy = ResetPolicy::kSoft) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t min_block_size() const noexcept { return size_t(1) << _min_block_size_shift; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t max_block_size() const noexcept { return size_t(1) << _max_block_size_shift; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t has_static_block() const noexcept { return _has_static_block; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t remaining_size() const noexcept { return (size_t)(_end - _ptr); }
  template<typename T = uint8_t>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* ptr() noexcept { return reinterpret_cast<T*>(_ptr); }
  template<typename T = uint8_t>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* end() noexcept { return reinterpret_cast<T*>(_end); }
  template<typename T>
  ASMJIT_INLINE void set_ptr(T* ptr) noexcept {
    uint8_t* p = reinterpret_cast<uint8_t*>(ptr);
    ASMJIT_ASSERT(p >= _ptr && p <= _end);
    _ptr = p;
  }
  template<typename T>
  ASMJIT_INLINE void set_end(T* end) noexcept {
    uint8_t* p = reinterpret_cast<uint8_t*>(end);
    ASMJIT_ASSERT(p >= _ptr && p <= _end);
    _end = p;
  }
  [[nodiscard]]
  ASMJIT_API void* _alloc_oneshot(size_t size) noexcept;
  template<typename T = void>
  [[nodiscard]]
  ASMJIT_INLINE T* alloc_oneshot(size_t size) noexcept {
    ASMJIT_ASSERT(Support::is_aligned(size, kAlignment));
#if defined(__GNUC__)
    if (__builtin_constant_p(size) && size <= 1024u) {
      uint8_t* after = _ptr + size;
      if (ASMJIT_UNLIKELY(after > _end)) {
        return static_cast<T*>(_alloc_oneshot(size));
      }
      void* p = static_cast<void*>(_ptr);
      _ptr = after;
      return static_cast<T*>(p);
    }
#endif
    if (ASMJIT_UNLIKELY(size > remaining_size())) {
      return static_cast<T*>(_alloc_oneshot(size));
    }
    void* p = static_cast<void*>(_ptr);
    _ptr += size;
    return static_cast<T*>(p);
  }
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE T* alloc_oneshot() noexcept {
    return alloc_oneshot<T>(aligned_size_of<T>());
  }
  [[nodiscard]]
  ASMJIT_API void* _alloc_oneshot_zeroed(size_t size) noexcept;
  template<typename T = void>
  [[nodiscard]]
  ASMJIT_INLINE T* alloc_oneshot_zeroed(size_t size) noexcept {
    return static_cast<T*>(_alloc_oneshot_zeroed(size));
  }
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE T* new_oneshot() noexcept {
    void* p = alloc_oneshot(aligned_size_of<T>());
    if (ASMJIT_UNLIKELY(!p)) {
      return nullptr;
    }
    return new(Support::PlacementNew{p}) T();
  }
  template<typename T, typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE T* new_oneshot(Args&&... args) noexcept {
    void* p = alloc_oneshot(aligned_size_of<T>());
    if (ASMJIT_UNLIKELY(!p)) {
      return nullptr;
    }
    return new(Support::PlacementNew{p}) T(std::forward<Args>(args)...);
  }
  [[nodiscard]]
  ASMJIT_API void* dup(const void* data, size_t size, bool null_terminate = false) noexcept;
  [[nodiscard]]
  ASMJIT_API char* sformat(const char* str, ...) noexcept;
  [[nodiscard]]
  ASMJIT_API void* _alloc_reusable(size_t size, Out<size_t> allocated_size) noexcept;
  [[nodiscard]]
  ASMJIT_API void* _alloc_reusable_zeroed(size_t size, Out<size_t> allocated_size) noexcept;
  ASMJIT_API void _release_dynamic(void* p, size_t size) noexcept;
  template<typename T = void>
  [[nodiscard]]
  inline T* alloc_reusable(size_t size) noexcept {
    size_t dummy_allocated_size;
    return static_cast<T*>(_alloc_reusable(size, Out(dummy_allocated_size)));
  }
  template<typename T = void>
  [[nodiscard]]
  inline T* alloc_reusable(size_t size, Out<size_t> allocated_size) noexcept {
    return static_cast<T*>(_alloc_reusable(size, allocated_size));
  }
  template<typename T = void>
  [[nodiscard]]
  inline T* alloc_reusable_zeroed(size_t size) noexcept {
    size_t dummy_allocated_size;
    return static_cast<T*>(_alloc_reusable_zeroed(size, Out(dummy_allocated_size)));
  }
  template<typename T = void>
  [[nodiscard]]
  inline T* alloc_reusable_zeroed(size_t size, Out<size_t> allocated_size) noexcept {
    return static_cast<T*>(_alloc_reusable_zeroed(size, allocated_size));
  }
  inline void free_reusable(void* p, size_t size) noexcept {
    ASMJIT_ASSERT(p != nullptr);
    ASMJIT_ASSERT(size != 0);
    size_t slot;
    if (_get_reusable_slot_index(size, Out(slot))) {
      static_cast<ReusableSlot*>(p)->next = static_cast<ReusableSlot*>(_reusable_slots[slot]);
      _reusable_slots[slot] = static_cast<ReusableSlot*>(p);
    }
    else {
      _release_dynamic(p, size);
    }
  }
  ASMJIT_API ArenaStatistics statistics() const noexcept;
};
template<size_t N>
class ArenaTmp : public Arena {
public:
  ASMJIT_NONCOPYABLE(ArenaTmp)
  struct alignas(Arena::kAlignment) Storage {
    uint8_t data[N];
  } _storage;
  ASMJIT_INLINE explicit ArenaTmp(size_t min_block_size) noexcept
    : Arena(min_block_size, Span<uint8_t>(_storage.data, N)) {}
};
ASMJIT_END_NAMESPACE
#endif
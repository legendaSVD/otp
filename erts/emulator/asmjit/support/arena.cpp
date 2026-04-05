#include <asmjit/core/api-build_p.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
static constexpr size_t kArenaAlignmentOverhead =
  (Arena::kAlignment <= Globals::kAllocAlignment)
    ? size_t(0)
    : Arena::kAlignment - Globals::kAllocAlignment;
static const Arena::ManagedBlock _arena_zero_block {};
static ASMJIT_INLINE Arena::ManagedBlock* Arena_get_zero_block() noexcept {
  return const_cast<Arena::ManagedBlock*>(&_arena_zero_block);
}
static ASMJIT_INLINE void* Arena_malloc(size_t size) noexcept {
  return ::malloc(size);
}
static ASMJIT_INLINE void Arena_free(void* ptr) noexcept {
  ::free(ptr);
}
static ASMJIT_INLINE void Arena_assign_block(Arena& arena, Arena::ManagedBlock* block) noexcept {
  arena._ptr = Support::align_up(block->data(), Arena::kAlignment);
  arena._end = block->end();
  arena._current_block = block;
  ASMJIT_ASSERT(arena._ptr <= arena._end);
}
[[maybe_unused]]
static bool Arena_has_dynamic_block(Arena& arena, Arena::DynamicBlock* block) noexcept {
  Arena::DynamicBlock* current = arena._dynamic_blocks;
  while (current) {
    if (current == block) {
      return true;
    }
    current = current->next;
  }
  return false;
}
void Arena::_init(size_t min_block_size, Span<uint8_t> static_arena_memory) noexcept {
  ASMJIT_ASSERT(min_block_size >= kMinManagedBlockSize);
  ASMJIT_ASSERT(min_block_size <= kMaxManagedBlockSize);
  ManagedBlock* block = Arena_get_zero_block();
  size_t block_size_shift = Support::bit_size_of<size_t> - Support::clz(min_block_size);
  _current_block_size_shift = uint8_t(block_size_shift);
  _min_block_size_shift = uint8_t(block_size_shift);
  _max_block_size_shift = uint8_t(26);
  _has_static_block = uint8_t(static_arena_memory.size() != 0u);
  _unused_byte_count = 0u;
  if (static_arena_memory.size()) {
    block = reinterpret_cast<ManagedBlock*>(static_arena_memory.data());
    block->next = nullptr;
    ASMJIT_ASSERT(static_arena_memory.size() >= sizeof(ManagedBlock));
    block->size = static_arena_memory.size() - sizeof(ManagedBlock);
  }
  _first_block = block;
  Arena_assign_block(*this, block);
}
void Arena::reset(ResetPolicy reset_policy) noexcept {
  ManagedBlock* first = _first_block;
  if (reset_policy == ResetPolicy::kHard) {
    ManagedBlock* current = first;
    if (first == &_arena_zero_block) {
      return;
    }
    if (has_static_block()) {
      current = current->next;
      first->next = nullptr;
    }
    else {
      first = Arena_get_zero_block();
      _first_block = first;
    }
    if (current) {
      do {
        ManagedBlock* next = current->next;
        Arena_free(current);
        current = next;
      } while (current);
    }
    _current_block_size_shift = _min_block_size_shift;
  }
  {
    DynamicBlock* current = _dynamic_blocks;
    while (current) {
      DynamicBlock* next = current->next;
      Arena_free(current);
      current = next;
    }
    memset(_reusable_slots, 0, sizeof(_reusable_slots));
    _dynamic_blocks = nullptr;
  }
  Arena_assign_block(*this, first);
  _unused_byte_count = 0u;
}
static ASMJIT_NOINLINE void Arena_make_block_leftover_reusable(Arena& arena, uint8_t* ptr, size_t size) noexcept {
  while (size >= Arena::kMinReusableSlotSize) {
    size_t saved_slot {};
    size_t saved_size {};
    if (!Arena::_get_reusable_slot_index(size / 2u, Out(saved_slot), Out(saved_size))) {
      saved_slot = Arena::kReusableSlotCount - 1u;
      saved_size = Arena::kMaxReusableSlotSize;
    }
    reinterpret_cast<Arena::ReusableSlot*>(ptr)->next = arena._reusable_slots[saved_slot];
    arena._reusable_slots[saved_slot] = reinterpret_cast<Arena::ReusableSlot*>(ptr);
    ptr += saved_size;
    size -= saved_size;
  }
  arena._ptr = ptr;
}
static ASMJIT_INLINE uint32_t Arena_get_unused_block_byte_count(Arena::ManagedBlock* block, const uint8_t* ptr) noexcept {
  return uint32_t(size_t(block->end() - ptr));
}
void* Arena::_alloc_oneshot(size_t size) noexcept {
  ASMJIT_ASSERT(Support::is_aligned(size, Arena::kAlignment));
  constexpr size_t kBlockSizeOverhead = sizeof(ManagedBlock) + Globals::kAllocOverhead + kArenaAlignmentOverhead;
  ManagedBlock* cur_block = _current_block;
  ManagedBlock* next = cur_block->next;
  uint32_t unused_byte_count = Arena_get_unused_block_byte_count(cur_block, _ptr);
  while (next) {
    uint8_t* ptr = Support::align_up(next->data(), Arena::kAlignment);
    uint8_t* end = next->end();
    if (size <= (size_t)(end - ptr)) {
      _current_block = next;
      _ptr = ptr + size;
      _end = end;
      _unused_byte_count += unused_byte_count;
      ASMJIT_ASSERT(_ptr <= _end);
      return static_cast<void*>(ptr);
    }
    ManagedBlock* block_to_free = next;
    cur_block->next = next;
    next = next->next;
    Arena_free(block_to_free);
  }
  uint32_t block_size_shift = uint32_t(_current_block_size_shift);
  size_t block_size = size_t(1) << block_size_shift;
  if (ASMJIT_UNLIKELY(size > block_size - kBlockSizeOverhead)) {
    if (ASMJIT_UNLIKELY(size > SIZE_MAX - kBlockSizeOverhead)) {
      return nullptr;
    }
    block_size = size + kArenaAlignmentOverhead + sizeof(ManagedBlock);
  }
  else {
    block_size -= Globals::kAllocOverhead;
  }
  ManagedBlock* new_block = static_cast<ManagedBlock*>(Arena_malloc(block_size));
  if (ASMJIT_UNLIKELY(!new_block)) {
    return nullptr;
  }
  ASMJIT_ASSERT(Support::is_aligned(new_block, Globals::kAllocAlignment));
  size_t real_block_size = block_size - sizeof(ManagedBlock);
  new_block->next = next;
  new_block->size = real_block_size;
  if (cur_block == &_arena_zero_block) {
    _first_block = new_block;
  }
  else {
    cur_block->next = new_block;
  }
  uint8_t* ptr = Support::align_up(new_block->data(), Arena::kAlignment);
  uint8_t* end = new_block->data() + real_block_size;
  _ptr = ptr + size;
  _end = end;
  _current_block = new_block;
  _current_block_size_shift = uint8_t(Support::min<uint32_t>(uint32_t(block_size_shift) + 1u, _max_block_size_shift));
  _unused_byte_count += unused_byte_count;
  ASMJIT_ASSERT(_ptr <= _end);
  return static_cast<void*>(ptr);
}
void* Arena::_alloc_oneshot_zeroed(size_t size) noexcept {
  ASMJIT_ASSERT(Support::is_aligned(size, Arena::kAlignment));
  void* p = alloc_oneshot(size);
  if (ASMJIT_UNLIKELY(!p)) {
    return p;
  }
  return memset(p, 0, size);
}
void* Arena::dup(const void* data, size_t size, bool null_terminate) noexcept {
  if (ASMJIT_UNLIKELY(!data || !size)) {
    return nullptr;
  }
  ASMJIT_ASSERT(size != SIZE_MAX);
  size_t alloc_size = Support::align_up(size + size_t(null_terminate), Arena::kAlignment);
  uint8_t* m = alloc_oneshot<uint8_t>(alloc_size);
  if (ASMJIT_UNLIKELY(!m)) {
    return nullptr;
  }
  static_assert(Arena::kAlignment == 8u, "the code below must be fixed if arena alignment has changed");
  Support::storeu<uint64_t>(m + alloc_size - sizeof(uint64_t), 0u);
  memcpy(m, data, size);
  return static_cast<void*>(m);
}
char* Arena::sformat(const char* fmt, ...) noexcept {
  if (ASMJIT_UNLIKELY(!fmt)) {
    return nullptr;
  }
  char buf[512];
  size_t size;
  va_list ap;
  va_start(ap, fmt);
  size = unsigned(vsnprintf(buf, ASMJIT_ARRAY_SIZE(buf) - 1, fmt, ap));
  va_end(ap);
  buf[size++] = 0;
  return static_cast<char*>(dup(buf, size));
}
void* Arena::_alloc_reusable(size_t size, Out<size_t> allocated_size) noexcept {
  size_t slot;
  if (_get_reusable_slot_index(size, Out(slot), allocated_size)) {
    uint8_t* p = reinterpret_cast<uint8_t*>(_reusable_slots[slot]);
    size = *allocated_size;
    if (p) {
      _reusable_slots[slot] = reinterpret_cast<ReusableSlot*>(p)->next;
      return p;
    }
    p = _ptr;
    size_t remaining_size = (size_t)(_end - p);
    if (ASMJIT_LIKELY(remaining_size >= size)) {
      _ptr = p + size;
      return p;
    }
    Arena_make_block_leftover_reusable(*this, p, remaining_size);
    p = static_cast<uint8_t*>(_alloc_oneshot(size));
    if (ASMJIT_UNLIKELY(!p)) {
      allocated_size = 0;
      return nullptr;
    }
    return p;
  }
  else {
    size_t dynamic_block_overhead = Support::align_up(sizeof(DynamicBlock) + sizeof(DynamicBlock*) + kArenaAlignmentOverhead, kAlignment);
    if (ASMJIT_UNLIKELY(size >= SIZE_MAX - dynamic_block_overhead)) {
      return nullptr;
    }
    void* p = Arena_malloc(size + dynamic_block_overhead);
    if (ASMJIT_UNLIKELY(!p)) {
      allocated_size = 0;
      return nullptr;
    }
    ASMJIT_ASSERT(Support::is_aligned(p, Globals::kAllocAlignment));
    DynamicBlock* dynamic_block = static_cast<DynamicBlock*>(p);
    DynamicBlock* next = _dynamic_blocks;
    if (next) {
      next->prev = dynamic_block;
    }
    dynamic_block->prev = nullptr;
    dynamic_block->next = next;
    _dynamic_blocks = dynamic_block;
    p = Support::align_up(static_cast<uint8_t*>(p) + sizeof(DynamicBlock) + sizeof(DynamicBlock*), kAlignment);
    reinterpret_cast<DynamicBlock**>(p)[-1] = dynamic_block;
    allocated_size = size;
    return p;
  }
}
void* Arena::_alloc_reusable_zeroed(size_t size, Out<size_t> allocated_size) noexcept {
  void* p = _alloc_reusable(size, allocated_size);
  if (ASMJIT_UNLIKELY(!p)) {
    return p;
  }
  return memset(p, 0, *allocated_size);
}
void Arena::_release_dynamic(void* p, size_t size) noexcept {
  Support::maybe_unused(size);
  DynamicBlock* dynamic_block = reinterpret_cast<DynamicBlock**>(p)[-1];
  ASMJIT_ASSERT(Arena_has_dynamic_block(*this, dynamic_block));
  DynamicBlock* prev = dynamic_block->prev;
  DynamicBlock* next = dynamic_block->next;
  if (prev) {
    prev->next = next;
  }
  else {
    _dynamic_blocks = next;
  }
  if (next) {
    next->prev = prev;
  }
  Arena_free(dynamic_block);
}
ArenaStatistics Arena::statistics() const noexcept {
  const ManagedBlock* block = _first_block;
  size_t block_count = 0u;
  size_t used_size = 0u;
  size_t reserved_size = 0u;
  while (block) {
    if (_ptr >= block->data() && _ptr <= block->end()) {
      size_t offset = size_t(_ptr - block->data());
      used_size = reserved_size + offset;
    }
    block_count++;
    reserved_size += block->size;
    block = block->next;
  }
  ArenaStatistics stats {};
  stats._block_count = block_count;
  stats._used_size = used_size;
  stats._reserved_size = reserved_size;
  stats._overhead_size = _unused_byte_count;
  return stats;
}
#if defined(ASMJIT_TEST)
UNIT(arena_oneshot) {
  struct SomeData {
    size_t _x;
    size_t _y;
    inline SomeData(size_t x, size_t y) noexcept
      : _x(x), _y(y) {}
  };
  constexpr size_t kN = 100000u;
  {
    Arena arena(1024u * 4u);
    for (size_t r = 0; r < 3u; r++) {
      for (size_t i = 0; i < kN; i++) {
        uint8_t* p = arena.alloc_oneshot<uint8_t>(32);
        EXPECT_NOT_NULL(p);
      }
      ArenaStatistics stats = arena.statistics();
      EXPECT_GE(stats.block_count(), 2u);
      EXPECT_GE(stats.used_size(), kN * 32u);
      EXPECT_GE(stats.reserved_size(), kN * 32u);
      EXPECT_GE(stats.reserved_size(), stats.used_size());
      arena.reset(r == 0 ? ResetPolicy::kSoft : ResetPolicy::kHard);
    }
  }
  {
    Arena arena(1024u * 4u);
    for (size_t r = 0; r < 3u; r++) {
      for (size_t i = 0; i < kN; i++) {
        SomeData* p = arena.new_oneshot<SomeData>(r, i);
        EXPECT_NOT_NULL(p);
      }
      arena.reset(r == 0 ? ResetPolicy::kSoft : ResetPolicy::kHard);
    }
  }
}
UNIT(arena_reusable_slots_check) {
  constexpr size_t kMinReusableSlotSize = Arena::kMinReusableSlotSize;
  constexpr size_t kMaxReusableSlotSize = Arena::kMaxReusableSlotSize;
  size_t expected_slot = 0;
  size_t expected_until = kMinReusableSlotSize;
  for (size_t size = 1; size <= kMaxReusableSlotSize; size++) {
    size_t acquired_slot;
    EXPECT_TRUE(Arena::_get_reusable_slot_index(size, Out(acquired_slot)));
    EXPECT_EQ(acquired_slot, expected_slot);
    EXPECT_LT(acquired_slot, Arena::kReusableSlotCount);
    if (size == expected_until) {
      expected_slot++;
      expected_until *= 2;
    }
  }
}
#endif
ASMJIT_END_NAMESPACE
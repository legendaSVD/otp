#include <asmjit/core/api-build_p.h>
#ifndef ASMJIT_NO_JIT
#include <asmjit/core/archtraits.h>
#include <asmjit/core/jitallocator.h>
#include <asmjit/core/osutils_p.h>
#include <asmjit/core/virtmem.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenalist.h>
#include <asmjit/support/arenapool.h>
#include <asmjit/support/arenatree.h>
#include <asmjit/support/support.h>
#if defined(ASMJIT_TEST)
#include <asmjit-testing/commons/random.h>
#endif
ASMJIT_BEGIN_NAMESPACE
static constexpr uint32_t kJitAllocatorMultiPoolCount = 3;
static constexpr uint32_t kJitAllocatorBaseGranularity = 64;
static constexpr uint32_t kJitAllocatorMaxBlockSize = 1024 * 1024 * 64;
static inline uint32_t JitAllocator_default_fill_pattern() noexcept {
#if ASMJIT_ARCH_X86
  return 0xCCCCCCCCu;
#else
  return 0u;
#endif
}
template<typename T, uint32_t B>
class BitVectorRangeIterator {
public:
  const T* _ptr;
  size_t _idx;
  size_t _end;
  T _bit_word;
  static inline constexpr uint32_t kBitWordSize = Support::bit_size_of<T>;
  static inline constexpr T kXorMask = B == 0 ? Support::bit_ones<T> : T(0);
  ASMJIT_INLINE BitVectorRangeIterator(const T* data, size_t bit_word_count) noexcept {
    init(data, bit_word_count);
  }
  ASMJIT_INLINE BitVectorRangeIterator(const T* data, size_t bit_word_count, size_t start, size_t end) noexcept {
    init(data, bit_word_count, start, end);
  }
  ASMJIT_INLINE void init(const T* data, size_t bit_word_count) noexcept {
    init(data, bit_word_count, 0, bit_word_count * kBitWordSize);
  }
  ASMJIT_INLINE void init(const T* data, size_t bit_word_count, size_t start, size_t end) noexcept {
    ASMJIT_ASSERT(bit_word_count >= (end + kBitWordSize - 1) / kBitWordSize);
    Support::maybe_unused(bit_word_count);
    size_t idx = Support::align_down(start, kBitWordSize);
    const T* ptr = data + (idx / kBitWordSize);
    T bit_word = 0;
    if (idx < end) {
      bit_word = (*ptr ^ kXorMask) & (Support::bit_ones<T> << (start % kBitWordSize));
    }
    _ptr = ptr;
    _idx = idx;
    _end = end;
    _bit_word = bit_word;
  }
  ASMJIT_INLINE bool next_range(Out<size_t> range_start, Out<size_t> range_end, size_t range_hint = std::numeric_limits<size_t>::max()) noexcept {
    while (_bit_word == 0) {
      _idx += kBitWordSize;
      if (_idx >= _end) {
        return false;
      }
      _bit_word = (*++_ptr) ^ kXorMask;
    }
    size_t i = Support::ctz(_bit_word);
    *range_start = _idx + i;
    _bit_word = ~(_bit_word ^ ~(Support::bit_ones<T> << i));
    if (_bit_word == 0) {
      *range_end = Support::min(_idx + kBitWordSize, _end);
      while (*range_end - *range_start < range_hint) {
        _idx += kBitWordSize;
        if (_idx >= _end) {
          break;
        }
        _bit_word = (*++_ptr) ^ kXorMask;
        if (_bit_word != Support::bit_ones<T>) {
          size_t j = Support::ctz(~_bit_word);
          *range_end = Support::min(_idx + j, _end);
          _bit_word = _bit_word ^ ~(Support::bit_ones<T> << j);
          break;
        }
        *range_end = Support::min(_idx + kBitWordSize, _end);
        _bit_word = 0;
        continue;
      }
      return true;
    }
    else {
      size_t j = Support::ctz(_bit_word);
      *range_end = Support::min(_idx + j, _end);
      _bit_word = ~(_bit_word ^ ~(Support::bit_ones<T> << j));
      return true;
    }
  }
};
class JitAllocatorBlock;
class JitAllocatorPool {
public:
  ASMJIT_NONCOPYABLE(JitAllocatorPool)
  ArenaList<JitAllocatorBlock> blocks;
  JitAllocatorBlock* cursor = nullptr;
  uint32_t block_count = 0;
  uint16_t granularity = 0;
  uint8_t granularity_log2 = 0;
  uint8_t empty_block_count = 0;
  size_t total_area_size[2] {};
  size_t total_area_used[2] {};
  size_t total_overhead_bytes = 0;
  ASMJIT_INLINE JitAllocatorPool(uint32_t granularity) noexcept
    : blocks(),
      granularity(uint16_t(granularity)),
      granularity_log2(uint8_t(Support::ctz(granularity))) {}
  ASMJIT_INLINE void reset() noexcept {
    blocks.reset();
    cursor = nullptr;
    block_count = 0u;
    total_area_size[0] = 0u;
    total_area_size[1] = 0u;
    total_area_used[0] = 0u;
    total_area_used[1] = 0u;
    total_overhead_bytes = 0u;
  }
  ASMJIT_INLINE_NODEBUG size_t byte_size_from_area_size(uint32_t area_size) const noexcept { return size_t(area_size) * granularity; }
  ASMJIT_INLINE_NODEBUG uint32_t area_size_from_byte_size(size_t size) const noexcept { return uint32_t((size + granularity - 1) >> granularity_log2); }
  ASMJIT_INLINE_NODEBUG size_t bit_word_count_from_area_size(uint32_t area_size) const noexcept {
    static constexpr uint32_t kBitWordSizeInBits = Support::bit_size_of<Support::BitWord>;
    return Support::align_up<size_t>(area_size, kBitWordSizeInBits) / kBitWordSizeInBits;
  }
};
class JitAllocatorBlock : public ArenaTreeNodeT<JitAllocatorBlock>,
                          public ArenaListNode<JitAllocatorBlock> {
public:
  ASMJIT_NONCOPYABLE(JitAllocatorBlock)
  enum Flags : uint32_t {
    kFlagInitialPadding = 0x00000001u,
    kFlagEmpty = 0x00000002u,
    kFlagDirty = 0x00000004u,
    kFlagIncremental = 0x00000008u,
    kFlagLargePages = 0x00000010u,
    kFlagDualMapped = 0x00000020u
  };
  static_assert(kFlagInitialPadding == 1, "JitAllocatorBlock::kFlagInitialPadding must be equal to 1");
  static inline uint32_t initial_area_start_by_flags(uint32_t flags) noexcept {
    return flags & kFlagInitialPadding;
  }
  JitAllocatorPool* _pool {};
  VirtMem::DualMapping _mapping {};
  size_t _block_size = 0;
  uint32_t _flags = 0;
  uint32_t _area_size = 0;
  uint32_t _area_used = 0;
  uint32_t _largest_unused_area = 0;
  uint32_t _search_start = 0;
  uint32_t _search_end = 0;
  Support::BitWord* _used_bit_vector {};
  Support::BitWord* _stop_bit_vector {};
  ASMJIT_INLINE JitAllocatorBlock(
    JitAllocatorPool* pool,
    VirtMem::DualMapping mapping,
    size_t block_size,
    uint32_t block_flags,
    Support::BitWord* used_bit_vector,
    Support::BitWord* stop_bit_vector,
    uint32_t area_size
  ) noexcept
    : ArenaTreeNodeT(),
      _pool(pool),
      _mapping(mapping),
      _block_size(block_size),
      _flags(block_flags),
      _area_size(area_size),
      _used_bit_vector(used_bit_vector),
      _stop_bit_vector(stop_bit_vector) {
    clear_block();
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG JitAllocatorPool* pool() const noexcept { return _pool; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* rx_ptr() const noexcept { return static_cast<uint8_t*>(_mapping.rx); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* rw_ptr() const noexcept { return static_cast<uint8_t*>(_mapping.rw); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(uint32_t f) const noexcept { return (_flags & f) != 0; }
  ASMJIT_INLINE_NODEBUG void add_flags(uint32_t f) noexcept { _flags |= f; }
  ASMJIT_INLINE_NODEBUG void clear_flags(uint32_t f) noexcept { _flags &= ~f; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return has_flag(kFlagEmpty); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_dirty() const noexcept { return has_flag(kFlagDirty); }
  ASMJIT_INLINE_NODEBUG void make_dirty() noexcept { add_flags(kFlagDirty); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_incremental() const noexcept { return has_flag(kFlagIncremental); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_large_pages() const noexcept { return has_flag(kFlagLargePages); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_initial_padding() const noexcept { return has_flag(kFlagInitialPadding); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t initial_area_start() const noexcept { return initial_area_start_by_flags(_flags); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t block_size() const noexcept { return _block_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t area_size() const noexcept { return _area_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t area_used() const noexcept { return _area_used; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t area_available() const noexcept { return _area_size - _area_used; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t largest_unused_area() const noexcept { return _largest_unused_area; }
  ASMJIT_INLINE void clear_block() noexcept {
    bool bit = has_initial_padding();
    size_t bit_word_count = _pool->bit_word_count_from_area_size(_area_size);
    memset(_used_bit_vector, 0, bit_word_count * sizeof(Support::BitWord));
    memset(_stop_bit_vector, 0, bit_word_count * sizeof(Support::BitWord));
    Support::bit_vector_set_bit(_used_bit_vector, 0, bit);
    Support::bit_vector_set_bit(_stop_bit_vector, 0, bit);
    uint32_t start = initial_area_start_by_flags(_flags);
    _area_used = start;
    _largest_unused_area = _area_size - start;
    _search_start = start;
    _search_end = _area_size;
    add_flags(JitAllocatorBlock::kFlagEmpty | JitAllocatorBlock::kFlagIncremental);
    clear_flags(JitAllocatorBlock::kFlagDirty);
  }
  ASMJIT_INLINE void mark_allocated_area(uint32_t allocated_area_start, uint32_t allocated_area_end) noexcept {
    uint32_t allocated_area_size = allocated_area_end - allocated_area_start;
    Support::bit_vector_fill(_used_bit_vector, allocated_area_start, allocated_area_size);
    Support::bit_vector_set_bit(_stop_bit_vector, allocated_area_end - 1, true);
    _pool->total_area_used[size_t(has_large_pages())] += allocated_area_size;
    _area_used += allocated_area_size;
    if (area_available() == 0) {
      _search_start = _area_size;
      _search_end = 0;
      _largest_unused_area = 0;
      clear_flags(kFlagDirty | kFlagEmpty);
    }
    else {
      if (_search_start == allocated_area_start) {
        _search_start = allocated_area_end;
      }
      if (_search_end == allocated_area_end) {
        _search_end = allocated_area_start;
      }
      add_flags(kFlagDirty);
      clear_flags(kFlagEmpty);
    }
  }
  ASMJIT_INLINE void mark_released_area(uint32_t released_area_start, uint32_t released_area_end) noexcept {
    uint32_t released_area_size = released_area_end - released_area_start;
    _pool->total_area_used[size_t(has_large_pages())] -= released_area_size;
    _area_used -= released_area_size;
    Support::bit_vector_clear(_used_bit_vector, released_area_start, released_area_size);
    Support::bit_vector_set_bit(_stop_bit_vector, released_area_end - 1, false);
    if (Support::bool_and(is_incremental(), _search_start == released_area_end)) {
      ASMJIT_ASSERT(_search_start >= released_area_size);
      _search_start -= released_area_size;
      _largest_unused_area += released_area_size;
    }
    else {
      _search_start = Support::min(_search_start, released_area_start);
      _search_end = Support::max(_search_end, released_area_end);
      clear_flags(kFlagDirty | kFlagIncremental);
      if (area_used() == initial_area_start()) {
        _search_start = initial_area_start();
        _search_end = _area_size;
        _largest_unused_area = _area_size - initial_area_start();
        add_flags(kFlagEmpty);
      }
      else {
        add_flags(kFlagDirty);
      }
    }
  }
  ASMJIT_INLINE void mark_shrunk_area(uint32_t shrunk_area_start, uint32_t shrunk_area_end) noexcept {
    uint32_t shrunk_area_size = shrunk_area_end - shrunk_area_start;
    ASMJIT_ASSERT(shrunk_area_start != 0);
    ASMJIT_ASSERT(shrunk_area_size != 0);
    _pool->total_area_used[size_t(has_large_pages())] -= shrunk_area_size;
    _area_used -= shrunk_area_size;
    if (Support::bool_and(is_incremental(), _search_start == shrunk_area_end)) {
      _search_start -= shrunk_area_size;
      _largest_unused_area += shrunk_area_size;
    }
    else {
      _search_start = Support::min(_search_start, shrunk_area_start);
      _search_end = Support::max(_search_end, shrunk_area_end);
      clear_flags(kFlagIncremental);
      add_flags(kFlagDirty);
    }
    Support::bit_vector_clear(_used_bit_vector, shrunk_area_start, shrunk_area_size);
    Support::bit_vector_set_bit(_stop_bit_vector, shrunk_area_end - 1, false);
    Support::bit_vector_set_bit(_stop_bit_vector, shrunk_area_start - 1, true);
  }
  ASMJIT_INLINE_NODEBUG bool operator<(const JitAllocatorBlock& other) const noexcept { return rx_ptr() < other.rx_ptr(); }
  ASMJIT_INLINE_NODEBUG bool operator>(const JitAllocatorBlock& other) const noexcept { return rx_ptr() > other.rx_ptr(); }
  ASMJIT_INLINE_NODEBUG bool operator<(const uint8_t* key) const noexcept { return rx_ptr() + _block_size <= key; }
  ASMJIT_INLINE_NODEBUG bool operator>(const uint8_t* key) const noexcept { return rx_ptr() > key; }
};
class JitAllocatorPrivateImpl : public JitAllocator::Impl {
public:
  mutable Lock lock;
  uint32_t page_size;
  size_t allocation_count;
  ArenaTree<JitAllocatorBlock> tree;
  JitAllocatorPool* pools;
  size_t pool_count;
  ASMJIT_INLINE JitAllocatorPrivateImpl(JitAllocatorPool* pools, size_t pool_count) noexcept
    : JitAllocator::Impl {},
      page_size(0),
      allocation_count(0),
      pools(pools),
      pool_count(pool_count) {}
  ASMJIT_INLINE ~JitAllocatorPrivateImpl() noexcept {}
};
static const JitAllocator::Impl JitAllocatorImpl_none {};
static const JitAllocator::CreateParams JitAllocatorParams_none {};
static inline JitAllocatorPrivateImpl* JitAllocator_new_impl(const JitAllocator::CreateParams* params) noexcept {
  VirtMem::Info vm_info = VirtMem::info();
  if (!params) {
    params = &JitAllocatorParams_none;
  }
  JitAllocatorOptions options = params->options;
  uint32_t block_size = params->block_size;
  uint32_t granularity = params->granularity;
  uint32_t fill_pattern = params->fill_pattern;
  size_t pool_count = 1;
  if (Support::test(options, JitAllocatorOptions::kUseMultiplePools)) {
    pool_count = kJitAllocatorMultiPoolCount;
  }
  if (block_size < 64 * 1024 || block_size > 256 * 1024 * 1024 || !Support::is_power_of_2(block_size)) {
    block_size = vm_info.page_granularity;
  }
  if (granularity < 64 || granularity > 256 || !Support::is_power_of_2(granularity)) {
    granularity = kJitAllocatorBaseGranularity;
  }
  if (uint32_t(options & JitAllocatorOptions::kCustomFillPattern) == 0) {
    fill_pattern = JitAllocator_default_fill_pattern();
  }
  size_t size = sizeof(JitAllocatorPrivateImpl) + sizeof(JitAllocatorPool) * pool_count;
  void* p = ::malloc(size);
  if (ASMJIT_UNLIKELY(!p)) {
    return nullptr;
  }
  VirtMem::HardenedRuntimeInfo hardened_rt_info = VirtMem::hardened_runtime_info();
  if (Support::test(hardened_rt_info.flags, VirtMem::HardenedRuntimeFlags::kEnabled)) {
    if (!Support::test(hardened_rt_info.flags, VirtMem::HardenedRuntimeFlags::kMapJit)) {
      options |= JitAllocatorOptions::kUseDualMapping;
    }
  }
  JitAllocatorPool* pools = reinterpret_cast<JitAllocatorPool*>((uint8_t*)p + sizeof(JitAllocatorPrivateImpl));
  JitAllocatorPrivateImpl* impl = new(Support::PlacementNew{p}) JitAllocatorPrivateImpl(pools, pool_count);
  impl->options = options;
  impl->block_size = block_size;
  impl->granularity = granularity;
  impl->fill_pattern = fill_pattern;
  impl->page_size = vm_info.page_size;
  for (size_t pool_id = 0; pool_id < pool_count; pool_id++) {
    new(Support::PlacementNew{&pools[pool_id]}) JitAllocatorPool(granularity << pool_id);
  }
  return impl;
}
static ASMJIT_INLINE void JitAllocator_destroy_impl(JitAllocatorPrivateImpl* impl) noexcept {
  impl->~JitAllocatorPrivateImpl();
  ::free(impl);
}
static ASMJIT_INLINE size_t JitAllocator_size_to_pool_id(const JitAllocatorPrivateImpl* impl, size_t size) noexcept {
  size_t pool_id = impl->pool_count - 1;
  size_t granularity = size_t(impl->granularity) << pool_id;
  while (pool_id) {
    if (Support::align_up(size, granularity) == size) {
      break;
    }
    pool_id--;
    granularity >>= 1;
  }
  return pool_id;
}
static ASMJIT_INLINE size_t JitAllocator_bit_vector_size_to_byte_size(uint32_t area_size) noexcept {
  static constexpr uint32_t kBitWordSizeInBits = Support::bit_size_of<Support::BitWord>;
  return ((area_size + kBitWordSizeInBits - 1u) / kBitWordSizeInBits) * sizeof(Support::BitWord);
}
static ASMJIT_INLINE size_t JitAllocator_calculate_ideal_block_size(JitAllocatorPrivateImpl* impl, JitAllocatorPool* pool, size_t allocation_size) noexcept {
  JitAllocatorBlock* last = pool->blocks.last();
  size_t block_size = last ? last->block_size() : size_t(impl->block_size);
  if (!Support::test(impl->options, JitAllocatorOptions::kDisableInitialPadding)) {
    size_t granularity = pool->granularity;
    if (SIZE_MAX - allocation_size < granularity) {
      return 0;
    }
    allocation_size += granularity;
  }
  if (block_size < kJitAllocatorMaxBlockSize) {
    block_size *= 2u;
  }
  if (allocation_size > block_size) {
    block_size = Support::align_up(allocation_size, impl->block_size);
    if (ASMJIT_UNLIKELY(block_size < allocation_size)) {
      return 0;
    }
  }
  return block_size;
}
ASMJIT_NOINLINE
ASMJIT_FAVOR_SPEED static void JitAllocator_fill_pattern(void* mem, uint32_t pattern, size_t byte_size) noexcept {
  if ((uintptr_t(mem) & 0x1u) && byte_size >= 1u) {
    static_cast<uint8_t*>(mem)[0] = uint8_t(pattern & 0xFF);
    mem = static_cast<uint8_t*>(mem) + 1;
    byte_size--;
  }
  if ((uintptr_t(mem) & 0x2u) && byte_size >= 2u) {
    static_cast<uint16_t*>(mem)[0] = uint16_t(pattern & 0xFFFF);
    mem = static_cast<uint16_t*>(mem) + 1;
    byte_size -= 2;
  }
  ASMJIT_ASSERT((byte_size & 0x3u) == 0u);
  uint32_t* mem32 = static_cast<uint32_t*>(mem);
  size_t n = byte_size / 4u;
  for (size_t i = 0; i < n; i++) {
    mem32[i] = pattern;
  }
}
static Error JitAllocator_new_block(JitAllocatorPrivateImpl* impl, JitAllocatorBlock** dst, JitAllocatorPool* pool, size_t block_size) noexcept {
  using Support::BitWord;
  static constexpr uint32_t kBitWordSizeInBits = Support::bit_size_of<Support::BitWord>;
  uint32_t block_flags = 0;
  if (!Support::test(impl->options, JitAllocatorOptions::kDisableInitialPadding)) {
    block_flags |= JitAllocatorBlock::kFlagInitialPadding;
  }
  VirtMem::DualMapping virt_mem {};
  VirtMem::MemoryFlags mem_flags = VirtMem::MemoryFlags::kAccessRWX;
  if (Support::test(impl->options, JitAllocatorOptions::kUseDualMapping)) {
    ASMJIT_PROPAGATE(VirtMem::alloc_dual_mapping(Out(virt_mem), block_size, mem_flags));
    block_flags |= JitAllocatorBlock::kFlagDualMapped;
  }
  else {
    bool allocate_regular_pages = true;
    if (Support::test(impl->options, JitAllocatorOptions::kUseLargePages)) {
      size_t large_page_size = VirtMem::large_page_size();
      bool try_large_page = block_size >= large_page_size || Support::test(impl->options, JitAllocatorOptions::kAlignBlockSizeToLargePage);
      if (large_page_size && try_large_page) {
        size_t large_block_size = Support::align_up(block_size, large_page_size);
        Error err = VirtMem::alloc(&virt_mem.rx, large_block_size, mem_flags | VirtMem::MemoryFlags::kMMapLargePages);
        if (err == Error::kOk) {
          allocate_regular_pages = false;
          block_size = large_block_size;
          block_flags |= JitAllocatorBlock::kFlagLargePages;
        }
      }
    }
    if (allocate_regular_pages) {
      ASMJIT_PROPAGATE(VirtMem::alloc(&virt_mem.rx, block_size, mem_flags));
    }
    virt_mem.rw = virt_mem.rx;
  }
  uint32_t area_size = uint32_t((block_size + pool->granularity - 1) >> pool->granularity_log2);
  uint32_t bit_word_count = (area_size + kBitWordSizeInBits - 1u) / kBitWordSizeInBits;
  uint8_t* block_ptr = static_cast<uint8_t*>(::malloc(sizeof(JitAllocatorBlock) + size_t(bit_word_count) * 2u * sizeof(BitWord)));
  if (ASMJIT_UNLIKELY(block_ptr == nullptr)) {
    if (Support::test(impl->options, JitAllocatorOptions::kUseDualMapping)) {
      (void)VirtMem::release_dual_mapping(virt_mem, block_size);
    }
    else {
      (void)VirtMem::release(virt_mem.rx, block_size);
    }
    return make_error(Error::kOutOfMemory);
  }
  if (Support::test(impl->options, JitAllocatorOptions::kFillUnusedMemory)) {
    VirtMem::ProtectJitReadWriteScope scope(virt_mem.rw, block_size);
    JitAllocator_fill_pattern(virt_mem.rw, impl->fill_pattern, block_size);
  }
  BitWord* bit_words = reinterpret_cast<BitWord*>(block_ptr + sizeof(JitAllocatorBlock));
  *dst = new(Support::PlacementNew{block_ptr}) JitAllocatorBlock(pool, virt_mem, block_size, block_flags, bit_words, bit_words + bit_word_count, area_size);
  return Error::kOk;
}
static void JitAllocatorImpl_deleteBlock(JitAllocatorPrivateImpl* impl, JitAllocatorBlock* block) noexcept {
  Support::maybe_unused(impl);
  if (block->has_flag(JitAllocatorBlock::kFlagDualMapped)) {
    (void)VirtMem::release_dual_mapping(block->_mapping, block->block_size());
  }
  else {
    (void)VirtMem::release(block->rx_ptr(), block->block_size());
  }
  ::free(block);
}
static void JitAllocatorImpl_insertBlock(JitAllocatorPrivateImpl* impl, JitAllocatorBlock* block) noexcept {
  JitAllocatorPool* pool = block->pool();
  if (!pool->cursor) {
    pool->cursor = block;
  }
  impl->tree.insert(block);
  pool->blocks.append(block);
  size_t stat_index = size_t(block->has_large_pages());
  pool->block_count++;
  pool->total_area_size[stat_index] += block->area_size();
  pool->total_area_used[stat_index] += block->area_used();
  pool->total_overhead_bytes += sizeof(JitAllocatorBlock) + JitAllocator_bit_vector_size_to_byte_size(block->area_size()) * 2u;
}
static void JitAllocatorImpl_removeBlock(JitAllocatorPrivateImpl* impl, JitAllocatorBlock* block) noexcept {
  JitAllocatorPool* pool = block->pool();
  if (pool->cursor == block) {
    pool->cursor = block->has_prev() ? block->prev() : block->next();
  }
  impl->tree.remove(block);
  pool->blocks.unlink(block);
  size_t stat_index = size_t(block->has_large_pages());
  pool->block_count--;
  pool->total_area_size[stat_index] -= block->area_size();
  pool->total_area_used[stat_index] -= block->area_used();
  pool->total_overhead_bytes -= sizeof(JitAllocatorBlock) + JitAllocator_bit_vector_size_to_byte_size(block->area_size()) * 2u;
}
static void JitAllocatorImpl_wipeOutBlock(JitAllocatorPrivateImpl* impl, JitAllocatorBlock* block) noexcept {
  if (block->has_flag(JitAllocatorBlock::kFlagEmpty)) {
    return;
  }
  JitAllocatorPool* pool = block->pool();
  if (Support::test(impl->options, JitAllocatorOptions::kFillUnusedMemory)) {
    VirtMem::protect_jit_memory(VirtMem::ProtectJitAccess::kReadWrite);
    uint32_t granularity = pool->granularity;
    uint8_t* rw_ptr = block->rw_ptr();
    BitVectorRangeIterator<Support::BitWord, 0> it(block->_used_bit_vector, pool->bit_word_count_from_area_size(block->area_size()));
    size_t range_start;
    size_t range_end;
    while (it.next_range(Out(range_start), Out(range_end))) {
      uint8_t* span_ptr = rw_ptr + range_start * granularity;
      size_t span_size = (range_end - range_start) * granularity;
      JitAllocator_fill_pattern(span_ptr, impl->fill_pattern, span_size);
      VirtMem::flush_instruction_cache(span_ptr, span_size);
    }
    VirtMem::protect_jit_memory(VirtMem::ProtectJitAccess::kReadExecute);
  }
  block->clear_block();
}
JitAllocator::JitAllocator(const CreateParams* params) noexcept {
  _impl = JitAllocator_new_impl(params);
  if (ASMJIT_UNLIKELY(!_impl)) {
    _impl = const_cast<JitAllocator::Impl*>(&JitAllocatorImpl_none);
  }
}
JitAllocator::~JitAllocator() noexcept {
  if (_impl == &JitAllocatorImpl_none) {
    return;
  }
  reset(ResetPolicy::kHard);
  JitAllocator_destroy_impl(static_cast<JitAllocatorPrivateImpl*>(_impl));
}
void JitAllocator::reset(ResetPolicy reset_policy) noexcept {
  if (_impl == &JitAllocatorImpl_none) {
    return;
  }
  JitAllocatorPrivateImpl* impl = static_cast<JitAllocatorPrivateImpl*>(_impl);
  impl->tree.reset();
  size_t pool_count = impl->pool_count;
  for (size_t pool_id = 0; pool_id < pool_count; pool_id++) {
    JitAllocatorPool& pool = impl->pools[pool_id];
    JitAllocatorBlock* block = pool.blocks.first();
    pool.reset();
    if (block) {
      JitAllocatorBlock* block_to_keep = nullptr;
      if (reset_policy != ResetPolicy::kHard && uint32_t(impl->options & JitAllocatorOptions::kImmediateRelease) == 0) {
        block_to_keep = block;
        block = block->next();
      }
      while (block) {
        JitAllocatorBlock* next = block->next();
        JitAllocatorImpl_deleteBlock(impl, block);
        block = next;
      }
      if (block_to_keep) {
        block_to_keep->_list_nodes[0] = nullptr;
        block_to_keep->_list_nodes[1] = nullptr;
        JitAllocatorImpl_wipeOutBlock(impl, block_to_keep);
        JitAllocatorImpl_insertBlock(impl, block_to_keep);
        pool.empty_block_count = 1;
      }
    }
  }
}
JitAllocator::Statistics JitAllocator::statistics() const noexcept {
  Statistics statistics;
  statistics.reset();
  if (ASMJIT_LIKELY(_impl != &JitAllocatorImpl_none)) {
    JitAllocatorPrivateImpl* impl = static_cast<JitAllocatorPrivateImpl*>(_impl);
    LockGuard guard(impl->lock);
    size_t pool_count = impl->pool_count;
    for (size_t pool_id = 0; pool_id < pool_count; pool_id++) {
      const JitAllocatorPool& pool = impl->pools[pool_id];
      statistics._block_count   += size_t(pool.block_count);
      statistics._reserved_size += size_t(pool.total_area_size[0] + pool.total_area_size[1]) * pool.granularity;
      statistics._used_size     += size_t(pool.total_area_used[0] + pool.total_area_used[1]) * pool.granularity;
      statistics._overhead_size += size_t(pool.total_overhead_bytes);
    }
    statistics._allocation_count = impl->allocation_count;
  }
  return statistics;
}
Error JitAllocator::alloc(Out<Span> out, size_t size) noexcept {
  constexpr uint32_t no_index = std::numeric_limits<uint32_t>::max();
  constexpr size_t max_request_size = std::numeric_limits<uint32_t>::max() / 2u;
  JitAllocatorPrivateImpl* impl = static_cast<JitAllocatorPrivateImpl*>(_impl);
  bool not_initialized = _impl == &JitAllocatorImpl_none;
  size = Support::align_up<size_t>(size, impl->granularity);
  out = Span{};
  if (ASMJIT_UNLIKELY(Support::bool_or(not_initialized, size - 1u >= max_request_size))) {
    return make_error(not_initialized ? Error::kNotInitialized  :
                      size == 0u      ? Error::kInvalidArgument : Error::kTooLarge);
  }
  LockGuard guard(impl->lock);
  JitAllocatorPool* pool = &impl->pools[JitAllocator_size_to_pool_id(impl, size)];
  uint32_t area_index = no_index;
  uint32_t area_size = uint32_t(pool->area_size_from_byte_size(size));
  JitAllocatorBlock* block = pool->cursor;
  if (block) {
    JitAllocatorBlock* initial = block;
    do {
      uint32_t largest_unused_area = block->largest_unused_area();
      if (Support::bool_and(block->is_incremental(), largest_unused_area >= area_size)) {
        area_index = block->_search_start;
        block->_largest_unused_area -= area_size;
        break;
      }
      else if (block->area_available() >= area_size) {
        if (Support::bool_or(block->is_dirty(), largest_unused_area >= area_size)) {
          BitVectorRangeIterator<Support::BitWord, 0> it(block->_used_bit_vector, pool->bit_word_count_from_area_size(block->area_size()), block->_search_start, block->_search_end);
          size_t range_start = 0;
          size_t range_end = block->area_size();
          size_t search_start = SIZE_MAX;
          size_t largest_area = 0;
          while (it.next_range(Out(range_start), Out(range_end), area_size)) {
            size_t range_size = range_end - range_start;
            if (range_size >= area_size) {
              area_index = uint32_t(range_start);
              break;
            }
            search_start = Support::min(search_start, range_start);
            largest_area = Support::max(largest_area, range_size);
          }
          if (area_index != no_index) {
            break;
          }
          if (search_start != SIZE_MAX) {
            size_t search_end = range_end;
            block->_search_start = uint32_t(search_start);
            block->_search_end = uint32_t(search_end);
            block->_largest_unused_area = uint32_t(largest_area);
            block->clear_flags(JitAllocatorBlock::kFlagDirty);
          }
        }
      }
      block = block->has_next() ? block->next() : pool->blocks.first();
    } while (block != initial);
  }
  if (area_index == no_index) {
    size_t block_size = JitAllocator_calculate_ideal_block_size(impl, pool, size);
    if (ASMJIT_UNLIKELY(!block_size)) {
      return make_error(Error::kOutOfMemory);
    }
    ASMJIT_PROPAGATE(JitAllocator_new_block(impl, &block, pool, block_size));
    area_index = block->initial_area_start();
    JitAllocatorImpl_insertBlock(impl, block);
    block->_search_start += area_size;
    block->_largest_unused_area -= area_size;
  }
  else if (block->has_flag(JitAllocatorBlock::kFlagEmpty)) {
    pool->empty_block_count--;
    block->clear_flags(JitAllocatorBlock::kFlagEmpty);
  }
  impl->allocation_count++;
  block->mark_allocated_area(area_index, area_index + area_size);
  size_t offset = pool->byte_size_from_area_size(area_index);
  ASMJIT_ASSERT(offset <= block->block_size() - size);
  out->_rx = block->rx_ptr() + offset;
  out->_rw = block->rw_ptr() + offset;
  out->_size = size;
  out->_block = static_cast<void*>(block);
  return Error::kOk;
}
Error JitAllocator::release(void* rx) noexcept {
  bool not_initialized = _impl == &JitAllocatorImpl_none;
  if (ASMJIT_UNLIKELY(Support::bool_or(not_initialized, !rx))) {
    return make_error(not_initialized ? Error::kNotInitialized : Error::kInvalidArgument);
  }
  JitAllocatorPrivateImpl* impl = static_cast<JitAllocatorPrivateImpl*>(_impl);
  LockGuard guard(impl->lock);
  JitAllocatorBlock* block = impl->tree.get(static_cast<uint8_t*>(rx));
  if (ASMJIT_UNLIKELY(!block)) {
    return make_error(Error::kInvalidState);
  }
  JitAllocatorPool* pool = block->pool();
  size_t offset = (size_t)((uint8_t*)rx - block->rx_ptr());
  uint32_t area_index = uint32_t(offset >> pool->granularity_log2);
  uint32_t area_end = uint32_t(Support::bit_vector_index_of(block->_stop_bit_vector, area_index, true)) + 1;
  uint32_t area_size = area_end - area_index;
  impl->allocation_count--;
  block->mark_released_area(area_index, area_end);
  if (Support::test(impl->options, JitAllocatorOptions::kFillUnusedMemory)) {
    uint8_t* span_ptr = block->rw_ptr() + area_index * pool->granularity;
    size_t span_size = area_size * pool->granularity;
    VirtMem::ProtectJitReadWriteScope scope(span_ptr, span_size);
    JitAllocator_fill_pattern(span_ptr, impl->fill_pattern, span_size);
  }
  if (block->is_empty()) {
    if (pool->empty_block_count || Support::test(impl->options, JitAllocatorOptions::kImmediateRelease)) {
      JitAllocatorImpl_removeBlock(impl, block);
      JitAllocatorImpl_deleteBlock(impl, block);
    }
    else {
      pool->empty_block_count++;
    }
  }
  return Error::kOk;
}
static Error JitAllocatorImpl_shrink(JitAllocatorPrivateImpl* impl, JitAllocator::Span& span, size_t new_size, bool already_under_write_scope) noexcept {
  JitAllocatorBlock* block = static_cast<JitAllocatorBlock*>(span._block);
  if (ASMJIT_UNLIKELY(!block)) {
    return make_error(Error::kInvalidArgument);
  }
  LockGuard guard(impl->lock);
  JitAllocatorPool* pool = block->pool();
  size_t offset = (size_t)((uint8_t*)span.rx() - block->rx_ptr());
  uint32_t area_start = uint32_t(offset >> pool->granularity_log2);
  bool is_used = Support::bit_vector_get_bit(block->_used_bit_vector, area_start);
  if (ASMJIT_UNLIKELY(!is_used)) {
    return make_error(Error::kInvalidArgument);
  }
  uint32_t area_end = uint32_t(Support::bit_vector_index_of(block->_stop_bit_vector, area_start, true)) + 1;
  uint32_t area_prev_size = area_end - area_start;
  uint32_t span_prev_size = area_prev_size * pool->granularity;
  uint32_t area_shrunk_size = pool->area_size_from_byte_size(new_size);
  if (ASMJIT_UNLIKELY(area_shrunk_size > area_prev_size)) {
    return make_error(Error::kInvalidArgument);
  }
  uint32_t area_diff = area_prev_size - area_shrunk_size;
  if (area_diff) {
    block->mark_shrunk_area(area_start + area_shrunk_size, area_end);
    span._size = pool->byte_size_from_area_size(area_shrunk_size);
  }
  if (new_size < span_prev_size && Support::test(impl->options, JitAllocatorOptions::kFillUnusedMemory)) {
    uint8_t* span_ptr = block->rw_ptr() + (area_start + area_shrunk_size) * pool->granularity;
    size_t span_size = area_diff * pool->granularity;
    if (!already_under_write_scope) {
      VirtMem::ProtectJitReadWriteScope scope(span_ptr, span_size, VirtMem::CachePolicy::kNeverFlush);
      JitAllocator_fill_pattern(span_ptr, impl->fill_pattern, span_size);
    }
    else {
      JitAllocator_fill_pattern(span_ptr, impl->fill_pattern, span_size);
    }
  }
  return Error::kOk;
}
Error JitAllocator::shrink(Span& span, size_t new_size) noexcept {
  bool not_initialized = _impl == &JitAllocatorImpl_none;
  if (ASMJIT_UNLIKELY(Support::bool_or(not_initialized, !span.rx()))) {
    return make_error(not_initialized ? Error::kNotInitialized : Error::kInvalidArgument);
  }
  if (ASMJIT_UNLIKELY(new_size == 0)) {
    Error err = release(span.rx());
    span = Span{};
    return err;
  }
  return JitAllocatorImpl_shrink(static_cast<JitAllocatorPrivateImpl*>(_impl), span, new_size, false);
}
Error JitAllocator::query(Out<Span> out, void* rx) const noexcept {
  *out = Span{};
  if (ASMJIT_UNLIKELY(_impl == &JitAllocatorImpl_none)) {
    return make_error(Error::kNotInitialized);
  }
  JitAllocatorPrivateImpl* impl = static_cast<JitAllocatorPrivateImpl*>(_impl);
  LockGuard guard(impl->lock);
  JitAllocatorBlock* block = impl->tree.get(static_cast<uint8_t*>(rx));
  if (ASMJIT_UNLIKELY(!block)) {
    return make_error(Error::kInvalidArgument);
  }
  JitAllocatorPool* pool = block->pool();
  size_t offset = (size_t)((uint8_t*)rx - block->rx_ptr());
  uint32_t area_start = uint32_t(offset >> pool->granularity_log2);
  bool is_used = Support::bit_vector_get_bit(block->_used_bit_vector, area_start);
  if (ASMJIT_UNLIKELY(!is_used)) {
    return make_error(Error::kInvalidArgument);
  }
  uint32_t area_end = uint32_t(Support::bit_vector_index_of(block->_stop_bit_vector, area_start, true)) + 1;
  size_t byte_offset = pool->byte_size_from_area_size(area_start);
  size_t byte_size = pool->byte_size_from_area_size(area_end - area_start);
  out->_rx = static_cast<uint8_t*>(block->_mapping.rx) + byte_offset;
  out->_rw = static_cast<uint8_t*>(block->_mapping.rw) + byte_offset;
  out->_size = byte_size;
  out->_block = static_cast<void*>(block);
  return Error::kOk;
}
static ASMJIT_INLINE VirtMem::CachePolicy JitAllocator_defaultPolicyForSpan(const JitAllocator::Span& span) noexcept {
  if (Support::test(span.flags(), JitAllocator::Span::Flags::kInstructionCacheClean)) {
    return VirtMem::CachePolicy::kNeverFlush;
  }
  else {
    return VirtMem::CachePolicy::kFlushAfterWrite;
  }
}
Error JitAllocator::write(Span& span, size_t offset, const void* src, size_t size, VirtMem::CachePolicy policy) noexcept {
  if (ASMJIT_UNLIKELY(span._block == nullptr || offset > span.size() || span.size() - offset < size)) {
    return make_error(Error::kInvalidArgument);
  }
  if (ASMJIT_UNLIKELY(size == 0)) {
    return Error::kOk;
  }
  if (policy == VirtMem::CachePolicy::kDefault) {
    policy = JitAllocator_defaultPolicyForSpan(span);
  }
  VirtMem::ProtectJitReadWriteScope write_scope(span.rx(), span.size(), policy);
  memcpy(static_cast<uint8_t*>(span.rw()) + offset, src, size);
  return Error::kOk;
}
Error JitAllocator::write(Span& span, WriteFunc write_fn, void* user_data, VirtMem::CachePolicy policy) noexcept {
  if (ASMJIT_UNLIKELY(span._block == nullptr) || span.size() == 0) {
    return make_error(Error::kInvalidArgument);
  }
  size_t size = span.size();
  if (ASMJIT_UNLIKELY(size == 0)) {
    return Error::kOk;
  }
  if (policy == VirtMem::CachePolicy::kDefault) {
    policy = JitAllocator_defaultPolicyForSpan(span);
  }
  VirtMem::ProtectJitReadWriteScope write_scope(span.rx(), span.size(), policy);
  ASMJIT_PROPAGATE(write_fn(span, user_data));
  if (span.size() != size) {
    std::swap(span._size, size);
    return JitAllocatorImpl_shrink(static_cast<JitAllocatorPrivateImpl*>(_impl), span, size, true);
  }
  return Error::kOk;
}
Error JitAllocator::begin_write_scope(WriteScopeData& scope, VirtMem::CachePolicy policy) noexcept {
  scope.policy = policy;
  scope.flags = 0u;
  scope.data[0] = 0u;
  scope.data[1] = 0u;
  return Error::kOk;
}
Error JitAllocator::end_write_scope(WriteScopeData& scope) noexcept {
  Support::maybe_unused(scope);
  return Error::kOk;
}
Error JitAllocator::flush_write_scope(WriteScopeData& scope) noexcept {
  Support::maybe_unused(scope);
  return Error::kOk;
}
Error JitAllocator::scoped_write(WriteScopeData& scope, Span& span, size_t offset, const void* src, size_t size) noexcept {
  return write(span, offset, src, size, scope.policy);
}
Error JitAllocator::scoped_write(WriteScopeData& scope, Span& span, WriteFunc write_fn, void* user_data) noexcept {
  return write(span, write_fn, user_data, scope.policy);
}
#if defined(ASMJIT_TEST)
namespace JitAllocatorUtils {
  static void fill_pattern_64(void* p_, uint64_t pattern, size_t size_in_bytes) noexcept {
    uint64_t* p = static_cast<uint64_t*>(p_);
    size_t n = size_in_bytes / 8u;
    for (size_t i = 0; i < n; i++) {
      p[i] = pattern;
    }
  }
  static bool verify_pattern_64(const void* p_, uint64_t pattern, size_t size_in_bytes) noexcept {
    const uint64_t* p = static_cast<const uint64_t*>(p_);
    size_t n = size_in_bytes / 8u;
    for (size_t i = 0; i < n; i++) {
      if (p[i] != pattern) {
        INFO("Pattern verification failed at 0x%p [%zu * 8]: value(0x%016llX) != expected(0x%016llX)",
          p,
          i,
          (unsigned long long)p[i],
          (unsigned long long)pattern);
        return false;
      }
    }
    return true;
  }
}
class JitAllocatorWrapper {
public:
  class Range {
  public:
    inline Range(uint8_t* addr, size_t size) noexcept
      : addr(addr),
        size(size) {}
    uint8_t* addr;
    size_t size;
  };
  class Record : public ArenaTreeNodeT<Record>,
                 public Range {
  public:
    void* _rw;
    uint64_t pattern;
    inline Record(void* rx, void* rw, size_t size, uint64_t pattern)
      : ArenaTreeNodeT<Record>(),
        Range(static_cast<uint8_t*>(rx), size),
        _rw(rw),
        pattern(pattern) {}
    inline void* rx() const noexcept { return addr; }
    inline void* rw() const noexcept { return _rw; }
    inline bool operator<(const Record& other) const noexcept { return addr < other.addr; }
    inline bool operator>(const Record& other) const noexcept { return addr > other.addr; }
    inline bool operator<(const uint8_t* key) const noexcept { return addr + size <= key; }
    inline bool operator>(const uint8_t* key) const noexcept { return addr > key; }
  };
  Arena _arena;
  ArenaPool<Record> _record_pool;
  ArenaTree<Record> _records;
  JitAllocator _allocator;
  TestUtils::Random _rng;
  explicit JitAllocatorWrapper(const JitAllocator::CreateParams* params) noexcept
    : _arena(1024u * 1024u),
      _allocator(params),
      _rng(0x123456789u) {}
  void _insert(void* rx_ptr, void* rw_ptr, size_t size) noexcept {
    uint8_t* ptr = static_cast<uint8_t*>(rx_ptr);
    uint8_t* end_ptr = ptr + size - 1u;
    Record* record;
    record = _records.get(ptr);
    EXPECT_NULL(record)
      .message("Address [%p:%p] collides with a newly allocated [%p:%p]\n", record->addr, record->addr + record->size, ptr, ptr + size);
    record = _records.get(end_ptr);
    EXPECT_NULL(record)
      .message("Address [%p:%p] collides with a newly allocated [%p:%p]\n", record->addr, record->addr + record->size, ptr, ptr + size);
    uint64_t pattern = _rng.next_uint64();
    record = new(Support::PlacementNew{_record_pool.alloc(_arena)}) Record(rx_ptr, rw_ptr, size, pattern);
    EXPECT_NOT_NULL(record);
    {
      VirtMem::ProtectJitReadWriteScope scope(rw_ptr, size);
      JitAllocatorUtils::fill_pattern_64(rw_ptr, pattern, size);
    }
    VirtMem::flush_instruction_cache(rx_ptr, size);
    EXPECT_TRUE(JitAllocatorUtils::verify_pattern_64(rx_ptr, pattern, size));
    _records.insert(record);
  }
  void _remove(void* p) noexcept {
    Record* record = _records.get(static_cast<uint8_t*>(p));
    EXPECT_NOT_NULL(record);
    EXPECT_TRUE(JitAllocatorUtils::verify_pattern_64(record->rx(), record->pattern, record->size));
    EXPECT_TRUE(JitAllocatorUtils::verify_pattern_64(record->rw(), record->pattern, record->size));
    _records.remove(record);
    _record_pool.release(record);
  }
  void* alloc(size_t size) noexcept {
    JitAllocator::Span span;
    Error err = _allocator.alloc(Out(span), size);
    EXPECT_EQ(err, Error::kOk)
      .message("JitAllocator failed to allocate %zu bytes\n", size);
    _insert(span.rx(), span.rw(), size);
    return span.rx();
  }
  void release(void* p) noexcept {
    _remove(p);
    EXPECT_EQ(_allocator.release(p), Error::kOk)
      .message("JitAllocator failed to release '%p'\n", p);
  }
  void shrink(void* p, size_t new_size) noexcept {
    Record* record = _records.get(static_cast<uint8_t*>(p));
    EXPECT_NOT_NULL(record);
    if (!new_size) {
      return release(p);
    }
    JitAllocator::Span span;
    EXPECT_EQ(_allocator.query(Out(span), p), Error::kOk);
    Error err = _allocator.shrink(span, new_size);
    EXPECT_EQ(err, Error::kOk)
      .message("JitAllocator failed to shrink %p to %zu bytes\n", p, new_size);
    record->size = new_size;
  }
};
static void JitAllocatorTest_shuffle(void** ptr_array, size_t count, TestUtils::Random& prng) noexcept {
  for (size_t i = 0; i < count; ++i)
    std::swap(ptr_array[i], ptr_array[size_t(prng.next_uint32() % count)]);
}
static void JitAllocatorTest_usage(JitAllocator& allocator) noexcept {
  JitAllocator::Statistics stats = allocator.statistics();
  INFO("    Block Count       : %9llu [Blocks]"        , (unsigned long long)(stats.block_count()));
  INFO("    Reserved (VirtMem): %9llu [Bytes]"         , (unsigned long long)(stats.reserved_size()));
  INFO("    Used     (VirtMem): %9llu [Bytes] (%.1f%%)", (unsigned long long)(stats.used_size()), stats.used_ratio() * 100.0);
  INFO("    Overhead (HeapMem): %9llu [Bytes] (%.1f%%)", (unsigned long long)(stats.overhead_size()), stats.overhead_ratio() * 100.0);
}
template<typename T, size_t kPatternSize, bool Bit>
static void BitVectorRangeIterator_testRandom(TestUtils::Random& rnd, size_t count) noexcept {
  for (size_t i = 0; i < count; i++) {
    T in[kPatternSize];
    T out[kPatternSize];
    for (size_t j = 0; j < kPatternSize; j++) {
      in[j] = T(uint64_t(rnd.next_uint32() & 0xFFu) * 0x0101010101010101);
      out[j] = Bit == 0 ? Support::bit_ones<T> : T(0);
    }
    {
      BitVectorRangeIterator<T, Bit> it(in, kPatternSize);
      size_t range_start, range_end;
      while (it.next_range(Out(range_start), Out(range_end))) {
        if (Bit) {
          Support::bit_vector_fill(out, range_start, range_end - range_start);
        }
        else {
          Support::bit_vector_clear(out, range_start, range_end - range_start);
        }
      }
    }
    for (size_t j = 0; j < kPatternSize; j++) {
      EXPECT_EQ(in[j], out[j])
        .message("Invalid pattern detected at [%zu] (%llX != %llX)", j, (unsigned long long)in[j], (unsigned long long)out[j]);
    }
  }
}
static void test_jit_allocator_reset_empty() noexcept {
  JitAllocator allocator;
  allocator.reset(ResetPolicy::kSoft);
}
static void test_jit_allocator_alloc_release() noexcept {
  size_t kCount = BrokenAPI::has_arg("--quick") ? 20000 : 100000;
  struct TestInfo {
    const char* name;
    JitAllocatorOptions options;
    uint32_t block_size;
    uint32_t granularity;
  };
  using Opt = JitAllocatorOptions;
  VirtMem::HardenedRuntimeInfo hri = VirtMem::hardened_runtime_info();
  TestInfo test_info_table[] = {
    { "Default"                                    , Opt::kNone, 0, 0 },
    { "16MB blocks"                                , Opt::kNone, 16 * 1024 * 1024, 0 },
    { "256B granularity"                           , Opt::kNone, 0, 256 },
    { "kUseMultiplePools"                          , Opt::kUseMultiplePools, 0, 0 },
    { "kFillUnusedMemory"                          , Opt::kFillUnusedMemory, 0, 0 },
    { "kImmediateRelease"                          , Opt::kImmediateRelease, 0, 0 },
    { "kDisableInitialPadding"                     , Opt::kDisableInitialPadding, 0, 0 },
    { "kUseLargePages"                             , Opt::kUseLargePages, 0, 0 },
    { "kUseLargePages | kFillUnusedMemory"         , Opt::kUseLargePages | Opt::kFillUnusedMemory, 0, 0 },
    { "kUseLargePages | kAlignBlockSizeToLargePage", Opt::kUseLargePages | Opt::kAlignBlockSizeToLargePage, 0, 0 },
    { "kUseDualMapping"                            , Opt::kUseDualMapping , 0, 0 },
    { "kUseDualMapping | kFillUnusedMemory"        , Opt::kUseDualMapping | Opt::kFillUnusedMemory, 0, 0 }
  };
  INFO("BitVectorRangeIterator<uint32_t>");
  {
    TestUtils::Random rnd;
    BitVectorRangeIterator_testRandom<uint32_t, 64, 0>(rnd, kCount);
  }
  INFO("BitVectorRangeIterator<uint64_t>");
  {
    TestUtils::Random rnd;
    BitVectorRangeIterator_testRandom<uint64_t, 64, 0>(rnd, kCount);
  }
  for (const TestInfo& test_info : test_info_table) {
    if (Support::test(test_info.options, JitAllocatorOptions::kUseDualMapping) &&
        !Support::test(hri.flags, VirtMem::HardenedRuntimeFlags::kDualMapping)) {
      continue;
    }
    INFO("JitAllocator(%s)", test_info.name);
    JitAllocator::CreateParams params {};
    params.options = test_info.options;
    params.block_size = test_info.block_size;
    params.granularity = test_info.granularity;
    size_t fixed_block_size = 256;
    JitAllocatorWrapper wrapper(&params);
    TestUtils::Random prng(100);
    size_t i;
    INFO("  Memory alloc/release test - %d allocations", kCount);
    void** ptr_array = (void**)::malloc(sizeof(void*) * size_t(kCount));
    EXPECT_NOT_NULL(ptr_array);
    INFO("  Allocating random blocks...");
    for (i = 0; i < kCount; i++) {
      ptr_array[i] = wrapper.alloc((prng.next_uint32() % 1024) + 8);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Releasing all allocated blocks from the beginning...");
    for (i = 0; i < kCount; i++) {
      wrapper.release(ptr_array[i]);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Allocating random blocks again...", kCount);
    for (i = 0; i < kCount; i++) {
      ptr_array[i] = wrapper.alloc((prng.next_uint32() % 1024) + 8);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Shuffling allocated blocks...");
    JitAllocatorTest_shuffle(ptr_array, unsigned(kCount), prng);
    INFO("  Releasing 50%% of allocated blocks...");
    for (i = 0; i < kCount / 2; i++) {
      wrapper.release(ptr_array[i]);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Allocating 50%% more blocks again...");
    for (i = 0; i < kCount / 2; i++) {
      ptr_array[i] = wrapper.alloc((prng.next_uint32() % 1024) + 8);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Releasing all allocated blocks from the end...");
    for (i = 0; i < kCount; i++) {
      wrapper.release(ptr_array[kCount - i - 1]);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Allocating %zuB blocks...", fixed_block_size);
    for (i = 0; i < kCount / 2; i++) {
      ptr_array[i] = wrapper.alloc(fixed_block_size);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Shrinking each %zuB block to 1 byte", fixed_block_size);
    for (i = 0; i < kCount / 2; i++) {
      wrapper.shrink(ptr_array[i], 1);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Allocating more 64B blocks...", 64);
    for (i = kCount / 2; i < kCount; i++) {
      ptr_array[i] = wrapper.alloc(64);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Releasing all blocks from the beginning...");
    for (i = 0; i < kCount; i++) {
      wrapper.release(ptr_array[i]);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Allocating %zuB blocks...", fixed_block_size);
    for (i = 0; i < kCount; i++) {
      ptr_array[i] = wrapper.alloc(fixed_block_size);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Shuffling allocated blocks...");
    JitAllocatorTest_shuffle(ptr_array, unsigned(kCount), prng);
    INFO("  Releasing 50%% of allocated blocks...");
    for (i = 0; i < kCount / 2; i++) {
      wrapper.release(ptr_array[i]);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Allocating 50%% more %zuB blocks again...", fixed_block_size);
    for (i = 0; i < kCount / 2; i++) {
      ptr_array[i] = wrapper.alloc(fixed_block_size);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    INFO("  Releasing all allocated blocks from the end...");
    for (i = 0; i < kCount; i++) {
      wrapper.release(ptr_array[kCount - i - 1]);
    }
    JitAllocatorTest_usage(wrapper._allocator);
    ::free(ptr_array);
  }
}
static void test_jit_allocator_query() noexcept {
  JitAllocator allocator;
  size_t allocated_size = 100;
  JitAllocator::Span allocated_span;
  EXPECT_EQ(allocator.alloc(Out(allocated_span), allocated_size), Error::kOk);
  EXPECT_NOT_NULL(allocated_span.rx());
  EXPECT_GE(allocated_span.size(), allocated_size);
  JitAllocator::Span queried_span;
  EXPECT_EQ(allocator.query(Out(queried_span), allocated_span.rx()), Error::kOk);
  EXPECT_EQ(allocated_span.rx(), queried_span.rx());
  EXPECT_EQ(allocated_span.rw(), queried_span.rw());
  EXPECT_EQ(allocated_span.size(), queried_span.size());
}
UNIT(jit_allocator) {
  test_jit_allocator_reset_empty();
  test_jit_allocator_alloc_release();
  test_jit_allocator_query();
}
#endif
ASMJIT_END_NAMESPACE
#endif
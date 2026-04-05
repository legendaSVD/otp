#ifndef ASMJIT_CORE_RASTACK_P_H_INCLUDED
#define ASMJIT_CORE_RASTACK_P_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER
#include <asmjit/core/radefs_p.h>
ASMJIT_BEGIN_NAMESPACE
struct RAStackSlot {
  enum Flags : uint16_t {
    kFlagRegHome = 0x0001u,
    kFlagStackArg = 0x0002u
  };
  enum ArgIndex : uint32_t {
    kNoArgIndex = 0xFF
  };
  uint8_t _base_reg_id;
  uint8_t _alignment;
  uint16_t _flags;
  uint32_t _size;
  uint32_t _use_count;
  uint32_t _weight;
  int32_t _offset;
  [[nodiscard]]
  inline uint32_t base_reg_id() const noexcept { return _base_reg_id; }
  inline void set_base_reg_id(uint32_t id) noexcept { _base_reg_id = uint8_t(id); }
  [[nodiscard]]
  inline uint32_t size() const noexcept { return _size; }
  [[nodiscard]]
  inline uint32_t alignment() const noexcept { return _alignment; }
  [[nodiscard]]
  inline uint32_t flags() const noexcept { return _flags; }
  [[nodiscard]]
  inline bool has_flag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  inline void add_flags(uint32_t flags) noexcept { _flags = uint16_t(_flags | flags); }
  [[nodiscard]]
  inline bool is_reg_home() const noexcept { return has_flag(kFlagRegHome); }
  [[nodiscard]]
  inline bool is_stack_arg() const noexcept { return has_flag(kFlagStackArg); }
  [[nodiscard]]
  inline uint32_t use_count() const noexcept { return _use_count; }
  inline void add_use_count(uint32_t n = 1) noexcept { _use_count += n; }
  [[nodiscard]]
  inline uint32_t weight() const noexcept { return _weight; }
  inline void set_weight(uint32_t weight) noexcept { _weight = weight; }
  [[nodiscard]]
  inline int32_t offset() const noexcept { return _offset; }
  inline void set_offset(int32_t offset) noexcept { _offset = offset; }
};
class RAStackAllocator {
public:
  ASMJIT_NONCOPYABLE(RAStackAllocator)
  enum Size : uint32_t {
    kSize1     = 0,
    kSize2     = 1,
    kSize4     = 2,
    kSize8     = 3,
    kSize16    = 4,
    kSize32    = 5,
    kSize64    = 6,
    kSizeCount = 7
  };
  Arena* _arena {};
  uint32_t _bytes_used {};
  uint32_t _stack_size {};
  uint32_t _alignment = 1;
  ArenaVector<RAStackSlot*> _slots;
  ASMJIT_INLINE_NODEBUG RAStackAllocator() noexcept {}
  ASMJIT_INLINE_NODEBUG void reset(Arena* arena) noexcept {
    _arena = arena;
    _bytes_used = 0;
    _stack_size = 0;
    _alignment = 1;
    _slots.reset();
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arena* arena() const noexcept { return _arena; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t bytes_used() const noexcept { return _bytes_used; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t stack_size() const noexcept { return _stack_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t alignment() const noexcept { return _alignment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<RAStackSlot*> slots() noexcept { return _slots.as_span(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t slot_count() const noexcept { return _slots.size(); }
  [[nodiscard]]
  RAStackSlot* new_slot(uint32_t base_reg_id, uint32_t size, uint32_t alignment, uint32_t flags = 0) noexcept;
  [[nodiscard]]
  Error calculate_stack_frame() noexcept;
  [[nodiscard]]
  Error adjust_slot_offsets(int32_t offset) noexcept;
};
ASMJIT_END_NAMESPACE
#endif
#endif
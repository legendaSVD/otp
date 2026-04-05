#ifndef ASMJIT_CORE_RAREG_P_H_INCLUDED
#define ASMJIT_CORE_RAREG_P_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER
#include <asmjit/core/radefs_p.h>
#include <asmjit/support/arenavector.h>
ASMJIT_BEGIN_NAMESPACE
enum class RAWorkRegFlags : uint32_t {
  kNone = 0,
  kAllocated = 0x00000001u,
  kMultiBlockUse = 0x00000008u,
  kStackUsed = 0x00000010u,
  kStackPreferred = 0x00000020u,
  kStackArgToStack = 0x00000040u,
  kLeadConsecutive = 0x00001000u,
  kProcessedConsecutive = 0x00002000u,
  kSingleBlockVisitedFlag = 0x40000000u,
  kSingleBlockLiveFlag = 0x80000000u
};
ASMJIT_DEFINE_ENUM_FLAGS(RAWorkRegFlags)
class RAWorkReg {
public:
  ASMJIT_NONCOPYABLE(RAWorkReg)
  static inline constexpr uint32_t kNoArgIndex = 0xFFu;
  RAWorkId _work_id = kBadWorkId;
  uint32_t _virt_id = 0;
  VirtReg* _virt_reg = nullptr;
  RATiedReg* _tied_reg = nullptr;
  RAStackSlot* _stack_slot = nullptr;
  OperandSignature _signature {};
  RAWorkRegFlags _flags = RAWorkRegFlags::kNone;
  RABlockId _single_basic_block_id = kBadBlockId;
  RegMask _use_id_mask = 0;
  RegMask _preferred_mask = 0xFFFFFFFFu;
  RegMask _consecutive_mask = 0xFFFFFFFFu;
  RegMask _clobber_survival_mask = 0;
  RegMask _allocated_mask = 0;
  uint64_t _reg_byte_mask = 0;
  uint8_t _arg_index = kNoArgIndex;
  uint8_t _arg_value_index = 0;
  uint8_t _home_reg_id = Reg::kIdBad;
  uint8_t _hint_reg_id = Reg::kIdBad;
  RALiveSpans _live_spans {};
  RALiveStats _live_stats {};
  ArenaVector<BaseNode*> _refs {};
  ArenaVector<BaseNode*> _writes {};
  ArenaBitSet _immediate_consecutives {};
  ASMJIT_INLINE_NODEBUG RAWorkReg(VirtReg* virt_reg, OperandSignature signature, RAWorkId work_id) noexcept
    : _work_id(work_id),
      _virt_id(virt_reg->id()),
      _virt_reg(virt_reg),
      _signature(signature),
      _hint_reg_id(uint8_t(virt_reg->home_id_hint())) {}
  [[nodiscard]]
  ASMJIT_INLINE RAWorkId work_id() const noexcept {
    ASMJIT_ASSERT(_work_id != kBadWorkId);
    return _work_id;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t virt_id() const noexcept { return _virt_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG TypeId type_id() const noexcept { return _virt_reg->type_id(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkRegFlags flags() const noexcept { return _flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(RAWorkRegFlags flag) const noexcept { return Support::test(_flags, flag); }
  ASMJIT_INLINE_NODEBUG void add_flags(RAWorkRegFlags flags) noexcept { _flags |= flags; }
  ASMJIT_INLINE_NODEBUG void xor_flags(RAWorkRegFlags flags) noexcept { _flags ^= flags; }
  ASMJIT_INLINE_NODEBUG void clear_flags(RAWorkRegFlags flags) noexcept { _flags &= ~flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_allocated() const noexcept { return has_flag(RAWorkRegFlags::kAllocated); }
  ASMJIT_INLINE_NODEBUG void mark_allocated() noexcept { add_flags(RAWorkRegFlags::kAllocated); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_within_single_basic_block() const noexcept { return !has_flag(RAWorkRegFlags::kMultiBlockUse); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlockId single_basic_block_id() const noexcept { return _single_basic_block_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_lead_consecutive() const noexcept { return has_flag(RAWorkRegFlags::kLeadConsecutive); }
  ASMJIT_INLINE_NODEBUG void mark_lead_consecutive() noexcept { add_flags(RAWorkRegFlags::kLeadConsecutive); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_processed_consecutive() const noexcept { return has_flag(RAWorkRegFlags::kProcessedConsecutive); }
  ASMJIT_INLINE_NODEBUG void mark_processed_consecutive() noexcept { add_flags(RAWorkRegFlags::kProcessedConsecutive); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_stack_used() const noexcept { return has_flag(RAWorkRegFlags::kStackUsed); }
  ASMJIT_INLINE_NODEBUG void mark_stack_used() noexcept { add_flags(RAWorkRegFlags::kStackUsed); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_stack_preferred() const noexcept { return has_flag(RAWorkRegFlags::kStackPreferred); }
  ASMJIT_INLINE_NODEBUG void mark_stack_preferred() noexcept { add_flags(RAWorkRegFlags::kStackPreferred); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OperandSignature signature() const noexcept { return _signature; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegType type() const noexcept { return _signature.reg_type(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegGroup group() const noexcept { return _signature.reg_group(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG VirtReg* virt_reg() const noexcept { return _virt_reg; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_tied_reg() const noexcept { return _tied_reg != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* tied_reg() const noexcept { return _tied_reg; }
  ASMJIT_INLINE_NODEBUG void set_tied_reg(RATiedReg* tied_reg) noexcept { _tied_reg = tied_reg; }
  ASMJIT_INLINE_NODEBUG void reset_tied_reg() noexcept { _tied_reg = nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_stack_slot() const noexcept { return _stack_slot != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAStackSlot* stack_slot() const noexcept { return _stack_slot; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RALiveSpans& live_spans() noexcept { return _live_spans; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RALiveSpans& live_spans() const noexcept { return _live_spans; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RALiveStats& live_stats() noexcept { return _live_stats; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RALiveStats& live_stats() const noexcept { return _live_stats; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_arg_index() const noexcept { return _arg_index != kNoArgIndex; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t arg_index() const noexcept { return _arg_index; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t arg_value_index() const noexcept { return _arg_value_index; }
  inline void set_arg_index(uint32_t arg_index, uint32_t value_index) noexcept {
    _arg_index = uint8_t(arg_index);
    _arg_value_index = uint8_t(value_index);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_home_reg_id() const noexcept { return _home_reg_id != Reg::kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t home_reg_id() const noexcept { return _home_reg_id; }
  ASMJIT_INLINE_NODEBUG void set_home_reg_id(uint32_t phys_id) noexcept { _home_reg_id = uint8_t(phys_id); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_hint_reg_id() const noexcept { return _hint_reg_id != Reg::kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t hint_reg_id() const noexcept { return _hint_reg_id; }
  ASMJIT_INLINE_NODEBUG void set_hint_reg_id(uint32_t phys_id) noexcept { _hint_reg_id = uint8_t(phys_id); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask use_id_mask() const noexcept { return _use_id_mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_use_id_mask() const noexcept { return _use_id_mask != 0u; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_multiple_use_ids() const noexcept { return Support::has_at_least_2_bits_set(_use_id_mask); }
  ASMJIT_INLINE_NODEBUG void add_use_id_mask(RegMask mask) noexcept { _use_id_mask |= mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask preferred_mask() const noexcept { return _preferred_mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_preferred_mask() const noexcept { return _preferred_mask != 0xFFFFFFFFu; }
  ASMJIT_INLINE_NODEBUG void restrict_preferred_mask(RegMask mask) noexcept { _preferred_mask &= mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask consecutive_mask() const noexcept { return _consecutive_mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_consecutive_mask() const noexcept { return _consecutive_mask != 0xFFFFFFFFu; }
  ASMJIT_INLINE_NODEBUG void restrict_consecutive_mask(RegMask mask) noexcept { _consecutive_mask &= mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask clobber_survival_mask() const noexcept { return _clobber_survival_mask; }
  ASMJIT_INLINE_NODEBUG void add_clobber_survival_mask(RegMask mask) noexcept { _clobber_survival_mask |= mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask allocated_mask() const noexcept { return _allocated_mask; }
  ASMJIT_INLINE_NODEBUG void add_allocated_mask(RegMask mask) noexcept { _allocated_mask |= mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t reg_byte_mask() const noexcept { return _reg_byte_mask; }
  ASMJIT_INLINE_NODEBUG void set_reg_byte_mask(uint64_t mask) noexcept { _reg_byte_mask = mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_immediate_consecutives() const noexcept { return !_immediate_consecutives.is_empty(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ArenaBitSet& immediate_consecutives() const noexcept { return _immediate_consecutives; }
  [[nodiscard]]
  inline Error add_immediate_consecutive(Arena& arena, RAWorkId work_id) noexcept {
    if (_immediate_consecutives.size() <= uint32_t(work_id)) {
      ASMJIT_PROPAGATE(_immediate_consecutives.resize(arena, uint32_t(work_id) + 1u));
    }
    _immediate_consecutives.set_bit(work_id, true);
    return Error::kOk;
  }
};
ASMJIT_END_NAMESPACE
#endif
#endif
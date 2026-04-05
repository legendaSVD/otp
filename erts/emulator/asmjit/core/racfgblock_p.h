#ifndef ASMJIT_CORE_RACFGBLOCK_P_H_INCLUDED
#define ASMJIT_CORE_RACFGBLOCK_P_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER
#include <asmjit/core/compilerdefs.h>
#include <asmjit/core/raassignment_p.h>
#include <asmjit/support/arenabitset_p.h>
#include <asmjit/support/arenavector.h>
#include <asmjit/support/support_p.h>
ASMJIT_BEGIN_NAMESPACE
enum class RABlockTimestamp : uint64_t {};
enum class RABlockFlags : uint32_t {
  kNone = 0,
  kIsConstructed = 0x00000001u,
  kIsReachable = 0x00000002u,
  kIsTargetable = 0x00000004u,
  kIsAllocated = 0x00000008u,
  kIsFuncExit = 0x00000010u,
  kIsEnqueued = 0x00000020u,
  kHasTerminator = 0x00000100u,
  kHasConsecutive = 0x00000200u,
  kHasJumpTable = 0x00000400u,
  kHasFixedRegs = 0x00000800u,
  kHasFuncCalls = 0x00001000u
};
ASMJIT_DEFINE_ENUM_FLAGS(RABlockFlags)
class RABlock {
public:
  ASMJIT_NONCOPYABLE(RABlock)
  using PhysToWorkMap = RAAssignment::PhysToWorkMap;
  using WorkToPhysMap = RAAssignment::WorkToPhysMap;
  static inline constexpr uint32_t kLiveIn = 0;
  static inline constexpr uint32_t kLiveOut = 1;
  static inline constexpr uint32_t kLiveKill = 2;
  static inline constexpr uint32_t kLiveCount = 3;
  RABlockId _block_id = kBadBlockId;
  RABlockFlags _flags {};
  mutable RABlockTimestamp _timestamp {};
  ArenaVector<RABlock*> _predecessors {};
  ArenaVector<RABlock*> _successors {};
  RABlock* _idom {};
  BaseNode* _first {};
  BaseNode* _last {};
  NodePosition _first_position {};
  NodePosition _end_position {};
  uint32_t _pov_index = Globals::kInvalidId;
  uint32_t _shared_assignment_id = Globals::kInvalidId;
  BitWord* _live_bits {};
  uint32_t _live_bits_size {};
  RARegsStats _regs_stats = RARegsStats();
  RALiveCount _max_live_count = RALiveCount();
  RegMask _entry_scratch_gp_regs {};
  RegMask _exit_scratch_gp_regs {};
  PhysToWorkMap* _entry_phys_to_work_map = nullptr;
  BaseRAPass* _ra = nullptr;
  ASMJIT_INLINE_NODEBUG RABlock(BaseRAPass* ra) noexcept
    : _ra(ra) {}
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arena& arena() const noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlockId block_id() const noexcept { return _block_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlockFlags flags() const noexcept { return _flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(RABlockFlags flag) const noexcept { return Support::test(_flags, flag); }
  ASMJIT_INLINE_NODEBUG void add_flags(RABlockFlags flags) noexcept { _flags |= flags; }
  ASMJIT_INLINE_NODEBUG void clear_flags(RABlockFlags flags) noexcept { _flags &= ~flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_assigned() const noexcept { return _block_id != kBadBlockId; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_constructed() const noexcept { return has_flag(RABlockFlags::kIsConstructed); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_reachable() const noexcept { return has_flag(RABlockFlags::kIsReachable); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_targetable() const noexcept { return has_flag(RABlockFlags::kIsTargetable); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_allocated() const noexcept { return has_flag(RABlockFlags::kIsAllocated); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_func_exit() const noexcept { return has_flag(RABlockFlags::kIsFuncExit); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_enqueued() const noexcept { return has_flag(RABlockFlags::kIsEnqueued); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_terminator() const noexcept { return has_flag(RABlockFlags::kHasTerminator); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_consecutive() const noexcept { return has_flag(RABlockFlags::kHasConsecutive); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_jump_table() const noexcept { return has_flag(RABlockFlags::kHasJumpTable); }
  ASMJIT_INLINE_NODEBUG void make_constructed(const RARegsStats& reg_stats) noexcept {
    _flags |= RABlockFlags::kIsConstructed;
    _regs_stats.combine_with(reg_stats);
  }
  ASMJIT_INLINE_NODEBUG void make_reachable() noexcept { _flags |= RABlockFlags::kIsReachable; }
  ASMJIT_INLINE_NODEBUG void make_targetable() noexcept { _flags |= RABlockFlags::kIsTargetable; }
  ASMJIT_INLINE_NODEBUG void make_allocated() noexcept { _flags |= RABlockFlags::kIsAllocated; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RARegsStats& regs_stats() const noexcept { return _regs_stats; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<RABlock*> predecessors() const noexcept { return _predecessors.as_span(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<RABlock*> successors() const noexcept { return _successors.as_span(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_predecessors() const noexcept { return !_predecessors.is_empty(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_successors() const noexcept { return !_successors.is_empty(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_successor(const RABlock* block) const noexcept {
    Span<RABlock*> span = successors();
    Span<RABlock*> pred = block->predecessors();
    if (pred.size() < span.size()) {
      span = pred;
      block = this;
    }
    return span.contains(block);
  }
  [[nodiscard]]
  Error append_successor(RABlock* successor) noexcept;
  [[nodiscard]]
  Error prepend_successor(RABlock* successor) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlock* consecutive() const noexcept { return has_consecutive() ? _successors[0] : nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlock* idom() noexcept { return _idom; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RABlock* idom() const noexcept { return _idom; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_pov_index() const noexcept { return _pov_index != Globals::kInvalidId; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t pov_index() const noexcept { return _pov_index; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* first() const noexcept { return _first; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* last() const noexcept { return _last; }
  ASMJIT_INLINE_NODEBUG void set_first(BaseNode* node) noexcept { _first = node; }
  ASMJIT_INLINE_NODEBUG void set_last(BaseNode* node) noexcept { _last = node; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodePosition first_position() const noexcept { return _first_position; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodePosition end_position() const noexcept { return _end_position; }
  ASMJIT_INLINE_NODEBUG void set_first_position(NodePosition position) noexcept { _first_position = position; }
  ASMJIT_INLINE_NODEBUG void set_end_position(NodePosition position) noexcept { _end_position = position; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlockTimestamp timestamp() const noexcept { return _timestamp; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_timestamp(RABlockTimestamp ts) const noexcept { return _timestamp == ts; }
  ASMJIT_INLINE_NODEBUG void set_timestamp(RABlockTimestamp ts) const noexcept { _timestamp = ts; }
  ASMJIT_INLINE_NODEBUG void reset_timestamp() const noexcept { _timestamp = RABlockTimestamp(0); }
  [[nodiscard]]
  ASMJIT_INLINE Error alloc_live_bits(size_t size) noexcept {
    size_t bit_word_count = BitOps::size_in_words<BitWord>(size);
    _live_bits = arena().alloc_oneshot_zeroed<BitWord>(Arena::aligned_size(bit_word_count * sizeof(BitWord) * kLiveCount));
    if (ASMJIT_UNLIKELY(!_live_bits)) {
      return make_error(Error::kOutOfMemory);
    }
    _live_bits_size = uint32_t(bit_word_count);
    return Error::kOk;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<BitWord> live_bits(uint32_t live_type) noexcept { return Span<BitWord>(_live_bits + live_type * _live_bits_size, _live_bits_size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<const BitWord> live_bits(uint32_t live_type) const noexcept { return Span<const BitWord>(_live_bits + live_type * _live_bits_size, _live_bits_size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<BitWord> live_in() noexcept { return live_bits(kLiveIn); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<const BitWord> live_in() const noexcept { return live_bits(kLiveIn); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<BitWord> live_out() noexcept { return live_bits(kLiveOut); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<const BitWord> live_out() const noexcept { return live_bits(kLiveOut); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<BitWord> kill() noexcept { return live_bits(kLiveKill); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<const BitWord> kill() const noexcept { return live_bits(kLiveKill); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask entry_scratch_gp_regs() const noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask exit_scratch_gp_regs() const noexcept { return _exit_scratch_gp_regs; }
  ASMJIT_INLINE_NODEBUG void add_entry_scratch_gp_regs(RegMask reg_mask) noexcept { _entry_scratch_gp_regs |= reg_mask; }
  ASMJIT_INLINE_NODEBUG void add_exit_scratch_gp_regs(RegMask reg_mask) noexcept { _exit_scratch_gp_regs |= reg_mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_entry_assignment() const noexcept { return _entry_phys_to_work_map != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG PhysToWorkMap* entry_phys_to_work_map() const noexcept { return _entry_phys_to_work_map; }
  ASMJIT_INLINE_NODEBUG void set_entry_assignment(PhysToWorkMap* phys_to_work_map) noexcept { _entry_phys_to_work_map = phys_to_work_map; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_shared_assignment_id() const noexcept { return _shared_assignment_id != Globals::kInvalidId; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t shared_assignment_id() const noexcept { return _shared_assignment_id; }
  ASMJIT_INLINE_NODEBUG void set_shared_assignment_id(uint32_t id) noexcept { _shared_assignment_id = id; }
};
ASMJIT_END_NAMESPACE
#endif
#endif
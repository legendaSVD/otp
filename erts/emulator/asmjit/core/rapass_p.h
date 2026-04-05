#ifndef ASMJIT_CORE_RAPASS_P_H_INCLUDED
#define ASMJIT_CORE_RAPASS_P_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER
#include <asmjit/core/compiler.h>
#include <asmjit/core/emithelper_p.h>
#include <asmjit/core/raassignment_p.h>
#include <asmjit/core/racfgblock_p.h>
#include <asmjit/core/radefs_p.h>
#include <asmjit/core/rainst_p.h>
#include <asmjit/core/rastack_p.h>
#include <asmjit/support/support_p.h>
ASMJIT_BEGIN_NAMESPACE
class BaseRAPass : public Pass {
public:
  ASMJIT_NONCOPYABLE(BaseRAPass)
  using Base = Pass;
  static inline constexpr uint32_t kCallArgWeight = 80;
  using PhysToWorkMap = RAAssignment::PhysToWorkMap;
  using WorkToPhysMap = RAAssignment::WorkToPhysMap;
  Arena* _arena {};
  BaseEmitHelper* _emit_helper_ptr = nullptr;
  Logger* _logger = nullptr;
  FormatOptions _format_options {};
  DiagnosticOptions _diagnostic_options {};
  FuncNode* _func = nullptr;
  BaseNode* _stop = nullptr;
  BaseNode* _injection_start = nullptr;
  BaseNode* _injection_end = nullptr;
  ArenaVector<RABlock*> _blocks {};
  ArenaVector<RABlock*> _exits {};
  ArenaVector<RABlock*> _pov {};
  uint32_t _instruction_count = 0;
  uint32_t _created_block_count = 0;
  ArenaVector<RASharedAssignment> _shared_assignments {};
  mutable uint64_t _last_timestamp = 0;
  const ArchTraits* _arch_traits = nullptr;
  RARegIndex _phys_reg_index = RARegIndex();
  RARegCount _phys_reg_count = RARegCount();
  uint32_t _phys_reg_total = 0;
  Support::Array<uint8_t, 2> _scratch_reg_indexes {};
  RARegMask _available_regs = RARegMask();
  RARegMask _clobbered_regs = RARegMask();
  ArenaVector<RAWorkReg*> _work_regs;
  Support::Array<ArenaVector<RAWorkReg*>, Globals::kNumVirtGroups> _work_regs_of_group;
  uint32_t _multi_work_reg_count = 0u;
  uint32_t _total_work_reg_count = 0u;
  Support::Array<RAStrategy, Globals::kNumVirtGroups> _strategy;
  RALiveCount _global_live_max_count = RALiveCount();
  Support::Array<RALiveSpans*, Globals::kNumVirtGroups> _global_live_spans {};
  Operand _temporary_mem = Operand();
  Reg _sp = Reg();
  Reg _fp = Reg();
  RAStackAllocator _stack_allocator {};
  FuncArgsAssignment _args_assignment {};
  uint32_t _num_stack_args_to_stack_slots = 0;
  uint32_t _max_work_reg_name_size = 0;
  StringTmp<192> _tmp_string;
  BaseRAPass(BaseCompiler& cc) noexcept;
  ~BaseRAPass() noexcept override;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseCompiler& cc() const noexcept { return static_cast<BaseCompiler&>(_cb); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Logger* logger() const noexcept { return _logger; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Logger* logger_if(DiagnosticOptions option) const noexcept { return Support::test(_diagnostic_options, option) ? _logger : nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_diagnostic_option(DiagnosticOptions option) const noexcept { return Support::test(_diagnostic_options, option); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arena& arena() const noexcept { return *_arena; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<RASharedAssignment> shared_assignments() const { return _shared_assignments.as_span(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t shared_assignment_count() const noexcept { return _shared_assignments.size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncNode* func() const noexcept { return _func; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* stop() const noexcept { return _stop; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t end_position() const noexcept { return _instruction_count * 2u; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RARegMask& available_regs() const noexcept { return _available_regs; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RARegMask& clobbered_regs() const noexcept { return _clobbered_regs; }
  inline void make_unavailable(RegGroup group, uint32_t reg_id) noexcept {
    _available_regs[group] &= ~Support::bit_mask<RegMask>(reg_id);
  }
  inline void make_unavailable(const RARegMask::RegMasks& regs) noexcept {
    _available_regs.clear(regs);
  }
  Error run(Arena& arena, Logger* logger) override;
  Error run_on_function(Arena& arena, FuncNode* func, bool last) noexcept;
  Error on_perform_all_steps() noexcept;
  virtual void on_init() noexcept;
  virtual void on_done() noexcept;
  [[nodiscard]]
  inline RABlock* entry_block() noexcept {
    ASMJIT_ASSERT(!_blocks.is_empty());
    return _blocks[0];
  }
  [[nodiscard]]
  inline const RABlock* entry_block() const noexcept {
    ASMJIT_ASSERT(!_blocks.is_empty());
    return _blocks[0];
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<RABlock*> blocks() noexcept { return _blocks.as_span(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t block_count() const noexcept { return _blocks.size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t reachable_block_count() const noexcept { return _pov.size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_dangling_blocks() const noexcept { return _created_block_count != block_count(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlockTimestamp next_timestamp() const noexcept { return RABlockTimestamp(++_last_timestamp); }
  [[nodiscard]]
  RABlock* new_block(BaseNode* initial_node = nullptr) noexcept;
  [[nodiscard]]
  RABlock* new_block_or_existing_at(LabelNode* label_node, BaseNode** stopped_at = nullptr) noexcept;
  [[nodiscard]]
  Error add_block(RABlock* block) noexcept;
  [[nodiscard]]
  inline Error add_exit_block(RABlock* block) noexcept {
    block->add_flags(RABlockFlags::kIsFuncExit);
    return _exits.append(arena(), block);
  }
  [[nodiscard]]
  ASMJIT_INLINE RAInst* new_ra_inst(InstRWFlags inst_rw_flags, RATiedFlags flags, uint32_t tied_reg_count, const RARegMask& clobbered_regs) noexcept {
    void* p = arena().alloc_oneshot(RAInst::size_of(tied_reg_count));
    if (ASMJIT_UNLIKELY(!p)) {
      return nullptr;
    }
    return new(Support::PlacementNew{p}) RAInst(inst_rw_flags, flags, tied_reg_count, clobbered_regs);
  }
  [[nodiscard]]
  ASMJIT_INLINE Error assign_ra_inst(BaseNode* node, RABlock* block, RAInstBuilder& ib) noexcept {
    RABlockId block_id = block->block_id();
    uint32_t tied_reg_count = ib.tied_reg_count();
    RAInst* ra_inst = new_ra_inst(ib.inst_rw_flags(), ib.aggregated_flags(), tied_reg_count, ib._clobbered);
    if (ASMJIT_UNLIKELY(!ra_inst)) {
      return make_error(Error::kOutOfMemory);
    }
    RARegIndex index;
    RATiedFlags flags_filter = ~ib.forbidden_flags();
    index.build_indexes(ib._count);
    ra_inst->_tied_index = index;
    ra_inst->_tied_count = ib._count;
    for (uint32_t i = 0; i < tied_reg_count; i++) {
      RATiedReg* tied_reg = ib[i];
      RAWorkReg* work_reg = tied_reg->work_reg();
      RegGroup group = work_reg->group();
      work_reg->reset_tied_reg();
      if (work_reg->_single_basic_block_id != block_id) {
        if (Support::bool_or(work_reg->_single_basic_block_id != kBadBlockId, !tied_reg->is_write_only())) {
          work_reg->add_flags(RAWorkRegFlags::kMultiBlockUse);
        }
        work_reg->_single_basic_block_id = block_id;
        tied_reg->add_flags(RATiedFlags::kFirst);
      }
      if (tied_reg->has_use_id()) {
        block->add_flags(RABlockFlags::kHasFixedRegs);
        ra_inst->_used_regs[group] |= Support::bit_mask<RegMask>(tied_reg->use_id());
      }
      if (tied_reg->has_out_id()) {
        block->add_flags(RABlockFlags::kHasFixedRegs);
      }
      RATiedReg& dst = ra_inst->_tied_regs[index.get(group)];
      index.add(group);
      dst = *tied_reg;
      dst._flags &= flags_filter;
      if (!tied_reg->is_duplicate()) {
        dst._use_reg_mask &= ~ib._used[group];
      }
    }
    node->set_pass_data<RAInst>(ra_inst);
    return Error::kOk;
  }
  [[nodiscard]]
  virtual Error build_cfg_nodes() noexcept;
  [[nodiscard]]
  Error init_shared_assignments(Span<uint32_t> shared_assignments_map) noexcept;
  [[nodiscard]]
  Error build_cfg_views() noexcept;
  [[nodiscard]]
  Error build_cfg_dominators() noexcept;
  [[nodiscard]]
  bool _strictly_dominates(const RABlock* a, const RABlock* b) const noexcept;
  [[nodiscard]]
  const RABlock* _nearest_common_dominator(const RABlock* a, const RABlock* b) const noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool dominates(const RABlock* a, const RABlock* b) const noexcept { return a == b ? true : _strictly_dominates(a, b); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool strictly_dominates(const RABlock* a, const RABlock* b) const noexcept { return a == b ? false : _strictly_dominates(a, b); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlock* nearest_common_dominator(RABlock* a, RABlock* b) const noexcept { return const_cast<RABlock*>(_nearest_common_dominator(a, b)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RABlock* nearest_common_dominator(const RABlock* a, const RABlock* b) const noexcept { return _nearest_common_dominator(a, b); }
  [[nodiscard]]
  Error remove_unreachable_code() noexcept;
  [[nodiscard]]
  BaseNode* find_successor_starting_at(BaseNode* node) noexcept;
  [[nodiscard]]
  bool is_next_to(BaseNode* node, BaseNode* target) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t register_size() const noexcept { return _sp.size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* work_reg_by_id(RAWorkId work_id) const noexcept { return _work_regs[uint32_t(work_id)]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ArenaVector<RAWorkReg*>& work_regs() noexcept { return _work_regs; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ArenaVector<RAWorkReg*>& work_regs(RegGroup group) noexcept { return _work_regs_of_group[group]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ArenaVector<RAWorkReg*>& work_regs() const noexcept { return _work_regs; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ArenaVector<RAWorkReg*>& work_regs(RegGroup group) const noexcept { return _work_regs_of_group[group]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t multi_work_reg_count() const noexcept { return _multi_work_reg_count; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t work_reg_count() const noexcept { return _total_work_reg_count; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t work_reg_count(RegGroup group) const noexcept { return _work_regs_of_group[group].size(); }
  inline void _build_phys_index() noexcept {
    _phys_reg_index.build_indexes(_phys_reg_count);
    _phys_reg_total = _phys_reg_index.get(RegGroup::kMaxVirt) + _phys_reg_count.get(RegGroup::kMaxVirt);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t phys_reg_index(RegGroup group) const noexcept { return _phys_reg_index.get(group); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t phys_reg_total() const noexcept { return _phys_reg_total; }
  [[nodiscard]]
  Error _as_work_reg(RAWorkReg** out, VirtReg* virt_reg) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE Error as_work_reg(RAWorkReg** out, VirtReg* virt_reg) noexcept {
    RAWorkReg* work_reg = virt_reg->work_reg();
    if (ASMJIT_LIKELY(work_reg)) {
      *out = work_reg;
      return Error::kOk;
    }
    else {
      return _as_work_reg(out, virt_reg);
    }
  }
  [[nodiscard]]
  ASMJIT_INLINE Error virt_index_as_work_reg(RAWorkReg** out, uint32_t virt_index) noexcept {
    Span<VirtReg*> virt_regs = cc().virt_regs();
    if (ASMJIT_UNLIKELY(virt_index >= virt_regs.size()))
      return make_error(Error::kInvalidVirtId);
    return as_work_reg(out, virt_regs[virt_index]);
  }
  [[nodiscard]]
  RAStackSlot* _create_stack_slot(RAWorkReg* work_reg) noexcept;
  [[nodiscard]]
  inline RAStackSlot* get_or_create_stack_slot(RAWorkReg* work_reg) noexcept {
    RAStackSlot* slot = work_reg->stack_slot();
    return slot ? slot : _create_stack_slot(work_reg);
  }
  [[nodiscard]]
  inline BaseMem work_reg_as_mem(RAWorkReg* work_reg) noexcept {
    (void)get_or_create_stack_slot(work_reg);
    return BaseMem(OperandSignature::from_op_type(OperandType::kMem) |
                   OperandSignature::from_mem_base_type(_sp.reg_type()) |
                   OperandSignature::from_bits(OperandSignature::kMemRegHomeFlag),
                   work_reg->virt_id(), 0, 0);
  }
  [[nodiscard]]
  WorkToPhysMap* new_work_to_phys_map() noexcept;
  [[nodiscard]]
  PhysToWorkMap* new_phys_to_work_map() noexcept;
  [[nodiscard]]
  inline PhysToWorkMap* clone_phys_to_work_map(const PhysToWorkMap* map) noexcept {
    return static_cast<PhysToWorkMap*>(arena().dup(map, PhysToWorkMap::size_of(_phys_reg_total)));
  }
  [[nodiscard]]
  Error build_reg_ids() noexcept;
  [[nodiscard]]
  Error build_liveness() noexcept;
  [[nodiscard]]
  Error assign_arg_index_to_work_regs() noexcept;
  [[nodiscard]]
  Error run_global_allocator() noexcept;
  [[nodiscard]]
  Error init_global_live_spans() noexcept;
  [[nodiscard]]
  Error bin_pack(RegGroup group) noexcept;
  [[nodiscard]]
  Error run_local_allocator() noexcept;
  [[nodiscard]]
  Error set_block_entry_assignment(RABlock* block, const RABlock* from_block, const RAAssignment& from_assignment) noexcept;
  [[nodiscard]]
  Error set_shared_assignment(uint32_t shared_assignment_id, const RAAssignment& from_assignment) noexcept;
  [[nodiscard]]
  Error block_entry_assigned(const PhysToWorkMap* phys_to_work_map) noexcept;
  [[nodiscard]]
  Error use_temporary_mem(BaseMem& out, uint32_t size, uint32_t alignment) noexcept;
  [[nodiscard]]
  virtual Error update_stack_frame() noexcept;
  [[nodiscard]]
  Error _mark_stack_args_to_keep() noexcept;
  [[nodiscard]]
  Error _update_stack_args() noexcept;
  [[nodiscard]]
  Error insert_prolog_epilog() noexcept;
  template<typename Lambda>
  ASMJIT_INLINE Error rewrite_iterate(Lambda&& fn) noexcept {
    RABlock* block = nullptr;
    BaseNode* first = _injection_start;
    BaseNode* stop = _injection_end;
    Span<RABlock*> pov = _pov.as_span();
    size_t pov_index = pov.size();
    if (first == nullptr) {
      ASMJIT_ASSERT(pov_index > 0u);
      block = pov[--pov_index];
      first = block->first();
      stop = block->last()->next();
    }
    for (;;) {
      ASMJIT_PROPAGATE(fn(first, stop, block));
      if (!pov_index) {
        break;
      }
      block = pov[--pov_index];
      first = block->first();
      stop = block->last()->next();
    }
    return Error::kOk;
  }
  [[nodiscard]]
  virtual Error rewrite() noexcept;
#ifndef ASMJIT_NO_LOGGING
  Error annotate_code() noexcept;
  Error dump_block_ids(String& sb, Span<RABlock*> blocks) noexcept;
  Error dump_block_liveness(String& sb, const RABlock* block) noexcept;
  Error dump_live_spans(String& sb) noexcept;
#endif
  [[nodiscard]]
  virtual Error emit_move(RAWorkReg* work_reg, uint32_t dst_phys_id, uint32_t src_phys_id) noexcept;
  [[nodiscard]]
  virtual Error emit_swap(RAWorkReg* a_reg, uint32_t a_phys_id, RAWorkReg* b_reg, uint32_t b_phys_id) noexcept;
  [[nodiscard]]
  virtual Error emit_load(RAWorkReg* work_reg, uint32_t dst_phys_id) noexcept;
  [[nodiscard]]
  virtual Error emit_save(RAWorkReg* work_reg, uint32_t src_phys_id) noexcept;
  [[nodiscard]]
  virtual Error emit_jump(const Label& label) noexcept;
  [[nodiscard]]
  virtual Error emit_pre_call(InvokeNode* invoke_node) noexcept;
};
inline Arena& RABlock::arena() const noexcept { return _ra->arena(); }
inline RegMask RABlock::entry_scratch_gp_regs() const noexcept {
  RegMask regs = _entry_scratch_gp_regs;
  if (has_shared_assignment_id()) {
    regs = _ra->_shared_assignments[_shared_assignment_id].entry_scratch_gp_regs();
  }
  return regs;
}
ASMJIT_END_NAMESPACE
#endif
#endif
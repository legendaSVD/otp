#ifndef ASMJIT_CORE_RAINST_P_H_INCLUDED
#define ASMJIT_CORE_RAINST_P_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER
#include <asmjit/core/compilerdefs.h>
#include <asmjit/core/radefs_p.h>
#include <asmjit/core/rareg_p.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/support_p.h>
ASMJIT_BEGIN_NAMESPACE
class RAInst {
public:
  ASMJIT_NONCOPYABLE(RAInst)
  InstRWFlags _inst_rw_flags;
  RATiedFlags _flags;
  uint32_t _tied_total;
  RARegIndex _tied_index;
  RARegCount _tied_count;
  RALiveCount _live_count;
  RARegMask _used_regs;
  RARegMask _clobbered_regs;
  RATiedReg _tied_regs[1];
  inline RAInst(InstRWFlags inst_rw_flags, RATiedFlags tied_flags, uint32_t tied_total, const RARegMask& clobbered_regs) noexcept {
    _inst_rw_flags = inst_rw_flags;
    _flags = tied_flags;
    _tied_total = tied_total;
    _tied_index.reset();
    _tied_count.reset();
    _live_count.reset();
    _used_regs.reset();
    _clobbered_regs = clobbered_regs;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstRWFlags inst_rw_flags() const noexcept { return _inst_rw_flags; };
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_inst_rw_flag(InstRWFlags flag) const noexcept { return Support::test(_inst_rw_flags, flag); }
  ASMJIT_INLINE_NODEBUG void add_inst_rw_flags(InstRWFlags flags) noexcept { _inst_rw_flags |= flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedFlags flags() const noexcept { return _flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(RATiedFlags flag) const noexcept { return Support::test(_flags, flag); }
  ASMJIT_INLINE_NODEBUG void set_flags(RATiedFlags flags) noexcept { _flags = flags; }
  ASMJIT_INLINE_NODEBUG void add_flags(RATiedFlags flags) noexcept { _flags |= flags; }
  ASMJIT_INLINE_NODEBUG void clear_flags(RATiedFlags flags) noexcept { _flags &= ~flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_reg_to_mem_patched() const noexcept { return has_flag(RATiedFlags::kInst_RegToMemPatched); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_transformable() const noexcept { return has_flag(RATiedFlags::kInst_IsTransformable); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* tied_regs() const noexcept { return const_cast<RATiedReg*>(_tied_regs); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* tied_regs(RegGroup group) const noexcept { return const_cast<RATiedReg*>(_tied_regs) + _tied_index.get(group); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t tied_count() const noexcept { return _tied_total; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t tied_count(RegGroup group) const noexcept { return _tied_count.get(group); }
  [[nodiscard]]
  inline RATiedReg* tied_at(size_t index) const noexcept {
    ASMJIT_ASSERT(index < _tied_total);
    return tied_regs() + index;
  }
  [[nodiscard]]
  inline RATiedReg* tied_of(RegGroup group, size_t index) const noexcept {
    ASMJIT_ASSERT(index < _tied_count.get(group));
    return tied_regs(group) + index;
  }
  [[nodiscard]]
  inline const RATiedReg* tied_reg_for_work_reg(RegGroup group, RAWorkReg* work_reg) const noexcept {
    const RATiedReg* array = tied_regs(group);
    size_t count = tied_count(group);
    for (size_t i = 0; i < count; i++) {
      const RATiedReg* tied_reg = &array[i];
      if (tied_reg->work_reg() == work_reg) {
        return tied_reg;
      }
    }
    return nullptr;
  }
  inline void set_tied_at(size_t index, RATiedReg& tied) noexcept {
    ASMJIT_ASSERT(index < _tied_total);
    _tied_regs[index] = tied;
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG size_t size_of(uint32_t tied_reg_count) noexcept {
    return Arena::aligned_size_of<RAInst>() - sizeof(RATiedReg) + tied_reg_count * sizeof(RATiedReg);
  }
};
class RAInstBuilder {
public:
  ASMJIT_NONCOPYABLE(RAInstBuilder)
  RABlockId _basic_block_id;
  InstRWFlags _inst_rw_flags;
  RATiedFlags _aggregated_flags;
  RATiedFlags _forbidden_flags;
  RARegCount _count;
  RARegsStats _stats;
  RARegMask _used;
  RARegMask _clobbered;
  RATiedReg* _cur;
  RATiedReg _tied_regs[128];
  ASMJIT_INLINE_NODEBUG explicit RAInstBuilder(RABlockId block_id = kBadBlockId) noexcept { reset(block_id); }
  ASMJIT_INLINE_NODEBUG void init(RABlockId block_id) noexcept { reset(block_id); }
  ASMJIT_INLINE_NODEBUG void reset(RABlockId block_id) noexcept {
    _basic_block_id = block_id;
    _inst_rw_flags = InstRWFlags::kNone;
    _aggregated_flags = RATiedFlags::kNone;
    _forbidden_flags = RATiedFlags::kNone;
    _count.reset();
    _stats.reset();
    _used.reset();
    _clobbered.reset();
    _cur = _tied_regs;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstRWFlags inst_rw_flags() const noexcept { return _inst_rw_flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_inst_rw_flag(InstRWFlags flag) const noexcept { return Support::test(_inst_rw_flags, flag); }
  ASMJIT_INLINE_NODEBUG void add_inst_rw_flags(InstRWFlags flags) noexcept { _inst_rw_flags |= flags; }
  ASMJIT_INLINE_NODEBUG void clear_inst_rw_flags(InstRWFlags flags) noexcept { _inst_rw_flags &= ~flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedFlags aggregated_flags() const noexcept { return _aggregated_flags; }
  ASMJIT_INLINE_NODEBUG void add_aggregated_flags(RATiedFlags flags) noexcept { _aggregated_flags |= flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedFlags forbidden_flags() const noexcept { return _forbidden_flags; }
  ASMJIT_INLINE_NODEBUG void add_forbidden_flags(RATiedFlags flags) noexcept { _forbidden_flags |= flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t tied_reg_count() const noexcept { return uint32_t((size_t)(_cur - _tied_regs)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* begin() noexcept { return _tied_regs; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* end() noexcept { return _cur; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RATiedReg* begin() const noexcept { return _tied_regs; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RATiedReg* end() const noexcept { return _cur; }
  [[nodiscard]]
  inline RATiedReg* operator[](size_t index) noexcept {
    ASMJIT_ASSERT(index < tied_reg_count());
    return &_tied_regs[index];
  }
  [[nodiscard]]
  inline const RATiedReg* operator[](size_t index) const noexcept {
    ASMJIT_ASSERT(index < tied_reg_count());
    return &_tied_regs[index];
  }
  [[nodiscard]]
  Error add(
    RAWorkReg* work_reg,
    RATiedFlags flags,
    RegMask use_reg_mask, uint32_t use_id, uint32_t use_rewrite_mask,
    RegMask out_reg_mask, uint32_t out_id, uint32_t out_rewrite_mask,
    uint32_t rm_size = 0,
    RAWorkReg* consecutive_parent = nullptr
  ) noexcept {
    RegGroup group = work_reg->group();
    RATiedReg* tied_reg = work_reg->tied_reg();
    if (use_id != Reg::kIdBad) {
      _stats.make_fixed(group);
      _used[group] |= Support::bit_mask<RegMask>(use_id);
      flags |= RATiedFlags::kUseFixed;
    }
    if (out_id != Reg::kIdBad) {
      _clobbered[group] |= Support::bit_mask<RegMask>(out_id);
      flags |= RATiedFlags::kOutFixed;
    }
    _aggregated_flags |= flags;
    _stats.make_used(group);
    if (!tied_reg) {
      ASMJIT_ASSERT(tied_reg_count() < ASMJIT_ARRAY_SIZE(_tied_regs));
      tied_reg = _cur++;
      tied_reg->init(work_reg, flags, use_reg_mask, use_id, use_rewrite_mask, out_reg_mask, out_id, out_rewrite_mask, rm_size, consecutive_parent);
      work_reg->set_tied_reg(tied_reg);
      _count.add(group);
      return Error::kOk;
    }
    else {
      if (consecutive_parent != tied_reg->consecutive_parent()) {
        if (tied_reg->has_consecutive_parent()) {
          return make_error(Error::kInvalidState);
        }
        tied_reg->_consecutive_parent = consecutive_parent;
      }
      if (use_id != Reg::kIdBad) {
        if (ASMJIT_UNLIKELY(tied_reg->has_use_id())) {
          return make_error(Error::kOverlappedRegs);
        }
        tied_reg->set_use_id(use_id);
      }
      if (out_id != Reg::kIdBad) {
        if (ASMJIT_UNLIKELY(tied_reg->has_out_id())) {
          return make_error(Error::kOverlappedRegs);
        }
        tied_reg->set_out_id(out_id);
      }
      tied_reg->add_ref_count();
      tied_reg->add_flags(flags);
      tied_reg->_use_reg_mask &= use_reg_mask;
      tied_reg->_use_rewrite_mask |= use_rewrite_mask;
      tied_reg->_out_reg_mask &= out_reg_mask;
      tied_reg->_out_rewrite_mask |= out_rewrite_mask;
      tied_reg->_rm_size = uint8_t(Support::max<uint32_t>(tied_reg->rm_size(), rm_size));
      return Error::kOk;
    }
  }
  [[nodiscard]]
  Error add_call_arg(RAWorkReg* work_reg, uint32_t use_id) noexcept {
    ASMJIT_ASSERT(use_id != Reg::kIdBad);
    RATiedFlags flags = RATiedFlags::kUse | RATiedFlags::kRead | RATiedFlags::kUseFixed;
    RegGroup group = work_reg->group();
    RegMask allocable = Support::bit_mask<RegMask>(use_id);
    _aggregated_flags |= flags;
    _used[group] |= allocable;
    _stats.make_fixed(group);
    _stats.make_used(group);
    RATiedReg* tied_reg = work_reg->tied_reg();
    if (!tied_reg) {
      ASMJIT_ASSERT(tied_reg_count() < ASMJIT_ARRAY_SIZE(_tied_regs));
      tied_reg = _cur++;
      tied_reg->init(work_reg, flags, allocable, use_id, 0, allocable, Reg::kIdBad, 0);
      work_reg->set_tied_reg(tied_reg);
      _count.add(group);
      return Error::kOk;
    }
    else {
      if (tied_reg->has_use_id()) {
        flags |= RATiedFlags::kDuplicate;
        tied_reg->_use_reg_mask |= allocable;
      }
      else {
        tied_reg->set_use_id(use_id);
        tied_reg->_use_reg_mask &= allocable;
      }
      tied_reg->add_ref_count();
      tied_reg->add_flags(flags);
      return Error::kOk;
    }
  }
  [[nodiscard]]
  Error add_call_ret(RAWorkReg* work_reg, uint32_t out_id) noexcept {
    ASMJIT_ASSERT(out_id != Reg::kIdBad);
    RATiedFlags flags = RATiedFlags::kOut | RATiedFlags::kWrite | RATiedFlags::kOutFixed;
    RegGroup group = work_reg->group();
    RegMask out_regs = Support::bit_mask<RegMask>(out_id);
    _aggregated_flags |= flags;
    _used[group] |= out_regs;
    _stats.make_fixed(group);
    _stats.make_used(group);
    RATiedReg* tied_reg = work_reg->tied_reg();
    if (!tied_reg) {
      ASMJIT_ASSERT(tied_reg_count() < ASMJIT_ARRAY_SIZE(_tied_regs));
      tied_reg = _cur++;
      tied_reg->init(work_reg, flags, Support::bit_ones<RegMask>, Reg::kIdBad, 0, out_regs, out_id, 0);
      work_reg->set_tied_reg(tied_reg);
      _count.add(group);
      return Error::kOk;
    }
    else {
      if (tied_reg->has_out_id()) {
        return make_error(Error::kOverlappedRegs);
      }
      tied_reg->add_ref_count();
      tied_reg->add_flags(flags);
      tied_reg->set_out_id(out_id);
      return Error::kOk;
    }
  }
};
ASMJIT_END_NAMESPACE
#endif
#endif
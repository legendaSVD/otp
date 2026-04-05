#include <asmjit/core/api-build_p.h>
#ifndef ASMJIT_NO_COMPILER
#include <asmjit/core/ralocal_p.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
static ASMJIT_INLINE RATiedReg* RALocal_findTiedReg(RATiedReg* tied_regs, size_t count, RAWorkReg* work_reg) noexcept {
  for (size_t i = 0; i < count; i++) {
    if (tied_regs[i].work_reg() == work_reg) {
      return &tied_regs[i];
    }
  }
  return nullptr;
}
Error RALocalAllocator::init() noexcept {
  PhysToWorkMap* phys_to_work_map;
  WorkToPhysMap* work_to_phys_map;
  phys_to_work_map = _pass.new_phys_to_work_map();
  work_to_phys_map = _pass.new_work_to_phys_map();
  if (!phys_to_work_map || !work_to_phys_map) {
    return make_error(Error::kOutOfMemory);
  }
  _cur_assignment.init_layout(_pass._phys_reg_count, _pass.work_regs());
  _cur_assignment.init_maps(phys_to_work_map, work_to_phys_map);
  phys_to_work_map = _pass.new_phys_to_work_map();
  work_to_phys_map = _pass.new_work_to_phys_map();
  _tmp_work_to_phys_map = _pass.new_work_to_phys_map();
  if (!phys_to_work_map || !work_to_phys_map || !_tmp_work_to_phys_map) {
    return make_error(Error::kOutOfMemory);
  }
  _tmp_assignment.init_layout(_pass._phys_reg_count, _pass.work_regs());
  _tmp_assignment.init_maps(phys_to_work_map, work_to_phys_map);
  return Error::kOk;
}
Error RALocalAllocator::make_initial_assignment() noexcept {
  FuncNode* func = _pass.func();
  RABlock* entry = _pass.entry_block();
  Span<BitWord> live_in = entry->live_in();
  uint32_t multi_work_reg_count = _pass._multi_work_reg_count;
  uint32_t arg_count = func->arg_count();
  uint32_t iter_count = 1;
  for (uint32_t iter = 0; iter < iter_count; iter++) {
    for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
      for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
        const RegOnly& reg_arg = func->arg_pack(arg_index)[value_index];
        if (!reg_arg.is_reg() || !_cc.is_virt_id_valid(reg_arg.id())) {
          continue;
        }
        VirtReg* virt_reg = _cc.virt_reg_by_id(reg_arg.id());
        RAWorkReg* work_reg = virt_reg->work_reg();
        if (!work_reg) {
          continue;
        }
        RAWorkId work_id = work_reg->work_id();
        if (uint32_t(work_id) >= multi_work_reg_count || !BitOps::bit_at(live_in, work_id)) {
          continue;
        }
        RegGroup group = work_reg->group();
        if (_cur_assignment.work_to_phys_id(group, work_id) != RAAssignment::kPhysNone) {
          continue;
        }
        RegMask allocable_regs = _available_regs[group] & ~_cur_assignment.assigned(group);
        if (iter == 0) {
          if (work_reg->has_home_reg_id()) {
            uint32_t phys_id = work_reg->home_reg_id();
            if (Support::bit_test(allocable_regs, phys_id)) {
              _cur_assignment.assign(group, work_id, phys_id, true);
              _pass._args_assignment.assign_reg_in_pack(arg_index, value_index, work_reg->type(), phys_id, work_reg->type_id());
              continue;
            }
          }
          iter_count = 2;
        }
        else {
          if (allocable_regs) {
            uint32_t phys_id = Support::ctz(allocable_regs);
            _cur_assignment.assign(group, work_id, phys_id, true);
            _pass._args_assignment.assign_reg_in_pack(arg_index, value_index, work_reg->type(), phys_id, work_reg->type_id());
          }
          else {
            RAStackSlot* slot = _pass.get_or_create_stack_slot(work_reg);
            if (ASMJIT_UNLIKELY(!slot)) {
              return make_error(Error::kOutOfMemory);
            }
            work_reg->add_flags(RAWorkRegFlags::kStackArgToStack);
            _pass._num_stack_args_to_stack_slots++;
          }
        }
      }
    }
  }
  return Error::kOk;
}
Error RALocalAllocator::replace_assignment(const PhysToWorkMap* phys_to_work_map) noexcept {
  _cur_assignment.copy_from(phys_to_work_map);
  return Error::kOk;
}
Error RALocalAllocator::switch_to_assignment(PhysToWorkMap* dst_phys_to_work_map, Span<const BitWord> live_in, bool dst_is_read_only, bool try_mode) noexcept {
  RAAssignment dst;
  RAAssignment& cur = _cur_assignment;
  dst.init_layout(_pass._phys_reg_count, _pass.work_regs());
  dst.init_maps(dst_phys_to_work_map, _tmp_work_to_phys_map);
  dst.assign_work_ids_from_phys_ids();
  uint32_t multi_work_reg_count = _pass._multi_work_reg_count;
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    if (!try_mode) {
      Support::BitWordIterator<RegMask> it(cur.assigned(group));
      while (it.has_next()) {
        uint32_t phys_id = it.next();
        RAWorkId work_id = cur.phys_to_work_id(group, phys_id);
        ASMJIT_ASSERT(work_id != kBadWorkId);
        if (uint32_t(work_id) >= multi_work_reg_count || !BitOps::bit_at(live_in, work_id)) {
          _unassign_reg(group, work_id, phys_id);
          continue;
        }
        uint32_t alt_id = dst.work_to_phys_id(group, work_id);
        if (alt_id == RAAssignment::kPhysNone) {
          ASMJIT_PROPAGATE(on_spill_reg(group, work_reg_by_id(work_id), work_id, phys_id));
        }
      }
    }
    int32_t run_id = -1;
    RegMask will_load_regs = 0;
    RegMask affected_regs = dst.assigned(group);
    while (affected_regs) {
      if (++run_id == 2) {
        if (!try_mode) {
          return make_error(Error::kInvalidState);
        }
        break;
      }
      Support::BitWordIterator<RegMask> it(affected_regs);
      while (it.has_next()) {
        uint32_t phys_id = it.next();
        RegMask phys_mask = Support::bit_mask<RegMask>(phys_id);
        RAWorkId cur_work_id = cur.phys_to_work_id(group, phys_id);
        RAWorkId dst_work_id = dst.phys_to_work_id(group, phys_id);
        ASMJIT_ASSERT(dst_work_id != kBadWorkId);
        ASMJIT_ASSERT(uint32_t(dst_work_id) < multi_work_reg_count);
        RAWorkReg* dst_work_reg = work_reg_by_id(dst_work_id);
        if (cur_work_id != kBadWorkId) {
          RAWorkReg* cur_work_reg = work_reg_by_id(cur_work_id);
          if (cur_work_id != dst_work_id) {
            if (run_id <= 0) {
              continue;
            }
            uint32_t alt_phys_id = cur.work_to_phys_id(group, dst_work_id);
            if (alt_phys_id == RAAssignment::kPhysNone) {
              continue;
            }
            run_id = -1;
            if (_arch_traits->has_inst_reg_swap(group)) {
              ASMJIT_PROPAGATE(on_swap_reg(group, cur_work_reg, cur_work_id, phys_id, dst_work_reg, dst_work_id, alt_phys_id));
            }
            else {
              if (!cur.is_phys_dirty(group, phys_id)) {
                _unassign_reg(group, cur_work_id, phys_id);
              }
              else {
                RegMask allocable_regs = _pass._available_regs[group] & ~cur.assigned(group);
                if (allocable_regs & ~dst.assigned(group)) {
                  allocable_regs &= ~dst.assigned(group);
                }
                if (allocable_regs) {
                  uint32_t tmp_phys_id = Support::ctz(allocable_regs);
                  ASMJIT_PROPAGATE(on_move_reg(group, cur_work_reg, cur_work_id, tmp_phys_id, phys_id));
                  _clobbered_regs[group] |= Support::bit_mask<RegMask>(tmp_phys_id);
                }
                else {
                  ASMJIT_PROPAGATE(on_spill_reg(group, cur_work_reg, cur_work_id, phys_id));
                }
              }
              goto Cleared;
            }
          }
        }
        else {
Cleared:
          uint32_t alt_phys_id = cur.work_to_phys_id(group, dst_work_id);
          if (alt_phys_id == RAAssignment::kPhysNone) {
            if (BitOps::bit_at(live_in, dst_work_id)) {
              will_load_regs |= phys_mask;
            }
            affected_regs &= ~phys_mask;
            continue;
          }
          ASMJIT_PROPAGATE(on_move_reg(group, dst_work_reg, dst_work_id, phys_id, alt_phys_id));
        }
        if ((dst.dirty(group) & phys_mask) != (cur.dirty(group) & phys_mask)) {
          if ((dst.dirty(group) & phys_mask) == 0) {
            ASMJIT_ASSERT(!dst.is_phys_dirty(group, phys_id) && cur.is_phys_dirty(group, phys_id));
            if (dst_is_read_only) {
              ASMJIT_PROPAGATE(on_save_reg(group, dst_work_reg, dst_work_id, phys_id));
            }
            else {
              dst.make_dirty(group, dst_work_id, phys_id);
            }
          }
          else {
            ASMJIT_ASSERT(dst.is_phys_dirty(group, phys_id) && !cur.is_phys_dirty(group, phys_id));
            cur.make_dirty(group, dst_work_id, phys_id);
          }
        }
        ASMJIT_ASSERT(dst.phys_to_work_id(group, phys_id) == cur.phys_to_work_id(group, phys_id));
        ASMJIT_ASSERT(dst.is_phys_dirty(group, phys_id) == cur.is_phys_dirty(group, phys_id));
        run_id = -1;
        affected_regs &= ~phys_mask;
      }
    }
    {
      Support::BitWordIterator<RegMask> it(will_load_regs);
      while (it.has_next()) {
        uint32_t phys_id = it.next();
        if (!cur.is_phys_assigned(group, phys_id)) {
          RAWorkId work_id = dst.phys_to_work_id(group, phys_id);
          ASMJIT_ASSERT(uint32_t(work_id) < multi_work_reg_count);
          ASMJIT_ASSERT(BitOps::bit_at(live_in, work_id) == true);
          RAWorkReg* work_reg = work_reg_by_id(work_id);
          ASMJIT_PROPAGATE(on_load_reg(group, work_reg, work_id, phys_id));
          if (dst.is_phys_dirty(group, phys_id)) {
            cur.make_dirty(group, work_id, phys_id);
          }
          ASMJIT_ASSERT(dst.is_phys_dirty(group, phys_id) == cur.is_phys_dirty(group, phys_id));
        }
        else {
          ASMJIT_ASSERT(try_mode == true);
        }
      }
    }
  }
  if (!try_mode) {
    ASMJIT_ASSERT(dst.equals(cur));
  }
  return Error::kOk;
}
Error RALocalAllocator::spill_scratch_gp_regs_before_entry(RegMask scratch_regs) noexcept {
  RegGroup group = RegGroup::kGp;
  Support::BitWordIterator<RegMask> it(scratch_regs);
  while (it.has_next()) {
    uint32_t phys_id = it.next();
    if (_cur_assignment.is_phys_assigned(group, phys_id)) {
      RAWorkId work_id = _cur_assignment.phys_to_work_id(group, phys_id);
      ASMJIT_PROPAGATE(on_spill_reg(group, work_reg_by_id(work_id), work_id, phys_id));
    }
  }
  return Error::kOk;
}
Error RALocalAllocator::alloc_instruction(InstNode* node) noexcept {
  RAInst* ra_inst = node->pass_data<RAInst>();
  RATiedReg* out_tied_regs[Globals::kMaxPhysRegs];
  RATiedReg* dup_tied_regs[Globals::kMaxPhysRegs];
  RATiedReg* consecutive_regs[kMaxConsecutiveRegs];
  _cc.set_cursor(node->prev());
  _node = node;
  _ra_inst = ra_inst;
  _tied_total = ra_inst->_tied_total;
  _tied_count = ra_inst->_tied_count;
  bool rm_allocated = false;
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    uint32_t i, count = this->tied_count(group);
    RATiedReg* tied_regs = this->tied_regs(group);
    RegMask will_use = _ra_inst->_used_regs[group];
    RegMask will_out = _ra_inst->_clobbered_regs[group];
    RegMask will_free = 0;
    uint32_t use_pending_count = count;
    uint32_t out_tied_count = 0;
    uint32_t dup_tied_count = 0;
    uint32_t consecutive_mask = 0;
    for (i = 0; i < count; i++) {
      RATiedReg* tied_reg = &tied_regs[i];
      if (tied_reg->has_any_consecutive_flag()) {
        uint32_t consecutive_offset = tied_reg->is_lead_consecutive() ? uint32_t(0) : tied_reg->consecutive_data();
        if (ASMJIT_UNLIKELY(Support::bit_test(consecutive_mask, consecutive_offset))) {
          return make_error(Error::kInvalidState);
        }
        consecutive_mask |= Support::bit_mask<uint32_t>(consecutive_offset);
        consecutive_regs[consecutive_offset] = tied_reg;
      }
      if (tied_reg->is_out_or_kill()) {
        out_tied_regs[out_tied_count++] = tied_reg;
      }
      if (tied_reg->is_duplicate()) {
        dup_tied_regs[dup_tied_count++] = tied_reg;
      }
      if (!tied_reg->is_use()) {
        tied_reg->mark_use_done();
        use_pending_count--;
        continue;
      }
      if (tied_reg->is_use_consecutive()) {
        continue;
      }
      RAWorkReg* work_reg = tied_reg->work_reg();
      RAWorkId work_id = work_reg->work_id();
      uint32_t assigned_id = _cur_assignment.work_to_phys_id(group, work_id);
      if (tied_reg->has_use_id()) {
        RegMask use_mask = Support::bit_mask<RegMask>(tied_reg->use_id());
        ASMJIT_ASSERT((will_use & use_mask) != 0);
        if (assigned_id == tied_reg->use_id()) {
          tied_reg->mark_use_done();
          if (tied_reg->is_write()) {
            _cur_assignment.make_dirty(group, work_id, assigned_id);
          }
          use_pending_count--;
          will_use |= use_mask;
        }
        else {
          will_free |= use_mask & _cur_assignment.assigned(group);
        }
      }
      else {
        RegMask allocable_regs = tied_reg->use_reg_mask();
        if (assigned_id != RAAssignment::kPhysNone) {
          RegMask assigned_mask = Support::bit_mask<RegMask>(assigned_id);
          if ((allocable_regs & ~will_use) & assigned_mask) {
            tied_reg->set_use_id(assigned_id);
            tied_reg->mark_use_done();
            if (tied_reg->is_write()) {
              _cur_assignment.make_dirty(group, work_id, assigned_id);
            }
            use_pending_count--;
            will_use |= assigned_mask;
          }
          else {
            will_free |= assigned_mask;
          }
        }
      }
    }
    uint32_t consecutive_count = 0;
    if (consecutive_mask) {
      if ((consecutive_mask & (consecutive_mask + 1u)) != 0) {
        return make_error(Error::kInvalidState);
      }
      consecutive_count = Support::ctz(~consecutive_mask);
      RATiedReg* lead = consecutive_regs[0];
      if (lead->is_use_consecutive()) {
        uint32_t best_score = 0;
        uint32_t best_lead_reg = 0xFFFFFFFF;
        RegMask allocable_regs = (_available_regs[group] | will_free) & ~will_use;
        uint32_t assignments[kMaxConsecutiveRegs];
        for (i = 0; i < consecutive_count; i++) {
          assignments[i] = _cur_assignment.work_to_phys_id(group, consecutive_regs[i]->work_reg()->work_id());
        }
        Support::BitWordIterator<uint32_t> it(lead->use_reg_mask());
        while (it.has_next()) {
          uint32_t reg_index = it.next();
          if (Support::bit_test(lead->use_reg_mask(), reg_index)) {
            uint32_t score = 15;
            for (i = 0; i < consecutive_count; i++) {
              uint32_t consecutive_index = reg_index + i;
              if (!Support::bit_test(allocable_regs, consecutive_index)) {
                score = 0;
                break;
              }
              RAWorkReg* work_reg = consecutive_regs[i]->work_reg();
              score += uint32_t(work_reg->home_reg_id() == consecutive_index);
              score += uint32_t(assignments[i] == consecutive_index) * 2;
            }
            if (score > best_score) {
              best_score = score;
              best_lead_reg = reg_index;
            }
          }
        }
        if (best_lead_reg == 0xFFFFFFFF) {
          return make_error(Error::kConsecutiveRegsAllocation);
        }
        for (i = 0; i < consecutive_count; i++) {
          uint32_t consecutive_index = best_lead_reg + i;
          RATiedReg* tied_reg = consecutive_regs[i];
          RegMask use_mask = Support::bit_mask<uint32_t>(consecutive_index);
          RAWorkReg* work_reg = tied_reg->work_reg();
          RAWorkId work_id = work_reg->work_id();
          uint32_t assigned_id = _cur_assignment.work_to_phys_id(group, work_id);
          tied_reg->set_use_id(consecutive_index);
          if (assigned_id == consecutive_index) {
            tied_reg->mark_use_done();
            if (tied_reg->is_write()) {
              _cur_assignment.make_dirty(group, work_id, assigned_id);
            }
            use_pending_count--;
            will_use |= use_mask;
          }
          else {
            will_use |= use_mask;
            will_free |= use_mask & _cur_assignment.assigned(group);
          }
        }
      }
    }
    if (use_pending_count) {
      RegMask live_regs = _cur_assignment.assigned(group) & ~will_free;
      for (i = 0; i < count; i++) {
        RATiedReg* tied_reg = &tied_regs[i];
        if (tied_reg->is_use_done()) {
          continue;
        }
        RAWorkReg* work_reg = tied_reg->work_reg();
        RAWorkId work_id = work_reg->work_id();
        uint32_t assigned_id = _cur_assignment.work_to_phys_id(group, work_id);
        if (!rm_allocated && tied_reg->has_use_rm()) {
          if (assigned_id == RAAssignment::kPhysNone && Support::is_power_of_2(tied_reg->use_rewrite_mask())) {
            uint32_t op_index = Support::ctz(tied_reg->use_rewrite_mask()) / uint32_t(sizeof(Operand) / sizeof(uint32_t));
            uint32_t rm_size = tied_reg->rm_size();
            if (rm_size <= work_reg->virt_reg()->virt_size()) {
              Operand& op = node->operands()[op_index];
              op = _pass.work_reg_as_mem(work_reg);
              op._signature.set_size(rm_size);
              tied_reg->_use_rewrite_mask = 0;
              tied_reg->mark_use_done();
              ra_inst->add_flags(RATiedFlags::kInst_RegToMemPatched);
              use_pending_count--;
              rm_allocated = true;
              continue;
            }
          }
        }
        if (!tied_reg->has_use_id()) {
          RegMask allocable_regs = tied_reg->use_reg_mask() & ~(will_free | will_use);
          uint32_t use_id = decide_on_assignment(group, work_reg, assigned_id, allocable_regs);
          RegMask use_mask = Support::bit_mask<RegMask>(use_id);
          will_use |= use_mask;
          will_free |= use_mask & live_regs;
          tied_reg->set_use_id(use_id);
          if (assigned_id != RAAssignment::kPhysNone) {
            RegMask assigned_mask = Support::bit_mask<RegMask>(assigned_id);
            will_free |= assigned_mask;
            live_regs &= ~assigned_mask;
            if (!(live_regs & use_mask)) {
              ASMJIT_PROPAGATE(on_move_reg(group, work_reg, work_id, use_id, assigned_id));
              tied_reg->mark_use_done();
              if (tied_reg->is_write()) {
                _cur_assignment.make_dirty(group, work_id, use_id);
              }
              use_pending_count--;
            }
          }
          else {
            if (!(live_regs & use_mask)) {
              ASMJIT_PROPAGATE(on_load_reg(group, work_reg, work_id, use_id));
              tied_reg->mark_use_done();
              if (tied_reg->is_write()) {
                _cur_assignment.make_dirty(group, work_id, use_id);
              }
              use_pending_count--;
            }
          }
          live_regs |= use_mask;
        }
      }
    }
    RegMask clobbered_by_inst = will_use | will_out;
    if (will_free) {
      RegMask allocable_regs = _available_regs[group] & ~(_cur_assignment.assigned(group) | will_free | will_use | will_out);
      Support::BitWordIterator<RegMask> it(will_free);
      do {
        uint32_t assigned_id = it.next();
        if (_cur_assignment.is_phys_assigned(group, assigned_id)) {
          RAWorkId work_id = _cur_assignment.phys_to_work_id(group, assigned_id);
          RAWorkReg* work_reg = work_reg_by_id(work_id);
          if (allocable_regs) {
            uint32_t reassigned_id = decide_on_reassignment(group, work_reg, assigned_id, allocable_regs, ra_inst);
            if (reassigned_id != RAAssignment::kPhysNone) {
              ASMJIT_PROPAGATE(on_move_reg(group, work_reg, work_id, reassigned_id, assigned_id));
              allocable_regs ^= Support::bit_mask<RegMask>(reassigned_id);
              _clobbered_regs[group] |= Support::bit_mask<RegMask>(reassigned_id);
              continue;
            }
          }
          ASMJIT_PROPAGATE(on_spill_reg(group, work_reg, work_id, assigned_id));
        }
      } while (it.has_next());
    }
    if (use_pending_count) {
      bool must_swap = false;
      do {
        uint32_t old_pending_count = use_pending_count;
        for (i = 0; i < count; i++) {
          RATiedReg* this_tied_reg = &tied_regs[i];
          if (this_tied_reg->is_use_done()) {
            continue;
          }
          RAWorkReg* this_work_reg = this_tied_reg->work_reg();
          RAWorkId this_work_id = this_work_reg->work_id();
          uint32_t this_phys_id = _cur_assignment.work_to_phys_id(group, this_work_id);
          uint32_t target_phys_id = this_tied_reg->use_id();
          ASMJIT_ASSERT(target_phys_id != this_phys_id);
          RAWorkId target_work_id = _cur_assignment.phys_to_work_id(group, target_phys_id);
          if (target_work_id != kBadWorkId) {
            RAWorkReg* target_work_reg = work_reg_by_id(target_work_id);
            if (_arch_traits->has_inst_reg_swap(group) && this_phys_id != RAAssignment::kPhysNone) {
              ASMJIT_PROPAGATE(on_swap_reg(group, this_work_reg, this_work_id, this_phys_id, target_work_reg, target_work_id, target_phys_id));
              this_tied_reg->mark_use_done();
              if (this_tied_reg->is_write()) {
                _cur_assignment.make_dirty(group, this_work_id, target_phys_id);
              }
              use_pending_count--;
              RATiedReg* target_tied_reg = RALocal_findTiedReg(tied_regs, count, target_work_reg);
              if (target_tied_reg && target_tied_reg->use_id() == this_phys_id) {
                target_tied_reg->mark_use_done();
                if (target_tied_reg->is_write()) {
                  _cur_assignment.make_dirty(group, target_work_id, this_phys_id);
                }
                use_pending_count--;
              }
              continue;
            }
            if (!must_swap) {
              continue;
            }
            RegMask available_regs = _available_regs[group] & ~_cur_assignment.assigned(group);
            if (available_regs) {
              uint32_t tmp_reg_id = pick_best_suitable_register(group, available_regs);
              ASMJIT_ASSERT(tmp_reg_id != RAAssignment::kPhysNone);
              ASMJIT_PROPAGATE(on_move_reg(group, this_work_reg, this_work_id, tmp_reg_id, this_phys_id));
              _clobbered_regs[group] |= Support::bit_mask<RegMask>(tmp_reg_id);
              break;
            }
            ASMJIT_PROPAGATE(on_spill_reg(group, target_work_reg, target_work_id, target_phys_id));
          }
          if (this_phys_id != RAAssignment::kPhysNone) {
            ASMJIT_PROPAGATE(on_move_reg(group, this_work_reg, this_work_id, target_phys_id, this_phys_id));
            this_tied_reg->mark_use_done();
            if (this_tied_reg->is_write()) {
              _cur_assignment.make_dirty(group, this_work_id, target_phys_id);
            }
            use_pending_count--;
          }
          else {
            ASMJIT_PROPAGATE(on_load_reg(group, this_work_reg, this_work_id, target_phys_id));
            this_tied_reg->mark_use_done();
            if (this_tied_reg->is_write()) {
              _cur_assignment.make_dirty(group, this_work_id, target_phys_id);
            }
            use_pending_count--;
          }
        }
        must_swap = (old_pending_count == use_pending_count);
      } while (use_pending_count);
    }
    uint32_t out_pending_count = out_tied_count;
    if (out_tied_count) {
      for (i = 0; i < out_tied_count; i++) {
        RATiedReg* tied_reg = out_tied_regs[i];
        RAWorkReg* work_reg = tied_reg->work_reg();
        RAWorkId work_id = work_reg->work_id();
        uint32_t phys_id = _cur_assignment.work_to_phys_id(group, work_id);
        if (phys_id != RAAssignment::kPhysNone) {
          _unassign_reg(group, work_id, phys_id);
          will_out &= ~Support::bit_mask<RegMask>(phys_id);
        }
        out_pending_count -= !tied_reg->is_out();
      }
    }
    if (will_out) {
      Support::BitWordIterator<RegMask> it(will_out);
      do {
        uint32_t phys_id = it.next();
        RAWorkId work_id = _cur_assignment.phys_to_work_id(group, phys_id);
        if (work_id == kBadWorkId) {
          continue;
        }
        ASMJIT_PROPAGATE(on_spill_reg(group, work_reg_by_id(work_id), work_id, phys_id));
      } while (it.has_next());
    }
    for (i = 0; i < dup_tied_count; i++) {
      RATiedReg* tied_reg = dup_tied_regs[i];
      RAWorkReg* work_reg = tied_reg->work_reg();
      uint32_t src_id = tied_reg->use_id();
      Support::BitWordIterator<RegMask> it(tied_reg->use_reg_mask());
      while (it.has_next()) {
        uint32_t dst_id = it.next();
        if (dst_id == src_id) {
          continue;
        }
        ASMJIT_PROPAGATE(_pass.emit_move(work_reg, dst_id, src_id));
      }
    }
    if (node->is_invoke() && group == RegGroup::kVec) {
      const InvokeNode* invoke_node = node->as<InvokeNode>();
      RegMask maybe_clobbered_regs = invoke_node->detail().call_conv().preserved_regs(group) & _cur_assignment.assigned(group);
      if (maybe_clobbered_regs) {
        uint32_t save_restore_vec_size = invoke_node->detail().call_conv().save_restore_reg_size(group);
        Support::BitWordIterator<RegMask> it(maybe_clobbered_regs);
        do {
          uint32_t phys_id = it.next();
          RAWorkId work_id = _cur_assignment.phys_to_work_id(group, phys_id);
          RAWorkReg* work_reg = work_reg_by_id(work_id);
          uint32_t virt_size = work_reg->virt_reg()->virt_size();
          if (virt_size > save_restore_vec_size) {
            ASMJIT_PROPAGATE(on_spill_reg(group, work_reg, work_id, phys_id));
          }
        } while (it.has_next());
      }
    }
    if (out_pending_count) {
      RegMask live_regs = _cur_assignment.assigned(group);
      RegMask out_regs = 0;
      RegMask avoid_regs = will_use & ~clobbered_by_inst;
      if (consecutive_count) {
        RATiedReg* lead = consecutive_regs[0];
        if (lead->is_out_consecutive()) {
          uint32_t best_score = 0;
          uint32_t best_lead_reg = 0xFFFFFFFF;
          RegMask allocable_regs = _available_regs[group] & ~(out_regs | avoid_regs);
          Support::BitWordIterator<uint32_t> it(lead->out_reg_mask());
          while (it.has_next()) {
            uint32_t reg_index = it.next();
            if (Support::bit_test(lead->out_reg_mask(), reg_index)) {
              uint32_t score = 15;
              for (i = 0; i < consecutive_count; i++) {
                uint32_t consecutive_index = reg_index + i;
                if (!Support::bit_test(allocable_regs, consecutive_index)) {
                  score = 0;
                  break;
                }
                RAWorkReg* work_reg = consecutive_regs[i]->work_reg();
                score += uint32_t(work_reg->home_reg_id() == consecutive_index);
              }
              if (score > best_score) {
                best_score = score;
                best_lead_reg = reg_index;
              }
            }
          }
          if (best_lead_reg == 0xFFFFFFFF) {
            return make_error(Error::kConsecutiveRegsAllocation);
          }
          for (i = 0; i < consecutive_count; i++) {
            uint32_t consecutive_index = best_lead_reg + i;
            RATiedReg* tied_reg = consecutive_regs[i];
            tied_reg->set_out_id(consecutive_index);
          }
        }
      }
      for (i = 0; i < out_tied_count; i++) {
        RATiedReg* tied_reg = out_tied_regs[i];
        if (!tied_reg->is_out()) {
          continue;
        }
        RegMask avoid_out = avoid_regs;
        if (tied_reg->is_unique()) {
          avoid_out |= will_use;
        }
        RAWorkReg* work_reg = tied_reg->work_reg();
        RAWorkId work_id = work_reg->work_id();
        uint32_t assigned_id = _cur_assignment.work_to_phys_id(group, work_id);
        if (assigned_id != RAAssignment::kPhysNone) {
          _unassign_reg(group, work_id, assigned_id);
        }
        uint32_t phys_id = tied_reg->out_id();
        if (phys_id == RAAssignment::kPhysNone) {
          RegMask allocable_regs = tied_reg->out_reg_mask() & ~(out_regs | avoid_out);
          if (!(allocable_regs & ~live_regs)) {
            RAWorkId spill_work_id;
            phys_id = decide_on_spill_for(group, work_reg, allocable_regs & live_regs, &spill_work_id);
            ASMJIT_PROPAGATE(on_spill_reg(group, work_reg_by_id(spill_work_id), spill_work_id, phys_id));
          }
          else {
            phys_id = decide_on_assignment(group, work_reg, RAAssignment::kPhysNone, allocable_regs & ~live_regs);
          }
        }
        ASMJIT_ASSERT(!_cur_assignment.is_phys_assigned(group, phys_id));
        if (!tied_reg->is_kill()) {
          ASMJIT_PROPAGATE(_assign_reg(group, work_id, phys_id, true));
        }
        tied_reg->set_out_id(phys_id);
        tied_reg->mark_out_done();
        out_regs |= Support::bit_mask<RegMask>(phys_id);
        live_regs &= ~Support::bit_mask<RegMask>(phys_id);
        out_pending_count--;
      }
      clobbered_by_inst |= out_regs;
      ASMJIT_ASSERT(out_pending_count == 0);
    }
    _clobbered_regs[group] |= clobbered_by_inst;
  }
  return Error::kOk;
}
Error RALocalAllocator::spill_after_allocation(InstNode* node) noexcept {
  RAInst* ra_inst = node->pass_data<RAInst>();
  uint32_t count = ra_inst->tied_count();
  for (uint32_t i = 0; i < count; i++) {
    RATiedReg* tied_reg = ra_inst->tied_at(i);
    if (tied_reg->is_last()) {
      RAWorkReg* work_reg = tied_reg->work_reg();
      if (!work_reg->has_home_reg_id()) {
        RAWorkId work_id = work_reg->work_id();
        RegGroup group = work_reg->group();
        uint32_t assigned_id = _cur_assignment.work_to_phys_id(group, work_id);
        if (assigned_id != RAAssignment::kPhysNone) {
          _cc.set_cursor(node);
          ASMJIT_PROPAGATE(on_spill_reg(group, work_reg, work_id, assigned_id));
        }
      }
    }
  }
  return Error::kOk;
}
Error RALocalAllocator::alloc_branch(InstNode* node, RABlock* target, RABlock* cont) noexcept {
  Support::maybe_unused(cont);
  _cc.set_cursor(node->prev());
  if (target->has_entry_assignment()) {
    ASMJIT_PROPAGATE(switch_to_assignment(target->entry_phys_to_work_map(), target->live_in(), target->is_allocated(), true));
  }
  ASMJIT_PROPAGATE(alloc_instruction(node));
  ASMJIT_PROPAGATE(spill_regs_before_entry(target));
  if (target->has_entry_assignment()) {
    BaseNode* injection_point = _pass._injection_end->prev();
    BaseNode* prev_cursor = _cc.set_cursor(injection_point);
    _tmp_assignment.copy_from(_cur_assignment);
    ASMJIT_PROPAGATE(switch_to_assignment(target->entry_phys_to_work_map(), target->live_in(), target->is_allocated(), false));
    BaseNode* cur_cursor = _cc.cursor();
    if (cur_cursor != injection_point) {
      Operand& target_op = node->op(node->op_count() - 1);
      if (ASMJIT_UNLIKELY(!target_op.is_label())) {
        return make_error(Error::kInvalidState);
      }
      Label trampoline = _cc.new_label();
      Label saved_target = target_op.as<Label>();
      target_op = trampoline;
      node->clear_options(InstOptions::kShortForm);
      ASMJIT_PROPAGATE(_pass.emit_jump(saved_target));
      _cc.set_cursor(injection_point);
      _cc.bind(trampoline);
      if (_pass._injection_start == nullptr) {
        _pass._injection_start = injection_point->next();
      }
    }
    _cc.set_cursor(prev_cursor);
    _cur_assignment.swap(_tmp_assignment);
  }
  else {
    ASMJIT_PROPAGATE(_pass.set_block_entry_assignment(target, block(), _cur_assignment));
  }
  return Error::kOk;
}
Error RALocalAllocator::alloc_jump_table(InstNode* node, Span<RABlock*> targets, RABlock* cont) noexcept {
  Support::maybe_unused(cont);
  if (targets.is_empty()) {
    return make_error(Error::kInvalidState);
  }
  _cc.set_cursor(node->prev());
  RABlock* any_target = targets[0];
  if (!any_target->has_shared_assignment_id()) {
    return make_error(Error::kInvalidState);
  }
  RASharedAssignment& shared_assignment = _pass._shared_assignments[any_target->shared_assignment_id()];
  ASMJIT_PROPAGATE(alloc_instruction(node));
  if (!shared_assignment.is_empty()) {
    ASMJIT_PROPAGATE(switch_to_assignment(
      shared_assignment.phys_to_work_map(),
      shared_assignment.live_in(),
      true,
      false
    ));
  }
  ASMJIT_PROPAGATE(spill_regs_before_entry(any_target));
  if (shared_assignment.is_empty()) {
    ASMJIT_PROPAGATE(_pass.set_block_entry_assignment(any_target, block(), _cur_assignment));
  }
  return Error::kOk;
}
uint32_t RALocalAllocator::decide_on_assignment(RegGroup group, RAWorkReg* work_reg, uint32_t phys_id, RegMask allocable_regs) const noexcept {
  ASMJIT_ASSERT(allocable_regs != 0);
  Support::maybe_unused(group, phys_id);
  if (work_reg->has_home_reg_id()) {
    uint32_t home_id = work_reg->home_reg_id();
    if (Support::bit_test(allocable_regs, home_id)) {
      return home_id;
    }
  }
  RegMask previously_assigned_regs = work_reg->allocated_mask();
  if (allocable_regs & previously_assigned_regs) {
    allocable_regs &= previously_assigned_regs;
  }
  return pick_best_suitable_register(group, allocable_regs);
}
uint32_t RALocalAllocator::decide_on_reassignment(RegGroup group, RAWorkReg* work_reg, uint32_t phys_id, RegMask allocable_regs, RAInst* ra_inst) const noexcept {
  ASMJIT_ASSERT(allocable_regs != 0);
  Support::maybe_unused(phys_id);
  if (work_reg->has_home_reg_id()) {
    if (Support::bit_test(allocable_regs, work_reg->home_reg_id())) {
      return work_reg->home_reg_id();
    }
  }
  const RATiedReg* tied_reg = ra_inst->tied_reg_for_work_reg(group, work_reg);
  if (tied_reg && tied_reg->is_out_or_kill()) {
    return Support::ctz(allocable_regs);
  }
  if (work_reg->is_within_single_basic_block()) {
    RegMask filtered_regs = allocable_regs & ~work_reg->clobber_survival_mask();
    if (filtered_regs) {
      return pick_best_suitable_register(group, filtered_regs);
    }
  }
  return RAAssignment::kPhysNone;
}
uint32_t RALocalAllocator::decide_on_spill_for(RegGroup group, RAWorkReg* work_reg, RegMask spillable_regs, RAWorkId* spill_work_id) const noexcept {
  Support::maybe_unused(work_reg);
  ASMJIT_ASSERT(spillable_regs != 0);
  Support::BitWordIterator<RegMask> it(spillable_regs);
  uint32_t best_phys_id = it.next();
  RAWorkId best_work_id = _cur_assignment.phys_to_work_id(group, best_phys_id);
  if (it.has_next()) {
    uint32_t best_cost = calc_spill_cost(group, work_reg_by_id(best_work_id), best_phys_id);
    do {
      uint32_t local_phys_id = it.next();
      RAWorkId local_work_id = _cur_assignment.phys_to_work_id(group, local_phys_id);
      uint32_t local_cost = calc_spill_cost(group, work_reg_by_id(local_work_id), local_phys_id);
      if (local_cost < best_cost) {
        best_cost = local_cost;
        best_phys_id = local_phys_id;
        best_work_id = local_work_id;
      }
    } while (it.has_next());
  }
  *spill_work_id = best_work_id;
  return best_phys_id;
}
ASMJIT_END_NAMESPACE
#endif
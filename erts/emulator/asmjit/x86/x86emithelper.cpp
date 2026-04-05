#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_X86)
#include <asmjit/core/formatter.h>
#include <asmjit/core/funcargscontext_p.h>
#include <asmjit/core/string.h>
#include <asmjit/core/type.h>
#include <asmjit/core/radefs_p.h>
#include <asmjit/x86/x86emithelper_p.h>
#include <asmjit/x86/x86emitter.h>
#include <asmjit/x86/x86formatter_p.h>
#include <asmjit/x86/x86instapi_p.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_SUB_NAMESPACE(x86)
static constexpr OperandSignature reg_size_to_gp_signature_table[8 + 1] = {
  OperandSignature{0},
  OperandSignature{RegTraits<RegType::kGp8Lo>::kSignature},
  OperandSignature{RegTraits<RegType::kGp16>::kSignature},
  OperandSignature{0},
  OperandSignature{RegTraits<RegType::kGp32>::kSignature},
  OperandSignature{0},
  OperandSignature{0},
  OperandSignature{0},
  OperandSignature{RegTraits<RegType::kGp64>::kSignature}
};
[[nodiscard]]
static inline uint32_t get_xmm_mov_inst(const FuncFrame& frame) {
  bool avx = frame.is_avx_enabled();
  bool aligned = frame.has_aligned_vec_save_restore();
  return aligned ? (avx ? Inst::kIdVmovaps : Inst::kIdMovaps)
                 : (avx ? Inst::kIdVmovups : Inst::kIdMovups);
}
[[nodiscard]]
static inline uint32_t kmovInstFromSize(uint32_t size) noexcept {
  switch (size) {
    case  1: return Inst::kIdKmovb;
    case  2: return Inst::kIdKmovw;
    case  4: return Inst::kIdKmovd;
    case  8: return Inst::kIdKmovq;
    default: return Inst::kIdNone;
  }
}
[[nodiscard]]
static inline uint32_t make_cast_op(TypeId dst, TypeId src) noexcept {
  return (uint32_t(dst) << 8) | uint32_t(src);
}
const EmitHelperInstructionIds _emit_helper_instruction_ids[2] = {
  {
    { Inst::kIdMovd, Inst::kIdMovss },
    { Inst::kIdMovq, Inst::kIdMovsd },
    { Inst::kIdMovups, Inst::kIdMovaps },
    { Inst::kIdMovupd, Inst::kIdMovapd },
    { Inst::kIdMovdqu, Inst::kIdMovdqa },
    { Inst::kIdMovlps },
    { Inst::kIdCvtss2sd, Inst::kIdCvtsd2ss },
    { Inst::kIdCvtps2pd, Inst::kIdCvtpd2ps }
  },
  {
    { Inst::kIdVmovd, Inst::kIdVmovss },
    { Inst::kIdVmovq, Inst::kIdVmovsd },
    { Inst::kIdVmovups, Inst::kIdVmovaps },
    { Inst::kIdVmovupd, Inst::kIdVmovapd },
    { Inst::kIdVmovdqu, Inst::kIdVmovdqa },
    { Inst::kIdVmovlps },
    { Inst::kIdVcvtss2sd, Inst::kIdVcvtsd2ss },
    { Inst::kIdVcvtps2pd, Inst::kIdVcvtpd2ps }
  }
};
ASMJIT_FAVOR_SIZE Error EmitHelper::emit_reg_move(
  const Operand_& dst_,
  const Operand_& src_, TypeId type_id, const char* comment) {
  ASMJIT_ASSERT(TypeUtils::is_valid(type_id) && !TypeUtils::is_abstract(type_id));
  Operand dst(dst_);
  Operand src(src_);
  InstId inst_id = Inst::kIdNone;
  uint32_t mem_flags = 0;
  uint32_t override_mem_size = 0;
  enum MemFlags : uint32_t {
    kDstMem = 0x1,
    kSrcMem = 0x2
  };
  if (dst.is_mem()) { mem_flags |= kDstMem; dst.as<Mem>().set_size(src.as<Mem>().size()); }
  if (src.is_mem()) { mem_flags |= kSrcMem; src.as<Mem>().set_size(dst.as<Mem>().size()); }
  switch (type_id) {
    case TypeId::kInt8:
    case TypeId::kUInt8:
    case TypeId::kInt16:
    case TypeId::kUInt16:
      if (mem_flags & kSrcMem) {
        inst_id = Inst::kIdMovzx;
        dst.set_signature(Reg::signature_of_t<RegType::kGp32>());
        break;
      }
      if (!mem_flags) {
        dst.set_signature(Reg::signature_of_t<RegType::kGp32>());
        src.set_signature(Reg::signature_of_t<RegType::kGp32>());
      }
      [[fallthrough]];
    case TypeId::kInt32:
    case TypeId::kUInt32:
    case TypeId::kInt64:
    case TypeId::kUInt64:
      inst_id = Inst::kIdMov;
      break;
    case TypeId::kMmx32:
      inst_id = Inst::kIdMovd;
      if (mem_flags) break;
      [[fallthrough]];
    case TypeId::kMmx64 : inst_id = Inst::kIdMovq ; break;
    case TypeId::kMask8 : inst_id = Inst::kIdKmovb; break;
    case TypeId::kMask16: inst_id = Inst::kIdKmovw; break;
    case TypeId::kMask32: inst_id = Inst::kIdKmovd; break;
    case TypeId::kMask64: inst_id = Inst::kIdKmovq; break;
    default: {
      TypeId scalar_type_id = TypeUtils::scalar_of(type_id);
      if (TypeUtils::is_vec32(type_id) && mem_flags) {
        inst_id = ids().movd_or_movss(scalar_type_id == TypeId::kFloat32);
        override_mem_size = 4;
        break;
      }
      if (TypeUtils::is_vec64(type_id) && mem_flags) {
        inst_id = ids().movq_or_movsd(scalar_type_id == TypeId::kFloat64);
        override_mem_size = 8;
        break;
      }
      if (scalar_type_id == TypeId::kFloat32) {
        inst_id = ids().movaps();
      }
      else if (scalar_type_id == TypeId::kFloat64) {
        inst_id = ids().movapd();
      }
      else if (!_avx512_enabled) {
        inst_id = ids().movdqa();
      }
      else {
        inst_id = Inst::kIdVmovdqa32;
      }
      break;
    }
  }
  if (!inst_id) {
    return make_error(Error::kInvalidState);
  }
  if (override_mem_size) {
    if (dst.is_mem()) {
      dst.as<Mem>().set_size(override_mem_size);
    }
    if (src.is_mem()) {
      src.as<Mem>().set_size(override_mem_size);
    }
  }
  _emitter->set_inline_comment(comment);
  return _emitter->emit(inst_id, dst, src);
}
ASMJIT_FAVOR_SIZE Error EmitHelper::emit_arg_move(
  const Reg& dst_, TypeId dst_type_id,
  const Operand_& src_, TypeId src_type_id, const char* comment) {
  if (dst_type_id == TypeId::kVoid) {
    dst_type_id = RegUtils::type_id_of(dst_.reg_type());
  }
  ASMJIT_ASSERT(TypeUtils::is_valid(dst_type_id) && !TypeUtils::is_abstract(dst_type_id));
  ASMJIT_ASSERT(TypeUtils::is_valid(src_type_id) && !TypeUtils::is_abstract(src_type_id));
  Reg dst(dst_.as<Reg>());
  Operand src(src_);
  uint32_t dst_size = TypeUtils::size_of(dst_type_id);
  uint32_t src_size = TypeUtils::size_of(src_type_id);
  InstId inst_id = Inst::kIdNone;
  for (;;) {
    if (TypeUtils::is_int(dst_type_id)) {
      if (TypeUtils::is_int(src_type_id)) {
        uint32_t cast_op = make_cast_op(dst_type_id, src_type_id);
        if (cast_op == make_cast_op(TypeId::kInt16, TypeId::kInt8 ) ||
            cast_op == make_cast_op(TypeId::kInt32, TypeId::kInt8 ) ||
            cast_op == make_cast_op(TypeId::kInt64, TypeId::kInt8 ) ||
            cast_op == make_cast_op(TypeId::kInt32, TypeId::kInt16) ||
            cast_op == make_cast_op(TypeId::kInt64, TypeId::kInt16) ||
            cast_op == make_cast_op(TypeId::kInt64, TypeId::kInt32)) {
          inst_id = (cast_op == make_cast_op(TypeId::kInt64, TypeId::kInt32)) ? Inst::kIdMovsxd : Inst::kIdMovsx;
          dst.set_signature(reg_size_to_gp_signature_table[dst_size]);
          if (src.is_reg()) {
            src.set_signature(reg_size_to_gp_signature_table[src_size]);
          }
          break;
        }
      }
      if (TypeUtils::is_int(src_type_id) || src_.is_mem()) {
        uint32_t mov_size = Support::min(src_size, dst_size);
        if (mov_size <= 4) {
          dst_size = 4;
        }
        inst_id = mov_size < 4 ? Inst::kIdMovzx : Inst::kIdMov;
        src_size = Support::min(src_size, mov_size);
        dst.set_signature(reg_size_to_gp_signature_table[dst_size]);
        if (src.is_reg()) {
          src.set_signature(reg_size_to_gp_signature_table[src_size]);
        }
        break;
      }
      src_size = Support::min(src_size, dst_size);
      if (TypeUtils::is_mmx(src_type_id)) {
        inst_id = Inst::kIdMovq;
        if (src_size == 8) {
          break;
        }
        inst_id = Inst::kIdMovd;
        dst.set_signature(Reg::signature_of_t<RegType::kGp32>());
        break;
      }
      if (TypeUtils::is_mask(src_type_id)) {
        inst_id = kmovInstFromSize(src_size);
        dst.set_signature(src_size <= 4 ? Reg::signature_of_t<RegType::kGp32>()
                                      : Reg::signature_of_t<RegType::kGp64>());
        break;
      }
      if (TypeUtils::is_vec(src_type_id)) {
        inst_id = ids().movq();
        if (src_size == 8) {
          break;
        }
        inst_id = ids().movd();
        dst.set_signature(Reg::signature_of_t<RegType::kGp32>());
        break;
      }
    }
    if (TypeUtils::is_mmx(dst_type_id)) {
      inst_id = Inst::kIdMovq;
      src_size = Support::min(src_size, dst_size);
      if (TypeUtils::is_int(src_type_id) || src.is_mem()) {
        if (src_size == 8) {
          break;
        }
        inst_id = Inst::kIdMovd;
        if (src.is_reg()) {
          src.set_signature(Reg::signature_of_t<RegType::kGp32>());
        }
        break;
      }
      if (TypeUtils::is_mmx(src_type_id)) {
        break;
      }
      inst_id = Inst::kIdMovdq2q;
      if (TypeUtils::is_vec(src_type_id)) {
        break;
      }
    }
    if (TypeUtils::is_mask(dst_type_id)) {
      src_size = Support::min(src_size, dst_size);
      if (TypeUtils::is_int(src_type_id) || TypeUtils::is_mask(src_type_id) || src.is_mem()) {
        inst_id = kmovInstFromSize(src_size);
        if (src.is_gp() && src_size <= 4) {
          src.set_signature(Reg::signature_of_t<RegType::kGp32>());
        }
        break;
      }
    }
    if (TypeUtils::is_vec(dst_type_id)) {
      dst.set_signature(Reg::signature_of_t<RegType::kVec128>());
      if (src.is_mm_reg()) {
        inst_id = Inst::kIdMovq2dq;
        break;
      }
      TypeId dst_scalar_id = TypeUtils::scalar_of(dst_type_id);
      TypeId src_scalar_id = TypeUtils::scalar_of(src_type_id);
      if (dst_scalar_id == TypeId::kFloat32 && src_scalar_id == TypeId::kFloat64) {
        src_size = Support::min(dst_size * 2, src_size);
        dst_size = src_size / 2;
        inst_id = (src_size <= 8) ? ids().cvtss2sd() : ids().cvtps2pd();
        if (dst_size == 32) {
          dst.set_signature(Reg::signature_of_t<RegType::kVec256>());
        }
        if (src.is_reg()) {
          src.set_signature(RegUtils::signature_of_vec_by_size(src_size));
        }
        break;
      }
      if (dst_scalar_id == TypeId::kFloat64 && src_scalar_id == TypeId::kFloat32) {
        src_size = Support::min(dst_size, src_size * 2) / 2;
        dst_size = src_size * 2;
        inst_id = (src_size <= 4) ? ids().cvtsd2ss() : ids().cvtpd2ps();
        dst.set_signature(RegUtils::signature_of_vec_by_size(dst_size));
        if (src.is_reg() && src_size >= 32) {
          src.set_signature(Reg::signature_of_t<RegType::kVec256>());
        }
        break;
      }
      src_size = Support::min(src_size, dst_size);
      if (src.is_gp() || src.is_mem()) {
        if (src_size <= 4) {
          inst_id = ids().movd();
          if (src.is_reg()) {
            src.set_signature(Reg::signature_of_t<RegType::kGp32>());
          }
          break;
        }
        if (src_size == 8) {
          inst_id = ids().movq();
          break;
        }
      }
      if (src.is_vec() || src.is_mem()) {
        inst_id = ids().movups_or_movaps(!Support::bool_and(src.is_mem(), src_size < _emitter->environment().stack_alignment()));
        OperandSignature signature = RegUtils::signature_of_vec_by_size(src_size);
        dst.set_signature(signature);
        if (src.is_reg()) {
          src.set_signature(signature);
        }
        break;
      }
    }
    return make_error(Error::kInvalidState);
  }
  if (src.is_mem())
    src.as<Mem>().set_size(src_size);
  _emitter->set_inline_comment(comment);
  return _emitter->emit(inst_id, dst, src);
}
Error EmitHelper::emit_reg_swap(
  const Reg& a,
  const Reg& b, const char* comment) {
  if (a.is_gp() && b.is_gp()) {
    _emitter->set_inline_comment(comment);
    return _emitter->emit(Inst::kIdXchg, a, b);
  }
  else {
    return make_error(Error::kInvalidState);
  }
}
static inline Error X86Internal_setup_save_restore_info(RegGroup group, const FuncFrame& frame, Out<Reg> reg_out, Out<uint32_t> inst_out, Out<uint32_t> size_out) noexcept {
  switch (group) {
    case RegGroup::kVec:
      reg_out = xmm(0);
      inst_out = get_xmm_mov_inst(frame);
      size_out = reg_out->size();
      return Error::kOk;
    case RegGroup::kMask:
      reg_out = k(0);
      inst_out = Inst::kIdKmovq;
      size_out = reg_out->size();
      return Error::kOk;
    case RegGroup::kX86_MM:
      reg_out = mm(0);
      inst_out = Inst::kIdMovq;
      size_out = reg_out->size();
      return Error::kOk;
    default:
      return make_error(Error::kInvalidState);
  }
}
ASMJIT_FAVOR_SIZE Error EmitHelper::emit_prolog(const FuncFrame& frame) {
  Emitter* emitter = _emitter->as<Emitter>();
  uint32_t gp_saved = frame.saved_regs(RegGroup::kGp);
  Gp zsp = emitter->zsp();
  Gp zbp = emitter->zbp();
  Gp gp_reg = zsp;
  Gp sa_reg = zsp;
  if (frame.has_indirect_branch_protection()) {
    InstId inst_id = emitter->is_32bit() ? Inst::kIdEndbr32 : Inst::kIdEndbr64;
    ASMJIT_PROPAGATE(emitter->emit(inst_id));
  }
  if (frame.has_preserved_fp()) {
    gp_saved &= ~Support::bit_mask<RegMask>(Gp::kIdBp);
    ASMJIT_PROPAGATE(emitter->push(zbp));
    ASMJIT_PROPAGATE(emitter->mov(zbp, zsp));
  }
  {
    Support::BitWordIterator<RegMask> it(gp_saved);
    while (it.has_next()) {
      gp_reg.set_id(it.next());
      ASMJIT_PROPAGATE(emitter->push(gp_reg));
    }
  }
  uint32_t sa_reg_id = frame.sa_reg_id();
  if (sa_reg_id != Reg::kIdBad && sa_reg_id != Gp::kIdSp) {
    sa_reg.set_id(sa_reg_id);
    if (frame.has_preserved_fp()) {
      if (sa_reg_id != Gp::kIdBp) {
        ASMJIT_PROPAGATE(emitter->mov(sa_reg, zbp));
      }
    }
    else {
      ASMJIT_PROPAGATE(emitter->mov(sa_reg, zsp));
    }
  }
  if (frame.has_dynamic_alignment()) {
    ASMJIT_PROPAGATE(emitter->and_(zsp, -int32_t(frame.final_stack_alignment())));
  }
  if (frame.has_stack_adjustment()) {
    ASMJIT_PROPAGATE(emitter->sub(zsp, frame.stack_adjustment()));
  }
  if (frame.has_dynamic_alignment() && frame.has_da_offset()) {
    Mem sa_mem = ptr(zsp, int32_t(frame.da_offset()));
    ASMJIT_PROPAGATE(emitter->mov(sa_mem, sa_reg));
  }
  {
    Mem x_base = ptr(zsp, int32_t(frame.extra_reg_save_offset()));
    for (RegGroup group : Support::enumerate(RegGroup(1), RegGroup::kMaxVirt)) {
      Support::BitWordIterator<RegMask> it(frame.saved_regs(group));
      if (it.has_next()) {
        Reg x_reg;
        uint32_t x_inst_id = 0;
        uint32_t x_size = 0;
        ASMJIT_PROPAGATE(X86Internal_setup_save_restore_info(group, frame, Out(x_reg), Out(x_inst_id), Out(x_size)));
        do {
          x_reg.set_id(it.next());
          ASMJIT_PROPAGATE(emitter->emit(x_inst_id, x_base, x_reg));
          x_base.add_offset_lo32(int32_t(x_size));
        } while (it.has_next());
      }
    }
  }
  return Error::kOk;
}
ASMJIT_FAVOR_SIZE Error EmitHelper::emit_epilog(const FuncFrame& frame) {
  Emitter* emitter = _emitter->as<Emitter>();
  uint32_t register_size = emitter->register_size();
  uint32_t gp_saved = frame.saved_regs(RegGroup::kGp);
  Gp zsp = emitter->zsp();
  Gp zbp = emitter->zbp();
  Gp gp_reg = emitter->zsp();
  if (frame.has_preserved_fp()) {
    gp_saved &= ~Support::bit_mask<RegMask>(Gp::kIdBp);
  }
  {
    Mem x_base = ptr(zsp, int32_t(frame.extra_reg_save_offset()));
    for (RegGroup group : Support::enumerate(RegGroup(1), RegGroup::kMaxVirt)) {
      Support::BitWordIterator<RegMask> it(frame.saved_regs(group));
      if (it.has_next()) {
        Reg x_reg;
        uint32_t x_inst_id;
        uint32_t x_size;
        ASMJIT_PROPAGATE(X86Internal_setup_save_restore_info(group, frame, Out(x_reg), Out(x_inst_id), Out(x_size)));
        do {
          x_reg.set_id(it.next());
          ASMJIT_PROPAGATE(emitter->emit(x_inst_id, x_reg, x_base));
          x_base.add_offset_lo32(int32_t(x_size));
        } while (it.has_next());
      }
    }
  }
  bool do_mmx_cleanup = frame.has_mmx_cleanup();
  bool do_avx_cleanup = frame.has_avx_cleanup();
  if (frame.has_avx_auto_cleanup() && frame.dirty_regs(RegGroup::kVec) != 0u) {
    do_avx_cleanup = true;
  }
  if (do_mmx_cleanup) {
    ASMJIT_PROPAGATE(emitter->emms());
  }
  if (do_avx_cleanup) {
    ASMJIT_PROPAGATE(emitter->vzeroupper());
  }
  if (frame.has_preserved_fp()) {
    int32_t count = int32_t(frame.push_pop_save_size() - register_size);
    if (!count) {
      ASMJIT_PROPAGATE(emitter->mov(zsp, zbp));
    }
    else {
      ASMJIT_PROPAGATE(emitter->lea(zsp, ptr(zbp, -count)));
    }
  }
  else {
    if (frame.has_dynamic_alignment() && frame.has_da_offset()) {
      Mem sa_mem = ptr(zsp, int32_t(frame.da_offset()));
      ASMJIT_PROPAGATE(emitter->mov(zsp, sa_mem));
    }
    else if (frame.has_stack_adjustment()) {
      ASMJIT_PROPAGATE(emitter->add(zsp, int32_t(frame.stack_adjustment())));
    }
  }
  if (gp_saved) {
    uint32_t i = gp_saved;
    uint32_t reg_id = 16;
    do {
      reg_id--;
      if (i & 0x8000) {
        gp_reg.set_id(reg_id);
        ASMJIT_PROPAGATE(emitter->pop(gp_reg));
      }
      i <<= 1;
    } while (reg_id != 0);
  }
  if (frame.has_preserved_fp()) {
    ASMJIT_PROPAGATE(emitter->pop(zbp));
  }
  if (frame.has_callee_stack_cleanup()) {
    ASMJIT_PROPAGATE(emitter->emit(Inst::kIdRet, int(frame.callee_stack_cleanup())));
  }
  else {
    ASMJIT_PROPAGATE(emitter->emit(Inst::kIdRet));
  }
  return Error::kOk;
}
static Error ASMJIT_CDECL Emitter_emitProlog(BaseEmitter* emitter, const FuncFrame& frame) {
  EmitHelper emit_helper(emitter, frame.is_avx_enabled(), frame.is_avx512_enabled());
  return emit_helper.emit_prolog(frame);
}
static Error ASMJIT_CDECL Emitter_emitEpilog(BaseEmitter* emitter, const FuncFrame& frame) {
  EmitHelper emit_helper(emitter, frame.is_avx_enabled(), frame.is_avx512_enabled());
  return emit_helper.emit_epilog(frame);
}
static Error ASMJIT_CDECL Emitter_emitArgsAssignment(BaseEmitter* emitter, const FuncFrame& frame, const FuncArgsAssignment& args) {
  EmitHelper emit_helper(emitter, frame.is_avx_enabled(), frame.is_avx512_enabled());
  return emit_helper.emit_args_assignment(frame, args);
}
void init_emitter_funcs(BaseEmitter* emitter) noexcept {
  emitter->_funcs.emit_prolog = Emitter_emitProlog;
  emitter->_funcs.emit_epilog = Emitter_emitEpilog;
  emitter->_funcs.emit_args_assignment = Emitter_emitArgsAssignment;
#ifndef ASMJIT_NO_LOGGING
  emitter->_funcs.format_instruction = FormatterInternal::format_instruction;
#endif
}
ASMJIT_END_SUB_NAMESPACE
#endif
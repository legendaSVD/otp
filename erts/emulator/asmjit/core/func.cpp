#include <asmjit/core/api-build_p.h>
#include <asmjit/core/archtraits.h>
#include <asmjit/core/func.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
#include <asmjit/core/funcargscontext_p.h>
#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86/x86func_p.h>
#endif
#if !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/arm/a64func_p.h>
#endif
ASMJIT_BEGIN_NAMESPACE
ASMJIT_FAVOR_SIZE Error CallConv::init(CallConvId call_conv_id, const Environment& environment) noexcept {
  reset();
#if !defined(ASMJIT_NO_X86)
  if (environment.is_family_x86()) {
    return x86::FuncInternal::init_call_conv(*this, call_conv_id, environment);
  }
#endif
#if !defined(ASMJIT_NO_AARCH64)
  if (environment.is_family_aarch64()) {
    return a64::FuncInternal::init_call_conv(*this, call_conv_id, environment);
  }
#endif
  return make_error(Error::kInvalidArgument);
}
ASMJIT_FAVOR_SIZE Error FuncDetail::init(const FuncSignature& signature, const Environment& environment) noexcept {
  CallConvId call_conv_id = signature.call_conv_id();
  uint32_t arg_count = signature.arg_count();
  if (ASMJIT_UNLIKELY(arg_count > Globals::kMaxFuncArgs)) {
    return make_error(Error::kInvalidArgument);
  }
  CallConv& cc = _call_conv;
  ASMJIT_PROPAGATE(cc.init(call_conv_id, environment));
  uint32_t register_size = Environment::reg_size_of_arch(cc.arch());
  uint32_t deabstract_delta = TypeUtils::deabstract_delta_of_size(register_size);
  const TypeId* signature_args = signature.args();
  for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
    FuncValuePack& arg_pack = _args[arg_index];
    arg_pack[0].init_type_id(TypeUtils::deabstract(signature_args[arg_index], deabstract_delta));
  }
  _arg_count = uint8_t(arg_count);
  _va_index = uint8_t(signature.va_index());
  TypeId ret = signature.ret();
  if (ret != TypeId::kVoid) {
    _rets[0].init_type_id(TypeUtils::deabstract(ret, deabstract_delta));
  }
#if !defined(ASMJIT_NO_X86)
  if (environment.is_family_x86()) {
    return x86::FuncInternal::init_func_detail(*this, signature, register_size);
  }
#endif
#if !defined(ASMJIT_NO_AARCH64)
  if (environment.is_family_aarch64()) {
    return a64::FuncInternal::init_func_detail(*this, signature);
  }
#endif
  return make_error(Error::kInvalidArgument);
}
ASMJIT_FAVOR_SIZE Error FuncFrame::init(const FuncDetail& func) noexcept {
  Arch arch = func.call_conv().arch();
  if (!Environment::is_valid_arch(arch)) {
    return make_error(Error::kInvalidArch);
  }
  const ArchTraits& arch_traits = ArchTraits::by_arch(arch);
  reset();
  _arch = arch;
  _sp_reg_id = uint8_t(arch_traits.sp_reg_id());
  _sa_reg_id = uint8_t(Reg::kIdBad);
  uint32_t natural_stack_alignment = func.call_conv().natural_stack_alignment();
  uint32_t min_dynamic_alignment = Support::max<uint32_t>(natural_stack_alignment, 16);
  if (min_dynamic_alignment == natural_stack_alignment) {
    min_dynamic_alignment <<= 1;
  }
  _natural_stack_alignment = uint8_t(natural_stack_alignment);
  _min_dynamic_alignment = uint8_t(min_dynamic_alignment);
  _red_zone_size = uint8_t(func.red_zone_size());
  _spill_zone_size = uint8_t(func.spill_zone_size());
  _final_stack_alignment = uint8_t(_natural_stack_alignment);
  if (func.has_flag(CallConvFlags::kCalleePopsStack)) {
    _callee_stack_cleanup = uint16_t(func.arg_stack_size());
  }
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    _dirty_regs[group] = func.used_regs(group);
    _preserved_regs[group] = func.preserved_regs(group);
  }
  _preserved_regs[RegGroup::kGp] &= ~Support::bit_mask<RegMask>(arch_traits.sp_reg_id());
  _save_restore_reg_size = func.call_conv()._save_restore_reg_size;
  _save_restore_alignment = func.call_conv()._save_restore_alignment;
  return Error::kOk;
}
ASMJIT_FAVOR_SIZE Error FuncFrame::finalize() noexcept {
  if (!Environment::is_valid_arch(arch())) {
    return make_error(Error::kInvalidArch);
  }
  const ArchTraits& arch_traits = ArchTraits::by_arch(arch());
  uint32_t register_size = _save_restore_reg_size[RegGroup::kGp];
  uint32_t vector_size = _save_restore_reg_size[RegGroup::kVec];
  uint32_t return_address_size = arch_traits.has_link_reg() ? 0u : register_size;
  uint32_t stack_alignment = _final_stack_alignment;
  ASMJIT_ASSERT(stack_alignment == Support::max(_natural_stack_alignment, _call_stack_alignment, _local_stack_alignment));
  bool has_fp = has_preserved_fp();
  bool has_da = has_dynamic_alignment();
  uint32_t kSp = arch_traits.sp_reg_id();
  uint32_t kFp = arch_traits.fp_reg_id();
  uint32_t kLr = arch_traits.link_reg_id();
  if (has_fp) {
    _dirty_regs[RegGroup::kGp] |= Support::bit_mask<RegMask>(kFp);
    if (kLr != Reg::kIdBad) {
      _dirty_regs[RegGroup::kGp] |= Support::bit_mask<RegMask>(kLr);
    }
  }
  uint32_t sa_reg_id = _sa_reg_id;
  if (sa_reg_id == Reg::kIdBad) {
    sa_reg_id = kSp;
  }
  if (has_da && sa_reg_id == kSp) {
    sa_reg_id = kFp;
  }
  if (sa_reg_id != kSp) {
    _dirty_regs[RegGroup::kGp] |= Support::bit_mask<RegMask>(sa_reg_id);
  }
  _sp_reg_id = uint8_t(kSp);
  _sa_reg_id = uint8_t(sa_reg_id);
  uint32_t save_restore_sizes[2] {};
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    save_restore_sizes[size_t(!arch_traits.has_inst_push_pop(group))]
      += Support::align_up(Support::popcnt(saved_regs(group)) * save_restore_reg_size(group), save_restore_alignment(group));
  }
  _push_pop_save_size  = uint16_t(save_restore_sizes[0]);
  _extra_reg_save_size = uint16_t(save_restore_sizes[1]);
  uint32_t v = 0;
  v += call_stack_size();
  v  = Support::align_up(v, stack_alignment);
  _local_stack_offset = v;
  v += local_stack_size();
  if (stack_alignment >= vector_size && _extra_reg_save_size) {
    add_attributes(FuncAttributes::kAlignedVecSR);
    v = Support::align_up(v, vector_size);
  }
  _extra_reg_save_offset = v;
  v += _extra_reg_save_size;
  if (has_da && !has_fp) {
    _da_offset = v;
    v += register_size;
  }
  else {
    _da_offset = FuncFrame::kTagInvalidOffset;
  }
  if (v || has_func_calls() || !return_address_size) {
    v += Support::align_up_diff(v + push_pop_save_size() + return_address_size, stack_alignment);
  }
  _push_pop_save_offset = v;
  _stack_adjustment = v;
  v += _push_pop_save_size;
  _final_stack_size = v;
  if (!arch_traits.has_link_reg()) {
    v += register_size;
  }
  if (has_da) {
    _stack_adjustment = Support::align_up(_stack_adjustment, stack_alignment);
  }
  _sa_offset_from_sp = has_da ? FuncFrame::kTagInvalidOffset : v;
  _sa_offset_from_sa = has_fp ? return_address_size + register_size
                          : return_address_size + _push_pop_save_size;
  return Error::kOk;
}
ASMJIT_FAVOR_SIZE Error FuncArgsAssignment::update_func_frame(FuncFrame& frame) const noexcept {
  Arch arch = frame.arch();
  const FuncDetail* func = func_detail();
  if (!func) {
    return make_error(Error::kInvalidState);
  }
  RAConstraints constraints;
  ASMJIT_PROPAGATE(constraints.init(arch));
  FuncArgsContext ctx;
  ASMJIT_PROPAGATE(ctx.init_work_data(frame, *this, &constraints));
  ASMJIT_PROPAGATE(ctx.mark_dst_regs_dirty(frame));
  ASMJIT_PROPAGATE(ctx.mark_scratch_regs(frame));
  ASMJIT_PROPAGATE(ctx.mark_stack_args_reg(frame));
  return Error::kOk;
}
#if defined(ASMJIT_TEST)
UNIT(func_signature) {
  FuncSignature signature;
  signature.set_ret_t<int8_t>();
  signature.add_arg_t<int16_t>();
  signature.add_arg(TypeId::kInt32);
  EXPECT_EQ(signature, FuncSignature::build<int8_t, int16_t, int32_t>());
}
#endif
ASMJIT_END_NAMESPACE
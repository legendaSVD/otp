#include <asmjit/core/api-build_p.h>
#include <asmjit/core/archtraits.h>
#include <asmjit/core/environment.h>
#include <asmjit/core/misc_p.h>
#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86/x86archtraits_p.h>
#endif
#if !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/arm/a64archtraits_p.h>
#endif
ASMJIT_BEGIN_NAMESPACE
static const constexpr ArchTraits no_arch_traits = {
  0xFFu, 0xFFu, 0xFFu, 0xFFu,
  { 0u, 0u, 0u },
  0u,
  0, 0,
  0u,
  {{
    InstHints::kNoHints,
    InstHints::kNoHints,
    InstHints::kNoHints,
    InstHints::kNoHints
  }},
  #define V(index) RegType::kNone
  {{ ASMJIT_LOOKUP_TABLE_32(V, 0) }},
  #undef V
  {
    ArchTypeNameId::kByte,
    ArchTypeNameId::kHalf,
    ArchTypeNameId::kWord,
    ArchTypeNameId::kQuad
  }
};
ASMJIT_VARAPI const ArchTraits _arch_traits[uint32_t(Arch::kMaxValue) + 1] = {
  no_arch_traits,
#if !defined(ASMJIT_NO_X86)
  x86::x86_arch_traits,
  x86::x64_arch_traits,
#else
  no_arch_traits,
  no_arch_traits,
#endif
  no_arch_traits,
  no_arch_traits,
  no_arch_traits,
#if !defined(ASMJIT_NO_AARCH64)
  a64::a64_arch_traits,
#else
  no_arch_traits,
#endif
  no_arch_traits,
  no_arch_traits,
  no_arch_traits,
  no_arch_traits
};
ASMJIT_FAVOR_SIZE Error ArchUtils::type_id_to_reg_signature(Arch arch, TypeId type_id, Out<TypeId> type_id_out, Out<OperandSignature> reg_signature_out) noexcept {
  const ArchTraits& arch_traits = ArchTraits::by_arch(arch);
  if (uint32_t(type_id) <= uint32_t(RegType::kMaxValue)) {
    type_id = RegUtils::type_id_of(RegType(uint32_t(type_id)));
  }
  if (ASMJIT_UNLIKELY(!TypeUtils::is_valid(type_id))) {
    return make_error(Error::kInvalidTypeId);
  }
  if (TypeUtils::is_abstract(type_id)) {
    bool is_32bit = Environment::is_32bit(arch);
    if (type_id == TypeId::kIntPtr) {
      type_id = is_32bit ? TypeId::kInt32 : TypeId::kInt64;
    }
    else {
      type_id = is_32bit ? TypeId::kUInt32 : TypeId::kUInt64;
    }
  }
  uint32_t size = TypeUtils::size_of(type_id);
  if (ASMJIT_UNLIKELY(!size)) {
    return make_error(Error::kInvalidTypeId);
  }
  if (ASMJIT_UNLIKELY(type_id == TypeId::kFloat80)) {
    return make_error(Error::kInvalidUseOfF80);
  }
  RegType reg_type = RegType::kNone;
  if (TypeUtils::is_between(type_id, TypeId::_kBaseStart, TypeId::_kVec32Start)) {
    reg_type = arch_traits._type_id_to_reg_type[uint32_t(type_id) - uint32_t(TypeId::_kBaseStart)];
    if (reg_type == RegType::kNone) {
      if (type_id == TypeId::kInt64 || type_id == TypeId::kUInt64) {
        return make_error(Error::kInvalidUseOfGpq);
      }
      else {
        return make_error(Error::kInvalidTypeId);
      }
    }
  }
  else {
    if (size <= 8 && arch_traits.has_reg_type(RegType::kVec64)) {
      reg_type = RegType::kVec64;
    }
    else if (size <= 16 && arch_traits.has_reg_type(RegType::kVec128)) {
      reg_type = RegType::kVec128;
    }
    else if (size == 32 && arch_traits.has_reg_type(RegType::kVec256)) {
      reg_type = RegType::kVec256;
    }
    else if (arch_traits.has_reg_type(RegType::kVec512)) {
      reg_type = RegType::kVec512;
    }
    else {
      return make_error(Error::kInvalidTypeId);
    }
  }
  *type_id_out = type_id;
  *reg_signature_out = RegUtils::signature_of(reg_type);
  return Error::kOk;
}
ASMJIT_END_NAMESPACE
#ifndef ASMJIT_CORE_ARCHTRAITS_H_INCLUDED
#define ASMJIT_CORE_ARCHTRAITS_H_INCLUDED
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
enum class Arch : uint8_t {
  kUnknown = 0,
  kX86 = 1,
  kX64 = 2,
  kRISCV32 = 3,
  kRISCV64 = 4,
  kARM = 5,
  kAArch64 = 6,
  kThumb = 7,
  kLA64 = 8,
  kMIPS32_LE = 9,
  kMIPS64_LE = 10,
  kARM_BE = 11,
  kAArch64_BE = 12,
  kThumb_BE = 13,
  kMIPS32_BE = 15,
  kMIPS64_BE = 16,
  kMaxValue = kMIPS64_BE,
  k32BitMask = 0x01,
  kBigEndian = kARM_BE,
  kHost =
#if defined(_DOXYGEN)
    DETECTED_AT_COMPILE_TIME
#else
    ASMJIT_ARCH_X86 == 32 ? kX86 :
    ASMJIT_ARCH_X86 == 64 ? kX64 :
    ASMJIT_ARCH_RISCV == 32 ? kRISCV32 :
    ASMJIT_ARCH_RISCV == 64 ? kRISCV64 :
    ASMJIT_ARCH_LA == 64 ? kLA64 :
    ASMJIT_ARCH_ARM == 32 && Support::ByteOrder::kNative == Support::ByteOrder::kLE ? kARM :
    ASMJIT_ARCH_ARM == 32 && Support::ByteOrder::kNative == Support::ByteOrder::kBE ? kARM_BE :
    ASMJIT_ARCH_ARM == 64 && Support::ByteOrder::kNative == Support::ByteOrder::kLE ? kAArch64 :
    ASMJIT_ARCH_ARM == 64 && Support::ByteOrder::kNative == Support::ByteOrder::kBE ? kAArch64_BE :
    ASMJIT_ARCH_MIPS == 32 && Support::ByteOrder::kNative == Support::ByteOrder::kLE ? kMIPS32_LE :
    ASMJIT_ARCH_MIPS == 32 && Support::ByteOrder::kNative == Support::ByteOrder::kBE ? kMIPS32_BE :
    ASMJIT_ARCH_MIPS == 64 && Support::ByteOrder::kNative == Support::ByteOrder::kLE ? kMIPS64_LE :
    ASMJIT_ARCH_MIPS == 64 && Support::ByteOrder::kNative == Support::ByteOrder::kBE ? kMIPS64_BE :
    kUnknown
#endif
};
enum class SubArch : uint8_t {
  kUnknown = 0,
  kMaxValue = kUnknown,
  kHost =
#if defined(_DOXYGEN)
    DETECTED_AT_COMPILE_TIME
#else
    kUnknown
#endif
};
enum class ArchTypeNameId : uint8_t {
  kDB = 0,
  kDW,
  kDD,
  kDQ,
  kByte,
  kHalf,
  kWord,
  kHWord,
  kDWord,
  kQWord,
  kXWord,
  kShort,
  kLong,
  kQuad,
  kMaxValue = kQuad
};
enum class InstHints : uint8_t {
  kNoHints = 0,
  kRegSwap = 0x01u,
  kPushPop = 0x02u
};
ASMJIT_DEFINE_ENUM_FLAGS(InstHints)
struct ArchTraits {
  uint8_t _sp_reg_id;
  uint8_t _fp_reg_id;
  uint8_t _link_reg_id;
  uint8_t _pc_reg_id;
  uint8_t _reserved[3];
  uint8_t _hw_stack_alignment;
  uint32_t _min_stack_offset;
  uint32_t _max_stack_offset;
  uint32_t _supported_reg_types;
  Support::Array<InstHints, Globals::kNumVirtGroups> _inst_hints;
  Support::Array<RegType, 32> _type_id_to_reg_type;
  ArchTypeNameId _type_name_id_table[4];
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sp_reg_id() const noexcept { return _sp_reg_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t fp_reg_id() const noexcept { return _fp_reg_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t link_reg_id() const noexcept { return _link_reg_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t pc_reg_id() const noexcept { return _pc_reg_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t hw_stack_alignment() const noexcept { return _hw_stack_alignment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_link_reg() const noexcept { return _link_reg_id != Reg::kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t min_stack_offset() const noexcept { return _min_stack_offset; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t max_stack_offset() const noexcept { return _max_stack_offset; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstHints inst_feature_hints(RegGroup group) const noexcept { return _inst_hints[group]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_inst_hint(RegGroup group, InstHints feature) const noexcept { return Support::test(_inst_hints[group], feature); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_inst_reg_swap(RegGroup group) const noexcept { return has_inst_hint(group, InstHints::kRegSwap); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_inst_push_pop(RegGroup group) const noexcept { return has_inst_hint(group, InstHints::kPushPop); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_reg_type(RegType type) const noexcept {
    if (ASMJIT_UNLIKELY(type > RegType::kMaxValue)) {
      type = RegType::kNone;
    }
    return Support::bit_test(_supported_reg_types, uint32_t(type));
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ArchTypeNameId* type_name_id_table() const noexcept { return _type_name_id_table; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ArchTypeNameId type_name_id_by_index(uint32_t index) const noexcept { return _type_name_id_table[index]; }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG const ArchTraits& by_arch(Arch arch) noexcept;
};
ASMJIT_VARAPI const ArchTraits _arch_traits[uint32_t(Arch::kMaxValue) + 1];
ASMJIT_INLINE_NODEBUG const ArchTraits& ArchTraits::by_arch(Arch arch) noexcept { return _arch_traits[uint32_t(arch)]; }
namespace ArchUtils {
ASMJIT_API Error type_id_to_reg_signature(Arch arch, TypeId type_id, Out<TypeId> type_id_out, Out<OperandSignature> reg_signature_out) noexcept;
}
ASMJIT_END_NAMESPACE
#endif
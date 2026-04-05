#ifndef ASMJIT_X86_X86INSTDB_H_INCLUDED
#define ASMJIT_X86_X86INSTDB_H_INCLUDED
#include <asmjit/support/span.h>
#include <asmjit/x86/x86globals.h>
ASMJIT_BEGIN_SUB_NAMESPACE(x86)
namespace InstDB {
enum class Mode : uint8_t {
  kNone = 0x00u,
  kX86 = 0x01u,
  kX64 = 0x02u,
  kAny = 0x03u
};
ASMJIT_DEFINE_ENUM_FLAGS(Mode)
static ASMJIT_INLINE_CONSTEXPR Mode mode_from_arch(Arch arch) noexcept {
  return arch == Arch::kX86 ? Mode::kX86 :
         arch == Arch::kX64 ? Mode::kX64 : Mode::kNone;
}
enum class OpFlags : uint64_t {
  kNone = 0u,
  kRegGpbLo        = 0x0000000000000001u,
  kRegGpbHi        = 0x0000000000000002u,
  kRegGpw          = 0x0000000000000004u,
  kRegGpd          = 0x0000000000000008u,
  kRegGpq          = 0x0000000000000010u,
  kRegXmm          = 0x0000000000000020u,
  kRegYmm          = 0x0000000000000040u,
  kRegZmm          = 0x0000000000000080u,
  kRegMm           = 0x0000000000000100u,
  kRegKReg         = 0x0000000000000200u,
  kRegSReg         = 0x0000000000000400u,
  kRegCReg         = 0x0000000000000800u,
  kRegDReg         = 0x0000000000001000u,
  kRegSt           = 0x0000000000002000u,
  kRegBnd          = 0x0000000000004000u,
  kRegTmm          = 0x0000000000008000u,
  kRegMask         = 0x000000000000FFFFu,
  kMemUnspecified  = 0x0000000000040000u,
  kMem8            = 0x0000000000080000u,
  kMem16           = 0x0000000000100000u,
  kMem32           = 0x0000000000200000u,
  kMem48           = 0x0000000000400000u,
  kMem64           = 0x0000000000800000u,
  kMem80           = 0x0000000001000000u,
  kMem128          = 0x0000000002000000u,
  kMem256          = 0x0000000004000000u,
  kMem512          = 0x0000000008000000u,
  kMem1024         = 0x0000000010000000u,
  kMemMask         = 0x000000001FFC0000u,
  kVm32x           = 0x0000000040000000u,
  kVm32y           = 0x0000000080000000u,
  kVm32z           = 0x0000000100000000u,
  kVm64x           = 0x0000000200000000u,
  kVm64y           = 0x0000000400000000u,
  kVm64z           = 0x0000000800000000u,
  kVmMask          = 0x0000000FC0000000u,
  kImmI4           = 0x0000001000000000u,
  kImmU4           = 0x0000002000000000u,
  kImmI8           = 0x0000004000000000u,
  kImmU8           = 0x0000008000000000u,
  kImmI16          = 0x0000010000000000u,
  kImmU16          = 0x0000020000000000u,
  kImmI32          = 0x0000040000000000u,
  kImmU32          = 0x0000080000000000u,
  kImmI64          = 0x0000100000000000u,
  kImmU64          = 0x0000200000000000u,
  kImmMask         = 0x00003FF000000000u,
  kRel8            = 0x0000400000000000u,
  kRel32           = 0x0000800000000000u,
  kRelMask         = 0x0000C00000000000u,
  kFlagMemBase     = 0x0001000000000000u,
  kFlagMemDs       = 0x0002000000000000u,
  kFlagMemEs       = 0x0004000000000000u,
  kFlagMib         = 0x0008000000000000u,
  kFlagTMem        = 0x0010000000000000u,
  kFlagImplicit    = 0x0080000000000000u,
  kFlagMask        = 0x009F000000000000u,
  kOpMask          = kRegMask | kMemMask | kVmMask | kImmMask | kRelMask
};
ASMJIT_DEFINE_ENUM_FLAGS(OpFlags)
struct OpSignature {
  uint64_t _flags : 56;
  uint64_t _reg_mask : 8;
  [[nodiscard]]
  inline OpFlags flags() const noexcept { return (OpFlags)_flags; }
  [[nodiscard]]
  inline bool has_flag(OpFlags flag) const noexcept { return (_flags & uint64_t(flag)) != 0; }
  [[nodiscard]]
  inline bool has_reg() const noexcept { return has_flag(OpFlags::kRegMask); }
  [[nodiscard]]
  inline bool has_mem() const noexcept { return has_flag(OpFlags::kMemMask); }
  [[nodiscard]]
  inline bool has_vm() const noexcept { return has_flag(OpFlags::kVmMask); }
  [[nodiscard]]
  inline bool has_imm() const noexcept { return has_flag(OpFlags::kImmMask); }
  [[nodiscard]]
  inline bool has_rel() const noexcept { return has_flag(OpFlags::kRelMask); }
  [[nodiscard]]
  inline bool is_implicit() const noexcept { return has_flag(OpFlags::kFlagImplicit); }
  [[nodiscard]]
  inline RegMask reg_mask() const noexcept { return _reg_mask; }
};
ASMJIT_VARAPI const OpSignature _op_signature_table[];
struct InstSignature {
  uint8_t _op_count : 3;
  uint8_t _mode : 2;
  uint8_t _implicit_op_count : 3;
  uint8_t _reserved;
  uint8_t _op_signature_indexes[Globals::kMaxOpCount];
  [[nodiscard]]
  inline Mode mode() const noexcept { return (Mode)_mode; }
  [[nodiscard]]
  inline bool supports_mode(Mode mode) const noexcept { return (uint8_t(_mode) & uint8_t(mode)) != 0; }
  [[nodiscard]]
  inline uint32_t op_count() const noexcept { return _op_count; }
  [[nodiscard]]
  inline uint32_t implicit_op_count() const noexcept { return _implicit_op_count; }
  [[nodiscard]]
  inline bool has_implicit_operands() const noexcept { return _implicit_op_count != 0; }
  [[nodiscard]]
  inline const uint8_t* op_signature_indexes() const noexcept { return _op_signature_indexes; }
  [[nodiscard]]
  inline uint8_t op_signature_index(size_t index) const noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxOpCount);
    return _op_signature_indexes[index];
  }
  [[nodiscard]]
  inline const OpSignature& op_signature(size_t index) const noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxOpCount);
    return _op_signature_table[_op_signature_indexes[index]];
  }
};
ASMJIT_VARAPI const InstSignature _inst_signature_table[];
enum class InstFlags : uint32_t {
  kNone = 0x00000000u,
  kFpu = 0x00000100u,
  kMmx = 0x00000200u,
  kVec = 0x00000400u,
  kFpuM16 = 0x00000800u,
  kFpuM32 = 0x00001000u,
  kFpuM64 = 0x00002000u,
  kFpuM80 = 0x00000800u,
  kRep = 0x00004000u,
  kRepIgnored = 0x00008000u,
  kLock = 0x00010000u,
  kXAcquire = 0x00020000u,
  kXRelease = 0x00040000u,
  kMib = 0x00080000u,
  kVsib = 0x00100000u,
  kTsib = 0x00200000u,
  kVex = 0x00400000u,
  kEvex = 0x00800000u,
  kPreferEvex = 0x01000000u,
  kEvexCompat = 0x02000000u,
  kEvexKReg = 0x04000000u,
  kEvexTwoOp = 0x08000000u,
  kEvexTransformable = 0x10000000u,
  kConsecutiveRegs = 0x20000000u
};
ASMJIT_DEFINE_ENUM_FLAGS(InstFlags)
enum class Avx512Flags : uint32_t {
  kNone = 0,
  k_ = 0x00000000u,
  kK = 0x00000001u,
  kZ = 0x00000002u,
  kER = 0x00000004u,
  kSAE = 0x00000008u,
  kB16 = 0x00000010u,
  kB32 = 0x00000020u,
  kB64 = 0x00000040u,
  kImplicitZ = 0x00000100,
};
ASMJIT_DEFINE_ENUM_FLAGS(Avx512Flags)
struct CommonInfo {
  uint32_t _flags;
  uint32_t _avx512_flags : 11;
  uint32_t _inst_signature_index : 11;
  uint32_t _inst_signature_count : 5;
  uint32_t _control_flow : 3;
  uint32_t _same_reg_hint : 2;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstFlags flags() const noexcept { return (InstFlags)_flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(InstFlags flag) const noexcept { return Support::test(_flags, flag); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Avx512Flags avx512_flags() const noexcept { return (Avx512Flags)_avx512_flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_flag(Avx512Flags flag) const noexcept { return Support::test(_avx512_flags, flag); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_fpu() const noexcept { return has_flag(InstFlags::kFpu); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mmx() const noexcept { return has_flag(InstFlags::kMmx); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_vec() const noexcept { return has_flag(InstFlags::kVec); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_sse() const noexcept { return (flags() & (InstFlags::kVec | InstFlags::kVex | InstFlags::kEvex)) == InstFlags::kVec; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_avx() const noexcept { return is_vec() && is_vex_or_evex(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_lock_prefix() const noexcept { return has_flag(InstFlags::kLock); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_rep_prefix() const noexcept { return has_flag(InstFlags::kRep); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_xacquire_prefix() const noexcept { return has_flag(InstFlags::kXAcquire); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_xrelease_prefix() const noexcept { return has_flag(InstFlags::kXRelease); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_rep_ignored() const noexcept { return has_flag(InstFlags::kRepIgnored); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mib_op() const noexcept { return has_flag(InstFlags::kMib); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_vsib_op() const noexcept { return has_flag(InstFlags::kVsib); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_tsib_op() const noexcept { return has_flag(InstFlags::kTsib); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_vex() const noexcept { return has_flag(InstFlags::kVex); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_evex() const noexcept { return has_flag(InstFlags::kEvex); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_vex_or_evex() const noexcept { return has_flag(InstFlags::kVex | InstFlags::kEvex); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool prefer_evex() const noexcept { return has_flag(InstFlags::kPreferEvex); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_evex_compatible() const noexcept { return has_flag(InstFlags::kEvexCompat); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_evex_kreg_only() const noexcept { return has_flag(InstFlags::kEvexKReg); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_evex_two_op_only() const noexcept { return has_flag(InstFlags::kEvexTwoOp); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_evex_transformable() const noexcept { return has_flag(InstFlags::kEvexTransformable); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_k() const noexcept { return has_avx512_flag(Avx512Flags::kK); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_z() const noexcept { return has_avx512_flag(Avx512Flags::kZ); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_er() const noexcept { return has_avx512_flag(Avx512Flags::kER); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_sae() const noexcept { return has_avx512_flag(Avx512Flags::kSAE); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_bcst() const noexcept { return has_avx512_flag(Avx512Flags::kB16 | Avx512Flags::kB32 | Avx512Flags::kB64); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_bcst16() const noexcept { return has_avx512_flag(Avx512Flags::kB16); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_bcst32() const noexcept { return has_avx512_flag(Avx512Flags::kB32); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_bcst64() const noexcept { return has_avx512_flag(Avx512Flags::kB64); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t broadcast_size() const noexcept {
    constexpr uint32_t kShift = Support::ctz_const<Avx512Flags::kB16>;
    return (uint32_t(_avx512_flags) & uint32_t(Avx512Flags::kB16 | Avx512Flags::kB32 | Avx512Flags::kB64)) >> (kShift - 1);
  }
  ASMJIT_INLINE_NODEBUG Span<const InstSignature> inst_signatures() const noexcept {
    return Span<const InstSignature>(_inst_signature_table + _inst_signature_index, _inst_signature_count);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstControlFlow control_flow() const noexcept { return (InstControlFlow)_control_flow; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstSameRegHint same_reg_hint() const noexcept { return (InstSameRegHint)_same_reg_hint; }
};
ASMJIT_VARAPI const CommonInfo _inst_common_info_table[];
struct InstInfo {
  uint32_t _reserved : 14;
  uint32_t _common_info_index : 10;
  uint32_t _additional_info_index : 8;
  uint8_t _encoding;
  uint8_t _main_opcode_value;
  uint8_t _main_opcode_index;
  uint8_t _alt_opcode_index;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CommonInfo& common_info() const noexcept { return _inst_common_info_table[_common_info_index]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstFlags flags() const noexcept { return common_info().flags(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(InstFlags flag) const noexcept { return common_info().has_flag(flag); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Avx512Flags avx512_flags() const noexcept { return common_info().avx512_flags(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_flag(Avx512Flags flag) const noexcept { return common_info().has_avx512_flag(flag); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_fpu() const noexcept { return common_info().is_fpu(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mmx() const noexcept { return common_info().is_mmx(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_vec() const noexcept { return common_info().is_vec(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_sse() const noexcept { return common_info().is_sse(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_avx() const noexcept { return common_info().is_avx(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_lock_prefix() const noexcept { return common_info().has_lock_prefix(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_rep_prefix() const noexcept { return common_info().has_rep_prefix(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_xacquire_prefix() const noexcept { return common_info().has_xacquire_prefix(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_xrelease_prefix() const noexcept { return common_info().has_xrelease_prefix(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_rep_ignored() const noexcept { return common_info().is_rep_ignored(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mib_op() const noexcept { return has_flag(InstFlags::kMib); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_vsib_op() const noexcept { return has_flag(InstFlags::kVsib); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_vex() const noexcept { return has_flag(InstFlags::kVex); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_evex() const noexcept { return has_flag(InstFlags::kEvex); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_vex_or_evex() const noexcept { return has_flag(InstFlags::kVex | InstFlags::kEvex); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_evex_compatible() const noexcept { return has_flag(InstFlags::kEvexCompat); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_evex_kreg_only() const noexcept { return has_flag(InstFlags::kEvexKReg); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_evex_two_op_only() const noexcept { return has_flag(InstFlags::kEvexTwoOp); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_evex_transformable() const noexcept { return has_flag(InstFlags::kEvexTransformable); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_k() const noexcept { return has_avx512_flag(Avx512Flags::kK); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_z() const noexcept { return has_avx512_flag(Avx512Flags::kZ); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_er() const noexcept { return has_avx512_flag(Avx512Flags::kER); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_sae() const noexcept { return has_avx512_flag(Avx512Flags::kSAE); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_bcst() const noexcept { return has_avx512_flag(Avx512Flags::kB16 | Avx512Flags::kB32 | Avx512Flags::kB64); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_bcst16() const noexcept { return has_avx512_flag(Avx512Flags::kB16); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_bcst32() const noexcept { return has_avx512_flag(Avx512Flags::kB32); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx512_bcst64() const noexcept { return has_avx512_flag(Avx512Flags::kB64); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstControlFlow control_flow() const noexcept { return common_info().control_flow(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstSameRegHint same_reg_hint() const noexcept { return common_info().same_reg_hint(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<const InstSignature> inst_signatures() const noexcept { return common_info().inst_signatures(); }
};
ASMJIT_VARAPI const InstInfo _inst_info_table[];
[[nodiscard]]
static inline const InstInfo& inst_info_by_id(InstId inst_id) noexcept {
  ASMJIT_ASSERT(Inst::is_defined_id(inst_id));
  return _inst_info_table[inst_id];
}
static_assert(sizeof(OpSignature) == 8, "InstDB::OpSignature must be 8 bytes long");
}
ASMJIT_END_SUB_NAMESPACE
#endif
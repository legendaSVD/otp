#ifndef ASMJIT_CORE_INST_H_INCLUDED
#define ASMJIT_CORE_INST_H_INCLUDED
#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/string.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
using InstId = uint32_t;
enum class InstIdParts : uint32_t {
  kRealId   = 0x0000FFFFu,
  kAbstract = 0x80000000u,
  kA32_DT   = 0x000F0000u,
  kA32_DT2  = 0x00F00000u,
  kARM_Cond = 0x78000000u
};
enum class InstOptions : uint32_t {
  kNone = 0,
  kReserved = 0x00000001u,
  kUnfollow = 0x00000002u,
  kOverwrite = 0x00000004u,
  kShortForm = 0x00000010u,
  kLongForm = 0x00000020u,
  kTaken = 0x00000040u,
  kNotTaken = 0x00000080u,
  kX86_ModMR = 0x00000100u,
  kX86_ModRM = 0x00000200u,
  kX86_Vex3 = 0x00000400u,
  kX86_Vex = 0x00000800u,
  kX86_Evex = 0x00001000u,
  kX86_Lock = 0x00002000u,
  kX86_Rep = 0x00004000u,
  kX86_Repne = 0x00008000u,
  kX86_XAcquire = 0x00010000u,
  kX86_XRelease = 0x00020000u,
  kX86_ER = 0x00040000u,
  kX86_SAE = 0x00080000u,
  kX86_RN_SAE = 0x00000000u,
  kX86_RD_SAE = 0x00200000u,
  kX86_RU_SAE = 0x00400000u,
  kX86_RZ_SAE = 0x00600000u,
  kX86_ZMask = 0x00800000u,
  kX86_ERMask = kX86_RZ_SAE,
  kX86_AVX512Mask = 0x00FC0000u,
  kX86_OpCodeB = 0x01000000u,
  kX86_OpCodeX = 0x02000000u,
  kX86_OpCodeR = 0x04000000u,
  kX86_OpCodeW = 0x08000000u,
  kX86_Rex = 0x40000000u,
  kX86_InvalidRex = 0x80000000u
};
ASMJIT_DEFINE_ENUM_FLAGS(InstOptions)
enum class InstControlFlow : uint32_t {
  kRegular = 0u,
  kJump = 1u,
  kBranch = 2u,
  kCall = 3u,
  kReturn = 4u,
  kMaxValue = kReturn
};
enum class InstSameRegHint : uint8_t {
  kNone = 0,
  kRO = 1,
  kWO = 2
};
enum class InstStringifyOptions : uint32_t {
  kNone = 0x00000000u,
  kAliases = 0x00000001u
};
ASMJIT_DEFINE_ENUM_FLAGS(InstStringifyOptions)
class BaseInst {
public:
  InstId _inst_id;
  InstOptions _options;
  RegOnly _extra_reg;
  enum Id : uint32_t {
    kIdNone = 0x00000000u,
    kIdAbstract = 0x80000000u
  };
  ASMJIT_INLINE_NODEBUG explicit BaseInst(InstId inst_id = 0, InstOptions options = InstOptions::kNone) noexcept
    : _inst_id(inst_id),
      _options(options),
      _extra_reg() {}
  ASMJIT_INLINE_NODEBUG BaseInst(InstId inst_id, InstOptions options, const RegOnly& extra_reg) noexcept
    : _inst_id(inst_id),
      _options(options),
      _extra_reg(extra_reg) {}
  ASMJIT_INLINE_NODEBUG BaseInst(InstId inst_id, InstOptions options, const Reg& extra_reg) noexcept
    : _inst_id(inst_id),
      _options(options),
      _extra_reg{extra_reg.signature(), extra_reg.id()} {}
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstId inst_id() const noexcept { return _inst_id; }
  ASMJIT_INLINE_NODEBUG void set_inst_id(InstId inst_id) noexcept { _inst_id = inst_id; }
  ASMJIT_INLINE_NODEBUG void reset_inst_id() noexcept { _inst_id = 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstId real_id() const noexcept { return _inst_id & uint32_t(InstIdParts::kRealId); }
  template<InstIdParts kPart>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t inst_id_part() const noexcept {
    return (uint32_t(_inst_id) & uint32_t(kPart)) >> Support::ctz_const<kPart>;
  }
  template<InstIdParts kPart>
  ASMJIT_INLINE_NODEBUG void set_inst_id_part(uint32_t value) noexcept {
    _inst_id = (_inst_id & ~uint32_t(kPart)) | (value << Support::ctz_const<kPart>);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstOptions options() const noexcept { return _options; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_option(InstOptions option) const noexcept { return Support::test(_options, option); }
  ASMJIT_INLINE_NODEBUG void set_options(InstOptions options) noexcept { _options = options; }
  ASMJIT_INLINE_NODEBUG void add_options(InstOptions options) noexcept { _options |= options; }
  ASMJIT_INLINE_NODEBUG void clear_options(InstOptions options) noexcept { _options &= ~options; }
  ASMJIT_INLINE_NODEBUG void reset_options() noexcept { _options = InstOptions::kNone; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_extra_reg() const noexcept { return _extra_reg.is_reg(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegOnly& extra_reg() noexcept { return _extra_reg; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegOnly& extra_reg() const noexcept { return _extra_reg; }
  ASMJIT_INLINE_NODEBUG void set_extra_reg(const Reg& reg) noexcept { _extra_reg.init(reg); }
  ASMJIT_INLINE_NODEBUG void set_extra_reg(const RegOnly& reg) noexcept { _extra_reg.init(reg); }
  ASMJIT_INLINE_NODEBUG void reset_extra_reg() noexcept { _extra_reg.reset(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG arm::CondCode arm_cond_code() const noexcept { return (arm::CondCode)inst_id_part<InstIdParts::kARM_Cond>(); }
  ASMJIT_INLINE_NODEBUG void set_arm_cond_code(arm::CondCode cc) noexcept { set_inst_id_part<InstIdParts::kARM_Cond>(uint32_t(cc)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG a32::DataType arm_dt() const noexcept { return (a32::DataType)inst_id_part<InstIdParts::kA32_DT>(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG a32::DataType arm_dt2() const noexcept { return (a32::DataType)inst_id_part<InstIdParts::kA32_DT2>(); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR InstId compose_arm_inst_id(uint32_t id, arm::CondCode cc) noexcept {
    return id | (uint32_t(cc) << Support::ctz_const<InstIdParts::kARM_Cond>);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR InstId compose_arm_inst_id(uint32_t id, a32::DataType dt, arm::CondCode cc = arm::CondCode::kAL) noexcept {
    return id | (uint32_t(dt) << Support::ctz_const<InstIdParts::kA32_DT>)
              | (uint32_t(cc) << Support::ctz_const<InstIdParts::kARM_Cond>);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR InstId compose_arm_inst_id(uint32_t id, a32::DataType dt, a32::DataType dt2, arm::CondCode cc = arm::CondCode::kAL) noexcept {
    return id | (uint32_t(dt) << Support::ctz_const<InstIdParts::kA32_DT>)
              | (uint32_t(dt2) << Support::ctz_const<InstIdParts::kA32_DT2>)
              | (uint32_t(cc) << Support::ctz_const<InstIdParts::kARM_Cond>);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR InstId extract_real_id(uint32_t id) noexcept {
    return id & uint32_t(InstIdParts::kRealId);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR arm::CondCode extract_arm_cond_code(uint32_t id) noexcept {
    return (arm::CondCode)((uint32_t(id) & uint32_t(InstIdParts::kARM_Cond)) >> Support::ctz_const<InstIdParts::kARM_Cond>);
  }
};
enum class CpuRWFlags : uint32_t {
  kNone = 0x00000000u,
  kOF = 0x00000001u,
  kCF = 0x00000002u,
  kZF = 0x00000004u,
  kSF = 0x00000008u,
  kX86_CF = kCF,
  kX86_OF = kOF,
  kX86_SF = kSF,
  kX86_ZF = kZF,
  kX86_AF = 0x00000100u,
  kX86_PF = 0x00000200u,
  kX86_DF = 0x00000400u,
  kX86_IF = 0x00000800u,
  kX86_AC = 0x00001000u,
  kX86_C0 = 0x00010000u,
  kX86_C1 = 0x00020000u,
  kX86_C2 = 0x00040000u,
  kX86_C3 = 0x00080000u,
  kARM_V = kOF,
  kARM_C = kCF,
  kARM_Z = kZF,
  kARM_N = kSF,
  kARM_Q = 0x00000100u,
  kARM_GE = 0x00000200u
};
ASMJIT_DEFINE_ENUM_FLAGS(CpuRWFlags)
enum class OpRWFlags : uint32_t {
  kNone = 0,
  kRead = 0x00000001u,
  kWrite = 0x00000002u,
  kRW = 0x00000003u,
  kRegMem = 0x00000004u,
  kConsecutive = 0x00000008u,
  kZExt = 0x00000010u,
  kUnique = 0x00000080u,
  kRegPhysId = 0x00000100u,
  kMemPhysId = 0x00000200u,
  kMemFake = 0x000000400u,
  kMemBaseRead = 0x00001000u,
  kMemBaseWrite = 0x00002000u,
  kMemBaseRW = 0x00003000u,
  kMemIndexRead = 0x00004000u,
  kMemIndexWrite = 0x00008000u,
  kMemIndexRW = 0x0000C000u,
  kMemBasePreModify = 0x00010000u,
  kMemBasePostModify = 0x00020000u
};
ASMJIT_DEFINE_ENUM_FLAGS(OpRWFlags)
static_assert(uint32_t(OpRWFlags::kRead) == 0x1, "OpRWFlags::kRead flag must be 0x1");
static_assert(uint32_t(OpRWFlags::kWrite) == 0x2, "OpRWFlags::kWrite flag must be 0x2");
static_assert(uint32_t(OpRWFlags::kRegMem) == 0x4, "OpRWFlags::kRegMem flag must be 0x4");
struct OpRWInfo {
  OpRWFlags _op_flags;
  uint8_t _phys_id;
  uint8_t _rm_size;
  uint8_t _consecutive_lead_count;
  uint8_t _reserved[1];
  uint64_t _read_byte_mask;
  uint64_t _write_byte_mask;
  uint64_t _extend_byte_mask;
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = OpRWInfo{}; }
  inline void reset(OpRWFlags op_flags, uint32_t register_size, uint32_t phys_id = Reg::kIdBad) noexcept {
    _op_flags = op_flags;
    _phys_id = uint8_t(phys_id);
    _rm_size = Support::test(op_flags, OpRWFlags::kRegMem) ? uint8_t(register_size) : uint8_t(0);
    _consecutive_lead_count = 0;
    _reset_reserved();
    uint64_t mask = Support::lsb_mask<uint64_t>(Support::min<uint32_t>(register_size, 64));
    _read_byte_mask = Support::test(op_flags, OpRWFlags::kRead) ? mask : uint64_t(0);
    _write_byte_mask = Support::test(op_flags, OpRWFlags::kWrite) ? mask : uint64_t(0);
    _extend_byte_mask = 0;
  }
  ASMJIT_INLINE_NODEBUG void _reset_reserved() noexcept {
    _reserved[0] = uint8_t(0);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OpRWFlags op_flags() const noexcept { return _op_flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_op_flag(OpRWFlags flag) const noexcept { return Support::test(_op_flags, flag); }
  ASMJIT_INLINE_NODEBUG void add_op_flags(OpRWFlags flags) noexcept { _op_flags |= flags; }
  ASMJIT_INLINE_NODEBUG void clear_op_flags(OpRWFlags flags) noexcept { _op_flags &= ~flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read() const noexcept { return has_op_flag(OpRWFlags::kRead); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_write() const noexcept { return has_op_flag(OpRWFlags::kWrite); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read_write() const noexcept { return (_op_flags & OpRWFlags::kRW) == OpRWFlags::kRW; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read_only() const noexcept { return (_op_flags & OpRWFlags::kRW) == OpRWFlags::kRead; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_write_only() const noexcept { return (_op_flags & OpRWFlags::kRW) == OpRWFlags::kWrite; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t consecutive_lead_count() const noexcept { return _consecutive_lead_count; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_rm() const noexcept { return has_op_flag(OpRWFlags::kRegMem); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_zext() const noexcept { return has_op_flag(OpRWFlags::kZExt); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_unique() const noexcept { return has_op_flag(OpRWFlags::kUnique); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_fake() const noexcept { return has_op_flag(OpRWFlags::kMemFake); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_used() const noexcept { return has_op_flag(OpRWFlags::kMemBaseRW); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_read() const noexcept { return has_op_flag(OpRWFlags::kMemBaseRead); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_write() const noexcept { return has_op_flag(OpRWFlags::kMemBaseWrite); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_read_write() const noexcept { return (_op_flags & OpRWFlags::kMemBaseRW) == OpRWFlags::kMemBaseRW; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_read_only() const noexcept { return (_op_flags & OpRWFlags::kMemBaseRW) == OpRWFlags::kMemBaseRead; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_write_only() const noexcept { return (_op_flags & OpRWFlags::kMemBaseRW) == OpRWFlags::kMemBaseWrite; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_pre_modify() const noexcept { return has_op_flag(OpRWFlags::kMemBasePreModify); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_post_modify() const noexcept { return has_op_flag(OpRWFlags::kMemBasePostModify); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_used() const noexcept { return has_op_flag(OpRWFlags::kMemIndexRW); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_read() const noexcept { return has_op_flag(OpRWFlags::kMemIndexRead); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_write() const noexcept { return has_op_flag(OpRWFlags::kMemIndexWrite); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_read_write() const noexcept { return (_op_flags & OpRWFlags::kMemIndexRW) == OpRWFlags::kMemIndexRW; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_read_only() const noexcept { return (_op_flags & OpRWFlags::kMemIndexRW) == OpRWFlags::kMemIndexRead; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_write_only() const noexcept { return (_op_flags & OpRWFlags::kMemIndexRW) == OpRWFlags::kMemIndexWrite; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t phys_id() const noexcept { return _phys_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_phys_id() const noexcept { return _phys_id != Reg::kIdBad; }
  ASMJIT_INLINE_NODEBUG void set_phys_id(uint32_t phys_id) noexcept { _phys_id = uint8_t(phys_id); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t rm_size() const noexcept { return _rm_size; }
  ASMJIT_INLINE_NODEBUG void set_rm_size(uint32_t rm_size) noexcept { _rm_size = uint8_t(rm_size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t read_byte_mask() const noexcept { return _read_byte_mask; }
  ASMJIT_INLINE_NODEBUG void set_read_byte_mask(uint64_t mask) noexcept { _read_byte_mask = mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t write_byte_mask() const noexcept { return _write_byte_mask; }
  ASMJIT_INLINE_NODEBUG void set_write_byte_mask(uint64_t mask) noexcept { _write_byte_mask = mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t extend_byte_mask() const noexcept { return _extend_byte_mask; }
  ASMJIT_INLINE_NODEBUG void set_extend_byte_mask(uint64_t mask) noexcept { _extend_byte_mask = mask; }
};
enum class InstRWFlags : uint32_t {
  kNone = 0x00000000u,
  kMovOp = 0x00000001u
};
ASMJIT_DEFINE_ENUM_FLAGS(InstRWFlags)
struct InstRWInfo {
  InstRWFlags _inst_flags;
  CpuRWFlags _read_flags;
  CpuRWFlags _write_flags;
  uint8_t _op_count;
  uint8_t _rm_feature;
  uint8_t _reserved[18];
  OpRWInfo _extra_reg;
  OpRWInfo _operands[Globals::kMaxOpCount];
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = InstRWInfo{}; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstRWFlags inst_flags() const noexcept { return _inst_flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_inst_flag(InstRWFlags flag) const noexcept { return Support::test(_inst_flags, flag); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mov_op() const noexcept { return has_inst_flag(InstRWFlags::kMovOp); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CpuRWFlags read_flags() const noexcept { return _read_flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CpuRWFlags write_flags() const noexcept { return _write_flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t rm_feature() const noexcept { return _rm_feature; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const OpRWInfo& extra_reg() const noexcept { return _extra_reg; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const OpRWInfo* operands() const noexcept { return _operands; }
  [[nodiscard]]
  inline const OpRWInfo& operand(size_t index) const noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxOpCount);
    return _operands[index];
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t op_count() const noexcept { return _op_count; }
};
enum class ValidationFlags : uint8_t {
  kNone = 0,
  kEnableVirtRegs = 0x01u
};
ASMJIT_DEFINE_ENUM_FLAGS(ValidationFlags)
namespace InstAPI {
#ifndef ASMJIT_NO_TEXT
ASMJIT_API Error inst_id_to_string(Arch arch, InstId inst_id, InstStringifyOptions options, String& output) noexcept;
[[nodiscard]]
ASMJIT_API InstId string_to_inst_id(Arch arch, const char* s, size_t len) noexcept;
#endif
#ifndef ASMJIT_NO_INTROSPECTION
[[nodiscard]]
ASMJIT_API Error validate(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count, ValidationFlags validation_flags = ValidationFlags::kNone) noexcept;
ASMJIT_API Error query_rw_info(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count, InstRWInfo* out) noexcept;
ASMJIT_API Error query_features(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count, CpuFeatures* out) noexcept;
#endif
}
ASMJIT_END_NAMESPACE
#endif
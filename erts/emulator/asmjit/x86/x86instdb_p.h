#ifndef ASMJIT_X86_X86INSTDB_P_H_INCLUDED
#define ASMJIT_X86_X86INSTDB_P_H_INCLUDED
#include <asmjit/core/instdb_p.h>
#include <asmjit/x86/x86instdb.h>
ASMJIT_BEGIN_SUB_NAMESPACE(x86)
namespace InstDB {
enum EncodingId : uint32_t {
  kEncodingNone = 0,
  kEncodingX86Op,
  kEncodingX86Op_Mod11RM,
  kEncodingX86Op_Mod11RM_I8,
  kEncodingX86Op_xAddr,
  kEncodingX86Op_xAX,
  kEncodingX86Op_xDX_xAX,
  kEncodingX86Op_MemZAX,
  kEncodingX86I_xAX,
  kEncodingX86M,
  kEncodingX86M_NoMemSize,
  kEncodingX86M_NoSize,
  kEncodingX86M_GPB,
  kEncodingX86M_GPB_MulDiv,
  kEncodingX86M_Only,
  kEncodingX86M_Only_EDX_EAX,
  kEncodingX86M_Nop,
  kEncodingX86R_Native,
  kEncodingX86R_FromM,
  kEncodingX86R32_EDX_EAX,
  kEncodingX86Rm,
  kEncodingX86Rm_Raw66H,
  kEncodingX86Rm_NoSize,
  kEncodingX86Mr,
  kEncodingX86Mr_NoSize,
  kEncodingX86Arith,
  kEncodingX86Bswap,
  kEncodingX86Bt,
  kEncodingX86Call,
  kEncodingX86Cmpxchg,
  kEncodingX86Cmpxchg8b_16b,
  kEncodingX86Crc,
  kEncodingX86Enter,
  kEncodingX86Imul,
  kEncodingX86In,
  kEncodingX86Ins,
  kEncodingX86IncDec,
  kEncodingX86Int,
  kEncodingX86Jcc,
  kEncodingX86JecxzLoop,
  kEncodingX86Jmp,
  kEncodingX86JmpRel,
  kEncodingX86LcallLjmp,
  kEncodingX86Lea,
  kEncodingX86Mov,
  kEncodingX86Movabs,
  kEncodingX86MovsxMovzx,
  kEncodingX86MovntiMovdiri,
  kEncodingX86EnqcmdMovdir64b,
  kEncodingX86Out,
  kEncodingX86Outs,
  kEncodingX86Push,
  kEncodingX86Pushw,
  kEncodingX86Pop,
  kEncodingX86Ret,
  kEncodingX86Rot,
  kEncodingX86Set,
  kEncodingX86ShldShrd,
  kEncodingX86StrRm,
  kEncodingX86StrMr,
  kEncodingX86StrMm,
  kEncodingX86Test,
  kEncodingX86Xadd,
  kEncodingX86Xchg,
  kEncodingX86Fence,
  kEncodingX86Bndmov,
  kEncodingFpuOp,
  kEncodingFpuArith,
  kEncodingFpuCom,
  kEncodingFpuFldFst,
  kEncodingFpuM,
  kEncodingFpuR,
  kEncodingFpuRDef,
  kEncodingFpuStsw,
  kEncodingExtRm,
  kEncodingExtRm_XMM0,
  kEncodingExtRm_ZDI,
  kEncodingExtRm_P,
  kEncodingExtRm_Wx,
  kEncodingExtRm_Wx_GpqOnly,
  kEncodingExtRmRi,
  kEncodingExtRmRi_P,
  kEncodingExtRmi,
  kEncodingExtRmi_P,
  kEncodingExtPextrw,
  kEncodingExtExtract,
  kEncodingExtMov,
  kEncodingExtMovbe,
  kEncodingExtMovd,
  kEncodingExtMovq,
  kEncodingExtExtrq,
  kEncodingExtInsertq,
  kEncodingExt3dNow,
  kEncodingVexOp,
  kEncodingVexOpMod,
  kEncodingVexKmov,
  kEncodingVexR_Wx,
  kEncodingVexM,
  kEncodingVexMr_Lx,
  kEncodingVexMr_VM,
  kEncodingVexMri,
  kEncodingVexMri_Lx,
  kEncodingVexMri_Vpextrw,
  kEncodingVexMvr_Wx,
  kEncodingVexRm,
  kEncodingVexRm_ZDI,
  kEncodingVexRm_Wx,
  kEncodingVexRm_Lx,
  kEncodingVexRm_Lx_Narrow,
  kEncodingVexRm_Lx_Bcst,
  kEncodingVexRm_VM,
  kEncodingVexRmi,
  kEncodingVexRmi_Wx,
  kEncodingVexRmi_Lx,
  kEncodingVexRvm,
  kEncodingVexRvm_Wx,
  kEncodingVexRvm_ZDX_Wx,
  kEncodingVexRvm_Lx,
  kEncodingVexRvm_Lx_KEvex,
  kEncodingVexRvm_Lx_2xK,
  kEncodingVexRvmr,
  kEncodingVexRvmr_Lx,
  kEncodingVexRvmi,
  kEncodingVexRvmi_KEvex,
  kEncodingVexRvmi_Lx,
  kEncodingVexRvmi_Lx_KEvex,
  kEncodingVexRmv,
  kEncodingVexRmv_Wx,
  kEncodingVexRmv_VM,
  kEncodingVexRmvRm_VM,
  kEncodingVexRmvi,
  kEncodingVexRmMr,
  kEncodingVexRmMr_Lx,
  kEncodingVexRvmRmv,
  kEncodingVexRvmRmi,
  kEncodingVexRvmRmi_Lx,
  kEncodingVexRvmRmvRmi,
  kEncodingVexRvmMr,
  kEncodingVexRvmMvr,
  kEncodingVexRvmMvr_Lx,
  kEncodingVexRvmVmi,
  kEncodingVexRvmVmi_Lx,
  kEncodingVexRvmVmi_Lx_MEvex,
  kEncodingVexVm,
  kEncodingVexVm_Wx,
  kEncodingVexVmi,
  kEncodingVexVmi_Lx,
  kEncodingVexVmi4_Wx,
  kEncodingVexVmi_Lx_MEvex,
  kEncodingVexRvrmRvmr,
  kEncodingVexRvrmRvmr_Lx,
  kEncodingVexRvrmiRvmri_Lx,
  kEncodingVexMovdMovq,
  kEncodingVexMovssMovsd,
  kEncodingFma4,
  kEncodingFma4_Lx,
  kEncodingAmxCfg,
  kEncodingAmxR,
  kEncodingAmxRm,
  kEncodingAmxMr,
  kEncodingAmxRmv,
  kEncodingCount
};
struct AdditionalInfo {
  uint8_t _inst_flags_index;
  uint8_t _rw_flags_index;
  uint8_t _features[6];
  inline const uint8_t* features_begin() const noexcept { return _features; }
  inline const uint8_t* features_end() const noexcept { return _features + ASMJIT_ARRAY_SIZE(_features); }
};
struct RWInfo {
  enum Category : uint8_t {
    kCategoryGeneric = 0,
    kCategoryGenericEx,
    kCategoryMov,
    kCategoryMovabs,
    kCategoryImul,
    kCategoryMovh64,
    kCategoryPunpcklxx,
    kCategoryVmaskmov,
    kCategoryVmovddup,
    kCategoryVmovmskpd,
    kCategoryVmovmskps,
    kCategoryVmov1_2,
    kCategoryVmov1_4,
    kCategoryVmov1_8,
    kCategoryVmov2_1,
    kCategoryVmov4_1,
    kCategoryVmov8_1
  };
  uint8_t category;
  uint8_t rm_info;
  uint8_t op_info_index[6];
};
struct RWInfoOp {
  uint64_t r_byte_mask;
  uint64_t w_byte_mask;
  uint8_t phys_id;
  uint8_t consecutive_lead_count;
  uint8_t reserved[2];
  OpRWFlags flags;
};
struct RWInfoRm {
  enum Category : uint8_t {
    kCategoryNone = 0,
    kCategoryFixed,
    kCategoryConsistent,
    kCategoryHalf,
    kCategoryQuarter,
    kCategoryEighth
  };
  enum Flags : uint8_t {
    kFlagAmbiguous = 0x01,
    kFlagPextrw = 0x02,
    kFlagMovssMovsd = 0x04,
    kFlagFeatureIfRMI = 0x08
  };
  uint8_t category;
  uint8_t rm_ops_mask;
  uint8_t fixed_size;
  uint8_t flags;
  uint8_t rm_feature;
};
struct RWFlagsInfoTable {
  uint32_t read_flags;
  uint32_t write_flags;
};
extern const uint8_t rw_info_index_a_table[Inst::_kIdCount];
extern const uint8_t rw_info_index_b_table[Inst::_kIdCount];
extern const RWInfo rw_info_a_table[];
extern const RWInfo rw_info_b_table[];
extern const RWInfoOp rw_info_op_table[];
extern const RWInfoRm rw_info_rm_table[];
extern const RWFlagsInfoTable rw_flags_info_table[];
extern const InstRWFlags inst_flags_table[];
extern const uint32_t main_opcode_table[];
extern const uint32_t alt_opcode_table[];
#ifndef ASMJIT_NO_TEXT
extern const InstNameIndex _inst_name_index;
extern const char _inst_name_string_table[];
extern const uint32_t _inst_name_index_table[];
extern const char alias_name_string_table[];
extern const uint32_t alias_name_index_table[];
extern const uint32_t alias_index_to_inst_id_table[];
static constexpr uint32_t kAliasTableSize = 44;
#endif
extern const AdditionalInfo additional_info_table[];
}
ASMJIT_END_SUB_NAMESPACE
#endif
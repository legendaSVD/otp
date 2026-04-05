#ifndef ASMJIT_X86_X86OPCODE_P_H_INCLUDED
#define ASMJIT_X86_X86OPCODE_P_H_INCLUDED
#include <asmjit/x86/x86globals.h>
ASMJIT_BEGIN_SUB_NAMESPACE(x86)
struct Opcode {
  uint32_t v;
  enum Bits : uint32_t {
    kMM_Shift      = 8,
    kMM_Mask       = 0x1Fu << kMM_Shift,
    kMM_00         = 0x00u << kMM_Shift,
    kMM_0F         = 0x01u << kMM_Shift,
    kMM_0F38       = 0x02u << kMM_Shift,
    kMM_0F3A       = 0x03u << kMM_Shift,
    kMM_0F01       = 0x04u << kMM_Shift,
    kMM_MAP5       = 0x05u << kMM_Shift,
    kMM_MAP6       = 0x06u << kMM_Shift,
    kMM_XOP08      = 0x08u << kMM_Shift,
    kMM_XOP09      = 0x09u << kMM_Shift,
    kMM_XOP0A      = 0x0Au << kMM_Shift,
    kMM_IsXOP_Shift= kMM_Shift + 3,
    kMM_IsXOP      = kMM_XOP08,
    kMM_ForceEvex  = 0x10u << kMM_Shift,
    kFPU_2B_Shift  = 10,
    kFPU_2B_Mask   = 0xFF << kFPU_2B_Shift,
    kCDSHL_Shift   = 13,
    kCDSHL_Mask    = 0x7u << kCDSHL_Shift,
    kCDSHL__       = 0x0u << kCDSHL_Shift,
    kCDSHL_0       = 0x0u << kCDSHL_Shift,
    kCDSHL_1       = 0x1u << kCDSHL_Shift,
    kCDSHL_2       = 0x2u << kCDSHL_Shift,
    kCDSHL_3       = 0x3u << kCDSHL_Shift,
    kCDSHL_4       = 0x4u << kCDSHL_Shift,
    kCDSHL_5       = 0x5u << kCDSHL_Shift,
    kCDTT_Shift    = 16,
    kCDTT_Mask     = 0x3u << kCDTT_Shift,
    kCDTT_None     = 0x0u << kCDTT_Shift,
    kCDTT_ByLL     = 0x1u << kCDTT_Shift,
    kCDTT_T1W      = 0x2u << kCDTT_Shift,
    kCDTT_DUP      = 0x3u << kCDTT_Shift,
    kCDTT__        = kCDTT_None,
    kCDTT_FV       = kCDTT_ByLL,
    kCDTT_HV       = kCDTT_ByLL,
    kCDTT_QV       = kCDTT_ByLL,
    kCDTT_FVM      = kCDTT_ByLL,
    kCDTT_T1S      = kCDTT_None,
    kCDTT_T1F      = kCDTT_None,
    kCDTT_T2       = kCDTT_None,
    kCDTT_T4       = kCDTT_None,
    kCDTT_T8       = kCDTT_None,
    kCDTT_HVM      = kCDTT_ByLL,
    kCDTT_QVM      = kCDTT_ByLL,
    kCDTT_OVM      = kCDTT_ByLL,
    kCDTT_128      = kCDTT_None,
    kModO_Shift    = 18,
    kModO_Mask     = 0x7u << kModO_Shift,
    kModO__        = 0x0u,
    kModO_0        = 0x0u << kModO_Shift,
    kModO_1        = 0x1u << kModO_Shift,
    kModO_2        = 0x2u << kModO_Shift,
    kModO_3        = 0x3u << kModO_Shift,
    kModO_4        = 0x4u << kModO_Shift,
    kModO_5        = 0x5u << kModO_Shift,
    kModO_6        = 0x6u << kModO_Shift,
    kModO_7        = 0x7u << kModO_Shift,
    kModRM_Shift    = 13,
    kModRM_Mask     = 0x7u << kModRM_Shift,
    kModRM__        = 0x0u,
    kModRM_0        = 0x0u << kModRM_Shift,
    kModRM_1        = 0x1u << kModRM_Shift,
    kModRM_2        = 0x2u << kModRM_Shift,
    kModRM_3        = 0x3u << kModRM_Shift,
    kModRM_4        = 0x4u << kModRM_Shift,
    kModRM_5        = 0x5u << kModRM_Shift,
    kModRM_6        = 0x6u << kModRM_Shift,
    kModRM_7        = 0x7u << kModRM_Shift,
    kPP_Shift      = 21,
    kPP_VEXMask    = 0x03u << kPP_Shift,
    kPP_FPUMask    = 0x07u << kPP_Shift,
    kPP_00         = 0x00u << kPP_Shift,
    kPP_66         = 0x01u << kPP_Shift,
    kPP_F3         = 0x02u << kPP_Shift,
    kPP_F2         = 0x03u << kPP_Shift,
    kPP_9B         = 0x07u << kPP_Shift,
    kREX_Shift     = 24,
    kREX_Mask      = 0x0Fu << kREX_Shift,
    kB             = 0x01u << kREX_Shift,
    kX             = 0x02u << kREX_Shift,
    kR             = 0x04u << kREX_Shift,
    kW             = 0x08u << kREX_Shift,
    kW_Shift       = kREX_Shift + 3,
    kW__           = 0u << kW_Shift,
    kW_x           = 0u << kW_Shift,
    kW_I           = 0u << kW_Shift,
    kW_0           = 0u << kW_Shift,
    kW_1           = 1u << kW_Shift,
    kEvex_W_Shift  = 28,
    kEvex_W_Mask   = 1u << kEvex_W_Shift,
    kEvex_W__      = 0u << kEvex_W_Shift,
    kEvex_W_x      = 0u << kEvex_W_Shift,
    kEvex_W_I      = 0u << kEvex_W_Shift,
    kEvex_W_0      = 0u << kEvex_W_Shift,
    kEvex_W_1      = 1u << kEvex_W_Shift,
    kLL_Shift      = 29,
    kLL_Mask       = 0x3u << kLL_Shift,
    kLL__          = 0x0u << kLL_Shift,
    kLL_x          = 0x0u << kLL_Shift,
    kLL_I          = 0x0u << kLL_Shift,
    kLL_0          = 0x0u << kLL_Shift,
    kLL_1          = 0x1u << kLL_Shift,
    kLL_2          = 0x2u << kLL_Shift,
    k0      = 0,
    k000000 = kPP_00 | kMM_00,
    k000F00 = kPP_00 | kMM_0F,
    k000F01 = kPP_00 | kMM_0F01,
    k000F0F = kPP_00 | kMM_0F,
    k000F38 = kPP_00 | kMM_0F38,
    k000F3A = kPP_00 | kMM_0F3A,
    k00MAP5 = kPP_00 | kMM_MAP5,
    k00MAP6 = kPP_00 | kMM_MAP6,
    k660000 = kPP_66 | kMM_00,
    k660F00 = kPP_66 | kMM_0F,
    k660F01 = kPP_66 | kMM_0F01,
    k660F38 = kPP_66 | kMM_0F38,
    k660F3A = kPP_66 | kMM_0F3A,
    k66MAP5 = kPP_66 | kMM_MAP5,
    k66MAP6 = kPP_66 | kMM_MAP6,
    kF20000 = kPP_F2 | kMM_00,
    kF20F00 = kPP_F2 | kMM_0F,
    kF20F01 = kPP_F2 | kMM_0F01,
    kF20F38 = kPP_F2 | kMM_0F38,
    kF20F3A = kPP_F2 | kMM_0F3A,
    kF2MAP5 = kPP_F2 | kMM_MAP5,
    kF2MAP6 = kPP_F2 | kMM_MAP6,
    kF30000 = kPP_F3 | kMM_00,
    kF30F00 = kPP_F3 | kMM_0F,
    kF30F01 = kPP_F3 | kMM_0F01,
    kF30F38 = kPP_F3 | kMM_0F38,
    kF30F3A = kPP_F3 | kMM_0F3A,
    kF3MAP5 = kPP_F3 | kMM_MAP5,
    kF3MAP6 = kPP_F3 | kMM_MAP6,
    kFPU_00 = kPP_00 | kMM_00,
    kFPU_9B = kPP_9B | kMM_00,
    kXOP_M8 = kPP_00 | kMM_XOP08,
    kXOP_M9 = kPP_00 | kMM_XOP09,
    kXOP_MA = kPP_00 | kMM_XOP0A
  };
  ASMJIT_INLINE_NODEBUG uint32_t get() const noexcept { return v; }
  ASMJIT_INLINE_NODEBUG bool has_w() const noexcept { return (v & kW) != 0; }
  ASMJIT_INLINE_NODEBUG bool has_66h() const noexcept { return (v & kPP_66) != 0; }
  ASMJIT_INLINE_NODEBUG Opcode& add(uint32_t x) noexcept { return operator+=(x); }
  ASMJIT_INLINE_NODEBUG Opcode& add_66h() noexcept { return operator|=(kPP_66); }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_66h_if(T exp) noexcept { return operator|=(uint32_t(exp) << kPP_Shift); }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_66h_by_size(T size) noexcept { return add_66h_if(size == 2); }
  ASMJIT_INLINE_NODEBUG Opcode& add_w() noexcept { return operator|=(kW); }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_w_if(T exp) noexcept { return operator|=(uint32_t(exp) << kW_Shift); }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_w_by_size(T size) noexcept { return add_w_if(size == 8); }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_prefix_by_size(T size) noexcept {
    static constexpr uint32_t mask[16] = {
      0,
      0,
      kPP_66,
      0,
      0,
      0,
      0,
      0,
      kW
    };
    return operator|=(mask[size & 0xF]);
  }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_arith_by_size(T size) noexcept {
    static const uint32_t mask[16] = {
      0,
      0,
      1 | kPP_66,
      0,
      1,
      0,
      0,
      0,
      1 | kW
    };
    return operator|=(mask[size & 0xF]);
  }
  ASMJIT_INLINE_NODEBUG Opcode& force_evex() noexcept { return operator|=(kMM_ForceEvex); }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& force_evex_if(T exp) noexcept { return operator|=(uint32_t(exp) << Support::ctz_const<uint32_t(kMM_ForceEvex)>); }
  ASMJIT_INLINE_NODEBUG uint32_t extract_mod_o() const noexcept {
    return (v >> kModO_Shift) & 0x07;
  }
  ASMJIT_INLINE_NODEBUG uint32_t extract_mod_rm() const noexcept {
    return (v >> kModRM_Shift) & 0x07;
  }
  ASMJIT_INLINE_NODEBUG uint32_t extract_rex(InstOptions options) const noexcept {
    return (v | uint32_t(options)) >> kREX_Shift;
  }
  ASMJIT_INLINE_NODEBUG uint32_t extract_ll_mmmmm(InstOptions options) const noexcept {
    uint32_t ll_mmmmm = uint32_t(v & (kLL_Mask | kMM_Mask));
    uint32_t vex_evex = uint32_t(options & InstOptions::kX86_Evex);
    return (ll_mmmmm | vex_evex) >> kMM_Shift;
  }
  ASMJIT_INLINE_NODEBUG Opcode& operator=(uint32_t x) noexcept { v = x; return *this; }
  ASMJIT_INLINE_NODEBUG Opcode& operator+=(uint32_t x) noexcept { v += x; return *this; }
  ASMJIT_INLINE_NODEBUG Opcode& operator-=(uint32_t x) noexcept { v -= x; return *this; }
  ASMJIT_INLINE_NODEBUG Opcode& operator&=(uint32_t x) noexcept { v &= x; return *this; }
  ASMJIT_INLINE_NODEBUG Opcode& operator|=(uint32_t x) noexcept { v |= x; return *this; }
  ASMJIT_INLINE_NODEBUG Opcode& operator^=(uint32_t x) noexcept { v ^= x; return *this; }
  ASMJIT_INLINE_NODEBUG uint32_t operator&(uint32_t x) const noexcept { return v & x; }
  ASMJIT_INLINE_NODEBUG uint32_t operator|(uint32_t x) const noexcept { return v | x; }
  ASMJIT_INLINE_NODEBUG uint32_t operator^(uint32_t x) const noexcept { return v ^ x; }
  ASMJIT_INLINE_NODEBUG uint32_t operator<<(uint32_t x) const noexcept { return v << x; }
  ASMJIT_INLINE_NODEBUG uint32_t operator>>(uint32_t x) const noexcept { return v >> x; }
};
ASMJIT_END_SUB_NAMESPACE
#endif
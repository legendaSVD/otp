#ifndef ASMJIT_X86_X86OPERAND_H_INCLUDED
#define ASMJIT_X86_X86OPERAND_H_INCLUDED
#include <asmjit/core/archtraits.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
#include <asmjit/x86/x86globals.h>
ASMJIT_BEGIN_SUB_NAMESPACE(x86)
class Mem;
class Gp;
class Vec;
class Mm;
class KReg;
class SReg;
class CReg;
class DReg;
class St;
class Bnd;
class Tmm;
class Rip;
class Gp : public UniGp {
public:
  enum Id : uint32_t {
    kIdAx  = 0,
    kIdCx  = 1,
    kIdDx  = 2,
    kIdBx  = 3,
    kIdSp  = 4,
    kIdBp  = 5,
    kIdSi  = 6,
    kIdDi  = 7,
    kIdR8  = 8,
    kIdR9  = 9,
    kIdR10 = 10,
    kIdR11 = 11,
    kIdR12 = 12,
    kIdR13 = 13,
    kIdR14 = 14,
    kIdR15 = 15
  };
  ASMJIT_DEFINE_ABSTRACT_REG(Gp, UniGp)
  static ASMJIT_INLINE_CONSTEXPR Gp make_r8(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp8Lo>(), reg_id); }
  [[nodiscard]]
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r8_lo(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp8Lo>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r8_hi(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp8Hi>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r16(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp16>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r32(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp32>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r64(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp64>(), reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r8() const noexcept { return make_r8(id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r8_lo() const noexcept { return make_r8_lo(id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r8_hi() const noexcept { return make_r8_hi(id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r16() const noexcept { return make_r16(id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r32() const noexcept { return make_r32(id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r64() const noexcept { return make_r64(id()); }
};
class Vec : public UniVec {
  ASMJIT_DEFINE_ABSTRACT_REG(Vec, UniVec)
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v128(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec128>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v256(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec256>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v512(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec512>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_xmm(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec128>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_ymm(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec256>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_zmm(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec512>(), reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_xmm() const noexcept { return is_vec128(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_ymm() const noexcept { return is_vec256(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_zmm() const noexcept { return is_vec512(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v128() const noexcept { return make_v128(id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v256() const noexcept { return make_v256(id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v512() const noexcept { return make_v512(id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec xmm() const noexcept { return make_v128(id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec ymm() const noexcept { return make_v256(id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec zmm() const noexcept { return make_v512(id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec half() const noexcept {
    return Vec(is_vec512() ? signature_of_t<RegType::kVec256>() : signature_of_t<RegType::kVec128>(), id());
  }
};
class SReg : public Reg {
  ASMJIT_DEFINE_FINAL_REG(SReg, Reg, RegTraits<RegType::kSegment>)
  enum Id : uint32_t {
    kIdNone = 0,
    kIdEs = 1,
    kIdCs = 2,
    kIdSs = 3,
    kIdDs = 4,
    kIdFs = 5,
    kIdGs = 6,
    kIdCount = 7
  };
};
class KReg : public Reg { ASMJIT_DEFINE_FINAL_REG(KReg, Reg, RegTraits<RegType::kMask>) };
class Mm : public Reg { ASMJIT_DEFINE_FINAL_REG(Mm, Reg, RegTraits<RegType::kX86_Mm>) };
class CReg : public Reg { ASMJIT_DEFINE_FINAL_REG(CReg, Reg, RegTraits<RegType::kControl>) };
class DReg : public Reg { ASMJIT_DEFINE_FINAL_REG(DReg, Reg, RegTraits<RegType::kDebug>) };
class St : public Reg { ASMJIT_DEFINE_FINAL_REG(St, Reg, RegTraits<RegType::kX86_St>) };
class Bnd : public Reg { ASMJIT_DEFINE_FINAL_REG(Bnd, Reg, RegTraits<RegType::kX86_Bnd>) };
class Tmm : public Reg { ASMJIT_DEFINE_FINAL_REG(Tmm, Reg, RegTraits<RegType::kTile>) };
class Rip : public Reg { ASMJIT_DEFINE_FINAL_REG(Rip, Reg, RegTraits<RegType::kPC>) };
#ifndef _DOXYGEN
namespace regs {
#endif
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpb(uint32_t reg_id) noexcept { return Gp::make_r8(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp8(uint32_t reg_id) noexcept { return Gp::make_r8(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpb_lo(uint32_t reg_id) noexcept { return Gp::make_r8_lo(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp8_lo(uint32_t reg_id) noexcept { return Gp::make_r8_lo(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpb_hi(uint32_t reg_id) noexcept { return Gp::make_r8_hi(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp8_hi(uint32_t reg_id) noexcept { return Gp::make_r8_hi(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpw(uint32_t reg_id) noexcept { return Gp::make_r16(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp16(uint32_t reg_id) noexcept { return Gp::make_r16(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpd(uint32_t reg_id) noexcept { return Gp::make_r32(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp32(uint32_t reg_id) noexcept { return Gp::make_r32(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpq(uint32_t reg_id) noexcept { return Gp::make_r64(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp64(uint32_t reg_id) noexcept { return Gp::make_r64(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec xmm(uint32_t reg_id) noexcept { return Vec::make_v128(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec ymm(uint32_t reg_id) noexcept { return Vec::make_v256(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec zmm(uint32_t reg_id) noexcept { return Vec::make_v512(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR KReg k(uint32_t reg_id) noexcept { return KReg(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mm mm(uint32_t reg_id) noexcept { return Mm(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR CReg cr(uint32_t reg_id) noexcept { return CReg(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR DReg dr(uint32_t reg_id) noexcept { return DReg(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR St st(uint32_t reg_id) noexcept { return St(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Bnd bnd(uint32_t reg_id) noexcept { return Bnd(reg_id); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Tmm tmm(uint32_t reg_id) noexcept { return Tmm(reg_id); }
static constexpr Gp al = Gp::make_r8(Gp::kIdAx);
static constexpr Gp bl = Gp::make_r8(Gp::kIdBx);
static constexpr Gp cl = Gp::make_r8(Gp::kIdCx);
static constexpr Gp dl = Gp::make_r8(Gp::kIdDx);
static constexpr Gp spl = Gp::make_r8(Gp::kIdSp);
static constexpr Gp bpl = Gp::make_r8(Gp::kIdBp);
static constexpr Gp sil = Gp::make_r8(Gp::kIdSi);
static constexpr Gp dil = Gp::make_r8(Gp::kIdDi);
static constexpr Gp r8b = Gp::make_r8(Gp::kIdR8);
static constexpr Gp r9b = Gp::make_r8(Gp::kIdR9);
static constexpr Gp r10b = Gp::make_r8(Gp::kIdR10);
static constexpr Gp r11b = Gp::make_r8(Gp::kIdR11);
static constexpr Gp r12b = Gp::make_r8(Gp::kIdR12);
static constexpr Gp r13b = Gp::make_r8(Gp::kIdR13);
static constexpr Gp r14b = Gp::make_r8(Gp::kIdR14);
static constexpr Gp r15b = Gp::make_r8(Gp::kIdR15);
static constexpr Gp ah = Gp::make_r8_hi(Gp::kIdAx);
static constexpr Gp bh = Gp::make_r8_hi(Gp::kIdBx);
static constexpr Gp ch = Gp::make_r8_hi(Gp::kIdCx);
static constexpr Gp dh = Gp::make_r8_hi(Gp::kIdDx);
static constexpr Gp ax = Gp::make_r16(Gp::kIdAx);
static constexpr Gp bx = Gp::make_r16(Gp::kIdBx);
static constexpr Gp cx = Gp::make_r16(Gp::kIdCx);
static constexpr Gp dx = Gp::make_r16(Gp::kIdDx);
static constexpr Gp sp = Gp::make_r16(Gp::kIdSp);
static constexpr Gp bp = Gp::make_r16(Gp::kIdBp);
static constexpr Gp si = Gp::make_r16(Gp::kIdSi);
static constexpr Gp di = Gp::make_r16(Gp::kIdDi);
static constexpr Gp r8w = Gp::make_r16(Gp::kIdR8);
static constexpr Gp r9w = Gp::make_r16(Gp::kIdR9);
static constexpr Gp r10w = Gp::make_r16(Gp::kIdR10);
static constexpr Gp r11w = Gp::make_r16(Gp::kIdR11);
static constexpr Gp r12w = Gp::make_r16(Gp::kIdR12);
static constexpr Gp r13w = Gp::make_r16(Gp::kIdR13);
static constexpr Gp r14w = Gp::make_r16(Gp::kIdR14);
static constexpr Gp r15w = Gp::make_r16(Gp::kIdR15);
static constexpr Gp eax = Gp::make_r32(Gp::kIdAx);
static constexpr Gp ebx = Gp::make_r32(Gp::kIdBx);
static constexpr Gp ecx = Gp::make_r32(Gp::kIdCx);
static constexpr Gp edx = Gp::make_r32(Gp::kIdDx);
static constexpr Gp esp = Gp::make_r32(Gp::kIdSp);
static constexpr Gp ebp = Gp::make_r32(Gp::kIdBp);
static constexpr Gp esi = Gp::make_r32(Gp::kIdSi);
static constexpr Gp edi = Gp::make_r32(Gp::kIdDi);
static constexpr Gp r8d = Gp::make_r32(Gp::kIdR8);
static constexpr Gp r9d = Gp::make_r32(Gp::kIdR9);
static constexpr Gp r10d = Gp::make_r32(Gp::kIdR10);
static constexpr Gp r11d = Gp::make_r32(Gp::kIdR11);
static constexpr Gp r12d = Gp::make_r32(Gp::kIdR12);
static constexpr Gp r13d = Gp::make_r32(Gp::kIdR13);
static constexpr Gp r14d = Gp::make_r32(Gp::kIdR14);
static constexpr Gp r15d = Gp::make_r32(Gp::kIdR15);
static constexpr Gp rax = Gp::make_r64(Gp::kIdAx);
static constexpr Gp rbx = Gp::make_r64(Gp::kIdBx);
static constexpr Gp rcx = Gp::make_r64(Gp::kIdCx);
static constexpr Gp rdx = Gp::make_r64(Gp::kIdDx);
static constexpr Gp rsp = Gp::make_r64(Gp::kIdSp);
static constexpr Gp rbp = Gp::make_r64(Gp::kIdBp);
static constexpr Gp rsi = Gp::make_r64(Gp::kIdSi);
static constexpr Gp rdi = Gp::make_r64(Gp::kIdDi);
static constexpr Gp r8 = Gp::make_r64(Gp::kIdR8);
static constexpr Gp r9 = Gp::make_r64(Gp::kIdR9);
static constexpr Gp r10 = Gp::make_r64(Gp::kIdR10);
static constexpr Gp r11 = Gp::make_r64(Gp::kIdR11);
static constexpr Gp r12 = Gp::make_r64(Gp::kIdR12);
static constexpr Gp r13 = Gp::make_r64(Gp::kIdR13);
static constexpr Gp r14 = Gp::make_r64(Gp::kIdR14);
static constexpr Gp r15 = Gp::make_r64(Gp::kIdR15);
static constexpr Vec xmm0 = Vec::make_v128(0);
static constexpr Vec xmm1 = Vec::make_v128(1);
static constexpr Vec xmm2 = Vec::make_v128(2);
static constexpr Vec xmm3 = Vec::make_v128(3);
static constexpr Vec xmm4 = Vec::make_v128(4);
static constexpr Vec xmm5 = Vec::make_v128(5);
static constexpr Vec xmm6 = Vec::make_v128(6);
static constexpr Vec xmm7 = Vec::make_v128(7);
static constexpr Vec xmm8 = Vec::make_v128(8);
static constexpr Vec xmm9 = Vec::make_v128(9);
static constexpr Vec xmm10 = Vec::make_v128(10);
static constexpr Vec xmm11 = Vec::make_v128(11);
static constexpr Vec xmm12 = Vec::make_v128(12);
static constexpr Vec xmm13 = Vec::make_v128(13);
static constexpr Vec xmm14 = Vec::make_v128(14);
static constexpr Vec xmm15 = Vec::make_v128(15);
static constexpr Vec xmm16 = Vec::make_v128(16);
static constexpr Vec xmm17 = Vec::make_v128(17);
static constexpr Vec xmm18 = Vec::make_v128(18);
static constexpr Vec xmm19 = Vec::make_v128(19);
static constexpr Vec xmm20 = Vec::make_v128(20);
static constexpr Vec xmm21 = Vec::make_v128(21);
static constexpr Vec xmm22 = Vec::make_v128(22);
static constexpr Vec xmm23 = Vec::make_v128(23);
static constexpr Vec xmm24 = Vec::make_v128(24);
static constexpr Vec xmm25 = Vec::make_v128(25);
static constexpr Vec xmm26 = Vec::make_v128(26);
static constexpr Vec xmm27 = Vec::make_v128(27);
static constexpr Vec xmm28 = Vec::make_v128(28);
static constexpr Vec xmm29 = Vec::make_v128(29);
static constexpr Vec xmm30 = Vec::make_v128(30);
static constexpr Vec xmm31 = Vec::make_v128(31);
static constexpr Vec ymm0 = Vec::make_v256(0);
static constexpr Vec ymm1 = Vec::make_v256(1);
static constexpr Vec ymm2 = Vec::make_v256(2);
static constexpr Vec ymm3 = Vec::make_v256(3);
static constexpr Vec ymm4 = Vec::make_v256(4);
static constexpr Vec ymm5 = Vec::make_v256(5);
static constexpr Vec ymm6 = Vec::make_v256(6);
static constexpr Vec ymm7 = Vec::make_v256(7);
static constexpr Vec ymm8 = Vec::make_v256(8);
static constexpr Vec ymm9 = Vec::make_v256(9);
static constexpr Vec ymm10 = Vec::make_v256(10);
static constexpr Vec ymm11 = Vec::make_v256(11);
static constexpr Vec ymm12 = Vec::make_v256(12);
static constexpr Vec ymm13 = Vec::make_v256(13);
static constexpr Vec ymm14 = Vec::make_v256(14);
static constexpr Vec ymm15 = Vec::make_v256(15);
static constexpr Vec ymm16 = Vec::make_v256(16);
static constexpr Vec ymm17 = Vec::make_v256(17);
static constexpr Vec ymm18 = Vec::make_v256(18);
static constexpr Vec ymm19 = Vec::make_v256(19);
static constexpr Vec ymm20 = Vec::make_v256(20);
static constexpr Vec ymm21 = Vec::make_v256(21);
static constexpr Vec ymm22 = Vec::make_v256(22);
static constexpr Vec ymm23 = Vec::make_v256(23);
static constexpr Vec ymm24 = Vec::make_v256(24);
static constexpr Vec ymm25 = Vec::make_v256(25);
static constexpr Vec ymm26 = Vec::make_v256(26);
static constexpr Vec ymm27 = Vec::make_v256(27);
static constexpr Vec ymm28 = Vec::make_v256(28);
static constexpr Vec ymm29 = Vec::make_v256(29);
static constexpr Vec ymm30 = Vec::make_v256(30);
static constexpr Vec ymm31 = Vec::make_v256(31);
static constexpr Vec zmm0 = Vec::make_v512(0);
static constexpr Vec zmm1 = Vec::make_v512(1);
static constexpr Vec zmm2 = Vec::make_v512(2);
static constexpr Vec zmm3 = Vec::make_v512(3);
static constexpr Vec zmm4 = Vec::make_v512(4);
static constexpr Vec zmm5 = Vec::make_v512(5);
static constexpr Vec zmm6 = Vec::make_v512(6);
static constexpr Vec zmm7 = Vec::make_v512(7);
static constexpr Vec zmm8 = Vec::make_v512(8);
static constexpr Vec zmm9 = Vec::make_v512(9);
static constexpr Vec zmm10 = Vec::make_v512(10);
static constexpr Vec zmm11 = Vec::make_v512(11);
static constexpr Vec zmm12 = Vec::make_v512(12);
static constexpr Vec zmm13 = Vec::make_v512(13);
static constexpr Vec zmm14 = Vec::make_v512(14);
static constexpr Vec zmm15 = Vec::make_v512(15);
static constexpr Vec zmm16 = Vec::make_v512(16);
static constexpr Vec zmm17 = Vec::make_v512(17);
static constexpr Vec zmm18 = Vec::make_v512(18);
static constexpr Vec zmm19 = Vec::make_v512(19);
static constexpr Vec zmm20 = Vec::make_v512(20);
static constexpr Vec zmm21 = Vec::make_v512(21);
static constexpr Vec zmm22 = Vec::make_v512(22);
static constexpr Vec zmm23 = Vec::make_v512(23);
static constexpr Vec zmm24 = Vec::make_v512(24);
static constexpr Vec zmm25 = Vec::make_v512(25);
static constexpr Vec zmm26 = Vec::make_v512(26);
static constexpr Vec zmm27 = Vec::make_v512(27);
static constexpr Vec zmm28 = Vec::make_v512(28);
static constexpr Vec zmm29 = Vec::make_v512(29);
static constexpr Vec zmm30 = Vec::make_v512(30);
static constexpr Vec zmm31 = Vec::make_v512(31);
static constexpr Mm mm0 = Mm(0);
static constexpr Mm mm1 = Mm(1);
static constexpr Mm mm2 = Mm(2);
static constexpr Mm mm3 = Mm(3);
static constexpr Mm mm4 = Mm(4);
static constexpr Mm mm5 = Mm(5);
static constexpr Mm mm6 = Mm(6);
static constexpr Mm mm7 = Mm(7);
static constexpr KReg k0 = KReg(0);
static constexpr KReg k1 = KReg(1);
static constexpr KReg k2 = KReg(2);
static constexpr KReg k3 = KReg(3);
static constexpr KReg k4 = KReg(4);
static constexpr KReg k5 = KReg(5);
static constexpr KReg k6 = KReg(6);
static constexpr KReg k7 = KReg(7);
static constexpr SReg no_seg = SReg(SReg::kIdNone);
static constexpr SReg es = SReg(SReg::kIdEs);
static constexpr SReg cs = SReg(SReg::kIdCs);
static constexpr SReg ss = SReg(SReg::kIdSs);
static constexpr SReg ds = SReg(SReg::kIdDs);
static constexpr SReg fs = SReg(SReg::kIdFs);
static constexpr SReg gs = SReg(SReg::kIdGs);
static constexpr CReg cr0 = CReg(0);
static constexpr CReg cr1 = CReg(1);
static constexpr CReg cr2 = CReg(2);
static constexpr CReg cr3 = CReg(3);
static constexpr CReg cr4 = CReg(4);
static constexpr CReg cr5 = CReg(5);
static constexpr CReg cr6 = CReg(6);
static constexpr CReg cr7 = CReg(7);
static constexpr CReg cr8 = CReg(8);
static constexpr CReg cr9 = CReg(9);
static constexpr CReg cr10 = CReg(10);
static constexpr CReg cr11 = CReg(11);
static constexpr CReg cr12 = CReg(12);
static constexpr CReg cr13 = CReg(13);
static constexpr CReg cr14 = CReg(14);
static constexpr CReg cr15 = CReg(15);
static constexpr DReg dr0 = DReg(0);
static constexpr DReg dr1 = DReg(1);
static constexpr DReg dr2 = DReg(2);
static constexpr DReg dr3 = DReg(3);
static constexpr DReg dr4 = DReg(4);
static constexpr DReg dr5 = DReg(5);
static constexpr DReg dr6 = DReg(6);
static constexpr DReg dr7 = DReg(7);
static constexpr DReg dr8 = DReg(8);
static constexpr DReg dr9 = DReg(9);
static constexpr DReg dr10 = DReg(10);
static constexpr DReg dr11 = DReg(11);
static constexpr DReg dr12 = DReg(12);
static constexpr DReg dr13 = DReg(13);
static constexpr DReg dr14 = DReg(14);
static constexpr DReg dr15 = DReg(15);
static constexpr St st0 = St(0);
static constexpr St st1 = St(1);
static constexpr St st2 = St(2);
static constexpr St st3 = St(3);
static constexpr St st4 = St(4);
static constexpr St st5 = St(5);
static constexpr St st6 = St(6);
static constexpr St st7 = St(7);
static constexpr Bnd bnd0 = Bnd(0);
static constexpr Bnd bnd1 = Bnd(1);
static constexpr Bnd bnd2 = Bnd(2);
static constexpr Bnd bnd3 = Bnd(3);
static constexpr Tmm tmm0 = Tmm(0);
static constexpr Tmm tmm1 = Tmm(1);
static constexpr Tmm tmm2 = Tmm(2);
static constexpr Tmm tmm3 = Tmm(3);
static constexpr Tmm tmm4 = Tmm(4);
static constexpr Tmm tmm5 = Tmm(5);
static constexpr Tmm tmm6 = Tmm(6);
static constexpr Tmm tmm7 = Tmm(7);
static constexpr Rip rip = Rip(0);
#ifndef _DOXYGEN
}
using namespace regs;
#endif
class Mem : public BaseMem {
public:
  static inline constexpr uint32_t kSignatureMemAddrTypeShift = 14;
  static inline constexpr uint32_t kSignatureMemAddrTypeMask = 0x03u << kSignatureMemAddrTypeShift;
  static inline constexpr uint32_t kSignatureMemShiftValueShift = 16;
  static inline constexpr uint32_t kSignatureMemShiftValueMask = 0x03u << kSignatureMemShiftValueShift;
  static inline constexpr uint32_t kSignatureMemSegmentShift = 18;
  static inline constexpr uint32_t kSignatureMemSegmentMask = 0x07u << kSignatureMemSegmentShift;
  static inline constexpr uint32_t kSignatureMemBroadcastShift = 21;
  static inline constexpr uint32_t kSignatureMemBroadcastMask = 0x7u << kSignatureMemBroadcastShift;
  enum class AddrType : uint32_t {
    kDefault = 0,
    kAbs = 1,
    kRel = 2,
    kMaxValue = kRel
  };
  enum class Broadcast : uint32_t {
    kNone = 0,
    k1To2 = 1,
    k1To4 = 2,
    k1To8 = 3,
    k1To16 = 4,
    k1To32 = 5,
    k1To64 = 6,
    kMaxValue = k1To64
  };
  ASMJIT_INLINE_CONSTEXPR Mem() noexcept
    : BaseMem() {}
  ASMJIT_INLINE_CONSTEXPR Mem(const Mem& other) noexcept
    : BaseMem(other) {}
  ASMJIT_INLINE_NODEBUG explicit Mem(Globals::NoInit_) noexcept
    : BaseMem(Globals::NoInit) {}
  ASMJIT_INLINE_CONSTEXPR Mem(const Signature& signature, uint32_t base_id, uint32_t index_id, int32_t offset) noexcept
    : BaseMem(signature, base_id, index_id, offset) {}
  ASMJIT_INLINE_CONSTEXPR Mem(const Label& base, int32_t off, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(RegType::kLabelTag) |
              Signature::from_size(size) |
              signature, base.id(), 0, off) {}
  ASMJIT_INLINE_CONSTEXPR Mem(const Label& base, const Reg& index, uint32_t shift, int32_t off, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(RegType::kLabelTag) |
              Signature::from_mem_index_type(index.reg_type()) |
              Signature::from_value<kSignatureMemShiftValueMask>(shift) |
              Signature::from_size(size) |
              signature, base.id(), index.id(), off) {}
  ASMJIT_INLINE_CONSTEXPR Mem(const Reg& base, int32_t off, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(base.reg_type()) |
              Signature::from_size(size) |
              signature, base.id(), 0, off) {}
  ASMJIT_INLINE_CONSTEXPR Mem(const Reg& base, const Reg& index, uint32_t shift, int32_t off, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(base.reg_type()) |
              Signature::from_mem_index_type(index.reg_type()) |
              Signature::from_value<kSignatureMemShiftValueMask>(shift) |
              Signature::from_size(size) |
              signature, base.id(), index.id(), off) {}
  ASMJIT_INLINE_CONSTEXPR explicit Mem(uint64_t base, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_size(size) |
              signature, uint32_t(base >> 32), 0, int32_t(uint32_t(base & 0xFFFFFFFFu))) {}
  ASMJIT_INLINE_CONSTEXPR Mem(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_index_type(index.reg_type()) |
              Signature::from_value<kSignatureMemShiftValueMask>(shift) |
              Signature::from_size(size) |
              signature, uint32_t(base >> 32), index.id(), int32_t(uint32_t(base & 0xFFFFFFFFu))) {}
  ASMJIT_INLINE_NODEBUG Mem& operator=(const Mem& other) noexcept = default;
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem clone() const noexcept { return Mem(*this); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem clone_adjusted(int64_t off) const noexcept {
    Mem result(*this);
    result.add_offset(off);
    return result;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem clone_resized(uint32_t size) const noexcept {
    Mem result(*this);
    result.set_size(size);
    return result;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem clone_broadcasted(Broadcast bcst) const noexcept {
    return Mem((_signature & ~Signature{kSignatureMemBroadcastMask}) | Signature::from_value<kSignatureMemBroadcastMask>(bcst), _base_id, _data[0], int32_t(_data[1]));
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Reg base_reg() const noexcept { return Reg::from_type_and_id(base_type(), base_id()); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Reg index_reg() const noexcept { return Reg::from_type_and_id(index_type(), index_id()); }
  using BaseMem::set_index;
  ASMJIT_INLINE_CONSTEXPR void set_index(const Reg& index, uint32_t shift) noexcept {
    set_index(index);
    set_shift(shift);
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_size() const noexcept { return _signature.has_field<Signature::kSizeMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_size(uint32_t s) const noexcept { return size() == s; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t size() const noexcept { return _signature.get_field<Signature::kSizeMask>(); }
  ASMJIT_INLINE_CONSTEXPR void set_size(uint32_t size) noexcept { _signature.set_field<Signature::kSizeMask>(size); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR AddrType addr_type() const noexcept { return (AddrType)_signature.get_field<kSignatureMemAddrTypeMask>(); }
  ASMJIT_INLINE_CONSTEXPR void set_addr_type(AddrType addr_type) noexcept { _signature.set_field<kSignatureMemAddrTypeMask>(uint32_t(addr_type)); }
  ASMJIT_INLINE_CONSTEXPR void reset_addr_type() noexcept { _signature.set_field<kSignatureMemAddrTypeMask>(uint32_t(AddrType::kDefault)); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_addr_abs() const noexcept { return addr_type() == AddrType::kAbs; }
  ASMJIT_INLINE_CONSTEXPR void set_addr_abs() noexcept { set_addr_type(AddrType::kAbs); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_addr_rel() const noexcept { return addr_type() == AddrType::kRel; }
  ASMJIT_INLINE_CONSTEXPR void set_addr_rel() noexcept { set_addr_type(AddrType::kRel); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_segment() const noexcept { return _signature.has_field<kSignatureMemSegmentMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR SReg segment() const noexcept { return SReg(segment_id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t segment_id() const noexcept { return _signature.get_field<kSignatureMemSegmentMask>(); }
  ASMJIT_INLINE_CONSTEXPR void set_segment(const SReg& seg) noexcept { set_segment(seg.id()); }
  ASMJIT_INLINE_CONSTEXPR void set_segment(uint32_t reg_id) noexcept { _signature.set_field<kSignatureMemSegmentMask>(reg_id); }
  ASMJIT_INLINE_CONSTEXPR void reset_segment() noexcept { _signature.set_field<kSignatureMemSegmentMask>(0); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_shift() const noexcept { return _signature.has_field<kSignatureMemShiftValueMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t shift() const noexcept { return _signature.get_field<kSignatureMemShiftValueMask>(); }
  ASMJIT_INLINE_CONSTEXPR void set_shift(uint32_t shift) noexcept { _signature.set_field<kSignatureMemShiftValueMask>(shift); }
  ASMJIT_INLINE_CONSTEXPR void reset_shift() noexcept { _signature.set_field<kSignatureMemShiftValueMask>(0); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_broadcast() const noexcept { return _signature.has_field<kSignatureMemBroadcastMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Broadcast get_broadcast() const noexcept { return (Broadcast)_signature.get_field<kSignatureMemBroadcastMask>(); }
  ASMJIT_INLINE_CONSTEXPR void set_broadcast(Broadcast b) noexcept { _signature.set_field<kSignatureMemBroadcastMask>(uint32_t(b)); }
  ASMJIT_INLINE_CONSTEXPR void reset_broadcast() noexcept { _signature.set_field<kSignatureMemBroadcastMask>(0); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to1() const noexcept { return clone_broadcasted(Broadcast::kNone); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to2() const noexcept { return clone_broadcasted(Broadcast::k1To2); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to4() const noexcept { return clone_broadcasted(Broadcast::k1To4); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to8() const noexcept { return clone_broadcasted(Broadcast::k1To8); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to16() const noexcept { return clone_broadcasted(Broadcast::k1To16); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to32() const noexcept { return clone_broadcasted(Broadcast::k1To32); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to64() const noexcept { return clone_broadcasted(Broadcast::k1To64); }
};
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, offset, size);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, const Vec& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Label& base, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, offset, size);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Label& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Label& base, const Vec& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Rip& rip_, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(rip_, offset, size);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(uint64_t base, uint32_t size = 0) noexcept {
  return Mem(base, size);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(uint64_t base, const Vec& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_abs(uint64_t base, uint32_t size = 0) noexcept {
  return Mem(base, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs));
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_abs(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs));
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_abs(uint64_t base, const Vec& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs));
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_rel(uint64_t base, uint32_t size = 0) noexcept {
  return Mem(base, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel));
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_rel(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel));
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_rel(uint64_t base, const Vec& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel));
}
#define ASMJIT_MEM_PTR(func, size)                                                              \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Gp& base, int32_t offset = 0) noexcept                                                \
      { return Mem(base, offset, size); }                                                       \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Gp& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0) noexcept           \
      { return Mem(base, index, shift, offset, size); }                                         \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Gp& base, const Vec& index, uint32_t shift = 0, int32_t offset = 0) noexcept          \
      { return Mem(base, index, shift, offset, size); }                                         \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Label& base, int32_t offset = 0) noexcept                                             \
      { return Mem(base, offset, size); }                                                       \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Label& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0) noexcept        \
      { return Mem(base, index, shift, offset, size); }                                         \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Rip& rip_, int32_t offset = 0) noexcept                                               \
      { return Mem(rip_, offset, size); }                                                       \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    uint64_t base) noexcept                                                                     \
      { return Mem(base, size); }                                                               \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    uint64_t base, const Gp& index, uint32_t shift = 0) noexcept                                \
      { return Mem(base, index, shift, size); }                                                 \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    uint64_t base, const Vec& index, uint32_t shift = 0) noexcept                               \
      { return Mem(base, index, shift, size); }                                                 \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_abs(                                                \
    uint64_t base) noexcept                                                                     \
      { return Mem(base, size,                                                                  \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs)); } \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_abs(                                                \
    uint64_t base, const Gp& index, uint32_t shift = 0) noexcept                                \
      { return Mem(base, index, shift, size,                                                    \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs)); } \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_abs(                                                \
    uint64_t base, const Vec& index, uint32_t shift = 0) noexcept                               \
      { return Mem(base, index, shift, size,                                                    \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs)); } \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_rel(                                                \
    uint64_t base) noexcept                                                                     \
      { return Mem(base, size,                                                                  \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel)); } \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_rel(                                                \
    uint64_t base, const Gp& index, uint32_t shift = 0) noexcept                                \
      { return Mem(base, index, shift, size,                                                    \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel)); } \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_rel(                                                \
    uint64_t base, const Vec& index, uint32_t shift = 0) noexcept                               \
      { return Mem(base, index, shift, size,                                                    \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel)); }
ASMJIT_MEM_PTR(ptr_8, 1)
ASMJIT_MEM_PTR(ptr_16, 2)
ASMJIT_MEM_PTR(ptr_32, 4)
ASMJIT_MEM_PTR(ptr_48, 6)
ASMJIT_MEM_PTR(ptr_64, 8)
ASMJIT_MEM_PTR(ptr_80, 10)
ASMJIT_MEM_PTR(ptr_128, 16)
ASMJIT_MEM_PTR(ptr_256, 32)
ASMJIT_MEM_PTR(ptr_512, 64)
ASMJIT_MEM_PTR(byte_ptr, 1)
ASMJIT_MEM_PTR(word_ptr, 2)
ASMJIT_MEM_PTR(dword_ptr, 4)
ASMJIT_MEM_PTR(fword_ptr, 6)
ASMJIT_MEM_PTR(qword_ptr, 8)
ASMJIT_MEM_PTR(tbyte_ptr, 10)
ASMJIT_MEM_PTR(tword_ptr, 10)
ASMJIT_MEM_PTR(oword_ptr, 16)
ASMJIT_MEM_PTR(dqword_ptr, 16)
ASMJIT_MEM_PTR(qqword_ptr, 32)
ASMJIT_MEM_PTR(xmmword_ptr, 16)
ASMJIT_MEM_PTR(ymmword_ptr, 32)
ASMJIT_MEM_PTR(zmmword_ptr, 64)
#undef ASMJIT_MEM_PTR
ASMJIT_END_SUB_NAMESPACE
#endif
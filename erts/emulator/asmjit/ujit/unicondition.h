#ifndef ASMJIT_UJIT_UNICONDITION_H_INCLUDED
#define ASMJIT_UJIT_UNICONDITION_H_INCLUDED
#include <asmjit/ujit/ujitbase.h>
#include <asmjit/ujit/uniop.h>
#if !defined(ASMJIT_NO_UJIT)
ASMJIT_BEGIN_SUB_NAMESPACE(ujit)
class UniCondition {
public:
  UniOpCond op;
  CondCode cond;
  Operand a;
  Operand b;
  ASMJIT_INLINE_NODEBUG UniCondition(UniOpCond op, CondCode cond, const Operand& a, const Operand& b) noexcept
    : op(op),
      cond(cond),
      a(a),
      b(b) {}
  ASMJIT_INLINE_NODEBUG UniCondition(const UniCondition& other) noexcept = default;
  ASMJIT_INLINE_NODEBUG UniCondition& operator=(const UniCondition& other) noexcept = default;
};
static ASMJIT_INLINE UniCondition and_z(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignAnd, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition and_z(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignAnd, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition and_z(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignAnd, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition and_nz(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignAnd, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition and_nz(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignAnd, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition and_nz(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignAnd, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition or_z(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignOr, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition or_z(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignOr, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition or_z(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignOr, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition or_nz(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignOr, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition or_nz(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignOr, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition or_nz(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignOr, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition xor_z(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignXor, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition xor_z(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignXor, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition xor_z(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignXor, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition xor_nz(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignXor, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition xor_nz(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignXor, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition xor_nz(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignXor, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition add_z(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition add_z(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition add_z(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition add_nz(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition add_nz(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition add_nz(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition add_c(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kCarry, a, b); }
static ASMJIT_INLINE UniCondition add_c(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kCarry, a, b); }
static ASMJIT_INLINE UniCondition add_c(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kCarry, a, b); }
static ASMJIT_INLINE UniCondition add_nc(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kNotCarry, a, b); }
static ASMJIT_INLINE UniCondition add_nc(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kNotCarry, a, b); }
static ASMJIT_INLINE UniCondition add_nc(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kNotCarry, a, b); }
static ASMJIT_INLINE UniCondition add_s(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kSign, a, b); }
static ASMJIT_INLINE UniCondition add_s(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kSign, a, b); }
static ASMJIT_INLINE UniCondition add_s(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kSign, a, b); }
static ASMJIT_INLINE UniCondition add_ns(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kNotSign, a, b); }
static ASMJIT_INLINE UniCondition add_ns(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kNotSign, a, b); }
static ASMJIT_INLINE UniCondition add_ns(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignAdd, CondCode::kNotSign, a, b); }
static ASMJIT_INLINE UniCondition sub_z(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition sub_z(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition sub_z(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition sub_nz(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition sub_nz(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition sub_nz(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition sub_c(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kUnsignedLT, a, b); }
static ASMJIT_INLINE UniCondition sub_c(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kUnsignedLT, a, b); }
static ASMJIT_INLINE UniCondition sub_c(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kUnsignedLT, a, b); }
static ASMJIT_INLINE UniCondition sub_nc(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kUnsignedGE, a, b); }
static ASMJIT_INLINE UniCondition sub_nc(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kUnsignedGE, a, b); }
static ASMJIT_INLINE UniCondition sub_nc(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kUnsignedGE, a, b); }
static ASMJIT_INLINE UniCondition sub_s(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kSign, a, b); }
static ASMJIT_INLINE UniCondition sub_s(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kSign, a, b); }
static ASMJIT_INLINE UniCondition sub_s(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kSign, a, b); }
static ASMJIT_INLINE UniCondition sub_ns(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kNotSign, a, b); }
static ASMJIT_INLINE UniCondition sub_ns(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kNotSign, a, b); }
static ASMJIT_INLINE UniCondition sub_ns(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kNotSign, a, b); }
static ASMJIT_INLINE UniCondition sub_ugt(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kUnsignedGT, a, b); }
static ASMJIT_INLINE UniCondition sub_ugt(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kUnsignedGT, a, b); }
static ASMJIT_INLINE UniCondition sub_ugt(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignSub, CondCode::kUnsignedGT, a, b); }
static ASMJIT_INLINE UniCondition shr_z(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignShr, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition shr_z(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignShr, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition shr_z(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignShr, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition shr_nz(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kAssignShr, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition shr_nz(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kAssignShr, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition shr_nz(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kAssignShr, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition cmp_eq(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kEqual, a, b); }
static ASMJIT_INLINE UniCondition cmp_eq(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kEqual, a, b); }
static ASMJIT_INLINE UniCondition cmp_eq(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kEqual, a, b); }
static ASMJIT_INLINE UniCondition cmp_ne(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kNotEqual, a, b); }
static ASMJIT_INLINE UniCondition cmp_ne(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kNotEqual, a, b); }
static ASMJIT_INLINE UniCondition cmp_ne(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kNotEqual, a, b); }
static ASMJIT_INLINE UniCondition scmp_lt(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedLT, a, b); }
static ASMJIT_INLINE UniCondition scmp_lt(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedLT, a, b); }
static ASMJIT_INLINE UniCondition scmp_lt(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedLT, a, b); }
static ASMJIT_INLINE UniCondition scmp_le(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedLE, a, b); }
static ASMJIT_INLINE UniCondition scmp_le(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedLE, a, b); }
static ASMJIT_INLINE UniCondition scmp_le(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedLE, a, b); }
static ASMJIT_INLINE UniCondition scmp_gt(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedGT, a, b); }
static ASMJIT_INLINE UniCondition scmp_gt(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedGT, a, b); }
static ASMJIT_INLINE UniCondition scmp_gt(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedGT, a, b); }
static ASMJIT_INLINE UniCondition scmp_ge(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedGE, a, b); }
static ASMJIT_INLINE UniCondition scmp_ge(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedGE, a, b); }
static ASMJIT_INLINE UniCondition scmp_ge(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kSignedGE, a, b); }
static ASMJIT_INLINE UniCondition ucmp_lt(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedLT, a, b); }
static ASMJIT_INLINE UniCondition ucmp_lt(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedLT, a, b); }
static ASMJIT_INLINE UniCondition ucmp_lt(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedLT, a, b); }
static ASMJIT_INLINE UniCondition ucmp_le(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedLE, a, b); }
static ASMJIT_INLINE UniCondition ucmp_le(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedLE, a, b); }
static ASMJIT_INLINE UniCondition ucmp_le(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedLE, a, b); }
static ASMJIT_INLINE UniCondition ucmp_gt(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedGT, a, b); }
static ASMJIT_INLINE UniCondition ucmp_gt(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedGT, a, b); }
static ASMJIT_INLINE UniCondition ucmp_gt(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedGT, a, b); }
static ASMJIT_INLINE UniCondition ucmp_ge(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedGE, a, b); }
static ASMJIT_INLINE UniCondition ucmp_ge(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedGE, a, b); }
static ASMJIT_INLINE UniCondition ucmp_ge(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kUnsignedGE, a, b); }
static ASMJIT_INLINE UniCondition test_z(const Gp& a) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kEqual, a, Imm(0)); }
static ASMJIT_INLINE UniCondition test_nz(const Gp& a) noexcept { return UniCondition(UniOpCond::kCompare, CondCode::kNotEqual, a, Imm(0)); }
static ASMJIT_INLINE UniCondition test_z(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kTest, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition test_z(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kTest, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition test_z(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kTest, CondCode::kZero, a, b); }
static ASMJIT_INLINE UniCondition test_nz(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kTest, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition test_nz(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kTest, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition test_nz(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kTest, CondCode::kNotZero, a, b); }
static ASMJIT_INLINE UniCondition bt_z(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kBitTest, CondCode::kBTZero, a, b); }
static ASMJIT_INLINE UniCondition bt_z(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kBitTest, CondCode::kBTZero, a, b); }
static ASMJIT_INLINE UniCondition bt_z(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kBitTest, CondCode::kBTZero, a, b); }
static ASMJIT_INLINE UniCondition bt_nz(const Gp& a, const Gp& b) noexcept { return UniCondition(UniOpCond::kBitTest, CondCode::kBTNotZero, a, b); }
static ASMJIT_INLINE UniCondition bt_nz(const Gp& a, const Mem& b) noexcept { return UniCondition(UniOpCond::kBitTest, CondCode::kBTNotZero, a, b); }
static ASMJIT_INLINE UniCondition bt_nz(const Gp& a, const Imm& b) noexcept { return UniCondition(UniOpCond::kBitTest, CondCode::kBTNotZero, a, b); }
ASMJIT_END_SUB_NAMESPACE
#endif
#endif
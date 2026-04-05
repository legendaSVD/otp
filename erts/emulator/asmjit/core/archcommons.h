#ifndef ASMJIT_CORE_ARCHCOMMONS_H_INCLUDED
#define ASMJIT_CORE_ARCHCOMMONS_H_INCLUDED
#include <asmjit/core/globals.h>
ASMJIT_BEGIN_SUB_NAMESPACE(arm)
enum class CondCode : uint8_t {
  kAL             = 0x00u,
  kNA             = 0x01u,
  kEQ             = 0x02u,
  kNE             = 0x03u,
  kCS             = 0x04u,
  kHS             = 0x04u,
  kLO             = 0x05u,
  kCC             = 0x05u,
  kMI             = 0x06u,
  kPL             = 0x07u,
  kVS             = 0x08u,
  kVC             = 0x09u,
  kHI             = 0x0Au,
  kLS             = 0x0Bu,
  kGE             = 0x0Cu,
  kLT             = 0x0Du,
  kGT             = 0x0Eu,
  kLE             = 0x0Fu,
  kZero           = kEQ,
  kNotZero        = kNE,
  kEqual          = kEQ,
  kNotEqual       = kNE,
  kCarry          = kCS,
  kNotCarry       = kCC,
  kSign           = kMI,
  kNotSign        = kPL,
  kNegative       = kMI,
  kPositive       = kPL,
  kOverflow       = kVS,
  kNotOverflow    = kVC,
  kSignedLT       = kLT,
  kSignedLE       = kLE,
  kSignedGT       = kGT,
  kSignedGE       = kGE,
  kUnsignedLT     = kLO,
  kUnsignedLE     = kLS,
  kUnsignedGT     = kHI,
  kUnsignedGE     = kHS,
  kBTZero         = kZero,
  kBTNotZero      = kNotZero,
  kAlways         = kAL,
  kMaxValue       = 0x0Fu
};
static constexpr CondCode _reverse_cond_table[] = {
  CondCode::kAL,
  CondCode::kNA,
  CondCode::kEQ,
  CondCode::kNE,
  CondCode::kLS,
  CondCode::kHI,
  CondCode::kMI,
  CondCode::kPL,
  CondCode::kVS,
  CondCode::kVC,
  CondCode::kLO,
  CondCode::kCS,
  CondCode::kLE,
  CondCode::kGT,
  CondCode::kLT,
  CondCode::kGE
};
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR CondCode reverse_cond(CondCode cond) noexcept { return _reverse_cond_table[uint8_t(cond)]; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR CondCode negate_cond(CondCode cond) noexcept { return CondCode(uint8_t(cond) ^ uint8_t(1)); }
enum class OffsetMode : uint32_t {
  kFixed = 0u,
  kPreIndex = 1u,
  kPostIndex = 2u
};
enum class ShiftOp : uint32_t {
  kLSL = 0x00u,
  kLSR = 0x01u,
  kASR = 0x02u,
  kROR = 0x03u,
  kRRX = 0x04u,
  kMSL = 0x05u,
  kUXTB = 0x06u,
  kUXTH = 0x07u,
  kUXTW = 0x08u,
  kUXTX = 0x09u,
  kSXTB = 0x0Au,
  kSXTH = 0x0Bu,
  kSXTW = 0x0Cu,
  kSXTX = 0x0Du
};
class Shift {
public:
  ShiftOp _op;
  uint32_t _value;
  ASMJIT_INLINE_NODEBUG Shift() noexcept = default;
  ASMJIT_INLINE_CONSTEXPR Shift(const Shift& other) noexcept = default;
  ASMJIT_INLINE_CONSTEXPR Shift(ShiftOp op, uint32_t value) noexcept
    : _op(op),
      _value(value) {}
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR ShiftOp op() const noexcept { return _op; }
  ASMJIT_INLINE_NODEBUG void set_pp(ShiftOp op) noexcept { _op = op; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t value() const noexcept { return _value; }
  ASMJIT_INLINE_NODEBUG void set_value(uint32_t value) noexcept { _value = value; }
};
ASMJIT_END_SUB_NAMESPACE
ASMJIT_BEGIN_SUB_NAMESPACE(a32)
using namespace arm;
enum class DataType : uint32_t {
  kNone = 0,
  kS8 = 1,
  kS16 = 2,
  kS32 = 3,
  kS64 = 4,
  kU8 = 5,
  kU16 = 6,
  kU32 = 7,
  kU64 = 8,
  kF16 = 10,
  kF32 = 11,
  kF64 = 12,
  kP8 = 13,
  kBF16 = 14,
  kP64 = 15,
  kMaxValue = 15
};
static ASMJIT_INLINE_NODEBUG uint32_t data_type_size(DataType dt) noexcept {
  static constexpr uint8_t table[] = { 0, 1, 2, 4, 8, 1, 2, 4, 8, 2, 4, 8, 1, 2, 8 };
  return table[size_t(dt)];
}
ASMJIT_END_SUB_NAMESPACE
ASMJIT_BEGIN_SUB_NAMESPACE(a64)
using namespace arm;
ASMJIT_END_SUB_NAMESPACE
#endif
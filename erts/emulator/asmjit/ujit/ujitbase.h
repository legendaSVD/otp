#ifndef ASMJIT_UJIT_JITBASE_P_H_INCLUDED
#define ASMJIT_UJIT_JITBASE_P_H_INCLUDED
#include <asmjit/host.h>
#if !defined(ASMJIT_NO_UJIT) && !defined(ASMJIT_HAS_HOST_BACKEND)
  #pragma message("ASMJIT_NO_UJIT wasn't defined, however, no backends were found! (ASMJIT_HAS_HOST_BACKEND not defined)")
#endif
#if !defined(ASMJIT_NO_UJIT)
ASMJIT_BEGIN_SUB_NAMESPACE(ujit)
using BackendCompiler = host::Compiler;
using CondCode = host::CondCode;
using Mem = host::Mem;
using Gp = host::Gp;
using Vec = host::Vec;
#if defined(ASMJIT_UJIT_X86)
static ASMJIT_INLINE_NODEBUG Mem mem_ptr(const Label& label, int32_t disp = 0) noexcept { return x86::ptr(label, disp); }
static ASMJIT_INLINE_NODEBUG Mem mem_ptr(const Gp& base, int32_t disp = 0) noexcept { return x86::ptr(base, disp); }
static ASMJIT_INLINE_NODEBUG Mem mem_ptr(const Gp& base, const Gp& index, uint32_t shift = 0, int32_t disp = 0) noexcept { return x86::ptr(base, index, shift, disp); }
#endif
#if defined(ASMJIT_UJIT_AARCH64)
static ASMJIT_INLINE_NODEBUG Mem mem_ptr(const Label& label, int32_t disp = 0) noexcept { return a64::ptr(label, disp); }
static ASMJIT_INLINE_NODEBUG Mem mem_ptr(const Gp& base, int32_t disp = 0) noexcept { return a64::ptr(base, disp); }
static ASMJIT_INLINE_NODEBUG Mem mem_ptr(const Gp& base, const Gp& index, uint32_t shift = 0) noexcept { return a64::ptr(base, index, a64::lsl(shift)); }
#endif
enum class Alignment : uint32_t {};
enum class ScalarOpBehavior : uint8_t {
  kZeroing,
  kPreservingVec128
};
enum class FloatToIntOutsideRangeBehavior : uint8_t {
  kSmallestValue,
  kSaturatedValue
};
enum class FMinFMaxOpBehavior : uint8_t {
  kFiniteValue,
  kTernaryLogic
};
enum class FMAddOpBehavior : uint8_t {
  kNoFMA,
  kFMAStoreToAny,
  kFMAStoreToAccumulator
};
enum class DataWidth : uint8_t {
  k8 = 0,
  k16 = 1,
  k32 = 2,
  k64 = 3,
  k128 = 4
};
enum class VecWidth : uint8_t {
  k128 = 0,
  k256 = 1,
  k512 = 2,
  k1024 = 3
};
enum class Bcst : uint8_t {
  k8 = 0,
  k16 = 1,
  k32 = 2,
  k64 = 3,
  kNA = 0xFE,
  kNA_Unique = 0xFF
};
namespace VecWidthUtils {
static ASMJIT_INLINE OperandSignature signature_of(VecWidth vw) noexcept {
  RegType reg_type = RegType(uint32_t(RegType::kVec128) + uint32_t(vw));
  uint32_t reg_size = 16u << uint32_t(vw);
  return OperandSignature::from_op_type(OperandType::kReg) | OperandSignature::from_reg_type_and_group(reg_type, RegGroup::kVec) | OperandSignature::from_size(reg_size);
}
static ASMJIT_INLINE TypeId type_id_of(VecWidth vw) noexcept {
#if defined(ASMJIT_UJIT_X86)
  static const TypeId table[] = {
    TypeId::kInt32x4,
    TypeId::kInt32x8,
    TypeId::kInt32x16
  };
  return table[size_t(vw)];
#endif
#if defined(ASMJIT_UJIT_AARCH64)
  Support::maybe_unused(vw);
  return TypeId::kInt32x4;
#endif
}
static ASMJIT_INLINE_NODEBUG VecWidth vec_width_of(const Vec& reg) noexcept {
  return VecWidth(uint32_t(reg.reg_type()) - uint32_t(RegType::kVec128));
}
static ASMJIT_INLINE_NODEBUG VecWidth vec_width_of(VecWidth vw, DataWidth data_width, uint32_t n) noexcept {
  return VecWidth(Support::min<uint32_t>((n << uint32_t(data_width)) >> 5, uint32_t(vw)));
}
static ASMJIT_INLINE Vec clone_vec_as(const Vec& src, VecWidth vw) noexcept {
  Vec result(src);
  result.set_signature(signature_of(vw));
  return result;
}
}
class OpArray {
public:
  using Op = Operand_;
  static inline constexpr size_t kMaxSize = 8;
  size_t _size;
  Operand_ v[kMaxSize];
  ASMJIT_INLINE_NODEBUG OpArray() noexcept { reset(); }
  ASMJIT_INLINE_NODEBUG explicit OpArray(const Op& op0) noexcept {
    init(op0);
  }
  ASMJIT_INLINE_NODEBUG OpArray(const Op& op0, const Op& op1) noexcept {
    init(op0, op1);
  }
  ASMJIT_INLINE_NODEBUG OpArray(const Op& op0, const Op& op1, const Op& op2) noexcept {
    init(op0, op1, op2);
  }
  ASMJIT_INLINE_NODEBUG OpArray(const Op& op0, const Op& op1, const Op& op2, const Op& op3) noexcept {
    init(op0, op1, op2, op3);
  }
  ASMJIT_INLINE_NODEBUG OpArray(const Op& op0, const Op& op1, const Op& op2, const Op& op3, const Op& op4) noexcept {
    init(op0, op1, op2, op3, op4);
  }
  ASMJIT_INLINE_NODEBUG OpArray(const Op& op0, const Op& op1, const Op& op2, const Op& op3, const Op& op4, const Op& op5) noexcept {
    init(op0, op1, op2, op3, op4, op5);
  }
  ASMJIT_INLINE_NODEBUG OpArray(const Op& op0, const Op& op1, const Op& op2, const Op& op3, const Op& op4, const Op& op5, const Op& op6) noexcept {
    init(op0, op1, op2, op3, op4, op5, op6);
  }
  ASMJIT_INLINE_NODEBUG OpArray(const Op& op0, const Op& op1, const Op& op2, const Op& op3, const Op& op4, const Op& op5, const Op& op6, const Op& op7) noexcept {
    init(op0, op1, op2, op3, op4, op5, op6, op7);
  }
  ASMJIT_INLINE_NODEBUG OpArray(const OpArray& other) noexcept { init(other); }
protected:
  ASMJIT_INLINE_NODEBUG OpArray(const OpArray& other, size_t from, size_t inc, size_t limit) noexcept {
    size_t di = 0;
    for (size_t si = from; si < limit; si += inc) {
      v[di++] = other[si];
    }
    _size = di;
  }
protected:
  ASMJIT_INLINE_NODEBUG OpArray& operator=(const OpArray& other) noexcept {
    init(other);
    return *this;
  }
  ASMJIT_INLINE_NODEBUG void _resetFrom(size_t index) noexcept {
    for (size_t i = index; i < kMaxSize; i++) {
      v[index].reset();
    }
  }
public:
  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _size = 0;
    for (size_t i = 0; i < kMaxSize; i++) {
      v[i].reset();
    }
  }
  ASMJIT_INLINE_NODEBUG void init(const Op* array, size_t size) noexcept {
    _size = size;
    if (size) {
      memcpy(v, array, size * sizeof(Op));
    }
    _resetFrom(size);
  }
  ASMJIT_INLINE_NODEBUG void init(const OpArray& other) noexcept {
    init(other.v, other._size);
  }
  ASMJIT_INLINE_NODEBUG void init(const Op& op0) noexcept {
    _size = 1;
    v[0] = op0;
    _resetFrom(1);
  }
  ASMJIT_INLINE_NODEBUG void init(const Op& op0, const Op& op1) noexcept {
    _size = 2;
    v[0] = op0;
    v[1] = op1;
    _resetFrom(2);
  }
  ASMJIT_INLINE_NODEBUG void init(const Op& op0, const Op& op1, const Op& op2) noexcept {
    _size = 3;
    v[0] = op0;
    v[1] = op1;
    v[2] = op2;
    _resetFrom(3);
  }
  ASMJIT_INLINE_NODEBUG void init(const Op& op0, const Op& op1, const Op& op2, const Op& op3) noexcept {
    _size = 4;
    v[0] = op0;
    v[1] = op1;
    v[2] = op2;
    v[3] = op3;
    _resetFrom(4);
  }
  ASMJIT_INLINE_NODEBUG void init(const Op& op0, const Op& op1, const Op& op2, const Op& op3, const Op& op4) noexcept {
    _size = 5;
    v[0] = op0;
    v[1] = op1;
    v[2] = op2;
    v[3] = op3;
    v[4] = op4;
    _resetFrom(5);
  }
  ASMJIT_INLINE_NODEBUG void init(const Op& op0, const Op& op1, const Op& op2, const Op& op3, const Op& op4, const Op& op5) noexcept {
    _size = 6;
    v[0] = op0;
    v[1] = op1;
    v[2] = op2;
    v[3] = op3;
    v[4] = op4;
    v[5] = op5;
    _resetFrom(6);
  }
  ASMJIT_INLINE_NODEBUG void init(const Op& op0, const Op& op1, const Op& op2, const Op& op3, const Op& op4, const Op& op5, const Op& op6) noexcept {
    _size = 7;
    v[0] = op0;
    v[1] = op1;
    v[2] = op2;
    v[3] = op3;
    v[4] = op4;
    v[5] = op5;
    v[6] = op6;
    _resetFrom(7);
  }
  ASMJIT_INLINE_NODEBUG void init(const Op& op0, const Op& op1, const Op& op2, const Op& op3, const Op& op4, const Op& op5, const Op& op6, const Op& op7) noexcept {
    _size = 7;
    v[0] = op0;
    v[1] = op1;
    v[2] = op2;
    v[3] = op3;
    v[4] = op4;
    v[5] = op5;
    v[6] = op6;
    v[7] = op7;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0u; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_scalar() const noexcept { return _size == 1u; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_vector() const noexcept { return _size > 1u; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t max_size() const noexcept { return kMaxSize; }
  [[nodiscard]]
  ASMJIT_INLINE bool equals(const OpArray& other) const noexcept {
    size_t count = size();
    if (count != other.size()) {
      return false;
    }
    for (size_t i = 0; i < count; i++) {
      if (v[i] != other.v[i]) {
        return false;
      }
    }
    return true;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const OpArray& other) const noexcept { return equals(other); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const OpArray& other) const noexcept { return !equals(other); }
  [[nodiscard]]
  ASMJIT_INLINE Operand_& operator[](size_t index) noexcept {
    ASMJIT_ASSERT(index < _size);
    return v[index];
  }
  [[nodiscard]]
  ASMJIT_INLINE const Operand_& operator[](size_t index) const noexcept {
    ASMJIT_ASSERT(index < _size);
    return v[index];
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OpArray lo() const noexcept { return OpArray(*this, 0u, 1u, (_size + 1u) / 2u); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OpArray hi() const noexcept { return OpArray(*this, _size > 1u ? (_size + 1u) / 2u : size_t(0u), 1u, _size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OpArray even() const noexcept { return OpArray(*this, 0u, 2u, _size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OpArray odd() const noexcept { return OpArray(*this, _size > 1u, 2u, _size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OpArray half() const noexcept { return OpArray(*this, 0u, 1u, (_size + 1u) / 2u); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OpArray every_nth(size_t n) const noexcept { return OpArray(*this, 0u, n, _size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OpArray even_odd(size_t from) const noexcept { return OpArray(*this, _size > 1u ? from : size_t(0), 2u, _size); }
};
class VecArray : public OpArray {
public:
  ASMJIT_INLINE_NODEBUG VecArray() noexcept
    : OpArray() {}
  ASMJIT_INLINE_NODEBUG VecArray(const VecArray& other) noexcept
    : OpArray(other) {}
  ASMJIT_INLINE_NODEBUG explicit VecArray(const Vec& op0) noexcept
    : OpArray(op0) {}
  ASMJIT_INLINE_NODEBUG VecArray(const Vec& op0, const Vec& op1) noexcept
    : OpArray(op0, op1) {}
  ASMJIT_INLINE_NODEBUG VecArray(const Vec& op0, const Vec& op1, const Vec& op2) noexcept
    : OpArray(op0, op1, op2) {}
  ASMJIT_INLINE_NODEBUG VecArray(const Vec& op0, const Vec& op1, const Vec& op2, const Vec& op3) noexcept
    : OpArray(op0, op1, op2, op3) {}
  ASMJIT_INLINE_NODEBUG VecArray(const Vec& op0, const Vec& op1, const Vec& op2, const Vec& op3, const Vec& op4) noexcept
    : OpArray(op0, op1, op2, op3, op4) {}
  ASMJIT_INLINE_NODEBUG VecArray(const Vec& op0, const Vec& op1, const Vec& op2, const Vec& op3, const Vec& op4, const Vec& op5) noexcept
    : OpArray(op0, op1, op2, op3, op4, op5) {}
  ASMJIT_INLINE_NODEBUG VecArray(const Vec& op0, const Vec& op1, const Vec& op2, const Vec& op3, const Vec& op4, const Vec& op5, const Vec& op6) noexcept
    : OpArray(op0, op1, op2, op3, op4, op5, op6) {}
  ASMJIT_INLINE_NODEBUG VecArray(const Vec& op0, const Vec& op1, const Vec& op2, const Vec& op3, const Vec& op4, const Vec& op5, const Vec& op6, const Vec& op7) noexcept
    : OpArray(op0, op1, op2, op3, op4, op5, op6, op7) {}
protected:
  ASMJIT_INLINE_NODEBUG VecArray(const VecArray& other, size_t from, size_t inc, size_t limit) noexcept
    : OpArray(other, from, inc, limit) {}
public:
  ASMJIT_INLINE_NODEBUG VecArray& operator=(const VecArray& other) noexcept {
    init(other);
    return *this;
  }
  ASMJIT_INLINE Vec& operator[](size_t index) noexcept {
    ASMJIT_ASSERT(index < _size);
    return static_cast<Vec&>(v[index]);
  }
  ASMJIT_INLINE const Vec& operator[](size_t index) const noexcept {
    ASMJIT_ASSERT(index < _size);
    return static_cast<const Vec&>(v[index]);
  }
public:
  ASMJIT_INLINE_NODEBUG void init(const Vec& op0) noexcept {
    OpArray::init(op0);
  }
  ASMJIT_INLINE_NODEBUG void init(const Vec& op0, const Vec& op1) noexcept {
    OpArray::init(op0, op1);
  }
  ASMJIT_INLINE_NODEBUG void init(const Vec& op0, const Vec& op1, const Vec& op2) noexcept {
    OpArray::init(op0, op1, op2);
  }
  ASMJIT_INLINE_NODEBUG void init(const Vec& op0, const Vec& op1, const Vec& op2, const Vec& op3) noexcept {
    OpArray::init(op0, op1, op2, op3);
  }
  ASMJIT_INLINE_NODEBUG void init(const Vec& op0, const Vec& op1, const Vec& op2, const Vec& op3, const Vec& op4) noexcept {
    OpArray::init(op0, op1, op2, op3, op4);
  }
  ASMJIT_INLINE_NODEBUG void init(const Vec& op0, const Vec& op1, const Vec& op2, const Vec& op3, const Vec& op4, const Vec& op5) noexcept {
    OpArray::init(op0, op1, op2, op3, op4, op5);
  }
  ASMJIT_INLINE_NODEBUG void init(const Vec& op0, const Vec& op1, const Vec& op2, const Vec& op3, const Vec& op4, const Vec& op5, const Vec& op6) noexcept {
    OpArray::init(op0, op1, op2, op3, op4, op5, op6);
  }
  ASMJIT_INLINE_NODEBUG void init(const Vec& op0, const Vec& op1, const Vec& op2, const Vec& op3, const Vec& op4, const Vec& op5, const Vec& op6, const Vec& op7) noexcept {
    OpArray::init(op0, op1, op2, op3, op4, op5, op6, op7);
  }
  ASMJIT_INLINE_NODEBUG void init(const Vec* array, size_t size) noexcept {
    OpArray::init(array, size);
  }
  ASMJIT_INLINE_NODEBUG void init(const VecArray& other) noexcept {
    OpArray::init(other);
  }
  ASMJIT_INLINE Vec& at_unrestricted(size_t index) noexcept {
    ASMJIT_ASSERT(index < kMaxSize);
    return static_cast<Vec&>(v[index]);
  }
  ASMJIT_INLINE const Vec& at_unrestricted(size_t index) const noexcept {
    ASMJIT_ASSERT(index < kMaxSize);
    return static_cast<const Vec&>(v[index]);
  }
  ASMJIT_INLINE void reassign(size_t index, const Vec& new_vec) noexcept {
    ASMJIT_ASSERT(index < kMaxSize);
    v[index] = new_vec;
  }
  ASMJIT_INLINE_NODEBUG VecArray lo() const noexcept { return VecArray(*this, 0, 1, (_size + 1) / 2); }
  ASMJIT_INLINE_NODEBUG VecArray hi() const noexcept { return VecArray(*this, _size > 1 ? (_size + 1) / 2 : 0, 1, _size); }
  ASMJIT_INLINE_NODEBUG VecArray even() const noexcept { return VecArray(*this, 0, 2, _size); }
  ASMJIT_INLINE_NODEBUG VecArray odd() const noexcept { return VecArray(*this, _size > 1, 2, _size); }
  ASMJIT_INLINE_NODEBUG VecArray even_odd(size_t from) const noexcept { return VecArray(*this, _size > 1 ? from : 0, 2, _size); }
  ASMJIT_INLINE_NODEBUG VecArray half() const noexcept { return VecArray(*this, 0, 1, (_size + 1) / 2); }
  ASMJIT_INLINE_NODEBUG VecArray every_nth(size_t n) const noexcept { return VecArray(*this, 0, n, _size); }
  ASMJIT_INLINE_NODEBUG void truncate(size_t new_size) noexcept {
    _size = Support::min(_size, new_size);
  }
  ASMJIT_INLINE VecArray clone_as(OperandSignature signature) const noexcept {
    VecArray out(*this);
    for (size_t i = 0; i < out.size(); i++) {
      out.v[i].set_signature(signature);
    }
    return out;
  }
  ASMJIT_INLINE_NODEBUG VecWidth vec_width() const noexcept { return VecWidthUtils::vec_width_of(v[0].as<Vec>()); }
  ASMJIT_INLINE_NODEBUG void set_vec_width(VecWidth vw) noexcept {
    OperandSignature signature = VecWidthUtils::signature_of(vw);
    for (size_t i = 0; i < size(); i++) {
      v[i].set_signature(signature);
    }
  }
  ASMJIT_INLINE_NODEBUG VecArray clone_as(VecWidth vw) const noexcept { return clone_as(VecWidthUtils::signature_of(vw)); }
  ASMJIT_INLINE_NODEBUG VecArray clone_as(const Reg& reg) const noexcept { return clone_as(reg.signature()); }
  ASMJIT_INLINE_NODEBUG bool is_vec128() const noexcept { return v[0].is_vec128(); }
  ASMJIT_INLINE_NODEBUG bool is_vec256() const noexcept { return v[0].is_vec256(); }
  ASMJIT_INLINE_NODEBUG bool is_vec512() const noexcept { return v[0].is_vec512(); }
#if defined(ASMJIT_UJIT_X86)
  ASMJIT_INLINE_NODEBUG VecArray xmm() const noexcept { return clone_as(OperandSignature{RegTraits<RegType::kVec128>::kSignature}); }
  ASMJIT_INLINE_NODEBUG VecArray ymm() const noexcept { return clone_as(OperandSignature{RegTraits<RegType::kVec256>::kSignature}); }
  ASMJIT_INLINE_NODEBUG VecArray zmm() const noexcept { return clone_as(OperandSignature{RegTraits<RegType::kVec512>::kSignature}); }
#endif
  ASMJIT_INLINE_NODEBUG Vec* begin() noexcept { return reinterpret_cast<Vec*>(v); }
  ASMJIT_INLINE_NODEBUG Vec* end() noexcept { return reinterpret_cast<Vec*>(v) + _size; }
  ASMJIT_INLINE_NODEBUG const Vec* begin() const noexcept { return reinterpret_cast<const Vec*>(v); }
  ASMJIT_INLINE_NODEBUG const Vec* end() const noexcept { return reinterpret_cast<const Vec*>(v) + _size; }
  ASMJIT_INLINE_NODEBUG const Vec* cbegin() const noexcept { return reinterpret_cast<const Vec*>(v); }
  ASMJIT_INLINE_NODEBUG const Vec* cend() const noexcept { return reinterpret_cast<const Vec*>(v) + _size; }
};
namespace OpUtils {
template<typename T>
static ASMJIT_INLINE void reset_var_array(T* array, size_t size) noexcept {
  for (size_t i = 0; i < size; i++)
    array[i].reset();
}
template<typename T>
static ASMJIT_INLINE void reset_var_struct(T* data, size_t size = sizeof(T)) noexcept {
  reset_var_array(reinterpret_cast<Reg*>(data), size / sizeof(Reg));
}
static ASMJIT_INLINE_NODEBUG const Operand_& first_op(const Operand_& operand) noexcept { return operand; }
static ASMJIT_INLINE_NODEBUG const Operand_& first_op(const OpArray& op_array) noexcept { return op_array[0]; }
static ASMJIT_INLINE_NODEBUG size_t count_op(const Operand_&) noexcept { return 1u; }
static ASMJIT_INLINE_NODEBUG size_t count_op(const OpArray& op_array) noexcept { return op_array.size(); }
}
struct Swizzle2 {
  uint32_t value;
  ASMJIT_INLINE_CONSTEXPR bool operator==(const Swizzle2& other) const noexcept { return value == other.value; }
  ASMJIT_INLINE_CONSTEXPR bool operator!=(const Swizzle2& other) const noexcept { return value != other.value; }
};
struct Swizzle4 {
  uint32_t value;
  ASMJIT_INLINE_CONSTEXPR bool operator==(const Swizzle4& other) const noexcept { return value == other.value; }
  ASMJIT_INLINE_CONSTEXPR bool operator!=(const Swizzle4& other) const noexcept { return value != other.value; }
};
static ASMJIT_INLINE_CONSTEXPR Swizzle2 swizzle(uint8_t b, uint8_t a) noexcept {
  return Swizzle2{(uint32_t(b) << 8) | a};
}
static ASMJIT_INLINE_CONSTEXPR Swizzle4 swizzle(uint8_t d, uint8_t c, uint8_t b, uint8_t a) noexcept {
  return Swizzle4{(uint32_t(d) << 24) | (uint32_t(c) << 16) | (uint32_t(b) << 8) | a};
}
enum class Perm2x128 : uint32_t {
  kALo = 0,
  kAHi = 1,
  kBLo = 2,
  kBHi = 3,
  kZero = 8
};
static ASMJIT_INLINE_CONSTEXPR uint8_t perm_2x128_imm(Perm2x128 hi, Perm2x128 lo) noexcept {
  return uint8_t((uint32_t(hi) << 4) | (uint32_t(lo)));
}
class ScopedInjector {
  ASMJIT_NONCOPYABLE(ScopedInjector)
public:
  BaseCompiler* _cc {};
  BaseNode** _hook {};
  BaseNode* _prev {};
  bool _hook_was_cursor = false;
  ASMJIT_INLINE ScopedInjector(BaseCompiler* cc, BaseNode** hook) noexcept
    : _cc(cc),
      _hook(hook),
      _prev(cc->set_cursor(*hook)),
      _hook_was_cursor(*hook == _prev) {}
  ASMJIT_INLINE ~ScopedInjector() noexcept {
    *_hook = _cc->cursor();
    if (!_hook_was_cursor) {
      _cc->set_cursor(_prev);
    }
  }
};
ASMJIT_END_SUB_NAMESPACE
#endif
#endif
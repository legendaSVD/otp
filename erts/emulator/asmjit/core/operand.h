#ifndef ASMJIT_CORE_OPERAND_H_INCLUDED
#define ASMJIT_CORE_OPERAND_H_INCLUDED
#include <asmjit/core/archcommons.h>
#include <asmjit/core/type.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
enum class OperandType : uint32_t {
  kNone = 0,
  kReg = 1,
  kMem = 2,
  kRegList = 3,
  kImm = 4,
  kLabel = 5,
  kMaxValue = kRegList
};
static_assert(uint32_t(OperandType::kMem) == uint32_t(OperandType::kReg) + 1,
              "AsmJit requires that `OperandType::kMem` equals `OperandType::kReg + 1`");
using RegMask = uint32_t;
enum class RegType : uint8_t {
  kNone = 0,
  kLabelTag = 1,
  kGp8Lo = 2,
  kGp8Hi = 3,
  kGp16 = 4,
  kGp32 = 5,
  kGp64 = 6,
  kVec8 = 7,
  kVec16 = 8,
  kVec32 = 9,
  kVec64 = 10,
  kVec128 = 11,
  kVec256 = 12,
  kVec512 = 13,
  kVec1024 = 14,
  kVecNLen = 15,
  kMask = 16,
  kTile = 17,
  kSegment = 25,
  kControl = 26,
  kDebug = 27,
  kX86_Mm = 28,
  kX86_St = 29,
  kX86_Bnd = 30,
  kPC = 31,
  kMaxValue = 31
};
ASMJIT_DEFINE_ENUM_COMPARE(RegType)
enum class RegGroup : uint8_t {
  kGp = 0,
  kVec = 1,
  kMask = 2,
  kExtra = 3,
  kTile = 4,
  kSegment = 10,
  kControl = 11,
  kDebug = 12,
  kX86_MM = kExtra,
  kX86_St = 13,
  kX86_Bnd = 14,
  kPC = 15,
  kMaxValue = 15,
  kMaxVirt = Globals::kNumVirtGroups - 1
};
ASMJIT_DEFINE_ENUM_COMPARE(RegGroup)
struct OperandSignature {
  static inline constexpr uint32_t kOpTypeShift = 0;
  static inline constexpr uint32_t kOpTypeMask = 0x07u << kOpTypeShift;
  static inline constexpr uint32_t kRegTypeShift = 3;
  static inline constexpr uint32_t kRegTypeMask = 0x1Fu << kRegTypeShift;
  static inline constexpr uint32_t kRegGroupShift = 8;
  static inline constexpr uint32_t kRegGroupMask = 0x0Fu << kRegGroupShift;
  static inline constexpr uint32_t kMemBaseTypeShift = 3;
  static inline constexpr uint32_t kMemBaseTypeMask = 0x1Fu << kMemBaseTypeShift;
  static inline constexpr uint32_t kMemIndexTypeShift = 8;
  static inline constexpr uint32_t kMemIndexTypeMask = 0x1Fu << kMemIndexTypeShift;
  static inline constexpr uint32_t kMemBaseIndexShift = 3;
  static inline constexpr uint32_t kMemBaseIndexMask = 0x3FFu << kMemBaseIndexShift;
  static inline constexpr uint32_t kMemRegHomeShift = 13;
  static inline constexpr uint32_t kMemRegHomeFlag = 0x01u << kMemRegHomeShift;
  static inline constexpr uint32_t kImmTypeShift = 3;
  static inline constexpr uint32_t kImmTypeMask = 0x01u << kImmTypeShift;
  static inline constexpr uint32_t kPredicateShift = 20;
  static inline constexpr uint32_t kPredicateMask = 0x0Fu << kPredicateShift;
  static inline constexpr uint32_t kSizeShift = 24;
  static inline constexpr uint32_t kSizeMask = 0xFFu << kSizeShift;
  uint32_t _bits;
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature from_bits(uint32_t bits) noexcept {
    return OperandSignature{bits};
  }
  template<uint32_t FieldMask, typename T>
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature from_value(const T& value) noexcept {
    return OperandSignature{uint32_t(value) << Support::ctz_const<FieldMask>};
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature from_op_type(OperandType op_type) noexcept {
    return OperandSignature{uint32_t(op_type) << kOpTypeShift};
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature from_reg_type(RegType reg_type) noexcept {
    return OperandSignature{uint32_t(reg_type) << kRegTypeShift};
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature from_reg_group(RegGroup reg_group) noexcept {
    return OperandSignature{uint32_t(reg_group) << kRegGroupShift};
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature from_reg_type_and_group(RegType reg_type, RegGroup reg_group) noexcept {
    return from_reg_type(reg_type) | from_reg_group(reg_group);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature from_mem_base_type(RegType base_type) noexcept {
    return OperandSignature{uint32_t(base_type) << kMemBaseTypeShift};
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature from_mem_index_type(RegType index_type) noexcept {
    return OperandSignature{uint32_t(index_type) << kMemIndexTypeShift};
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature from_predicate(uint32_t predicate) noexcept {
    return OperandSignature{predicate << kPredicateShift};
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature from_size(uint32_t size) noexcept {
    return OperandSignature{size << kSizeShift};
  }
  ASMJIT_INLINE_CONSTEXPR bool operator!() const noexcept { return _bits == 0; }
  ASMJIT_INLINE_CONSTEXPR explicit operator bool() const noexcept { return _bits != 0; }
  ASMJIT_INLINE_CONSTEXPR OperandSignature& operator|=(uint32_t x) noexcept { _bits |= x; return *this; }
  ASMJIT_INLINE_CONSTEXPR OperandSignature& operator&=(uint32_t x) noexcept { _bits &= x; return *this; }
  ASMJIT_INLINE_CONSTEXPR OperandSignature& operator^=(uint32_t x) noexcept { _bits ^= x; return *this; }
  ASMJIT_INLINE_CONSTEXPR OperandSignature& operator|=(const OperandSignature& other) noexcept { return operator|=(other._bits); }
  ASMJIT_INLINE_CONSTEXPR OperandSignature& operator&=(const OperandSignature& other) noexcept { return operator&=(other._bits); }
  ASMJIT_INLINE_CONSTEXPR OperandSignature& operator^=(const OperandSignature& other) noexcept { return operator^=(other._bits); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandSignature operator~() const noexcept { return OperandSignature{~_bits}; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandSignature operator|(uint32_t x) const noexcept { return OperandSignature{_bits | x}; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandSignature operator&(uint32_t x) const noexcept { return OperandSignature{_bits & x}; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandSignature operator^(uint32_t x) const noexcept { return OperandSignature{_bits ^ x}; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandSignature operator|(const OperandSignature& other) const noexcept { return OperandSignature{_bits | other._bits}; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandSignature operator&(const OperandSignature& other) const noexcept { return OperandSignature{_bits & other._bits}; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandSignature operator^(const OperandSignature& other) const noexcept { return OperandSignature{_bits ^ other._bits}; }
  ASMJIT_INLINE_CONSTEXPR bool operator==(uint32_t x) const noexcept { return _bits == x; }
  ASMJIT_INLINE_CONSTEXPR bool operator!=(uint32_t x) const noexcept { return _bits != x; }
  ASMJIT_INLINE_CONSTEXPR bool operator==(const OperandSignature& other) const noexcept { return _bits == other._bits; }
  ASMJIT_INLINE_CONSTEXPR bool operator!=(const OperandSignature& other) const noexcept { return _bits != other._bits; }
  ASMJIT_INLINE_CONSTEXPR void reset() noexcept { _bits = 0; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t bits() const noexcept { return _bits; }
  ASMJIT_INLINE_CONSTEXPR void set_bits(uint32_t bits) noexcept { _bits = bits; }
  template<uint32_t FieldMask>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_field() const noexcept {
    return (_bits & FieldMask) != 0;
  }
  template<uint32_t FieldMask>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_field(uint32_t value) const noexcept {
    return (_bits & FieldMask) != value << Support::ctz_const<FieldMask>;
  }
  template<uint32_t FieldMask>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t get_field() const noexcept {
    return (_bits >> Support::ctz_const<FieldMask>) & (FieldMask >> Support::ctz_const<FieldMask>);
  }
  template<uint32_t FieldMask>
  ASMJIT_INLINE_CONSTEXPR void set_field(uint32_t value) noexcept {
    ASMJIT_ASSERT(((value << Support::ctz_const<FieldMask>) & ~FieldMask) == 0);
    _bits = (_bits & ~FieldMask) | (value << Support::ctz_const<FieldMask>);
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandSignature subset(uint32_t mask) const noexcept { return OperandSignature{_bits & mask}; }
  template<uint32_t FieldMask, uint32_t kFieldShift = Support::ctz_const<FieldMask>>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandSignature replaced_value(uint32_t value) const noexcept { return OperandSignature{(_bits & ~FieldMask) | (value << kFieldShift)}; }
  template<uint32_t FieldMask>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool matches_signature(const OperandSignature& signature) const noexcept {
    return (_bits & FieldMask) == signature._bits;
  }
  template<uint32_t FieldMask>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool matches_fields(uint32_t bits) const noexcept {
    return (_bits & FieldMask) == bits;
  }
  template<uint32_t FieldMask>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool matches_fields(const OperandSignature& fields) const noexcept {
    return (_bits & FieldMask) == fields._bits;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_valid() const noexcept { return _bits != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandType op_type() const noexcept { return (OperandType)get_field<kOpTypeMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_op_type(OperandType op_type) const noexcept { return get_field<kOpTypeMask>() == uint32_t(op_type); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg() const noexcept { return is_op_type(OperandType::kReg); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg(RegType reg_type) const noexcept {
    constexpr uint32_t kMask = kOpTypeMask | kRegTypeMask;
    return subset(kMask) == (from_op_type(OperandType::kReg) | from_reg_type(reg_type));
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg(RegGroup reg_group) const noexcept {
    constexpr uint32_t kMask = kOpTypeMask | kRegGroupMask;
    return subset(kMask) == (from_op_type(OperandType::kReg) | from_reg_group(reg_group));
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegType reg_type() const noexcept { return (RegType)get_field<kRegTypeMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegGroup reg_group() const noexcept { return (RegGroup)get_field<kRegGroupMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegType mem_base_type() const noexcept { return (RegType)get_field<kMemBaseTypeMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegType mem_index_type() const noexcept { return (RegType)get_field<kMemIndexTypeMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t predicate() const noexcept { return get_field<kPredicateMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t size() const noexcept { return get_field<kSizeMask>(); }
  ASMJIT_INLINE_CONSTEXPR void set_op_type(OperandType op_type) noexcept { set_field<kOpTypeMask>(uint32_t(op_type)); }
  ASMJIT_INLINE_CONSTEXPR void set_reg_type(RegType reg_type) noexcept { set_field<kRegTypeMask>(uint32_t(reg_type)); }
  ASMJIT_INLINE_CONSTEXPR void set_reg_group(RegGroup reg_group) noexcept { set_field<kRegGroupMask>(uint32_t(reg_group)); }
  ASMJIT_INLINE_CONSTEXPR void set_mem_base_type(RegType base_type) noexcept { set_field<kMemBaseTypeMask>(uint32_t(base_type)); }
  ASMJIT_INLINE_CONSTEXPR void set_mem_index_type(RegType index_type) noexcept { set_field<kMemIndexTypeMask>(uint32_t(index_type)); }
  ASMJIT_INLINE_CONSTEXPR void set_predicate(uint32_t predicate) noexcept { set_field<kPredicateMask>(predicate); }
  ASMJIT_INLINE_CONSTEXPR void set_size(uint32_t size) noexcept { set_field<kSizeMask>(size); }
};
struct Operand_ {
  using Signature = OperandSignature;
  static inline constexpr uint32_t kDataMemIndexId = 0;
  static inline constexpr uint32_t kDataMemOffsetLo = 1;
  static inline constexpr uint32_t kDataImmValueLo = Support::ByteOrder::kNative == Support::ByteOrder::kLE ? 0 : 1;
  static inline constexpr uint32_t kDataImmValueHi = Support::ByteOrder::kNative == Support::ByteOrder::kLE ? 1 : 0;
  static inline constexpr uint32_t kVirtIdMin = 256;
  static inline constexpr uint32_t kVirtIdMax = Globals::kInvalidId - 1;
  static inline constexpr uint32_t kVirtIdCount = uint32_t(kVirtIdMax - kVirtIdMin + 1);
  Signature _signature;
  uint32_t _base_id;
  uint32_t _data[2];
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR bool is_virt_id(uint32_t virt_id) noexcept { return virt_id - kVirtIdMin < uint32_t(kVirtIdCount); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR uint32_t virt_index_to_virt_id(uint32_t virt_index) noexcept { return virt_index + kVirtIdMin; }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR uint32_t virt_id_to_index(uint32_t virt_id) noexcept { return virt_id - kVirtIdMin; }
  ASMJIT_INLINE_CONSTEXPR void _init_reg(const Signature& signature, uint32_t id) noexcept {
    _signature = signature;
    _base_id = id;
    _data[0] = 0;
    _data[1] = 0;
  }
  ASMJIT_INLINE_CONSTEXPR void copy_from(const Operand_& other) noexcept {
    _signature._bits = other._signature._bits;
    _base_id = other._base_id;
    _data[0] = other._data[0];
    _data[1] = other._data[1];
  }
  ASMJIT_INLINE_CONSTEXPR void reset() noexcept {
    _signature.reset();
    _base_id = 0;
    _data[0] = 0;
    _data[1] = 0;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool operator==(const Operand_& other) const noexcept { return equals(other); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool operator!=(const Operand_& other) const noexcept { return !equals(other); }
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR T& as() noexcept { return static_cast<T&>(*this); }
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T& as() const noexcept { return static_cast<const T&>(*this); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool equals(const Operand_& other) const noexcept {
    return Support::bool_and(
      _signature == other._signature,
      _base_id == other._base_id,
      _data[0] == other._data[0],
      _data[1] == other._data[1]
    );
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Signature signature() const noexcept { return _signature; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_signature(const Operand_& other) const noexcept { return _signature == other._signature; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_signature(Signature sign) const noexcept { return _signature == sign; }
  ASMJIT_INLINE_CONSTEXPR void set_signature(const Signature& signature) noexcept { _signature = signature; }
  ASMJIT_INLINE_CONSTEXPR void set_signature(uint32_t signature) noexcept { _signature._bits = signature; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandType op_type() const noexcept { return _signature.op_type(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_op_type(OperandType op_type) const noexcept { return _signature.is_op_type(op_type); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_none() const noexcept { return _signature == Signature::from_bits(0); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg() const noexcept { return is_op_type(OperandType::kReg); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg_list() const noexcept { return is_op_type(OperandType::kRegList); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_mem() const noexcept { return is_op_type(OperandType::kMem); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_imm() const noexcept { return is_op_type(OperandType::kImm); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_label() const noexcept { return is_op_type(OperandType::kLabel); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg_or_mem() const noexcept {
    return Support::is_between(op_type(), OperandType::kReg, OperandType::kMem);
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg_or_reg_list_or_mem() const noexcept {
    return Support::is_between(op_type(), OperandType::kReg, OperandType::kRegList);
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t id() const noexcept { return _base_id; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_phys_reg() const noexcept { return is_reg() && _base_id < 0xFFu; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_virt_reg() const noexcept { return is_reg() && _base_id > 0xFFu; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg(RegType reg_type) const noexcept { return _signature.is_reg(reg_type); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg(RegType reg_type, uint32_t reg_id) const noexcept { return Support::bool_and(is_reg(reg_type), _base_id == reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg(RegGroup reg_group) const noexcept { return _signature.is_reg(reg_group); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg(RegGroup reg_group, uint32_t reg_id) const noexcept { return Support::bool_and(is_reg(reg_group), _base_id == reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_pc() const noexcept { return is_reg(RegType::kPC); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp() const noexcept { return is_reg(RegGroup::kGp); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp(uint32_t reg_id) const noexcept { return is_reg(RegGroup::kGp, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8() const noexcept { return Support::bool_or(is_reg(RegType::kGp8Lo), is_reg(RegType::kGp8Hi)); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8(uint32_t reg_id) const noexcept { return Support::bool_and(is_gp8(), id() == reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8_lo() const noexcept { return is_reg(RegType::kGp8Lo); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8_lo(uint32_t reg_id) const noexcept { return is_reg(RegType::kGp8Lo, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8_hi() const noexcept { return is_reg(RegType::kGp8Hi); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8_hi(uint32_t reg_id) const noexcept { return is_reg(RegType::kGp8Hi, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp16() const noexcept { return is_reg(RegType::kGp16); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp16(uint32_t reg_id) const noexcept { return is_reg(RegType::kGp16, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp32() const noexcept { return is_reg(RegType::kGp32); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp32(uint32_t reg_id) const noexcept { return is_reg(RegType::kGp32, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp64() const noexcept { return is_reg(RegType::kGp64); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp64(uint32_t reg_id) const noexcept { return is_reg(RegType::kGp64, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec() const noexcept { return is_reg(RegGroup::kVec); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec(uint32_t reg_id) const noexcept { return is_reg(RegGroup::kVec, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec8() const noexcept { return is_reg(RegType::kVec8); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec8(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec8, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec16() const noexcept { return is_reg(RegType::kVec16); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec16(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec16, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec32() const noexcept { return is_reg(RegType::kVec32); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec32(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec32, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec64() const noexcept { return is_reg(RegType::kVec64); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec64(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec64, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec128() const noexcept { return is_reg(RegType::kVec128); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec128(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec128, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec256() const noexcept { return is_reg(RegType::kVec256); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec256(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec256, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec512() const noexcept { return is_reg(RegType::kVec512); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec512(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec512, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_mask_reg() const noexcept { return is_reg(RegType::kMask); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_mask_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kMask, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_kreg() const noexcept { return is_reg(RegType::kMask); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_kreg(uint32_t reg_id) const noexcept { return is_reg(RegType::kMask, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_tile_reg() const noexcept { return is_reg(RegType::kTile); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_tile_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kTile, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_tmm_reg() const noexcept { return is_reg(RegType::kTile); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_tmm_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kTile, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_segment_reg() const noexcept { return is_reg(RegType::kSegment); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_segment_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kSegment, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_control_reg() const noexcept { return is_reg(RegType::kControl); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_control_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kControl, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_debug_reg() const noexcept { return is_reg(RegType::kDebug); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_debug_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kDebug, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_mm_reg() const noexcept { return is_reg(RegType::kX86_Mm); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_mm_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kX86_Mm, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_st_reg() const noexcept { return is_reg(RegType::kX86_St); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_st_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kX86_St, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_bnd_reg() const noexcept { return is_reg(RegType::kX86_Bnd); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_bnd_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kX86_Bnd, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg_list(RegType type) const noexcept {
    return _signature.subset(Signature::kOpTypeMask | Signature::kRegTypeMask) == (Signature::from_op_type(OperandType::kRegList) | Signature::from_reg_type(type));
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t x86_rm_size() const noexcept { return _signature.size(); }
};
class Operand : public Operand_ {
public:
  ASMJIT_INLINE_CONSTEXPR Operand() noexcept
    : Operand_{ Signature::from_op_type(OperandType::kNone), 0u, { 0u, 0u }} {}
  ASMJIT_INLINE_CONSTEXPR Operand(const Operand& other) noexcept = default;
  ASMJIT_INLINE_CONSTEXPR explicit Operand(const Operand_& other)
    : Operand_(other) {}
  ASMJIT_INLINE_CONSTEXPR Operand(Globals::Init_, const Signature& u0, uint32_t u1, uint32_t u2, uint32_t u3) noexcept
    : Operand_{{u0._bits}, u1, {u2, u3}} {}
  ASMJIT_INLINE_NODEBUG explicit Operand(Globals::NoInit_) noexcept {}
  ASMJIT_INLINE_CONSTEXPR Operand& operator=(const Operand& other) noexcept {
    copy_from(other);
    return *this;
  }
  ASMJIT_INLINE_CONSTEXPR Operand& operator=(const Operand_& other) noexcept {
    copy_from(other);
    return *this;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Operand clone() const noexcept { return Operand(*this); }
};
static_assert(sizeof(Operand) == 16, "asmjit::Operand must be exactly 16 bytes long");
class Label : public Operand {
public:
  ASMJIT_INLINE_CONSTEXPR Label() noexcept
    : Operand(Globals::Init, Signature::from_op_type(OperandType::kLabel), Globals::kInvalidId, 0, 0) {}
  ASMJIT_INLINE_CONSTEXPR Label(const Label& other) noexcept
    : Operand(other) {}
  ASMJIT_INLINE_CONSTEXPR explicit Label(uint32_t id) noexcept
    : Operand(Globals::Init, Signature::from_op_type(OperandType::kLabel), id, 0, 0) {}
  ASMJIT_INLINE_NODEBUG explicit Label(Globals::NoInit_) noexcept
    : Operand(Globals::NoInit) {}
  ASMJIT_INLINE_CONSTEXPR void reset() noexcept {
    _signature = Signature::from_op_type(OperandType::kLabel);
    _base_id = Globals::kInvalidId;
    _data[0] = 0;
    _data[1] = 0;
  }
  ASMJIT_INLINE_CONSTEXPR Label& operator=(const Label& other) noexcept {
    copy_from(other);
    return *this;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_valid() const noexcept { return _base_id != Globals::kInvalidId; }
  ASMJIT_INLINE_CONSTEXPR void set_id(uint32_t id) noexcept { _base_id = id; }
};
template<RegType kRegType>
struct RegTraits {
  static inline constexpr TypeId kTypeId = TypeId::kVoid;
  static inline constexpr uint32_t kValid = 0;
  static inline constexpr RegType kType = RegType::kNone;
  static inline constexpr RegGroup kGroup = RegGroup::kGp;
  static inline constexpr uint32_t kSize = 0u;
  static inline constexpr uint32_t kSignature = 0;
};
#define ASMJIT_DEFINE_REG_TRAITS(REG_TYPE, GROUP, SIZE, TYPE_ID) \
template<>                                                       \
struct RegTraits<REG_TYPE> {                                     \
  static inline constexpr uint32_t kValid = 1;                   \
  static inline constexpr RegType kType = REG_TYPE;              \
  static inline constexpr RegGroup kGroup = GROUP;               \
  static inline constexpr uint32_t kSize = SIZE;                 \
  static inline constexpr TypeId kTypeId = TYPE_ID;              \
                                                                 \
  static inline constexpr uint32_t kSignature =                  \
    (OperandSignature::from_op_type(OperandType::kReg) |         \
     OperandSignature::from_reg_type(kType)            |         \
     OperandSignature::from_reg_group(kGroup)          |         \
     OperandSignature::from_size(kSize)).bits();                 \
                                                                 \
}
ASMJIT_DEFINE_REG_TRAITS(RegType::kPC            , RegGroup::kPC          , 8  , TypeId::kInt64   );
ASMJIT_DEFINE_REG_TRAITS(RegType::kGp8Lo         , RegGroup::kGp          , 1  , TypeId::kInt8    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kGp8Hi         , RegGroup::kGp          , 1  , TypeId::kInt8    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kGp16          , RegGroup::kGp          , 2  , TypeId::kInt16   );
ASMJIT_DEFINE_REG_TRAITS(RegType::kGp32          , RegGroup::kGp          , 4  , TypeId::kInt32   );
ASMJIT_DEFINE_REG_TRAITS(RegType::kGp64          , RegGroup::kGp          , 8  , TypeId::kInt64   );
ASMJIT_DEFINE_REG_TRAITS(RegType::kVec8          , RegGroup::kVec         , 1  , TypeId::kVoid    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kVec16         , RegGroup::kVec         , 2  , TypeId::kVoid    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kVec32         , RegGroup::kVec         , 4  , TypeId::kInt32x1 );
ASMJIT_DEFINE_REG_TRAITS(RegType::kVec64         , RegGroup::kVec         , 8  , TypeId::kInt32x2 );
ASMJIT_DEFINE_REG_TRAITS(RegType::kVec128        , RegGroup::kVec         , 16 , TypeId::kInt32x4 );
ASMJIT_DEFINE_REG_TRAITS(RegType::kVec256        , RegGroup::kVec         , 32 , TypeId::kInt32x8 );
ASMJIT_DEFINE_REG_TRAITS(RegType::kVec512        , RegGroup::kVec         , 64 , TypeId::kInt32x16);
ASMJIT_DEFINE_REG_TRAITS(RegType::kVecNLen       , RegGroup::kVec         , 0  , TypeId::kVoid    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kMask          , RegGroup::kMask        , 0  , TypeId::kVoid    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kTile          , RegGroup::kTile        , 0  , TypeId::kVoid    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kSegment       , RegGroup::kSegment     , 2  , TypeId::kVoid    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kControl       , RegGroup::kControl     , 0  , TypeId::kVoid    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kDebug         , RegGroup::kDebug       , 0  , TypeId::kVoid    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kX86_Mm        , RegGroup::kX86_MM      , 8  , TypeId::kMmx64   );
ASMJIT_DEFINE_REG_TRAITS(RegType::kX86_St        , RegGroup::kX86_St      , 10 , TypeId::kFloat80 );
ASMJIT_DEFINE_REG_TRAITS(RegType::kX86_Bnd       , RegGroup::kX86_Bnd     , 16 , TypeId::kVoid    );
#undef ASMJIT_DEFINE_REG_TRAITS
namespace RegUtils {
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR OperandSignature signature_of(RegType reg_type) noexcept {
  constexpr uint32_t signature_table[] = {
    RegTraits<RegType( 0)>::kSignature, RegTraits<RegType( 1)>::kSignature, RegTraits<RegType( 2)>::kSignature, RegTraits<RegType( 3)>::kSignature,
    RegTraits<RegType( 4)>::kSignature, RegTraits<RegType( 5)>::kSignature, RegTraits<RegType( 6)>::kSignature, RegTraits<RegType( 7)>::kSignature,
    RegTraits<RegType( 8)>::kSignature, RegTraits<RegType( 9)>::kSignature, RegTraits<RegType(10)>::kSignature, RegTraits<RegType(11)>::kSignature,
    RegTraits<RegType(12)>::kSignature, RegTraits<RegType(13)>::kSignature, RegTraits<RegType(14)>::kSignature, RegTraits<RegType(15)>::kSignature,
    RegTraits<RegType(16)>::kSignature, RegTraits<RegType(17)>::kSignature, RegTraits<RegType(18)>::kSignature, RegTraits<RegType(19)>::kSignature,
    RegTraits<RegType(20)>::kSignature, RegTraits<RegType(21)>::kSignature, RegTraits<RegType(22)>::kSignature, RegTraits<RegType(23)>::kSignature,
    RegTraits<RegType(24)>::kSignature, RegTraits<RegType(25)>::kSignature, RegTraits<RegType(26)>::kSignature, RegTraits<RegType(27)>::kSignature,
    RegTraits<RegType(28)>::kSignature, RegTraits<RegType(29)>::kSignature, RegTraits<RegType(30)>::kSignature, RegTraits<RegType(31)>::kSignature
  };
  return OperandSignature{signature_table[size_t(reg_type)]};
}
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG OperandSignature signature_of_vec_by_size(uint32_t size) noexcept {
  RegType reg_type = RegType(Support::ctz((size | 0x40u) & 0x0Fu) - 4u + uint32_t(RegType::kVec128));
  return signature_of(reg_type);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR RegGroup group_of(RegType reg_type) noexcept {
  constexpr RegGroup reg_group_table[] = {
    RegTraits<RegType( 0)>::kGroup, RegTraits<RegType( 1)>::kGroup, RegTraits<RegType( 2)>::kGroup, RegTraits<RegType( 3)>::kGroup,
    RegTraits<RegType( 4)>::kGroup, RegTraits<RegType( 5)>::kGroup, RegTraits<RegType( 6)>::kGroup, RegTraits<RegType( 7)>::kGroup,
    RegTraits<RegType( 8)>::kGroup, RegTraits<RegType( 9)>::kGroup, RegTraits<RegType(10)>::kGroup, RegTraits<RegType(11)>::kGroup,
    RegTraits<RegType(12)>::kGroup, RegTraits<RegType(13)>::kGroup, RegTraits<RegType(14)>::kGroup, RegTraits<RegType(15)>::kGroup,
    RegTraits<RegType(16)>::kGroup, RegTraits<RegType(17)>::kGroup, RegTraits<RegType(18)>::kGroup, RegTraits<RegType(19)>::kGroup,
    RegTraits<RegType(20)>::kGroup, RegTraits<RegType(21)>::kGroup, RegTraits<RegType(22)>::kGroup, RegTraits<RegType(23)>::kGroup,
    RegTraits<RegType(24)>::kGroup, RegTraits<RegType(25)>::kGroup, RegTraits<RegType(26)>::kGroup, RegTraits<RegType(27)>::kGroup,
    RegTraits<RegType(28)>::kGroup, RegTraits<RegType(29)>::kGroup, RegTraits<RegType(30)>::kGroup, RegTraits<RegType(31)>::kGroup
  };
  return reg_group_table[size_t(reg_type)];
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR TypeId type_id_of(RegType reg_type) noexcept {
  constexpr TypeId type_id_table[] = {
    RegTraits<RegType( 0)>::kTypeId, RegTraits<RegType( 1)>::kTypeId, RegTraits<RegType( 2)>::kTypeId, RegTraits<RegType( 3)>::kTypeId,
    RegTraits<RegType( 4)>::kTypeId, RegTraits<RegType( 5)>::kTypeId, RegTraits<RegType( 6)>::kTypeId, RegTraits<RegType( 7)>::kTypeId,
    RegTraits<RegType( 8)>::kTypeId, RegTraits<RegType( 9)>::kTypeId, RegTraits<RegType(10)>::kTypeId, RegTraits<RegType(11)>::kTypeId,
    RegTraits<RegType(12)>::kTypeId, RegTraits<RegType(13)>::kTypeId, RegTraits<RegType(14)>::kTypeId, RegTraits<RegType(15)>::kTypeId,
    RegTraits<RegType(16)>::kTypeId, RegTraits<RegType(17)>::kTypeId, RegTraits<RegType(18)>::kTypeId, RegTraits<RegType(19)>::kTypeId,
    RegTraits<RegType(20)>::kTypeId, RegTraits<RegType(21)>::kTypeId, RegTraits<RegType(22)>::kTypeId, RegTraits<RegType(23)>::kTypeId,
    RegTraits<RegType(24)>::kTypeId, RegTraits<RegType(25)>::kTypeId, RegTraits<RegType(26)>::kTypeId, RegTraits<RegType(27)>::kTypeId,
    RegTraits<RegType(28)>::kTypeId, RegTraits<RegType(29)>::kTypeId, RegTraits<RegType(30)>::kTypeId, RegTraits<RegType(31)>::kTypeId
  };
  return type_id_table[size_t(reg_type)];
}
}
class Reg : public Operand {
public:
  static inline constexpr uint32_t kIdBad = 0xFFu;
  static inline constexpr uint32_t kBaseSignatureMask =
    Signature::kOpTypeMask   |
    Signature::kRegTypeMask  |
    Signature::kRegGroupMask |
    Signature::kSizeMask;
  static inline constexpr uint32_t kTypeNone = uint32_t(RegType::kNone);
  static inline constexpr uint32_t kSignature = Signature::from_op_type(OperandType::kReg).bits();
  template<RegType kRegType>
  static ASMJIT_INLINE_CONSTEXPR Signature signature_of_t() noexcept { return Signature{RegTraits<kRegType>::kSignature}; }
  static ASMJIT_INLINE_CONSTEXPR Signature signature_of(RegType reg_type) noexcept { return RegUtils::signature_of(reg_type); }
  ASMJIT_INLINE_CONSTEXPR Reg() noexcept
    : Operand(Globals::Init, Signature::from_op_type(OperandType::kReg), kIdBad, 0u, 0u) {}
  ASMJIT_INLINE_CONSTEXPR Reg(const Reg& other) noexcept
    : Operand(other) {}
  ASMJIT_INLINE_CONSTEXPR Reg(const Reg& other, uint32_t id) noexcept
    : Operand(Globals::Init, other._signature, id, 0u, 0u) {}
  ASMJIT_INLINE_CONSTEXPR Reg(const Signature& signature, uint32_t id) noexcept
    : Operand(Globals::Init, signature, id, 0u, 0u) {}
  ASMJIT_INLINE_NODEBUG explicit Reg(Globals::NoInit_) noexcept
    : Operand(Globals::NoInit) {}
  static ASMJIT_INLINE_CONSTEXPR Reg from_type_and_id(RegType type, uint32_t id) noexcept { return Reg(signature_of(type), id); }
  ASMJIT_INLINE_CONSTEXPR Reg& operator=(const Reg& other) noexcept {
    copy_from(other);
    return *this;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandSignature base_signature() const noexcept { return _signature & kBaseSignatureMask; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_base_signature(uint32_t signature) const noexcept { return base_signature() == signature; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_base_signature(const OperandSignature& signature) const noexcept { return base_signature() == signature; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_base_signature(const Reg& other) const noexcept { return base_signature() == other.base_signature(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegType reg_type() const noexcept { return _signature.reg_type(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegGroup reg_group() const noexcept { return _signature.reg_group(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_same(const Reg& other) const noexcept { return (_signature == other._signature) & (_base_id == other._base_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_valid() const noexcept { return Support::bool_and(_signature != 0u, _base_id != kIdBad); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_phys_reg() const noexcept { return _base_id < kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_virt_reg() const noexcept { return _base_id > kIdBad; }
  using Operand_::is_reg;
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg(RegType reg_type) const noexcept {
#if defined(__GNUC__)
    if (__builtin_constant_p(reg_type)) {
      return _signature.is_reg(reg_type);
    }
#endif
    return _signature.subset(Signature::kRegTypeMask) == Signature::from_reg_type(reg_type);
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg(RegType reg_type, uint32_t reg_id) const noexcept { return Support::bool_and(is_reg(reg_type), _base_id == reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg(RegGroup reg_group) const noexcept { return _signature.subset(Signature::kRegGroupMask) == Signature::from_reg_group(reg_group); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg(RegGroup reg_group, uint32_t reg_id) const noexcept { return Support::bool_and(is_reg(reg_group), _base_id == reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_pc() const noexcept { return is_reg(RegType::kPC); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp() const noexcept { return is_reg(RegGroup::kGp); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp(uint32_t reg_id) const noexcept { return is_reg(RegGroup::kGp, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8() const noexcept { return Support::bool_or(is_reg(RegType::kGp8Lo), is_reg(RegType::kGp8Hi)); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8(uint32_t reg_id) const noexcept { return Support::bool_and(is_gp8(), id() == reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8_lo() const noexcept { return is_reg(RegType::kGp8Lo); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8_lo(uint32_t reg_id) const noexcept { return is_reg(RegType::kGp8Lo, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8_hi() const noexcept { return is_reg(RegType::kGp8Hi); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp8_hi(uint32_t reg_id) const noexcept { return is_reg(RegType::kGp8Hi, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp16() const noexcept { return is_reg(RegType::kGp16); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp16(uint32_t reg_id) const noexcept { return is_reg(RegType::kGp16, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp32() const noexcept { return is_reg(RegType::kGp32); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp32(uint32_t reg_id) const noexcept { return is_reg(RegType::kGp32, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp64() const noexcept { return is_reg(RegType::kGp64); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp64(uint32_t reg_id) const noexcept { return is_reg(RegType::kGp64, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec() const noexcept { return is_reg(RegGroup::kVec); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec(uint32_t reg_id) const noexcept { return is_reg(RegGroup::kVec, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec8() const noexcept { return is_reg(RegType::kVec8); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec8(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec8, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec16() const noexcept { return is_reg(RegType::kVec16); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec16(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec16, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec32() const noexcept { return is_reg(RegType::kVec32); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec32(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec32, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec64() const noexcept { return is_reg(RegType::kVec64); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec64(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec64, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec128() const noexcept { return is_reg(RegType::kVec128); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec128(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec128, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec256() const noexcept { return is_reg(RegType::kVec256); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec256(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec256, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec512() const noexcept { return is_reg(RegType::kVec512); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec512(uint32_t reg_id) const noexcept { return is_reg(RegType::kVec512, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_mask_reg() const noexcept { return is_reg(RegType::kMask); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_mask_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kMask, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_kreg() const noexcept { return is_reg(RegType::kMask); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_kreg(uint32_t reg_id) const noexcept { return is_reg(RegType::kMask, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_tile_reg() const noexcept { return is_reg(RegType::kTile); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_tile_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kTile, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_tmm_reg() const noexcept { return is_reg(RegType::kTile); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_tmm_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kTile, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_segment_reg() const noexcept { return is_reg(RegType::kSegment); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_segment_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kSegment, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_control_reg() const noexcept { return is_reg(RegType::kControl); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_control_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kControl, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_debug_reg() const noexcept { return is_reg(RegType::kDebug); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_debug_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kDebug, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_mm_reg() const noexcept { return is_reg(RegType::kX86_Mm); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_mm_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kX86_Mm, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_st_reg() const noexcept { return is_reg(RegType::kX86_St); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_st_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kX86_St, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_bnd_reg() const noexcept { return is_reg(RegType::kX86_Bnd); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_bnd_reg(uint32_t reg_id) const noexcept { return is_reg(RegType::kX86_Bnd, reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_size() const noexcept { return _signature.has_field<Signature::kSizeMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_size(uint32_t s) const noexcept { return size() == s; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t size() const noexcept { return _signature.get_field<Signature::kSizeMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t predicate() const noexcept { return _signature.get_field<Signature::kPredicateMask>(); }
  ASMJIT_INLINE_CONSTEXPR void set_predicate(uint32_t predicate) noexcept { _signature.set_field<Signature::kPredicateMask>(predicate); }
  ASMJIT_INLINE_CONSTEXPR void reset_predicate() noexcept { _signature.set_field<Signature::kPredicateMask>(0); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Reg clone() const noexcept { return Reg(*this); }
  template<typename RegT>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegT clone_as() const noexcept { return RegT(Signature(RegT::kSignature), id()); }
  template<typename RegT>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegT clone_as(const RegT& other) const noexcept { return RegT(other.signature(), id()); }
  template<RegType kRegType>
  ASMJIT_INLINE_CONSTEXPR void set_reg_t(uint32_t id) noexcept {
    set_signature(RegTraits<kRegType>::kSignature);
    set_id(id);
  }
  ASMJIT_INLINE_CONSTEXPR void set_id(uint32_t id) noexcept { _base_id = id; }
  ASMJIT_INLINE_CONSTEXPR void set_signature_and_id(const OperandSignature& signature, uint32_t id) noexcept {
    _signature = signature;
    _base_id = id;
  }
};
#define ASMJIT_DEFINE_ABSTRACT_REG(REG, BASE)                                            \
public:                                                                                  \
                                      \
  ASMJIT_INLINE_CONSTEXPR REG() noexcept                                                 \
    : BASE(Signature{kSignature}, kIdBad) {}                                             \
                                                                                         \
                                     \
  ASMJIT_INLINE_CONSTEXPR REG(const REG& other) noexcept                                 \
    : BASE(other) {}                                                                     \
                                                                                         \
                        \
  ASMJIT_INLINE_CONSTEXPR REG(const Reg& other, uint32_t id) noexcept                    \
    : BASE(other, id) {}                                                                 \
                                                                                         \
                                 \
  ASMJIT_INLINE_CONSTEXPR REG(const OperandSignature& sgn, uint32_t id) noexcept         \
    : BASE(sgn, id) {}                                                                   \
                                                                                         \
                \
  ASMJIT_INLINE_NODEBUG explicit REG(Globals::NoInit_) noexcept                          \
    : BASE(Globals::NoInit) {}                                                           \
                                                                                         \
                                 \
  static ASMJIT_INLINE_NODEBUG REG from_type_and_id(RegType type, uint32_t id) noexcept {\
    return REG(signature_of(type), id);                                                  \
  }                                                                                      \
                                                                                         \
                                          \
  [[nodiscard]]                                                                          \
  ASMJIT_INLINE_CONSTEXPR REG clone() const noexcept { return REG(*this); }              \
                                                                                         \
        \
  ASMJIT_INLINE_CONSTEXPR REG& operator=(const REG& other) noexcept {                    \
    copy_from(other);                                                                    \
    return *this;                                                                        \
  }
#define ASMJIT_DEFINE_FINAL_REG(REG, BASE, TRAITS)                                       \
public:                                                                                  \
  static inline constexpr RegType kThisType = TRAITS::kType;                             \
  static inline constexpr RegGroup kThisGroup = TRAITS::kGroup;                          \
  static inline constexpr uint32_t kThisSize  = TRAITS::kSize;                           \
  static inline constexpr uint32_t kSignature = TRAITS::kSignature;                      \
                                                                                         \
  ASMJIT_DEFINE_ABSTRACT_REG(REG, BASE)                                                  \
                                                                                         \
                             \
  ASMJIT_INLINE_CONSTEXPR explicit REG(uint32_t id) noexcept                             \
    : BASE(Signature{kSignature}, id) {}
class UniGp : public Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(UniGp, Reg)
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR UniGp make_r32(uint32_t reg_id) noexcept { return UniGp(signature_of_t<RegType::kGp32>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR UniGp make_r64(uint32_t reg_id) noexcept { return UniGp(signature_of_t<RegType::kGp64>(), reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR UniGp r32() const noexcept { return UniGp(signature_of_t<RegType::kGp32>(), id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR UniGp r64() const noexcept { return UniGp(signature_of_t<RegType::kGp64>(), id()); }
};
class UniVec : public Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(UniVec, Reg)
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR UniVec make_v128(uint32_t reg_id) noexcept { return UniVec(signature_of_t<RegType::kVec128>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR UniVec make_v256(uint32_t reg_id) noexcept { return UniVec(signature_of_t<RegType::kVec256>(), reg_id); }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR UniVec make_v512(uint32_t reg_id) noexcept { return UniVec(signature_of_t<RegType::kVec512>(), reg_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR UniVec v128() const noexcept { return UniVec(signature_of_t<RegType::kVec128>(), id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR UniVec v256() const noexcept { return UniVec(signature_of_t<RegType::kVec256>(), id()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR UniVec v512() const noexcept { return UniVec(signature_of_t<RegType::kVec512>(), id()); }
};
struct RegOnly {
  using Signature = OperandSignature;
  Signature _signature;
  uint32_t _id;
  ASMJIT_INLINE_CONSTEXPR void init(const OperandSignature& signature, uint32_t id) noexcept {
    _signature = signature;
    _id = id;
  }
  ASMJIT_INLINE_CONSTEXPR void init(const Reg& reg) noexcept { init(reg.signature(), reg.id()); }
  ASMJIT_INLINE_CONSTEXPR void init(const RegOnly& reg) noexcept { init(reg.signature(), reg.id()); }
  ASMJIT_INLINE_CONSTEXPR void reset() noexcept { init(Signature::from_bits(0), 0); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_none() const noexcept { return _signature == 0; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg() const noexcept { return _signature != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_phys_reg() const noexcept { return _id < Reg::kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_virt_reg() const noexcept { return _id > Reg::kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OperandSignature signature() const noexcept { return _signature; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t id() const noexcept { return _id; }
  ASMJIT_INLINE_CONSTEXPR void set_id(uint32_t id) noexcept { _id = id; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegType type() const noexcept { return _signature.reg_type(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegGroup group() const noexcept { return _signature.reg_group(); }
  template<typename RegT>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegT to_reg() const noexcept { return RegT(_signature, _id); }
};
class BaseRegList : public Operand {
public:
  static inline constexpr uint32_t kSignature = Signature::from_op_type(OperandType::kRegList).bits();
  ASMJIT_INLINE_CONSTEXPR BaseRegList() noexcept
    : Operand(Globals::Init, Signature::from_op_type(OperandType::kRegList), 0, 0, 0) {}
  ASMJIT_INLINE_CONSTEXPR BaseRegList(const BaseRegList& other) noexcept
    : Operand(other) {}
  ASMJIT_INLINE_CONSTEXPR BaseRegList(const BaseRegList& other, RegMask reg_mask) noexcept
    : Operand(Globals::Init, other._signature, reg_mask, 0, 0) {}
  ASMJIT_INLINE_CONSTEXPR BaseRegList(const Signature& signature, RegMask reg_mask) noexcept
    : Operand(Globals::Init, signature, reg_mask, 0, 0) {}
  ASMJIT_INLINE_NODEBUG explicit BaseRegList(Globals::NoInit_) noexcept
    : Operand(Globals::NoInit) {}
  ASMJIT_INLINE_CONSTEXPR BaseRegList& operator=(const BaseRegList& other) noexcept {
    copy_from(other);
    return *this;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_valid() const noexcept { return bool(unsigned(_signature != 0u) & unsigned(_base_id != 0u)); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_type(RegType type) const noexcept { return _signature.subset(Signature::kRegTypeMask) == Signature::from_reg_type(type); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_group(RegGroup group) const noexcept { return _signature.subset(Signature::kRegGroupMask) == Signature::from_reg_group(group); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_gp() const noexcept { return is_group(RegGroup::kGp); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec() const noexcept { return is_group(RegGroup::kVec); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegType reg_type() const noexcept { return _signature.reg_type(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegGroup reg_group() const noexcept { return _signature.reg_group(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t size() const noexcept { return _signature.get_field<Signature::kSizeMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegMask list() const noexcept { return _base_id; }
  ASMJIT_INLINE_CONSTEXPR void set_list(RegMask mask) noexcept { _base_id = mask; }
  ASMJIT_INLINE_CONSTEXPR void reset_list() noexcept { _base_id = 0; }
  ASMJIT_INLINE_CONSTEXPR void add_list(RegMask mask) noexcept { _base_id |= mask; }
  ASMJIT_INLINE_CONSTEXPR void clear_list(RegMask mask) noexcept { _base_id &= ~mask; }
  ASMJIT_INLINE_CONSTEXPR void and_list(RegMask mask) noexcept { _base_id &= mask; }
  ASMJIT_INLINE_CONSTEXPR void xor_list(RegMask mask) noexcept { _base_id ^= mask; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_reg(uint32_t phys_id) const noexcept { return phys_id < 32u ? (_base_id & (1u << phys_id)) != 0 : false; }
  ASMJIT_INLINE_CONSTEXPR void add_reg(uint32_t phys_id) noexcept { add_list(1u << phys_id); }
  ASMJIT_INLINE_CONSTEXPR void clear_reg(uint32_t phys_id) noexcept { clear_list(1u << phys_id); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR BaseRegList clone() const noexcept { return BaseRegList(*this); }
  template<typename RegListT>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegListT clone_as() const noexcept { return RegListT(Signature(RegListT::kSignature), list()); }
  template<typename RegListT>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegListT clone_as(const RegListT& other) const noexcept { return RegListT(other.signature(), list()); }
};
template<typename RegT>
class RegListT : public BaseRegList {
public:
  ASMJIT_INLINE_CONSTEXPR RegListT() noexcept
    : BaseRegList() {}
  ASMJIT_INLINE_CONSTEXPR RegListT(const RegListT& other) noexcept
    : BaseRegList(other) {}
  ASMJIT_INLINE_CONSTEXPR RegListT(const RegListT& other, RegMask reg_mask) noexcept
    : BaseRegList(other, reg_mask) {}
  ASMJIT_INLINE_CONSTEXPR RegListT(const Signature& signature, RegMask reg_mask) noexcept
    : BaseRegList(signature, reg_mask) {}
  ASMJIT_INLINE_NODEBUG RegListT(const Signature& signature, std::initializer_list<RegT> regs) noexcept
    : BaseRegList(signature, RegMask(0)) { add_regs(regs); }
  ASMJIT_INLINE_NODEBUG explicit RegListT(Globals::NoInit_) noexcept
    : BaseRegList(Globals::NoInit) {}
  ASMJIT_INLINE_CONSTEXPR RegListT& operator=(const RegListT& other) noexcept {
    copy_from(other);
    return *this;
  }
  using BaseRegList::add_list;
  using BaseRegList::clear_list;
  using BaseRegList::and_list;
  using BaseRegList::xor_list;
  ASMJIT_INLINE_CONSTEXPR void add_list(const RegListT<RegT>& other) noexcept { add_list(other.list()); }
  ASMJIT_INLINE_CONSTEXPR void clear_list(const RegListT<RegT>& other) noexcept { clear_list(other.list()); }
  ASMJIT_INLINE_CONSTEXPR void and_list(const RegListT<RegT>& other) noexcept { and_list(other.list()); }
  ASMJIT_INLINE_CONSTEXPR void xor_list(const RegListT<RegT>& other) noexcept { xor_list(other.list()); }
  using BaseRegList::add_reg;
  using BaseRegList::clear_reg;
  ASMJIT_INLINE_CONSTEXPR void add_reg(const RegT& reg) noexcept {
    if (reg.id() < 32u) {
      add_reg(reg.id());
    }
  }
  ASMJIT_INLINE_CONSTEXPR void add_regs(std::initializer_list<RegT> regs) noexcept {
    for (const RegT& reg : regs) {
      add_reg(reg);
    }
  }
  ASMJIT_INLINE_CONSTEXPR void clear_reg(const RegT& reg) noexcept {
    if (reg.id() < 32u) {
      clear_reg(reg.id());
    }
  }
  ASMJIT_INLINE_CONSTEXPR void clear_regs(std::initializer_list<RegT> regs) noexcept {
    for (const RegT& reg : regs) {
      clear_reg(reg);
    }
  }
};
class BaseMem : public Operand {
public:
  ASMJIT_INLINE_CONSTEXPR BaseMem() noexcept
      : Operand(Globals::Init, Signature::from_op_type(OperandType::kMem), 0, 0, 0) {}
  ASMJIT_INLINE_CONSTEXPR BaseMem(const BaseMem& other) noexcept
    : Operand(other) {}
  ASMJIT_INLINE_CONSTEXPR explicit BaseMem(const Reg& base_reg, int32_t offset = 0) noexcept
    : Operand(Globals::Init,
              Signature::from_op_type(OperandType::kMem) | Signature::from_mem_base_type(base_reg.reg_type()),
              base_reg.id(),
              0,
              uint32_t(offset)) {}
  ASMJIT_INLINE_CONSTEXPR BaseMem(const OperandSignature& u0, uint32_t base_id, uint32_t index_id, int32_t offset) noexcept
    : Operand(Globals::Init, u0, base_id, index_id, uint32_t(offset)) {}
  ASMJIT_INLINE_NODEBUG explicit BaseMem(Globals::NoInit_) noexcept
    : Operand(Globals::NoInit) {}
  ASMJIT_INLINE_CONSTEXPR void reset() noexcept {
    _signature = Signature::from_op_type(OperandType::kMem);
    _base_id = 0;
    _data[0] = 0;
    _data[1] = 0;
  }
  ASMJIT_INLINE_CONSTEXPR BaseMem& operator=(const BaseMem& other) noexcept {
    copy_from(other);
    return *this;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR BaseMem clone() const noexcept { return BaseMem(*this); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR BaseMem clone_adjusted(int64_t off) const noexcept {
    BaseMem result(*this);
    result.add_offset(off);
    return result;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_reg_home() const noexcept { return _signature.has_field<Signature::kMemRegHomeFlag>(); }
  ASMJIT_INLINE_CONSTEXPR void set_reg_home() noexcept { _signature |= Signature::kMemRegHomeFlag; }
  ASMJIT_INLINE_CONSTEXPR void clear_reg_home() noexcept { _signature &= ~Signature::kMemRegHomeFlag; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_base() const noexcept {
    return (_signature & Signature::kMemBaseTypeMask) != 0;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_index() const noexcept {
    return (_signature & Signature::kMemIndexTypeMask) != 0;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_base_or_index() const noexcept {
    return (_signature & Signature::kMemBaseIndexMask) != 0;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_base_and_index() const noexcept {
    return (_signature & Signature::kMemBaseTypeMask) != 0 && (_signature & Signature::kMemIndexTypeMask) != 0;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_base_label() const noexcept {
    return _signature.subset(Signature::kMemBaseTypeMask) == Signature::from_mem_base_type(RegType::kLabelTag);
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_base_reg() const noexcept {
    return _signature.subset(Signature::kMemBaseTypeMask).bits() > Signature::from_mem_base_type(RegType::kLabelTag).bits();
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_index_reg() const noexcept {
    return _signature.subset(Signature::kMemIndexTypeMask).bits() > Signature::from_mem_index_type(RegType::kLabelTag).bits();
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegType base_type() const noexcept { return _signature.mem_base_type(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR RegType index_type() const noexcept { return _signature.mem_index_type(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t base_and_index_types() const noexcept { return _signature.get_field<Signature::kMemBaseIndexMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t base_id() const noexcept { return _base_id; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t index_id() const noexcept { return _data[kDataMemIndexId]; }
  ASMJIT_INLINE_CONSTEXPR void set_base_id(uint32_t id) noexcept { _base_id = id; }
  ASMJIT_INLINE_CONSTEXPR void set_base_type(RegType reg_type) noexcept { _signature.set_mem_base_type(reg_type); }
  ASMJIT_INLINE_CONSTEXPR void set_index_id(uint32_t id) noexcept { _data[kDataMemIndexId] = id; }
  ASMJIT_INLINE_CONSTEXPR void set_index_type(RegType reg_type) noexcept { _signature.set_mem_index_type(reg_type); }
  ASMJIT_INLINE_CONSTEXPR void set_base(const Reg& base) noexcept { return _set_base(base.reg_type(), base.id()); }
  ASMJIT_INLINE_CONSTEXPR void set_index(const Reg& index) noexcept { return _set_index(index.reg_type(), index.id()); }
  ASMJIT_INLINE_CONSTEXPR void _set_base(RegType type, uint32_t id) noexcept {
    _signature.set_field<Signature::kMemBaseTypeMask>(uint32_t(type));
    _base_id = id;
  }
  ASMJIT_INLINE_CONSTEXPR void _set_index(RegType type, uint32_t id) noexcept {
    _signature.set_field<Signature::kMemIndexTypeMask>(uint32_t(type));
    _data[kDataMemIndexId] = id;
  }
  ASMJIT_INLINE_CONSTEXPR void reset_base() noexcept { _set_base(RegType::kNone, 0); }
  ASMJIT_INLINE_CONSTEXPR void reset_index() noexcept { _set_index(RegType::kNone, 0); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_offset_64bit() const noexcept { return base_type() == RegType::kNone; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_offset() const noexcept {
    return (_data[kDataMemOffsetLo] | uint32_t(_base_id & Support::bool_as_mask<uint32_t>(is_offset_64bit()))) != 0;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR int64_t offset() const noexcept {
    return is_offset_64bit() ? int64_t(uint64_t(_data[kDataMemOffsetLo]) | (uint64_t(_base_id) << 32))
                             : int64_t(int32_t(_data[kDataMemOffsetLo]));
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR int32_t offset_lo32() const noexcept { return int32_t(_data[kDataMemOffsetLo]); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR int32_t offset_hi32() const noexcept { return int32_t(_base_id); }
  ASMJIT_INLINE_CONSTEXPR void set_offset(int64_t offset) noexcept {
    uint32_t lo = uint32_t(uint64_t(offset) & 0xFFFFFFFFu);
    uint32_t hi = uint32_t(uint64_t(offset) >> 32);
    uint32_t hi_msk = Support::bool_as_mask<uint32_t>(is_offset_64bit());
    _data[kDataMemOffsetLo] = lo;
    _base_id = (hi & hi_msk) | (_base_id & ~hi_msk);
  }
  ASMJIT_INLINE_CONSTEXPR void set_offset_lo32(int32_t offset) noexcept { _data[kDataMemOffsetLo] = uint32_t(offset); }
  ASMJIT_INLINE_CONSTEXPR void add_offset(int64_t offset) noexcept {
    if (is_offset_64bit()) {
      int64_t result = offset + int64_t(uint64_t(_data[kDataMemOffsetLo]) | (uint64_t(_base_id) << 32));
      _data[kDataMemOffsetLo] = uint32_t(uint64_t(result) & 0xFFFFFFFFu);
      _base_id                 = uint32_t(uint64_t(result) >> 32);
    }
    else {
      _data[kDataMemOffsetLo] += uint32_t(uint64_t(offset) & 0xFFFFFFFFu);
    }
  }
  ASMJIT_INLINE_CONSTEXPR void add_offset_lo32(int32_t offset) noexcept { _data[kDataMemOffsetLo] += uint32_t(offset); }
  ASMJIT_INLINE_CONSTEXPR void reset_offset() noexcept { set_offset(0); }
  ASMJIT_INLINE_CONSTEXPR void reset_offset_lo32() noexcept { set_offset_lo32(0); }
};
enum class ImmType : uint32_t {
  kInt = 0,
  kDouble = 1
};
namespace ImmUtils {
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG int64_t imm_value_from_t(const T& x) noexcept {
  if constexpr (std::is_floating_point_v<T>) {
    return int64_t(Support::bit_cast<uint64_t>(double(x)));
  }
  else {
    return int64_t(x);
  }
}
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG T imm_value_to_t(int64_t x) noexcept {
  if constexpr (std::is_floating_point_v<T>) {
    return T(Support::bit_cast<double>(x));
  }
  else {
    return T(uint64_t(x) & Support::bit_ones<std::make_unsigned_t<T>>);
  }
}
}
class Imm : public Operand {
public:
  template<typename T>
  struct IsConstexprConstructibleAsImmType
    : public std::integral_constant<bool, std::is_enum_v<T> || std::is_pointer_v<T> || std::is_integral_v<T> || std::is_function_v<T>> {};
  template<typename T>
  struct IsConvertibleToImmType
    : public std::integral_constant<bool, IsConstexprConstructibleAsImmType<T>::value || std::is_floating_point_v<T>> {};
  ASMJIT_INLINE_CONSTEXPR Imm() noexcept
    : Operand(Globals::Init, Signature::from_op_type(OperandType::kImm), 0, 0, 0) {}
  ASMJIT_INLINE_CONSTEXPR Imm(const Imm& other) noexcept
    : Operand(other) {}
  ASMJIT_INLINE_CONSTEXPR Imm(const arm::Shift& shift) noexcept
    : Operand(Globals::Init,
              Signature::from_op_type(OperandType::kImm) | Signature::from_predicate(uint32_t(shift.op())),
              0,
              Support::unpack_u32_at_0(shift.value()),
              Support::unpack_u32_at_1(shift.value())) {}
  template<typename T, typename = typename std::enable_if<IsConstexprConstructibleAsImmType<std::decay_t<T>>::value>::type>
  ASMJIT_INLINE_CONSTEXPR Imm(const T& val, const uint32_t predicate = 0) noexcept
    : Operand(Globals::Init,
              Signature::from_op_type(OperandType::kImm) | Signature::from_predicate(predicate),
              0,
              Support::unpack_u32_at_0(int64_t(val)),
              Support::unpack_u32_at_1(int64_t(val))) {}
  ASMJIT_INLINE_NODEBUG Imm(const float& val, const uint32_t predicate = 0) noexcept
    : Operand(Globals::Init,
              Signature::from_op_type(OperandType::kImm) | Signature::from_predicate(predicate),
              0,
              0,
              0) { set_value(val); }
  ASMJIT_INLINE_NODEBUG Imm(const double& val, const uint32_t predicate = 0) noexcept
    : Operand(Globals::Init,
              Signature::from_op_type(OperandType::kImm) | Signature::from_predicate(predicate),
              0,
              0,
              0) { set_value(val); }
  ASMJIT_INLINE_NODEBUG explicit Imm(Globals::NoInit_) noexcept
    : Operand(Globals::NoInit) {}
  ASMJIT_INLINE_CONSTEXPR Imm& operator=(const Imm& other) noexcept {
    copy_from(other);
    return *this;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR ImmType type() const noexcept { return (ImmType)_signature.get_field<Signature::kImmTypeMask>(); }
  ASMJIT_INLINE_CONSTEXPR void set_type(ImmType type) noexcept { _signature.set_field<Signature::kImmTypeMask>(uint32_t(type)); }
  ASMJIT_INLINE_CONSTEXPR void reset_type() noexcept { set_type(ImmType::kInt); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t predicate() const noexcept { return _signature.get_field<Signature::kPredicateMask>(); }
  ASMJIT_INLINE_CONSTEXPR void set_predicate(uint32_t predicate) noexcept { _signature.set_field<Signature::kPredicateMask>(predicate); }
  ASMJIT_INLINE_CONSTEXPR void reset_predicate() noexcept { _signature.set_field<Signature::kPredicateMask>(0); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR int64_t value() const noexcept {
    return int64_t((uint64_t(_data[kDataImmValueHi]) << 32) | _data[kDataImmValueLo]);
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_int() const noexcept { return type() == ImmType::kInt; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_double() const noexcept { return type() == ImmType::kDouble; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_int8() const noexcept { return type() == ImmType::kInt && Support::is_int_n<8>(value()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_uint8() const noexcept { return type() == ImmType::kInt && Support::is_uint_n<8>(value()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_int16() const noexcept { return type() == ImmType::kInt && Support::is_int_n<16>(value()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_uint16() const noexcept { return type() == ImmType::kInt && Support::is_uint_n<16>(value()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_int32() const noexcept { return type() == ImmType::kInt && Support::is_int_n<32>(value()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_uint32() const noexcept { return type() == ImmType::kInt && _data[kDataImmValueHi] == 0; }
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T value_as() const noexcept { return ImmUtils::imm_value_to_t<T>(value()); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR int32_t int_lo32() const noexcept { return int32_t(_data[kDataImmValueLo]); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR int32_t int_hi32() const noexcept { return int32_t(_data[kDataImmValueHi]); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t uint_lo32() const noexcept { return _data[kDataImmValueLo]; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t uint_hi32() const noexcept { return _data[kDataImmValueHi]; }
  template<typename T>
  ASMJIT_INLINE_NODEBUG void set_value(const T& val) noexcept {
    _set_value_internal(ImmUtils::imm_value_from_t(val),
                        std::is_floating_point_v<T> ? ImmType::kDouble : ImmType::kInt);
  }
  ASMJIT_INLINE_CONSTEXPR void _set_value_internal(int64_t val, ImmType type) noexcept {
    set_type(type);
    _data[kDataImmValueHi] = uint32_t(uint64_t(val) >> 32);
    _data[kDataImmValueLo] = uint32_t(uint64_t(val) & 0xFFFFFFFFu);
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Imm clone() const noexcept { return Imm(*this); }
  ASMJIT_INLINE_NODEBUG void sign_extend_int8() noexcept { set_value(int64_t(value_as<int8_t>())); }
  ASMJIT_INLINE_NODEBUG void sign_extend_int16() noexcept { set_value(int64_t(value_as<int16_t>())); }
  ASMJIT_INLINE_NODEBUG void sign_extend_int32() noexcept { set_value(int64_t(value_as<int32_t>())); }
  ASMJIT_INLINE_NODEBUG void zero_extend_uint8() noexcept { set_value(value_as<uint8_t>()); }
  ASMJIT_INLINE_NODEBUG void zero_extend_uint16() noexcept { set_value(value_as<uint16_t>()); }
  ASMJIT_INLINE_NODEBUG void zero_extend_uint32() noexcept { _data[kDataImmValueHi] = 0u; }
};
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Imm imm(const T& val) noexcept { return Imm(val); }
namespace Globals {
  static constexpr const Operand none;
}
namespace Support {
template<typename T, bool kIsImm>
struct ForwardOpImpl {
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG const T& forward(const T& value) noexcept { return value; }
};
template<typename T>
struct ForwardOpImpl<T, true> {
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG Imm forward(const T& value) noexcept { return Imm(value); }
};
template<typename T>
struct ForwardOp : public ForwardOpImpl<T, Imm::IsConvertibleToImmType<std::decay_t<T>>::value> {};
}
ASMJIT_END_NAMESPACE
#endif
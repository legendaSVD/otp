#ifndef ASMJIT_CORE_FIXUP_H_INCLUDED
#define ASMJIT_CORE_FIXUP_H_INCLUDED
#include <asmjit/core/globals.h>
ASMJIT_BEGIN_NAMESPACE
enum class OffsetType : uint8_t {
  kSignedOffset,
  kUnsignedOffset,
  kAArch64_ADR,
  kAArch64_ADRP,
  kThumb32_ADR,
  kThumb32_BLX,
  kThumb32_B,
  kThumb32_BCond,
  kAArch32_ADR,
  kAArch32_U23_SignedOffset,
  kAArch32_U23_0To3At0_4To7At8,
  kAArch32_1To24At0_0At24,
  kMaxValue = kAArch32_1To24At0_0At24
};
struct OffsetFormat {
  OffsetType _type;
  uint8_t _flags;
  uint8_t _region_size;
  uint8_t _value_size;
  uint8_t _value_offset;
  uint8_t _imm_bit_count;
  uint8_t _imm_bit_shift;
  uint8_t _imm_discard_lsb;
  ASMJIT_INLINE_NODEBUG OffsetType type() const noexcept { return _type; }
  ASMJIT_INLINE_NODEBUG bool has_sign_bit() const noexcept {
    return _type == OffsetType::kThumb32_ADR ||
           _type == OffsetType::kAArch32_ADR ||
           _type == OffsetType::kAArch32_U23_SignedOffset ||
           _type == OffsetType::kAArch32_U23_0To3At0_4To7At8;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t flags() const noexcept { return _flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t region_size() const noexcept { return _region_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t value_offset() const noexcept { return _value_offset; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t value_size() const noexcept { return _value_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t imm_bit_count() const noexcept { return _imm_bit_count; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t imm_bit_shift() const noexcept { return _imm_bit_shift; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t imm_discard_lsb() const noexcept { return _imm_discard_lsb; }
  inline void reset_to_simple_value(OffsetType type, size_t value_size) noexcept {
    ASMJIT_ASSERT(value_size <= 8u);
    _type = type;
    _flags = uint8_t(0);
    _region_size = uint8_t(value_size);
    _value_size = uint8_t(value_size);
    _value_offset = uint8_t(0);
    _imm_bit_count = uint8_t(value_size * 8u);
    _imm_bit_shift = uint8_t(0);
    _imm_discard_lsb = uint8_t(0);
  }
  inline void reset_to_imm_value(OffsetType type, size_t value_size, uint32_t imm_bit_shift, uint32_t imm_bit_count, uint32_t imm_discard_lsb) noexcept {
    ASMJIT_ASSERT(value_size <= 8u);
    ASMJIT_ASSERT(imm_bit_shift < value_size * 8u);
    ASMJIT_ASSERT(imm_bit_count <= 64u);
    ASMJIT_ASSERT(imm_discard_lsb <= 64u);
    _type = type;
    _flags = uint8_t(0);
    _region_size = uint8_t(value_size);
    _value_size = uint8_t(value_size);
    _value_offset = uint8_t(0);
    _imm_bit_count = uint8_t(imm_bit_count);
    _imm_bit_shift = uint8_t(imm_bit_shift);
    _imm_discard_lsb = uint8_t(imm_discard_lsb);
  }
  inline void set_region(size_t region_size, size_t value_offset) noexcept {
    _region_size = uint8_t(region_size);
    _value_offset = uint8_t(value_offset);
  }
  inline void set_leading_and_trailing_size(size_t leading_size, size_t trailing_size) noexcept {
    _region_size = uint8_t(leading_size + trailing_size + _value_size);
    _value_offset = uint8_t(leading_size);
  }
};
struct Fixup {
  Fixup* next;
  uint32_t section_id;
  uint32_t label_or_reloc_id;
  size_t offset;
  intptr_t rel;
  OffsetFormat format;
};
ASMJIT_END_NAMESPACE
#endif
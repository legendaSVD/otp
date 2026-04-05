#ifndef ASMJIT_CORE_COMPILERDEFS_H_INCLUDED
#define ASMJIT_CORE_COMPILERDEFS_H_INCLUDED
#include <asmjit/core/api-config.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
#include <asmjit/support/arenastring.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
class RAWorkReg;
enum class VirtRegFlags : uint8_t {
  kNone = 0x00u,
  kIsFixed = 0x01u,
  kIsStackArea = 0x02u,
  kHasStackSlot = 0x04u,
  kAlignmentLog2Mask = 0xE0u
};
ASMJIT_DEFINE_ENUM_FLAGS(VirtRegFlags)
class VirtReg {
public:
  ASMJIT_NONCOPYABLE(VirtReg)
  static constexpr inline uint32_t kAlignmentLog2Mask  = uint32_t(VirtRegFlags::kAlignmentLog2Mask);
  static constexpr inline uint32_t kAlignmentLog2Shift = Support::ctz_const<kAlignmentLog2Mask>;
  static ASMJIT_INLINE_CONSTEXPR VirtRegFlags _flags_from_alignment_log2(uint32_t alignment_log2) noexcept {
    return VirtRegFlags(alignment_log2 << kAlignmentLog2Shift);
  }
  static ASMJIT_INLINE_CONSTEXPR uint32_t _alignment_log2_from_flags(VirtRegFlags flags) noexcept {
    return uint32_t(flags) >> kAlignmentLog2Shift;
  }
  uint32_t _id = 0;
  uint32_t _virt_size = 0;
  RegType _reg_type = RegType::kNone;
  VirtRegFlags _reg_flags = VirtRegFlags::kNone;
  uint8_t _weight = 0;
  TypeId _type_id = TypeId::kVoid;
  uint8_t _home_id_hint = Reg::kIdBad;
  int32_t _stack_offset = 0;
  ArenaString<16> _name {};
  RAWorkReg* _work_reg = nullptr;
  ASMJIT_INLINE_NODEBUG VirtReg(RegType reg_type, VirtRegFlags reg_flags, uint32_t id, uint32_t virt_size, TypeId type_id) noexcept
    : _id(id),
      _virt_size(virt_size),
      _reg_type(reg_type),
      _reg_flags(reg_flags),
      _type_id(type_id) {}
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t id() const noexcept { return _id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegType reg_type() const noexcept { return _reg_type; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG VirtRegFlags reg_flags() const noexcept { return _reg_flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t virt_size() const noexcept { return _virt_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t alignment() const noexcept { return 1u << _alignment_log2_from_flags(_reg_flags); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG TypeId type_id() const noexcept { return _type_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t weight() const noexcept { return _weight; }
  ASMJIT_INLINE_NODEBUG void set_weight(uint32_t weight) noexcept { _weight = uint8_t(weight); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_fixed() const noexcept { return Support::test(_reg_flags, VirtRegFlags::kIsFixed); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_stack_area() const noexcept { return Support::test(_reg_flags, VirtRegFlags::kIsStackArea); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_stack_slot() const noexcept { return Support::test(_reg_flags, VirtRegFlags::kHasStackSlot); }
  ASMJIT_INLINE_NODEBUG void assign_stack_slot(int32_t stack_offset) noexcept {
    _reg_flags |= VirtRegFlags::kHasStackSlot;
    _stack_offset = stack_offset;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_home_id_hint() const noexcept { return _home_id_hint != Reg::kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t home_id_hint() const noexcept { return _home_id_hint; }
  ASMJIT_INLINE_NODEBUG void set_home_id_hint(uint32_t home_id) noexcept { _home_id_hint = uint8_t(home_id); }
  ASMJIT_INLINE_NODEBUG void reset_home_id_hint() noexcept { _home_id_hint = Reg::kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG int32_t stack_offset() const noexcept { return _stack_offset; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return _name.data(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t name_size() const noexcept { return _name.size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_work_reg() const noexcept { return _work_reg != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* work_reg() const noexcept { return _work_reg; }
  ASMJIT_INLINE_NODEBUG void set_work_reg(RAWorkReg* work_reg) noexcept { _work_reg = work_reg; }
  ASMJIT_INLINE_NODEBUG void reset_work_reg() noexcept { _work_reg = nullptr; }
};
ASMJIT_END_NAMESPACE
#endif
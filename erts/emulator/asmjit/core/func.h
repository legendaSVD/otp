#ifndef ASMJIT_CORE_FUNC_H_INCLUDED
#define ASMJIT_CORE_FUNC_H_INCLUDED
#include <asmjit/core/archtraits.h>
#include <asmjit/core/environment.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
enum class CallConvId : uint8_t {
  kCDecl = 0,
  kStdCall = 1,
  kFastCall = 2,
  kVectorCall = 3,
  kThisCall = 4,
  kRegParm1 = 5,
  kRegParm2 = 6,
  kRegParm3 = 7,
  kLightCall2 = 16,
  kLightCall3 = 17,
  kLightCall4 = 18,
  kSoftFloat = 30,
  kHardFloat = 31,
  kX64SystemV = 32,
  kX64Windows = 33,
  kMaxValue = kX64Windows
};
enum class CallConvStrategy : uint8_t {
  kDefault = 0,
  kX64Windows = 1,
  kX64VectorCall = 2,
  kAArch64Apple = 3,
  kMaxValue = kX64VectorCall
};
enum class CallConvFlags : uint32_t {
  kNone = 0,
  kCalleePopsStack = 0x0001u,
  kIndirectVecArgs = 0x0002u,
  kPassFloatsByVec = 0x0004u,
  kPassVecByStackIfVA = 0x0008u,
  kPassMmxByGp = 0x0010u,
  kPassMmxByXmm = 0x0020u,
  kVarArgCompatible = 0x0080u
};
ASMJIT_DEFINE_ENUM_FLAGS(CallConvFlags)
struct CallConv {
  static inline constexpr uint32_t kMaxRegArgsPerGroup = 16;
  Arch _arch;
  CallConvId _id;
  CallConvStrategy _strategy;
  uint8_t _red_zone_size;
  uint8_t _spill_zone_size;
  uint8_t _natural_stack_alignment;
  uint8_t _reserved[2];
  CallConvFlags _flags;
  Support::Array<uint8_t, Globals::kNumVirtGroups> _save_restore_reg_size;
  Support::Array<uint8_t, Globals::kNumVirtGroups> _save_restore_alignment;
  Support::Array<RegMask, Globals::kNumVirtGroups> _passed_regs;
  Support::Array<RegMask, Globals::kNumVirtGroups> _preserved_regs;
  union RegOrder {
    uint8_t id[kMaxRegArgsPerGroup];
    uint32_t packed[(kMaxRegArgsPerGroup + 3) / 4];
  };
  Support::Array<RegOrder, Globals::kNumVirtGroups> _passed_order;
  ASMJIT_API Error init(CallConvId call_conv_id, const Environment& environment) noexcept;
  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    *this = CallConv{};
    memset(_passed_order.data(), 0xFF, sizeof(_passed_order));
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _arch; }
  ASMJIT_INLINE_NODEBUG void set_arch(Arch arch) noexcept { _arch = arch; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CallConvId id() const noexcept { return _id; }
  ASMJIT_INLINE_NODEBUG void set_id(CallConvId call_conv_id) noexcept { _id = call_conv_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CallConvStrategy strategy() const noexcept { return _strategy; }
  ASMJIT_INLINE_NODEBUG void set_strategy(CallConvStrategy strategy) noexcept { _strategy = strategy; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(CallConvFlags flag) const noexcept { return Support::test(_flags, flag); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CallConvFlags flags() const noexcept { return _flags; }
  ASMJIT_INLINE_NODEBUG void set_flags(CallConvFlags flag) noexcept { _flags = flag; };
  ASMJIT_INLINE_NODEBUG void add_flags(CallConvFlags flags) noexcept { _flags |= flags; };
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_red_zone() const noexcept { return _red_zone_size != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_spill_zone() const noexcept { return _spill_zone_size != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t red_zone_size() const noexcept { return _red_zone_size; }
  ASMJIT_INLINE_NODEBUG void set_red_zone_size(uint32_t size) noexcept { _red_zone_size = uint8_t(size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t spill_zone_size() const noexcept { return _spill_zone_size; }
  ASMJIT_INLINE_NODEBUG void set_spill_zone_size(uint32_t size) noexcept { _spill_zone_size = uint8_t(size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t natural_stack_alignment() const noexcept { return _natural_stack_alignment; }
  ASMJIT_INLINE_NODEBUG void set_natural_stack_alignment(uint32_t value) noexcept { _natural_stack_alignment = uint8_t(value); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t save_restore_reg_size(RegGroup group) const noexcept { return _save_restore_reg_size[group]; }
  ASMJIT_INLINE_NODEBUG void set_save_restore_reg_size(RegGroup group, uint32_t size) noexcept { _save_restore_reg_size[group] = uint8_t(size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t save_restore_alignment(RegGroup group) const noexcept { return _save_restore_alignment[group]; }
  ASMJIT_INLINE_NODEBUG void set_save_restore_alignment(RegGroup group, uint32_t alignment) noexcept { _save_restore_alignment[group] = uint8_t(alignment); }
  [[nodiscard]]
  ASMJIT_INLINE const uint8_t* passed_order(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _passed_order[size_t(group)].id;
  }
  [[nodiscard]]
  ASMJIT_INLINE RegMask passed_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _passed_regs[size_t(group)];
  }
  ASMJIT_INLINE void _set_passed_as_packed(RegGroup group, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _passed_order[group].packed[0] = p0;
    _passed_order[group].packed[1] = p1;
    _passed_order[group].packed[2] = p2;
    _passed_order[group].packed[3] = p3;
  }
  ASMJIT_INLINE void set_passed_to_none(RegGroup group) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _set_passed_as_packed(group, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu);
    _passed_regs[size_t(group)] = 0u;
  }
  ASMJIT_INLINE void set_passed_order(RegGroup group, uint32_t a0, uint32_t a1 = 0xFF, uint32_t a2 = 0xFF, uint32_t a3 = 0xFF, uint32_t a4 = 0xFF, uint32_t a5 = 0xFF, uint32_t a6 = 0xFF, uint32_t a7 = 0xFF) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _set_passed_as_packed(group, Support::bytepack32_4x8(a0, a1, a2, a3),
                                 Support::bytepack32_4x8(a4, a5, a6, a7),
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu);
    _passed_regs[group] = (a0 != 0xFF ? 1u << a0 : 0u) |
                         (a1 != 0xFF ? 1u << a1 : 0u) |
                         (a2 != 0xFF ? 1u << a2 : 0u) |
                         (a3 != 0xFF ? 1u << a3 : 0u) |
                         (a4 != 0xFF ? 1u << a4 : 0u) |
                         (a5 != 0xFF ? 1u << a5 : 0u) |
                         (a6 != 0xFF ? 1u << a6 : 0u) |
                         (a7 != 0xFF ? 1u << a7 : 0u) ;
  }
  [[nodiscard]]
  ASMJIT_INLINE RegMask preserved_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _preserved_regs[group];
  }
  ASMJIT_INLINE void set_preserved_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _preserved_regs[group] = regs;
  }
};
struct FuncSignature {
  static inline constexpr uint8_t kNoVarArgs = 0xFFu;
  CallConvId _call_conv_id = CallConvId::kCDecl;
  uint8_t _arg_count = 0;
  uint8_t _va_index = kNoVarArgs;
  TypeId _ret = TypeId::kVoid;
  uint8_t _reserved[4] {};
  TypeId _args[Globals::kMaxFuncArgs] {};
  ASMJIT_INLINE_CONSTEXPR FuncSignature() = default;
  ASMJIT_INLINE_CONSTEXPR FuncSignature(const FuncSignature& other) = default;
  ASMJIT_INLINE_CONSTEXPR FuncSignature(CallConvId call_conv_id, uint32_t va_index = kNoVarArgs) noexcept
    : _call_conv_id(call_conv_id),
      _va_index(uint8_t(va_index)) {}
  template<typename... Args>
  ASMJIT_INLINE_CONSTEXPR FuncSignature(CallConvId call_conv_id, uint32_t va_index, TypeId ret, Args&&...args) noexcept
    : _call_conv_id(call_conv_id),
      _arg_count(uint8_t(sizeof...(args))),
      _va_index(uint8_t(va_index)),
      _ret(ret),
      _args{std::forward<Args>(args)...} {}
  template<typename... RetValueAndArgs>
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR FuncSignature build(CallConvId call_conv_id = CallConvId::kCDecl, uint32_t va_index = kNoVarArgs) noexcept {
    return FuncSignature(call_conv_id, va_index, (TypeId(TypeUtils::TypeIdOfT<RetValueAndArgs>::kTypeId))... );
  }
  ASMJIT_INLINE FuncSignature& operator=(const FuncSignature& other) noexcept = default;
  [[nodiscard]]
  ASMJIT_INLINE bool operator==(const FuncSignature& other) const noexcept { return equals(other); }
  [[nodiscard]]
  ASMJIT_INLINE bool operator!=(const FuncSignature& other) const noexcept { return !equals(other); }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = FuncSignature{}; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool equals(const FuncSignature& other) const noexcept {
    return _call_conv_id == other._call_conv_id &&
           _arg_count == other._arg_count &&
           _va_index == other._va_index &&
           _ret == other._ret &&
           memcmp(_args, other._args, sizeof(_args)) == 0;
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR CallConvId call_conv_id() const noexcept { return _call_conv_id; }
  ASMJIT_INLINE_CONSTEXPR void set_call_conv_id(CallConvId call_conv_id) noexcept { _call_conv_id = call_conv_id; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_ret() const noexcept { return _ret != TypeId::kVoid; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR TypeId ret() const noexcept { return _ret; }
  ASMJIT_INLINE_CONSTEXPR void set_ret(TypeId ret_type) noexcept { _ret = ret_type; }
  template<typename T>
  ASMJIT_INLINE_CONSTEXPR void set_ret_t() noexcept { set_ret(TypeId(TypeUtils::TypeIdOfT<T>::kTypeId)); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const TypeId* args() const noexcept { return _args; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t arg_count() const noexcept { return _arg_count; }
  [[nodiscard]]
  ASMJIT_INLINE TypeId arg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < _arg_count);
    return _args[i];
  }
  ASMJIT_INLINE void set_arg(uint32_t index, TypeId arg_type) noexcept {
    ASMJIT_ASSERT(index < _arg_count);
    _args[index] = arg_type;
  }
  template<typename T>
  ASMJIT_INLINE void set_arg_t(uint32_t index) noexcept { set_arg(index, TypeId(TypeUtils::TypeIdOfT<T>::kTypeId)); }
  [[nodiscard]]
  ASMJIT_INLINE bool can_add_arg() const noexcept { return _arg_count < Globals::kMaxFuncArgs; }
  ASMJIT_INLINE void add_arg(TypeId type) noexcept {
    ASMJIT_ASSERT(_arg_count < Globals::kMaxFuncArgs);
    _args[_arg_count++] = type;
  }
  template<typename T>
  ASMJIT_INLINE void add_arg_t() noexcept { add_arg(TypeId(TypeUtils::TypeIdOfT<T>::kTypeId)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_var_args() const noexcept { return _va_index != kNoVarArgs; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t va_index() const noexcept { return _va_index; }
  ASMJIT_INLINE_NODEBUG void set_va_index(uint32_t index) noexcept { _va_index = uint8_t(index); }
  ASMJIT_INLINE_NODEBUG void reset_va_index() noexcept { _va_index = kNoVarArgs; }
};
struct FuncValue {
  enum Bits : uint32_t {
    kTypeIdShift      = 0,
    kTypeIdMask       = 0x000000FFu,
    kFlagIsReg        = 0x00000100u,
    kFlagIsStack      = 0x00000200u,
    kFlagIsIndirect   = 0x00000400u,
    kFlagIsDone       = 0x00000800u,
    kStackOffsetShift = 12,
    kStackOffsetMask  = 0xFFFFF000u,
    kRegIdShift       = 16,
    kRegIdMask        = 0x00FF0000u,
    kRegTypeShift     = 24,
    kRegTypeMask      = 0xFF000000u
  };
  uint32_t _data;
  ASMJIT_INLINE_NODEBUG void init_type_id(TypeId type_id) noexcept {
    _data = uint32_t(type_id) << kTypeIdShift;
  }
  ASMJIT_INLINE_NODEBUG void init_reg(RegType reg_type, uint32_t reg_id, TypeId type_id, uint32_t flags = 0) noexcept {
    _data = (uint32_t(reg_type) << kRegTypeShift) | (reg_id << kRegIdShift) | (uint32_t(type_id) << kTypeIdShift) | kFlagIsReg | flags;
  }
  ASMJIT_INLINE_NODEBUG void init_stack(int32_t offset, TypeId type_id) noexcept {
    _data = (uint32_t(offset) << kStackOffsetShift) | (uint32_t(type_id) << kTypeIdShift) | kFlagIsStack;
  }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _data = 0; }
  ASMJIT_INLINE void assign_reg_data(RegType reg_type, uint32_t reg_id) noexcept {
    ASMJIT_ASSERT((_data & (kRegTypeMask | kRegIdMask)) == 0);
    _data |= (uint32_t(reg_type) << kRegTypeShift) | (reg_id << kRegIdShift) | kFlagIsReg;
  }
  ASMJIT_INLINE void assign_stack_offset(int32_t offset) noexcept {
    ASMJIT_ASSERT((_data & kStackOffsetMask) == 0);
    _data |= (uint32_t(offset) << kStackOffsetShift) | kFlagIsStack;
  }
  ASMJIT_INLINE_NODEBUG explicit operator bool() const noexcept { return _data != 0; }
  ASMJIT_INLINE_NODEBUG void _replace_value(uint32_t mask, uint32_t value) noexcept { _data = (_data & ~mask) | value; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(uint32_t flag) const noexcept { return Support::test(_data, flag); }
  ASMJIT_INLINE_NODEBUG void add_flags(uint32_t flags) noexcept { _data |= flags; }
  ASMJIT_INLINE_NODEBUG void clear_flags(uint32_t flags) noexcept { _data &= ~flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_initialized() const noexcept { return _data != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_reg() const noexcept { return has_flag(kFlagIsReg); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_stack() const noexcept { return has_flag(kFlagIsStack); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_assigned() const noexcept { return has_flag(kFlagIsReg | kFlagIsStack); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_indirect() const noexcept { return has_flag(kFlagIsIndirect); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_done() const noexcept { return has_flag(kFlagIsDone); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegType reg_type() const noexcept { return RegType((_data & kRegTypeMask) >> kRegTypeShift); }
  ASMJIT_INLINE_NODEBUG void set_reg_type(RegType reg_type) noexcept { _replace_value(kRegTypeMask, uint32_t(reg_type) << kRegTypeShift); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t reg_id() const noexcept { return (_data & kRegIdMask) >> kRegIdShift; }
  ASMJIT_INLINE_NODEBUG void set_reg_id(uint32_t reg_id) noexcept { _replace_value(kRegIdMask, reg_id << kRegIdShift); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG int32_t stack_offset() const noexcept { return int32_t(_data & kStackOffsetMask) >> kStackOffsetShift; }
  ASMJIT_INLINE_NODEBUG void set_stack_offset(int32_t offset) noexcept { _replace_value(kStackOffsetMask, uint32_t(offset) << kStackOffsetShift); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_type_id() const noexcept { return Support::test(_data, kTypeIdMask); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG TypeId type_id() const noexcept { return TypeId((_data & kTypeIdMask) >> kTypeIdShift); }
  ASMJIT_INLINE_NODEBUG void set_type_id(TypeId type_id) noexcept { _replace_value(kTypeIdMask, uint32_t(type_id) << kTypeIdShift); }
};
struct FuncValuePack {
public:
  FuncValue _values[Globals::kMaxValuePack];
  ASMJIT_INLINE void reset() noexcept {
    for (FuncValue& value : _values) {
      value.reset();
    }
  }
  [[nodiscard]]
  ASMJIT_INLINE uint32_t count() const noexcept {
    uint32_t n = Globals::kMaxValuePack;
    while (n && !_values[n - 1])
      n--;
    return n;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncValue* values() noexcept { return _values; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncValue* values() const noexcept { return _values; }
  ASMJIT_INLINE void reset_value(size_t index) noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    _values[index].reset();
  }
  ASMJIT_INLINE bool has_value(size_t index) noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    return _values[index].is_initialized();
  }
  ASMJIT_INLINE void assign_reg(size_t index, const Reg& reg, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    ASMJIT_ASSERT(reg.is_phys_reg());
    _values[index].init_reg(reg.reg_type(), reg.id(), type_id);
  }
  ASMJIT_INLINE void assign_reg(size_t index, RegType reg_type, uint32_t reg_id, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    _values[index].init_reg(reg_type, reg_id, type_id);
  }
  ASMJIT_INLINE void assign_stack(size_t index, int32_t offset, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    _values[index].init_stack(offset, type_id);
  }
  [[nodiscard]]
  ASMJIT_INLINE FuncValue& operator[](size_t index) {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    return _values[index];
  }
  [[nodiscard]]
  ASMJIT_INLINE const FuncValue& operator[](size_t index) const {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    return _values[index];
  }
};
enum class FuncAttributes : uint32_t {
  kNoAttributes = 0,
  kHasVarArgs = 0x00000001u,
  kHasPreservedFP = 0x00000010u,
  kHasFuncCalls = 0x00000020u,
  kAlignedVecSR = 0x00000040u,
  kIndirectBranchProtection = 0x00000080u,
  kIsFinalized = 0x00000800u,
  kX86_AVXEnabled = 0x00010000u,
  kX86_AVX512Enabled = 0x00020000u,
  kX86_MMXCleanup = 0x00040000u,
  kX86_AVXCleanup = 0x00080000u,
  kX86_AVXAutoCleanup = 0x00100000u
};
ASMJIT_DEFINE_ENUM_FLAGS(FuncAttributes)
class FuncDetail {
public:
  static inline constexpr uint8_t kNoVarArgs = 0xFFu;
  CallConv _call_conv {};
  uint8_t _arg_count = 0;
  uint8_t _va_index = 0;
  uint16_t _reserved = 0;
  Support::Array<RegMask, Globals::kNumVirtGroups> _used_regs {};
  uint32_t _arg_stack_size = 0;
  FuncValuePack _rets {};
  FuncValuePack _args[Globals::kMaxFuncArgs] {};
  ASMJIT_INLINE_NODEBUG FuncDetail() noexcept {}
  ASMJIT_INLINE_NODEBUG FuncDetail(const FuncDetail& other) noexcept = default;
  ASMJIT_API Error init(const FuncSignature& signature, const Environment& environment) noexcept;
  ASMJIT_INLINE_NODEBUG FuncDetail& operator=(const FuncDetail& other) noexcept = default;
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = FuncDetail{}; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CallConv& call_conv() const noexcept { return _call_conv; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CallConvFlags flags() const noexcept { return _call_conv.flags(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(CallConvFlags flag) const noexcept { return _call_conv.has_flag(flag); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_ret() const noexcept { return bool(_rets[0]); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t arg_count() const noexcept { return _arg_count; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncValuePack& ret_pack() noexcept { return _rets; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncValuePack& ret_pack() const noexcept { return _rets; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncValue& ret(size_t value_index = 0) noexcept { return _rets[value_index]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncValue& ret(size_t value_index = 0) const noexcept { return _rets[value_index]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncValuePack* arg_packs() noexcept { return _args; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncValuePack* arg_packs() const noexcept { return _args; }
  [[nodiscard]]
  ASMJIT_INLINE FuncValuePack& arg_pack(size_t arg_index) noexcept {
    ASMJIT_ASSERT(arg_index < Globals::kMaxFuncArgs);
    return _args[arg_index];
  }
  [[nodiscard]]
  ASMJIT_INLINE const FuncValuePack& arg_pack(size_t arg_index) const noexcept {
    ASMJIT_ASSERT(arg_index < Globals::kMaxFuncArgs);
    return _args[arg_index];
  }
  [[nodiscard]]
  ASMJIT_INLINE FuncValue& arg(size_t arg_index, size_t value_index = 0) noexcept {
    ASMJIT_ASSERT(arg_index < Globals::kMaxFuncArgs);
    return _args[arg_index][value_index];
  }
  [[nodiscard]]
  ASMJIT_INLINE const FuncValue& arg(size_t arg_index, size_t value_index = 0) const noexcept {
    ASMJIT_ASSERT(arg_index < Globals::kMaxFuncArgs);
    return _args[arg_index][value_index];
  }
  ASMJIT_INLINE void reset_arg(size_t arg_index) noexcept {
    ASMJIT_ASSERT(arg_index < Globals::kMaxFuncArgs);
    _args[arg_index].reset();
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_var_args() const noexcept { return _va_index != kNoVarArgs; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t va_index() const noexcept { return _va_index; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_stack_args() const noexcept { return _arg_stack_size != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t arg_stack_size() const noexcept { return _arg_stack_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t red_zone_size() const noexcept { return _call_conv.red_zone_size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t spill_zone_size() const noexcept { return _call_conv.spill_zone_size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t natural_stack_alignment() const noexcept { return _call_conv.natural_stack_alignment(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask passed_regs(RegGroup group) const noexcept { return _call_conv.passed_regs(group); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask preserved_regs(RegGroup group) const noexcept { return _call_conv.preserved_regs(group); }
  [[nodiscard]]
  ASMJIT_INLINE RegMask used_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _used_regs[size_t(group)];
  }
  ASMJIT_INLINE void add_used_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _used_regs[size_t(group)] |= regs;
  }
};
class FuncFrame {
public:
  static inline constexpr uint32_t kTagInvalidOffset = 0xFFFFFFFFu;
  using RegMasks = Support::Array<RegMask, Globals::kNumVirtGroups>;
  FuncAttributes _attributes {};
  Arch _arch {};
  uint8_t _sp_reg_id = uint8_t(Reg::kIdBad);
  uint8_t _sa_reg_id = uint8_t(Reg::kIdBad);
  uint8_t _red_zone_size = 0;
  uint8_t _spill_zone_size = 0;
  uint8_t _natural_stack_alignment = 0;
  uint8_t _min_dynamic_alignment = 0;
  uint8_t _call_stack_alignment = 0;
  uint8_t _local_stack_alignment = 0;
  uint8_t _final_stack_alignment = 0;
  uint16_t _callee_stack_cleanup = 0;
  uint32_t _call_stack_size = 0;
  uint32_t _local_stack_size = 0;
  uint32_t _final_stack_size = 0;
  uint32_t _local_stack_offset = 0;
  uint32_t _da_offset = 0;
  uint32_t _sa_offset_from_sp = 0;
  uint32_t _sa_offset_from_sa = 0;
  uint32_t _stack_adjustment = 0;
  RegMasks _dirty_regs {};
  RegMasks _preserved_regs {};
  RegMasks _unavailable_regs {};
  Support::Array<uint8_t, Globals::kNumVirtGroups> _save_restore_reg_size {};
  Support::Array<uint8_t, Globals::kNumVirtGroups> _save_restore_alignment {};
  uint16_t _push_pop_save_size = 0;
  uint16_t _extra_reg_save_size = 0;
  uint32_t _push_pop_save_offset = 0;
  uint32_t _extra_reg_save_offset = 0;
  ASMJIT_INLINE_NODEBUG FuncFrame() noexcept = default;
  ASMJIT_INLINE_NODEBUG FuncFrame(const FuncFrame& other) noexcept = default;
  ASMJIT_API Error init(const FuncDetail& func) noexcept;
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = FuncFrame{}; }
  ASMJIT_INLINE_NODEBUG FuncFrame& operator=(const FuncFrame& other) noexcept = default;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _arch; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncAttributes attributes() const noexcept { return _attributes; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_attribute(FuncAttributes attr) const noexcept { return Support::test(_attributes, attr); }
  ASMJIT_INLINE_NODEBUG void add_attributes(FuncAttributes attrs) noexcept { _attributes |= attrs; }
  ASMJIT_INLINE_NODEBUG void clear_attributes(FuncAttributes attrs) noexcept { _attributes &= ~attrs; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_var_args() const noexcept { return has_attribute(FuncAttributes::kHasVarArgs); }
  ASMJIT_INLINE_NODEBUG void set_var_args() noexcept { add_attributes(FuncAttributes::kHasVarArgs); }
  ASMJIT_INLINE_NODEBUG void reset_var_args() noexcept { clear_attributes(FuncAttributes::kHasVarArgs); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_preserved_fp() const noexcept { return has_attribute(FuncAttributes::kHasPreservedFP); }
  ASMJIT_INLINE_NODEBUG void set_preserved_fp() noexcept { add_attributes(FuncAttributes::kHasPreservedFP); }
  ASMJIT_INLINE_NODEBUG void reset_preserved_fp() noexcept { clear_attributes(FuncAttributes::kHasPreservedFP); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_func_calls() const noexcept { return has_attribute(FuncAttributes::kHasFuncCalls); }
  ASMJIT_INLINE_NODEBUG void set_func_calls() noexcept { add_attributes(FuncAttributes::kHasFuncCalls); }
  ASMJIT_INLINE_NODEBUG void reset_func_calls() noexcept { clear_attributes(FuncAttributes::kHasFuncCalls); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_indirect_branch_protection() const noexcept { return has_attribute(FuncAttributes::kIndirectBranchProtection); }
  ASMJIT_INLINE_NODEBUG void set_indirect_branch_protection() noexcept { add_attributes(FuncAttributes::kIndirectBranchProtection); }
  ASMJIT_INLINE_NODEBUG void reset_indirect_branch_protection() noexcept { clear_attributes(FuncAttributes::kIndirectBranchProtection); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_avx_enabled() const noexcept { return has_attribute(FuncAttributes::kX86_AVXEnabled); }
  ASMJIT_INLINE_NODEBUG void set_avx_enabled() noexcept { add_attributes(FuncAttributes::kX86_AVXEnabled); }
  ASMJIT_INLINE_NODEBUG void reset_avx_enabled() noexcept { clear_attributes(FuncAttributes::kX86_AVXEnabled); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_avx512_enabled() const noexcept { return has_attribute(FuncAttributes::kX86_AVX512Enabled); }
  ASMJIT_INLINE_NODEBUG void set_avx512_enabled() noexcept { add_attributes(FuncAttributes::kX86_AVX512Enabled); }
  ASMJIT_INLINE_NODEBUG void reset_avx512_enabled() noexcept { clear_attributes(FuncAttributes::kX86_AVX512Enabled); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_mmx_cleanup() const noexcept { return has_attribute(FuncAttributes::kX86_MMXCleanup); }
  ASMJIT_INLINE_NODEBUG void set_mmx_cleanup() noexcept { add_attributes(FuncAttributes::kX86_MMXCleanup); }
  ASMJIT_INLINE_NODEBUG void reset_mmx_cleanup() noexcept { clear_attributes(FuncAttributes::kX86_MMXCleanup); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx_cleanup() const noexcept { return has_attribute(FuncAttributes::kX86_AVXCleanup); }
  ASMJIT_INLINE_NODEBUG void set_avx_cleanup() noexcept { add_attributes(FuncAttributes::kX86_AVXCleanup); }
  ASMJIT_INLINE_NODEBUG void reset_avx_cleanup() noexcept { clear_attributes(FuncAttributes::kX86_AVXCleanup); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx_auto_cleanup() const noexcept { return has_attribute(FuncAttributes::kX86_AVXAutoCleanup); }
  ASMJIT_INLINE_NODEBUG void set_avx_auto_cleanup() noexcept { add_attributes(FuncAttributes::kX86_AVXAutoCleanup); }
  ASMJIT_INLINE_NODEBUG void reset_avx_auto_cleanup() noexcept { clear_attributes(FuncAttributes::kX86_AVXAutoCleanup); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_call_stack() const noexcept { return _call_stack_size != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_local_stack() const noexcept { return _local_stack_size != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_aligned_vec_save_restore() const noexcept { return has_attribute(FuncAttributes::kAlignedVecSR); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_dynamic_alignment() const noexcept { return _final_stack_alignment >= _min_dynamic_alignment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_red_zone() const noexcept { return _red_zone_size != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t red_zone_size() const noexcept { return _red_zone_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_spill_zone() const noexcept { return _spill_zone_size != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t spill_zone_size() const noexcept { return _spill_zone_size; }
  ASMJIT_INLINE_NODEBUG void reset_red_zone() noexcept { _red_zone_size = 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t natural_stack_alignment() const noexcept { return _natural_stack_alignment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t min_dynamic_alignment() const noexcept { return _min_dynamic_alignment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_callee_stack_cleanup() const noexcept { return _callee_stack_cleanup != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t callee_stack_cleanup() const noexcept { return _callee_stack_cleanup; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t call_stack_alignment() const noexcept { return _call_stack_alignment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t local_stack_alignment() const noexcept { return _local_stack_alignment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t final_stack_alignment() const noexcept { return _final_stack_alignment; }
  ASMJIT_INLINE void set_call_stack_alignment(uint32_t alignment) noexcept {
    _call_stack_alignment = uint8_t(alignment);
    _final_stack_alignment = Support::max(_natural_stack_alignment, _call_stack_alignment, _local_stack_alignment);
  }
  ASMJIT_INLINE void set_local_stack_alignment(uint32_t value) noexcept {
    _local_stack_alignment = uint8_t(value);
    _final_stack_alignment = Support::max(_natural_stack_alignment, _call_stack_alignment, _local_stack_alignment);
  }
  ASMJIT_INLINE void update_call_stack_alignment(uint32_t alignment) noexcept {
    _call_stack_alignment = uint8_t(Support::max<uint32_t>(_call_stack_alignment, alignment));
    _final_stack_alignment = Support::max(_final_stack_alignment, _call_stack_alignment);
  }
  ASMJIT_INLINE void update_local_stack_alignment(uint32_t alignment) noexcept {
    _local_stack_alignment = uint8_t(Support::max<uint32_t>(_local_stack_alignment, alignment));
    _final_stack_alignment = Support::max(_final_stack_alignment, _local_stack_alignment);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t call_stack_size() const noexcept { return _call_stack_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t local_stack_size() const noexcept { return _local_stack_size; }
  ASMJIT_INLINE_NODEBUG void set_call_stack_size(uint32_t size) noexcept { _call_stack_size = size; }
  ASMJIT_INLINE_NODEBUG void set_local_stack_size(uint32_t size) noexcept { _local_stack_size = size; }
  ASMJIT_INLINE_NODEBUG void update_call_stack_size(uint32_t size) noexcept { _call_stack_size = Support::max(_call_stack_size, size); }
  ASMJIT_INLINE_NODEBUG void update_local_stack_size(uint32_t size) noexcept { _local_stack_size = Support::max(_local_stack_size, size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t final_stack_size() const noexcept { return _final_stack_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t local_stack_offset() const noexcept { return _local_stack_offset; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_da_offset() const noexcept { return _da_offset != kTagInvalidOffset; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t da_offset() const noexcept { return _da_offset; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sa_offset(uint32_t reg_id) const noexcept {
    return reg_id == _sp_reg_id ? sa_offset_from_sp() : sa_offset_from_sa();
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sa_offset_from_sp() const noexcept { return _sa_offset_from_sp; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sa_offset_from_sa() const noexcept { return _sa_offset_from_sa; }
  [[nodiscard]]
  inline RegMask dirty_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _dirty_regs[group];
  }
  inline void set_dirty_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _dirty_regs[group] = regs;
  }
  inline void add_dirty_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _dirty_regs[group] |= regs;
  }
  inline void add_dirty_regs(const Reg& reg) noexcept {
    ASMJIT_ASSERT(reg.id() < Globals::kMaxPhysRegs);
    add_dirty_regs(reg.reg_group(), Support::bit_mask<RegMask>(reg.id()));
  }
  template<typename... Args>
  inline void add_dirty_regs(const Reg& reg, Args&&... args) noexcept {
    add_dirty_regs(reg);
    add_dirty_regs(std::forward<Args>(args)...);
  }
  ASMJIT_INLINE_NODEBUG void set_all_dirty() noexcept {
    for (size_t i = 0; i < ASMJIT_ARRAY_SIZE(_dirty_regs); i++) {
      _dirty_regs[i] = 0xFFFFFFFFu;
    }
  }
  ASMJIT_INLINE void set_all_dirty(RegGroup group) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _dirty_regs[group] = 0xFFFFFFFFu;
  }
  [[nodiscard]]
  ASMJIT_INLINE RegMask saved_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _dirty_regs[group] & _preserved_regs[group];
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegMasks& dirty_regs() const noexcept { return _dirty_regs; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegMasks& preserved_regs() const noexcept { return _preserved_regs; }
  [[nodiscard]]
  ASMJIT_INLINE RegMask preserved_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _preserved_regs[group];
  }
  ASMJIT_INLINE void set_unavailable_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _unavailable_regs[group] = regs;
  }
  ASMJIT_INLINE void add_unavailable_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _unavailable_regs[group] |= regs;
  }
  ASMJIT_INLINE void add_unavailable_regs(const Reg& reg) noexcept {
    ASMJIT_ASSERT(reg.id() < Globals::kMaxPhysRegs);
    add_unavailable_regs(reg.reg_group(), Support::bit_mask<RegMask>(reg.id()));
  }
  template<typename... Args>
  ASMJIT_INLINE void add_unavailable_regs(const Reg& reg, Args&&... args) noexcept {
    add_unavailable_regs(reg);
    add_unavailable_regs(std::forward<Args>(args)...);
  }
  ASMJIT_INLINE_NODEBUG void clear_unavailable_regs() noexcept {
    for (size_t i = 0; i < ASMJIT_ARRAY_SIZE(_unavailable_regs); i++)
      _unavailable_regs[i] = 0;
  }
  ASMJIT_INLINE void clear_unavailable_regs(RegGroup group) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _unavailable_regs[group] = 0;
  }
  [[nodiscard]]
  ASMJIT_INLINE RegMask unavailable_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _unavailable_regs[group];
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegMasks& unavailable_regs() const noexcept {
    return _unavailable_regs;
  }
  [[nodiscard]]
  ASMJIT_INLINE uint32_t save_restore_reg_size(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _save_restore_reg_size[group];
  }
  [[nodiscard]]
  ASMJIT_INLINE uint32_t save_restore_alignment(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _save_restore_alignment[group];
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_sa_reg_id() const noexcept { return _sa_reg_id != Reg::kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sa_reg_id() const noexcept { return _sa_reg_id; }
  ASMJIT_INLINE_NODEBUG void set_sa_reg_id(uint32_t reg_id) { _sa_reg_id = uint8_t(reg_id); }
  ASMJIT_INLINE_NODEBUG void reset_sa_reg_id() { set_sa_reg_id(Reg::kIdBad); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t push_pop_save_size() const noexcept { return _push_pop_save_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t push_pop_save_offset() const noexcept { return _push_pop_save_offset; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t extra_reg_save_size() const noexcept { return _extra_reg_save_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t extra_reg_save_offset() const noexcept { return _extra_reg_save_offset; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_stack_adjustment() const noexcept { return _stack_adjustment != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t stack_adjustment() const noexcept { return _stack_adjustment; }
  ASMJIT_API Error finalize() noexcept;
};
class FuncArgsAssignment {
public:
  const FuncDetail* _func_detail {};
  uint8_t _sa_reg_id = uint8_t(Reg::kIdBad);
  uint8_t _reserved[3] {};
  FuncValuePack _arg_packs[Globals::kMaxFuncArgs] {};
  ASMJIT_INLINE_NODEBUG explicit FuncArgsAssignment(const FuncDetail* fd = nullptr) noexcept
    : _func_detail(fd),
      _sa_reg_id(uint8_t(Reg::kIdBad)) {}
  ASMJIT_INLINE_NODEBUG FuncArgsAssignment(const FuncArgsAssignment& other) noexcept = default;
  ASMJIT_INLINE void reset(const FuncDetail* fd = nullptr) noexcept {
    _func_detail = fd;
    _sa_reg_id = uint8_t(Reg::kIdBad);
    memset(_reserved, 0, sizeof(_reserved));
    memset(_arg_packs, 0, sizeof(_arg_packs));
  }
  ASMJIT_INLINE_NODEBUG FuncArgsAssignment& operator=(const FuncArgsAssignment& other) noexcept = default;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncDetail* func_detail() const noexcept { return _func_detail; }
  ASMJIT_INLINE_NODEBUG void set_func_detail(const FuncDetail* fd) noexcept { _func_detail = fd; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_sa_reg_id() const noexcept { return _sa_reg_id != Reg::kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sa_reg_id() const noexcept { return _sa_reg_id; }
  ASMJIT_INLINE_NODEBUG void set_sa_reg_id(uint32_t reg_id) { _sa_reg_id = uint8_t(reg_id); }
  ASMJIT_INLINE_NODEBUG void reset_sa_reg_id() { _sa_reg_id = uint8_t(Reg::kIdBad); }
  [[nodiscard]]
  ASMJIT_INLINE FuncValue& arg(size_t arg_index, size_t value_index) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    return _arg_packs[arg_index][value_index];
  }
  [[nodiscard]]
  ASMJIT_INLINE const FuncValue& arg(size_t arg_index, size_t value_index) const noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    return _arg_packs[arg_index][value_index];
  }
  [[nodiscard]]
  ASMJIT_INLINE bool is_assigned(size_t arg_index, size_t value_index) const noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    return _arg_packs[arg_index][value_index].is_assigned();
  }
  ASMJIT_INLINE void assign_reg(size_t arg_index, const Reg& reg, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    ASMJIT_ASSERT(reg.is_phys_reg());
    _arg_packs[arg_index][0].init_reg(reg.reg_type(), reg.id(), type_id);
  }
  ASMJIT_INLINE void assign_reg(size_t arg_index, RegType reg_type, uint32_t reg_id, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    _arg_packs[arg_index][0].init_reg(reg_type, reg_id, type_id);
  }
  ASMJIT_INLINE void assign_stack(size_t arg_index, int32_t offset, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    _arg_packs[arg_index][0].init_stack(offset, type_id);
  }
  ASMJIT_INLINE void assign_reg_in_pack(size_t arg_index, size_t value_index, const Reg& reg, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    ASMJIT_ASSERT(reg.is_phys_reg());
    _arg_packs[arg_index][value_index].init_reg(reg.reg_type(), reg.id(), type_id);
  }
  ASMJIT_INLINE void assign_reg_in_pack(size_t arg_index, size_t value_index, RegType reg_type, uint32_t reg_id, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    _arg_packs[arg_index][value_index].init_reg(reg_type, reg_id, type_id);
  }
  ASMJIT_INLINE void assign_stack_in_pack(size_t arg_index, size_t value_index, int32_t offset, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    _arg_packs[arg_index][value_index].init_stack(offset, type_id);
  }
  ASMJIT_INLINE void _assign_all_internal(size_t arg_index, const Reg& reg) noexcept {
    assign_reg(arg_index, reg);
  }
  template<typename... Args>
  ASMJIT_INLINE void _assign_all_internal(size_t arg_index, const Reg& reg, Args&&... args) noexcept {
    assign_reg(arg_index, reg);
    _assign_all_internal(arg_index + 1, std::forward<Args>(args)...);
  }
  template<typename... Args>
  ASMJIT_INLINE void assign_all(Args&&... args) noexcept {
    _assign_all_internal(0, std::forward<Args>(args)...);
  }
  ASMJIT_API Error update_func_frame(FuncFrame& frame) const noexcept;
};
ASMJIT_END_NAMESPACE
#endif
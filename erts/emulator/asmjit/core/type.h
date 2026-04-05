#ifndef ASMJIT_CORE_TYPE_H_INCLUDED
#define ASMJIT_CORE_TYPE_H_INCLUDED
#include <asmjit/core/globals.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
enum class TypeId : uint8_t {
  kVoid = 0,
  _kBaseStart = 32,
  _kBaseEnd = 44,
  _kIntStart = 32,
  _kIntEnd = 41,
  kIntPtr = 32,
  kUIntPtr = 33,
  kInt8 = 34,
  kUInt8 = 35,
  kInt16 = 36,
  kUInt16 = 37,
  kInt32 = 38,
  kUInt32 = 39,
  kInt64 = 40,
  kUInt64 = 41,
  _kFloatStart  = 42,
  _kFloatEnd = 44,
  kFloat32 = 42,
  kFloat64 = 43,
  kFloat80 = 44,
  _kMaskStart = 45,
  _kMaskEnd = 48,
  kMask8 = 45,
  kMask16 = 46,
  kMask32 = 47,
  kMask64 = 48,
  _kMmxStart = 49,
  _kMmxEnd = 50,
  kMmx32 = 49,
  kMmx64 = 50,
  _kVec32Start  = 51,
  _kVec32End = 60,
  kInt8x4 = 51,
  kUInt8x4 = 52,
  kInt16x2 = 53,
  kUInt16x2 = 54,
  kInt32x1 = 55,
  kUInt32x1 = 56,
  kFloat32x1 = 59,
  _kVec64Start  = 61,
  _kVec64End = 70,
  kInt8x8 = 61,
  kUInt8x8 = 62,
  kInt16x4 = 63,
  kUInt16x4 = 64,
  kInt32x2 = 65,
  kUInt32x2 = 66,
  kInt64x1 = 67,
  kUInt64x1 = 68,
  kFloat32x2 = 69,
  kFloat64x1 = 70,
  _kVec128Start = 71,
  _kVec128End = 80,
  kInt8x16 = 71,
  kUInt8x16 = 72,
  kInt16x8 = 73,
  kUInt16x8 = 74,
  kInt32x4 = 75,
  kUInt32x4 = 76,
  kInt64x2 = 77,
  kUInt64x2 = 78,
  kFloat32x4 = 79,
  kFloat64x2 = 80,
  _kVec256Start = 81,
  _kVec256End = 90,
  kInt8x32 = 81,
  kUInt8x32 = 82,
  kInt16x16 = 83,
  kUInt16x16 = 84,
  kInt32x8 = 85,
  kUInt32x8 = 86,
  kInt64x4 = 87,
  kUInt64x4 = 88,
  kFloat32x8 = 89,
  kFloat64x4 = 90,
  _kVec512Start = 91,
  _kVec512End = 100,
  kInt8x64 = 91,
  kUInt8x64 = 92,
  kInt16x32 = 93,
  kUInt16x32 = 94,
  kInt32x16 = 95,
  kUInt32x16 = 96,
  kInt64x8 = 97,
  kUInt64x8 = 98,
  kFloat32x16 = 99,
  kFloat64x8 = 100,
  kLastAssigned = kFloat64x8,
  kMaxValue = 255
};
ASMJIT_DEFINE_ENUM_COMPARE(TypeId)
namespace TypeUtils {
struct TypeData {
  TypeId scalar_of[uint32_t(TypeId::kMaxValue) + 1];
  uint8_t size_of[uint32_t(TypeId::kMaxValue) + 1];
};
ASMJIT_VARAPI const TypeData _type_data;
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG TypeId scalar_of(TypeId type_id) noexcept { return _type_data.scalar_of[uint32_t(type_id)]; }
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG uint32_t size_of(TypeId type_id) noexcept { return _type_data.size_of[uint32_t(type_id)]; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_between(TypeId type_id, TypeId a, TypeId b) noexcept {
  return Support::is_between(uint32_t(type_id), uint32_t(a), uint32_t(b));
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_void(TypeId type_id) noexcept { return type_id == TypeId::kVoid; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_valid(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kIntStart, TypeId::_kVec512End); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_scalar(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kBaseStart, TypeId::_kBaseEnd); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_abstract(TypeId type_id) noexcept { return is_between(type_id, TypeId::kIntPtr, TypeId::kUIntPtr); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_int(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kIntStart, TypeId::_kIntEnd); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_int8(TypeId type_id) noexcept { return type_id == TypeId::kInt8; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_uint8(TypeId type_id) noexcept { return type_id == TypeId::kUInt8; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_int16(TypeId type_id) noexcept { return type_id == TypeId::kInt16; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_uint16(TypeId type_id) noexcept { return type_id == TypeId::kUInt16; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_int32(TypeId type_id) noexcept { return type_id == TypeId::kInt32; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_uint32(TypeId type_id) noexcept { return type_id == TypeId::kUInt32; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_int64(TypeId type_id) noexcept { return type_id == TypeId::kInt64; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_uint64(TypeId type_id) noexcept { return type_id == TypeId::kUInt64; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_gp8(TypeId type_id) noexcept { return is_between(type_id, TypeId::kInt8, TypeId::kUInt8); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_gp16(TypeId type_id) noexcept { return is_between(type_id, TypeId::kInt16, TypeId::kUInt16); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_gp32(TypeId type_id) noexcept { return is_between(type_id, TypeId::kInt32, TypeId::kUInt32); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_gp64(TypeId type_id) noexcept { return is_between(type_id, TypeId::kInt64, TypeId::kUInt64); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_float(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kFloatStart, TypeId::_kFloatEnd); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_float32(TypeId type_id) noexcept { return type_id == TypeId::kFloat32; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_float64(TypeId type_id) noexcept { return type_id == TypeId::kFloat64; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_float80(TypeId type_id) noexcept { return type_id == TypeId::kFloat80; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_mask(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kMaskStart, TypeId::_kMaskEnd); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_mask8(TypeId type_id) noexcept { return type_id == TypeId::kMask8; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_mask16(TypeId type_id) noexcept { return type_id == TypeId::kMask16; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_mask32(TypeId type_id) noexcept { return type_id == TypeId::kMask32; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_mask64(TypeId type_id) noexcept { return type_id == TypeId::kMask64; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_mmx(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kMmxStart, TypeId::_kMmxEnd); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_mmx32(TypeId type_id) noexcept { return type_id == TypeId::kMmx32; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_mmx64(TypeId type_id) noexcept { return type_id == TypeId::kMmx64; }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_vec(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kVec32Start, TypeId::_kVec512End); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_vec32(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kVec32Start, TypeId::_kVec32End); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_vec64(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kVec64Start, TypeId::_kVec64End); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_vec128(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kVec128Start, TypeId::_kVec128End); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_vec256(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kVec256Start, TypeId::_kVec256End); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool is_vec512(TypeId type_id) noexcept { return is_between(type_id, TypeId::_kVec512Start, TypeId::_kVec512End); }
enum TypeCategory : uint32_t {
  kTypeCategoryUnknown = 0,
  kTypeCategoryEnum = 1,
  kTypeCategoryIntegral = 2,
  kTypeCategoryFloatingPoint = 3,
  kTypeCategoryFunction = 4
};
template<typename T, TypeCategory kCategory>
struct TypeIdOfT_ByCategory {};
template<typename T>
struct TypeIdOfT_ByCategory<T, kTypeCategoryIntegral> {
  static inline constexpr uint32_t kTypeId = uint32_t(
    (sizeof(T) == 1 &&  std::is_signed_v<T>) ? TypeId::kInt8 :
    (sizeof(T) == 1 && !std::is_signed_v<T>) ? TypeId::kUInt8 :
    (sizeof(T) == 2 &&  std::is_signed_v<T>) ? TypeId::kInt16 :
    (sizeof(T) == 2 && !std::is_signed_v<T>) ? TypeId::kUInt16 :
    (sizeof(T) == 4 &&  std::is_signed_v<T>) ? TypeId::kInt32 :
    (sizeof(T) == 4 && !std::is_signed_v<T>) ? TypeId::kUInt32 :
    (sizeof(T) == 8 &&  std::is_signed_v<T>) ? TypeId::kInt64 :
    (sizeof(T) == 8 && !std::is_signed_v<T>) ? TypeId::kUInt64 : TypeId::kVoid);
};
template<typename T>
struct TypeIdOfT_ByCategory<T, kTypeCategoryFloatingPoint> {
  static inline constexpr uint32_t kTypeId = uint32_t(
    (sizeof(T) == 4 ) ? TypeId::kFloat32 :
    (sizeof(T) == 8 ) ? TypeId::kFloat64 :
    (sizeof(T) >= 10) ? TypeId::kFloat80 : TypeId::kVoid);
};
template<typename T>
struct TypeIdOfT_ByCategory<T, kTypeCategoryEnum>
  : public TypeIdOfT_ByCategory<std::underlying_type_t<T>, kTypeCategoryIntegral> {};
template<typename T>
struct TypeIdOfT_ByCategory<T, kTypeCategoryFunction> {
  static inline constexpr uint32_t kTypeId = uint32_t(TypeId::kUIntPtr);
};
#ifdef _DOXYGEN
template<typename T>
struct TypeIdOfT {
  static inline constexpr TypeId kTypeId = _TypeIdDeducedAtCompileTime_;
};
#else
template<typename T>
struct TypeIdOfT
  : public TypeIdOfT_ByCategory<T,
    std::is_enum_v<T>           ? kTypeCategoryEnum          :
    std::is_integral_v<T>       ? kTypeCategoryIntegral      :
    std::is_floating_point_v<T> ? kTypeCategoryFloatingPoint :
    std::is_function_v<T>       ? kTypeCategoryFunction      : kTypeCategoryUnknown> {};
#endif
template<typename T>
struct TypeIdOfT<T*> {
  static inline constexpr uint32_t kTypeId = uint32_t(TypeId::kUIntPtr);
};
template<typename T>
struct TypeIdOfT<T&> {
  static inline constexpr uint32_t kTypeId = uint32_t(TypeId::kUIntPtr);
};
template<typename T>
static ASMJIT_INLINE_CONSTEXPR TypeId type_id_of_t() noexcept { return TypeId(TypeIdOfT<T>::kTypeId); }
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR uint32_t deabstract_delta_of_size(uint32_t register_size) noexcept {
  return register_size >= 8 ? uint32_t(TypeId::kInt64) - uint32_t(TypeId::kIntPtr)
                            : uint32_t(TypeId::kInt32) - uint32_t(TypeId::kIntPtr);
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR TypeId deabstract(TypeId type_id, uint32_t deabstract_delta) noexcept {
  return is_abstract(type_id) ? TypeId(uint32_t(type_id) + deabstract_delta) : type_id;
}
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR TypeId scalar_to_vector(TypeId scalar_type_id, TypeId vec_start_id) noexcept {
  return TypeId(uint32_t(vec_start_id) + uint32_t(scalar_type_id) - uint32_t(TypeId::kInt8));
}
}
namespace Type {
struct Bool {};
struct Int8 {};
struct UInt8 {};
struct Int16 {};
struct UInt16 {};
struct Int32 {};
struct UInt32 {};
struct Int64 {};
struct UInt64 {};
struct IntPtr {};
struct UIntPtr {};
struct Float32 {};
struct Float64 {};
struct Vec128 {};
struct Vec256 {};
struct Vec512 {};
}
#define ASMJIT_DEFINE_TYPE_ID(T, TYPE_ID)                         \
namespace TypeUtils {                                             \
  template<>                                                      \
  struct TypeIdOfT<T> {                                           \
    static inline constexpr uint32_t kTypeId = uint32_t(TYPE_ID); \
  };                                                              \
}
ASMJIT_DEFINE_TYPE_ID(void         , TypeId::kVoid)
ASMJIT_DEFINE_TYPE_ID(Type::Bool   , TypeId::kUInt8)
ASMJIT_DEFINE_TYPE_ID(Type::Int8   , TypeId::kInt8)
ASMJIT_DEFINE_TYPE_ID(Type::UInt8  , TypeId::kUInt8)
ASMJIT_DEFINE_TYPE_ID(Type::Int16  , TypeId::kInt16)
ASMJIT_DEFINE_TYPE_ID(Type::UInt16 , TypeId::kUInt16)
ASMJIT_DEFINE_TYPE_ID(Type::Int32  , TypeId::kInt32)
ASMJIT_DEFINE_TYPE_ID(Type::UInt32 , TypeId::kUInt32)
ASMJIT_DEFINE_TYPE_ID(Type::Int64  , TypeId::kInt64)
ASMJIT_DEFINE_TYPE_ID(Type::UInt64 , TypeId::kUInt64)
ASMJIT_DEFINE_TYPE_ID(Type::IntPtr , TypeId::kIntPtr)
ASMJIT_DEFINE_TYPE_ID(Type::UIntPtr, TypeId::kUIntPtr)
ASMJIT_DEFINE_TYPE_ID(Type::Float32, TypeId::kFloat32)
ASMJIT_DEFINE_TYPE_ID(Type::Float64, TypeId::kFloat64)
ASMJIT_DEFINE_TYPE_ID(Type::Vec128 , TypeId::kInt32x4)
ASMJIT_DEFINE_TYPE_ID(Type::Vec256 , TypeId::kInt32x8)
ASMJIT_DEFINE_TYPE_ID(Type::Vec512 , TypeId::kInt32x16)
#undef ASMJIT_DEFINE_TYPE_ID
ASMJIT_END_NAMESPACE
#endif
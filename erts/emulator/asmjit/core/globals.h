#ifndef ASMJIT_CORE_GLOBALS_H_INCLUDED
#define ASMJIT_CORE_GLOBALS_H_INCLUDED
#include <asmjit/core/api-config.h>
ASMJIT_BEGIN_NAMESPACE
namespace Support {
template<typename Dst, typename Src>
static inline Dst ptr_cast_impl(Src p) noexcept { return (Dst)p; }
struct PlacementNew { void* ptr; };
}
#if defined(ASMJIT_NO_STDCXX)
namespace Support {
  ASMJIT_INLINE void* operator_new(size_t n) noexcept { return malloc(n); }
  ASMJIT_INLINE void operator_delete(void* p) noexcept { if (p) free(p); }
}
#define ASMJIT_BASE_CLASS(TYPE)                                                                    \
  ASMJIT_INLINE void* operator new(size_t n) noexcept { return Support::operator_new(n); }         \
  ASMJIT_INLINE void operator delete(void* ptr) noexcept { Support::operator_delete(ptr); }        \
                                                                                                   \
  ASMJIT_INLINE void* operator new(size_t, void* ptr) noexcept { return ptr; }                     \
  ASMJIT_INLINE void operator delete(void*, void*) noexcept {}                                     \
                                                                                                   \
  ASMJIT_INLINE void* operator new(size_t, Support::PlacementNew ptr) noexcept { return ptr.ptr; } \
  ASMJIT_INLINE void operator delete(void*, Support::PlacementNew) noexcept {}
#else
#define ASMJIT_BASE_CLASS(TYPE)
#endif
enum class ResetPolicy : uint32_t {
  kSoft = 0,
  kHard = 1
};
namespace Globals {
static constexpr uint32_t kAllocOverhead = uint32_t(sizeof(intptr_t) * 4u);
static constexpr uint32_t kAllocAlignment = 8u;
static constexpr uint32_t kGrowThreshold = 1024u * 1024u * 16u;
static constexpr uint32_t kMaxTreeHeight = (ASMJIT_ARCH_BITS == 32 ? 30 : 61) + 1;
static constexpr uint32_t kMaxOpCount = 6;
static constexpr uint32_t kMaxFuncArgs = 32;
static constexpr uint32_t kMaxValuePack = 4;
static constexpr uint32_t kMaxPhysRegs = 32;
static constexpr uint32_t kMaxAlignment = 64;
static constexpr uint32_t kMaxLabelNameSize = 2048;
static constexpr uint32_t kMaxSectionNameSize = 35;
static constexpr uint32_t kMaxCommentSize = 1024;
static constexpr uint32_t kInvalidId = 0xFFFFFFFFu;
static constexpr uint64_t kNoBaseAddress = ~uint64_t(0);
static constexpr uint32_t kNumVirtGroups = 4;
struct Init_ {};
struct NoInit_ {};
static const constexpr Init_ Init {};
static const constexpr NoInit_ NoInit {};
template<typename T>
static ASMJIT_INLINE_NODEBUG bool is_npos(const T& index) noexcept { return index == T(~T(0)); }
}
template<typename Func>
static ASMJIT_INLINE_NODEBUG Func ptr_as_func(void* p) noexcept {
  return Support::ptr_cast_impl<Func, void*>(p);
}
template<typename Func>
static ASMJIT_INLINE_NODEBUG Func ptr_as_func(void* p, size_t offset) noexcept {
  return Support::ptr_cast_impl<Func, void*>(static_cast<void*>(static_cast<char*>(p) + offset));
}
template<typename Func>
static ASMJIT_INLINE_NODEBUG void* func_as_ptr(Func func) noexcept {
  return Support::ptr_cast_impl<void*, Func>(func);
}
enum class Error : uint32_t {
  kOk = 0,
  kOutOfMemory,
  kInvalidArgument,
  kInvalidState,
  kInvalidArch,
  kNotInitialized,
  kAlreadyInitialized,
  kFeatureNotEnabled,
  kTooManyHandles,
  kTooLarge,
  kNoCodeGenerated,
  kInvalidDirective,
  kInvalidLabel,
  kTooManyLabels,
  kLabelAlreadyBound,
  kLabelAlreadyDefined,
  kLabelNameTooLong,
  kInvalidLabelName,
  kInvalidParentLabel,
  kInvalidSection,
  kTooManySections,
  kInvalidSectionName,
  kTooManyRelocations,
  kInvalidRelocEntry,
  kRelocOffsetOutOfRange,
  kInvalidAssignment,
  kInvalidInstruction,
  kInvalidRegType,
  kInvalidRegGroup,
  kInvalidPhysId,
  kInvalidVirtId,
  kInvalidElementIndex,
  kInvalidPrefixCombination,
  kInvalidLockPrefix,
  kInvalidXAcquirePrefix,
  kInvalidXReleasePrefix,
  kInvalidRepPrefix,
  kInvalidRexPrefix,
  kInvalidExtraReg,
  kInvalidKMaskUse,
  kInvalidKZeroUse,
  kInvalidBroadcast,
  kInvalidEROrSAE,
  kInvalidAddress,
  kInvalidAddressIndex,
  kInvalidAddressScale,
  kInvalidAddress64Bit,
  kInvalidAddress64BitZeroExtension,
  kInvalidDisplacement,
  kInvalidSegment,
  kInvalidImmediate,
  kInvalidOperandSize,
  kAmbiguousOperandSize,
  kOperandSizeMismatch,
  kInvalidOption,
  kOptionAlreadyDefined,
  kInvalidTypeId,
  kInvalidUseOfGpbHi,
  kInvalidUseOfGpq,
  kInvalidUseOfF80,
  kNotConsecutiveRegs,
  kConsecutiveRegsAllocation,
  kIllegalVirtReg,
  kTooManyVirtRegs,
  kNoMorePhysRegs,
  kOverlappedRegs,
  kOverlappingStackRegWithRegArg,
  kExpressionLabelNotBound,
  kExpressionOverflow,
  kFailedToOpenAnonymousMemory,
  kFailedToOpenFile,
  kProtectionFailure,
  kMaxValue = kProtectionFailure,
  kByPass = 0xFFFFFFFFu
};
static inline constexpr Error kErrorOk = Error::kOk;
#if defined(ASMJIT_BUILD_DEBUG)
  #define ASMJIT_ASSERT(...)                                                       \
    do {                                                                           \
      if (ASMJIT_UNLIKELY(!(__VA_ARGS__))) {                                       \
        ::asmjit::DebugUtils::assertion_failure(__FILE__, __LINE__, #__VA_ARGS__); \
      }                                                                            \
    } while (0)
#else
  #define ASMJIT_ASSERT(...) ((void)0)
#endif
#if defined(ASMJIT_BUILD_DEBUG)
  #define ASMJIT_NOT_REACHED() ::asmjit::DebugUtils::assertion_failure(__FILE__, __LINE__, "ASMJIT_NOT_REACHED()")
#elif defined(__GNUC__)
  #define ASMJIT_NOT_REACHED() __builtin_unreachable()
#else
  #define ASMJIT_NOT_REACHED() ASMJIT_ASSUME(0)
#endif
#define ASMJIT_PROPAGATE(...)                                          \
  do {                                                                 \
    ::asmjit::Error error_to_propagate = __VA_ARGS__;                  \
    if (ASMJIT_UNLIKELY(error_to_propagate != ::asmjit::Error::kOk)) { \
      return error_to_propagate;                                       \
    }                                                                  \
  } while (0)
[[nodiscard]]
static constexpr Error make_error(Error err) noexcept { return err; }
namespace DebugUtils {
[[nodiscard]]
ASMJIT_API const char* error_as_string(Error err) noexcept;
ASMJIT_API void debug_output(const char* str) noexcept;
[[noreturn]]
ASMJIT_API void assertion_failure(const char* file, int line, const char* msg) noexcept;
}
template<typename T>
class Out {
protected:
  T& _val;
public:
  ASMJIT_INLINE_NODEBUG explicit Out(T& val) noexcept
    : _val(val) {}
  ASMJIT_INLINE_NODEBUG Out& operator=(const T& val) noexcept {
    _val = val;
    return *this;
  }
  ASMJIT_INLINE_NODEBUG T& value() const noexcept { return _val; }
  ASMJIT_INLINE_NODEBUG T& operator*() const noexcept { return _val; }
  ASMJIT_INLINE_NODEBUG T* operator->() const noexcept { return &_val; }
};
ASMJIT_END_NAMESPACE
ASMJIT_INLINE_NODEBUG void* operator new(size_t, const asmjit::Support::PlacementNew& p) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
  __assume(p.ptr != nullptr);
#endif
  return p.ptr;
}
ASMJIT_INLINE_NODEBUG void operator delete(void*, const asmjit::Support::PlacementNew&) noexcept {}
#endif
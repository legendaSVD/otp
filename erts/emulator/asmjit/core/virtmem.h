#ifndef ASMJIT_CORE_VIRTMEM_H_INCLUDED
#define ASMJIT_CORE_VIRTMEM_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_JIT
#include <asmjit/core/globals.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
namespace VirtMem {
enum class CachePolicy : uint32_t {
  kDefault = 0,
  kFlushAfterWrite = 1,
  kNeverFlush = 2
};
ASMJIT_API void flush_instruction_cache(void* p, size_t size) noexcept;
struct Info {
  uint32_t page_size;
  uint32_t page_granularity;
};
[[nodiscard]]
ASMJIT_API Info info() noexcept;
[[nodiscard]]
ASMJIT_API size_t large_page_size() noexcept;
enum class MemoryFlags : uint32_t {
  kNone = 0,
  kAccessRead = 0x00000001u,
  kAccessWrite = 0x00000002u,
  kAccessExecute = 0x00000004u,
  kAccessReadWrite = kAccessRead | kAccessWrite,
  kAccessRW = kAccessRead | kAccessWrite,
  kAccessRX = kAccessRead | kAccessExecute,
  kAccessRWX = kAccessRead | kAccessWrite | kAccessExecute,
  kMMapEnableMapJit = 0x00000010u,
  kMMapMaxAccessRead = 0x00000020u,
  kMMapMaxAccessWrite = 0x00000040u,
  kMMapMaxAccessExecute = 0x00000080u,
  kMMapMaxAccessReadWrite = kMMapMaxAccessRead | kMMapMaxAccessWrite,
  kMMapMaxAccessRW = kMMapMaxAccessRead | kMMapMaxAccessWrite,
  kMMapMaxAccessRX = kMMapMaxAccessRead | kMMapMaxAccessExecute,
  kMMapMaxAccessRWX = kMMapMaxAccessRead | kMMapMaxAccessWrite | kMMapMaxAccessExecute,
  kMapShared = 0x00000100u,
  kMMapLargePages = 0x00000200u,
  kMappingPreferTmp = 0x80000000u
};
ASMJIT_DEFINE_ENUM_FLAGS(MemoryFlags)
[[nodiscard]]
ASMJIT_API Error alloc(void** p, size_t size, MemoryFlags flags) noexcept;
[[nodiscard]]
ASMJIT_API Error release(void* p, size_t size) noexcept;
[[nodiscard]]
ASMJIT_API Error protect(void* p, size_t size, MemoryFlags flags) noexcept;
struct DualMapping {
  void* rx;
  void* rw;
};
[[nodiscard]]
ASMJIT_API Error alloc_dual_mapping(Out<DualMapping> dm, size_t size, MemoryFlags flags) noexcept;
[[nodiscard]]
ASMJIT_API Error release_dual_mapping(DualMapping& dm, size_t size) noexcept;
enum class HardenedRuntimeFlags : uint32_t {
  kNone = 0,
  kEnabled = 0x00000001u,
  kMapJit = 0x00000002u,
  kDualMapping = 0x00000004u
};
ASMJIT_DEFINE_ENUM_FLAGS(HardenedRuntimeFlags)
struct HardenedRuntimeInfo {
  HardenedRuntimeFlags flags;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(HardenedRuntimeFlags flag) const noexcept { return Support::test(flags, flag); }
};
[[nodiscard]]
ASMJIT_API HardenedRuntimeInfo hardened_runtime_info() noexcept;
enum class ProtectJitAccess : uint32_t {
  kReadWrite = 0,
  kReadExecute = 1
};
ASMJIT_API void protect_jit_memory(ProtectJitAccess access) noexcept;
class ProtectJitReadWriteScope {
public:
  ASMJIT_NONCOPYABLE(ProtectJitReadWriteScope)
  void* _rx_ptr;
  size_t _size;
  CachePolicy _policy;
  ASMJIT_INLINE ProtectJitReadWriteScope(
    void* rx_ptr,
    size_t size,
    CachePolicy policy = CachePolicy::kDefault
  ) noexcept
    : _rx_ptr(rx_ptr),
      _size(size),
      _policy(policy) {
    protect_jit_memory(ProtectJitAccess::kReadWrite);
  }
  ASMJIT_INLINE  ~ProtectJitReadWriteScope() noexcept {
    protect_jit_memory(ProtectJitAccess::kReadExecute);
    if (_policy != CachePolicy::kNeverFlush) {
      flush_instruction_cache(_rx_ptr, _size);
    }
  }
};
}
ASMJIT_END_NAMESPACE
#endif
#endif
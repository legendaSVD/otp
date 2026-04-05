#ifndef ASMJIT_CORE_JITALLOCATOR_H_INCLUDED
#define ASMJIT_CORE_JITALLOCATOR_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_JIT
#include <asmjit/core/globals.h>
#include <asmjit/core/virtmem.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
enum class JitAllocatorOptions : uint32_t {
  kNone = 0,
  kUseDualMapping = 0x00000001u,
  kUseMultiplePools = 0x00000002u,
  kFillUnusedMemory = 0x00000004u,
  kImmediateRelease = 0x00000008u,
  kDisableInitialPadding = 0x00000010u,
  kUseLargePages = 0x00000020u,
  kAlignBlockSizeToLargePage = 0x00000040u,
  kCustomFillPattern = 0x10000000u
};
ASMJIT_DEFINE_ENUM_FLAGS(JitAllocatorOptions)
class JitAllocator {
public:
  ASMJIT_NONCOPYABLE(JitAllocator)
  struct Impl {
    JitAllocatorOptions options;
    uint32_t block_size;
    uint32_t granularity;
    uint32_t fill_pattern;
  };
  Impl* _impl;
  struct CreateParams {
    JitAllocatorOptions options = JitAllocatorOptions::kNone;
    uint32_t block_size = 0;
    uint32_t granularity = 0;
    uint32_t fill_pattern = 0;
    ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = CreateParams{}; }
  };
  ASMJIT_API explicit JitAllocator(const CreateParams* params = nullptr) noexcept;
  ASMJIT_API ~JitAllocator() noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_initialized() const noexcept { return _impl->block_size == 0; }
  ASMJIT_API void reset(ResetPolicy reset_policy = ResetPolicy::kSoft) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG JitAllocatorOptions options() const noexcept { return _impl->options; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_option(JitAllocatorOptions option) const noexcept { return uint32_t(_impl->options & option) != 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t block_size() const noexcept { return _impl->block_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t granularity() const noexcept { return _impl->granularity; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t fill_pattern() const noexcept { return _impl->fill_pattern; }
  class Span {
  public:
    enum class Flags : uint32_t {
      kNone = 0u,
      kInstructionCacheClean = 0x00000001u
    };
    void* _rx = nullptr;
    void* _rw = nullptr;
    size_t _size = 0;
    void* _block = nullptr;
    Flags _flags = Flags::kNone;
    uint32_t _reserved = 0;
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG void* rx() const noexcept { return _rx; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG void* rw() const noexcept { return _rw; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG Flags flags() const noexcept { return _flags; }
    ASMJIT_INLINE_NODEBUG void shrink(size_t new_size) noexcept { _size = Support::min(_size, new_size); }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool is_directly_writable() const noexcept { return _rw != nullptr; }
  };
  [[nodiscard]]
  ASMJIT_API Error alloc(Out<Span> out, size_t size) noexcept;
  ASMJIT_API Error release(void* rx) noexcept;
  ASMJIT_API Error shrink(Span& span, size_t new_size) noexcept;
  [[nodiscard]]
  ASMJIT_API Error query(Out<Span> out, void* rx) const noexcept;
  using WriteFunc = Error (ASMJIT_CDECL*)(Span& span, void* user_data) noexcept;
  ASMJIT_API Error write(
    Span& span,
    size_t offset,
    const void* src,
    size_t size,
    VirtMem::CachePolicy policy = VirtMem::CachePolicy::kDefault) noexcept;
  ASMJIT_API Error write(
    Span& span,
    WriteFunc write_fn,
    void* user_data,
    VirtMem::CachePolicy policy = VirtMem::CachePolicy::kDefault) noexcept;
  template<class Lambda>
  ASMJIT_INLINE Error write(
    Span& span,
    Lambda&& lambda_fn,
    VirtMem::CachePolicy policy = VirtMem::CachePolicy::kDefault) noexcept {
    WriteFunc wrapper_func = [](Span& span, void* user_data) noexcept -> Error {
      Lambda& lambda_fn = *static_cast<Lambda*>(user_data);
      return lambda_fn(span);
    };
    return write(span, wrapper_func, (void*)(&lambda_fn), policy);
  }
  struct WriteScopeData {
    VirtMem::CachePolicy policy;
    uint32_t flags;
    uintptr_t data[2];
  };
  ASMJIT_API Error begin_write_scope(WriteScopeData& scope, VirtMem::CachePolicy policy = VirtMem::CachePolicy::kDefault) noexcept;
  ASMJIT_API Error end_write_scope(WriteScopeData& scope) noexcept;
  ASMJIT_API Error flush_write_scope(WriteScopeData& scope) noexcept;
  ASMJIT_API Error scoped_write(WriteScopeData& scope, Span& span, size_t offset, const void* src, size_t size) noexcept;
  ASMJIT_API Error scoped_write(WriteScopeData& scope, Span& span, WriteFunc write_fn, void* user_data) noexcept;
  template<class Lambda>
  ASMJIT_INLINE Error scoped_write(WriteScopeData& scope, Span& span, Lambda&& lambda_fn) noexcept {
    WriteFunc wrapper_func = [](Span& span, void* user_data) noexcept -> Error {
      Lambda& lambda_fn = *static_cast<Lambda*>(user_data);
      return lambda_fn(span);
    };
    return scoped_write(scope, span, wrapper_func, (void*)(&lambda_fn));
  }
  class WriteScope {
  public:
    ASMJIT_NONCOPYABLE(WriteScope)
    JitAllocator& _allocator;
    WriteScopeData _scope_data;
    ASMJIT_INLINE explicit WriteScope(JitAllocator& allocator, VirtMem::CachePolicy policy = VirtMem::CachePolicy::kDefault) noexcept
      : _allocator(allocator) { _allocator.begin_write_scope(_scope_data, policy); }
    ASMJIT_INLINE ~WriteScope() noexcept {
      _allocator.end_write_scope(_scope_data);
    }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG JitAllocator& allocator() const noexcept { return _allocator; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG VirtMem::CachePolicy policy() const noexcept { return _scope_data.policy; }
    ASMJIT_INLINE_NODEBUG Error write(Span& span, size_t offset, const void* src, size_t size) noexcept {
      return _allocator.scoped_write(_scope_data, span, offset, src, size);
    }
    ASMJIT_INLINE_NODEBUG Error write(Span& span, WriteFunc write_fn, void* user_data) noexcept {
      return _allocator.scoped_write(_scope_data, span, write_fn, user_data);
    }
    template<class Lambda>
    ASMJIT_INLINE_NODEBUG Error write(Span& span, Lambda&& lambda_fn) noexcept {
      return _allocator.scoped_write(_scope_data, span, lambda_fn);
    }
    ASMJIT_INLINE_NODEBUG Error flush() noexcept {
      return _allocator.flush_write_scope(_scope_data);
    }
  };
  struct Statistics {
    size_t _block_count;
    size_t _allocation_count;
    size_t _used_size;
    size_t _reserved_size;
    size_t _overhead_size;
    ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = Statistics{}; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t block_count() const noexcept { return _block_count; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t allocation_count() const noexcept { return _allocation_count; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t used_size() const noexcept { return _used_size; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t unused_size() const noexcept { return _reserved_size - _used_size; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t reserved_size() const noexcept { return _reserved_size; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t overhead_size() const noexcept { return _overhead_size; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG double used_ratio() const noexcept {
      return (double(used_size()) / (double(reserved_size()) + 1e-16));
    }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG double unused_ratio() const noexcept {
      return (double(unused_size()) / (double(reserved_size()) + 1e-16));
    }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG double overhead_ratio() const noexcept {
      return (double(overhead_size()) / (double(reserved_size()) + 1e-16));
    }
  };
  [[nodiscard]]
  ASMJIT_API Statistics statistics() const noexcept;
};
ASMJIT_END_NAMESPACE
#endif
#endif
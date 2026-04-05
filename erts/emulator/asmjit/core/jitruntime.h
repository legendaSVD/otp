#ifndef ASMJIT_CORE_JITRUNTIME_H_INCLUDED
#define ASMJIT_CORE_JITRUNTIME_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_JIT
#include <asmjit/core/codeholder.h>
#include <asmjit/core/jitallocator.h>
#include <asmjit/core/target.h>
ASMJIT_BEGIN_NAMESPACE
class CodeHolder;
class ASMJIT_VIRTAPI JitRuntime : public Target {
public:
  ASMJIT_NONCOPYABLE(JitRuntime)
  JitAllocator _allocator;
  ASMJIT_API explicit JitRuntime(const JitAllocator::CreateParams* params = nullptr) noexcept;
  ASMJIT_INLINE explicit JitRuntime(const JitAllocator::CreateParams& params) noexcept
    : JitRuntime(&params) {}
  ASMJIT_API ~JitRuntime() noexcept override;
  ASMJIT_INLINE_NODEBUG void reset(ResetPolicy reset_policy = ResetPolicy::kSoft) noexcept {
    _allocator.reset(reset_policy);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG JitAllocator& allocator() const noexcept { return const_cast<JitAllocator&>(_allocator); }
  template<typename Func>
  ASMJIT_INLINE_NODEBUG Error add(Func* dst, CodeHolder* code) noexcept {
    return _add(Support::ptr_cast_impl<void**, Func*>(dst), code);
  }
  template<typename Func>
  ASMJIT_INLINE_NODEBUG Error release(Func p) noexcept {
    return _release(Support::ptr_cast_impl<void*, Func>(p));
  }
  ASMJIT_API virtual Error _add(void** dst, CodeHolder* code) noexcept;
  ASMJIT_API virtual Error _release(void* p) noexcept;
};
ASMJIT_END_NAMESPACE
#endif
#endif
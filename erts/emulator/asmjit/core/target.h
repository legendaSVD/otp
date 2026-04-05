#ifndef ASMJIT_CORE_TARGET_H_INCLUDED
#define ASMJIT_CORE_TARGET_H_INCLUDED
#include <asmjit/core/archtraits.h>
#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/func.h>
ASMJIT_BEGIN_NAMESPACE
class ASMJIT_VIRTAPI Target {
public:
  ASMJIT_BASE_CLASS(Target)
  ASMJIT_NONCOPYABLE(Target)
  Environment _environment;
  CpuFeatures _cpu_features;
  CpuHints _cpu_hints;
  ASMJIT_API Target() noexcept;
  ASMJIT_API virtual ~Target() noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Environment& environment() const noexcept { return _environment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _environment.arch(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SubArch sub_arch() const noexcept { return _environment.sub_arch(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CpuFeatures& cpu_features() const noexcept { return _cpu_features; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CpuHints cpu_hints() const noexcept { return _cpu_hints; }
};
ASMJIT_END_NAMESPACE
#endif
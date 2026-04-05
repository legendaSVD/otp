#include <asmjit/core/api-build_p.h>
#include <asmjit/core/target.h>
ASMJIT_BEGIN_NAMESPACE
Target::Target() noexcept
  : _environment{},
    _cpu_features{},
    _cpu_hints{} {}
Target::~Target() noexcept {}
ASMJIT_END_NAMESPACE
#include <asmjit/core/api-build_p.h>
#include <asmjit/core/environment.h>
ASMJIT_BEGIN_NAMESPACE
uint32_t Environment::stack_alignment() const noexcept {
  if (is_64bit()) {
    return 16;
  }
  else {
    if (is_platform_linux() ||
        is_platform_bsd() ||
        is_platform_apple() ||
        is_platform_haiku()) {
      return 16u;
    }
    if (is_family_arm()) {
      return 8;
    }
    return 4;
  }
}
ASMJIT_END_NAMESPACE
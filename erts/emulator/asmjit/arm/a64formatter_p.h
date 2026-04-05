#ifndef ASMJIT_ARM_A64FORMATTER_P_H_INCLUDED
#define ASMJIT_ARM_A64FORMATTER_P_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_LOGGING
#include <asmjit/core/formatter.h>
#include <asmjit/core/string.h>
#include <asmjit/support/span.h>
#include <asmjit/arm/armformatter_p.h>
#include <asmjit/arm/a64globals.h>
ASMJIT_BEGIN_SUB_NAMESPACE(a64)
namespace FormatterInternal {
using namespace arm::FormatterInternal;
Error ASMJIT_CDECL format_instruction(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  const BaseInst& inst, Span<const Operand_> operands) noexcept;
}
ASMJIT_END_SUB_NAMESPACE
#endif
#endif
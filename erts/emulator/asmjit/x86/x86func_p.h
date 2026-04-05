#ifndef ASMJIT_X86_X86FUNC_P_H_INCLUDED
#define ASMJIT_X86_X86FUNC_P_H_INCLUDED
#include <asmjit/core/func.h>
ASMJIT_BEGIN_SUB_NAMESPACE(x86)
namespace FuncInternal {
Error init_call_conv(CallConv& cc, CallConvId call_conv_id, const Environment& environment) noexcept;
Error init_func_detail(FuncDetail& func, const FuncSignature& signature, uint32_t register_size) noexcept;
}
ASMJIT_END_SUB_NAMESPACE
#endif
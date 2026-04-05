#ifndef ASMJIT_HOST_H_INCLUDED
#define ASMJIT_HOST_H_INCLUDED
#include <asmjit/core.h>
#if ASMJIT_ARCH_X86 != 0 && !defined(ASMJIT_NO_X86)
#include <asmjit/x86.h>
ASMJIT_BEGIN_NAMESPACE
namespace host { using namespace x86; }
ASMJIT_END_NAMESPACE
#endif
#if ASMJIT_ARCH_ARM == 64 && !defined(ASMJIT_NO_AARCH64)
#include <asmjit/a64.h>
ASMJIT_BEGIN_NAMESPACE
namespace host { using namespace a64; }
ASMJIT_END_NAMESPACE
#endif
#endif
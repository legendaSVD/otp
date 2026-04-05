#ifndef ASMJIT_ASMJIT_H_INCLUDED
#define ASMJIT_ASMJIT_H_INCLUDED
#pragma message("asmjit/asmjit.h is deprecated! Please use asmjit/[core|x86|a64|host].h instead.")
#include <asmjit/core.h>
#ifndef ASMJIT_NO_X86
  #include <asmjit/x86.h>
#endif
#endif
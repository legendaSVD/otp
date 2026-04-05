#ifndef ASMJIT_ARM_A64ARCHTRAITS_P_H_INCLUDED
#define ASMJIT_ARM_A64ARCHTRAITS_P_H_INCLUDED
#include <asmjit/core/archtraits.h>
#include <asmjit/core/misc_p.h>
#include <asmjit/core/type.h>
#include <asmjit/arm/a64globals.h>
#include <asmjit/arm/a64operand.h>
ASMJIT_BEGIN_SUB_NAMESPACE(a64)
static const constexpr ArchTraits a64_arch_traits = {
  Gp::kIdSp, Gp::kIdFp, Gp::kIdLr, 0xFFu,
  { 0u, 0u, 0u },
  16u,
  4095, 65520,
  0u | (1u << uint32_t(RegType::kGp32  ))
     | (1u << uint32_t(RegType::kGp64  ))
     | (1u << uint32_t(RegType::kVec8  ))
     | (1u << uint32_t(RegType::kVec16 ))
     | (1u << uint32_t(RegType::kVec32 ))
     | (1u << uint32_t(RegType::kVec64 ))
     | (1u << uint32_t(RegType::kVec128))
     | (1u << uint32_t(RegType::kMask  )),
  {{
    InstHints::kPushPop,
    InstHints::kPushPop,
    InstHints::kNoHints,
    InstHints::kNoHints
  }},
  #define V(index) (index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt8)    ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt8)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt16)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt16)  ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt32)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt32)  ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt64)   ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt64)  ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kIntPtr)  ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUIntPtr) ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat32) ? RegType::kVec32  : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat64) ? RegType::kVec64  : RegType::kNone)
  {{ ASMJIT_LOOKUP_TABLE_32(V, 0) }},
  #undef V
  {
    ArchTypeNameId::kByte,
    ArchTypeNameId::kHWord,
    ArchTypeNameId::kWord,
    ArchTypeNameId::kXWord
  }
};
ASMJIT_END_SUB_NAMESPACE
#endif
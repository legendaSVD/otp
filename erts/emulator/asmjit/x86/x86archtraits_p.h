#ifndef ASMJIT_X86_X86ARCHTRAITS_P_H_INCLUDED
#define ASMJIT_X86_X86ARCHTRAITS_P_H_INCLUDED
#include <asmjit/core/archtraits.h>
#include <asmjit/core/misc_p.h>
#include <asmjit/x86/x86operand.h>
ASMJIT_BEGIN_SUB_NAMESPACE(x86)
static const constexpr ArchTraits x86_arch_traits = {
  Gp::kIdSp, Gp::kIdBp, 0xFF, 0xFF,
  { 0, 0, 0 },
  1,
  0x7FFFFFFFu, 0x7FFFFFFFu,
  0u | (1u << uint32_t(RegType::kGp8Lo  ))
     | (1u << uint32_t(RegType::kGp8Hi  ))
     | (1u << uint32_t(RegType::kGp16   ))
     | (1u << uint32_t(RegType::kGp32   ))
     | (1u << uint32_t(RegType::kGp64   ))
     | (1u << uint32_t(RegType::kVec128 ))
     | (1u << uint32_t(RegType::kVec256 ))
     | (1u << uint32_t(RegType::kVec512 ))
     | (1u << uint32_t(RegType::kMask   ))
     | (1u << uint32_t(RegType::kSegment))
     | (1u << uint32_t(RegType::kControl))
     | (1u << uint32_t(RegType::kDebug  ))
     | (1u << uint32_t(RegType::kX86_Mm ))
     | (1u << uint32_t(RegType::kX86_St ))
     | (1u << uint32_t(RegType::kX86_Bnd))
     | (1u << uint32_t(RegType::kPC     )),
  {{
    InstHints::kRegSwap | InstHints::kPushPop,
    InstHints::kNoHints,
    InstHints::kNoHints,
    InstHints::kNoHints
  }},
  #define V(index) (index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt8)    ? RegType::kGp8Lo     : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt8)   ? RegType::kGp8Lo     : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt16)   ? RegType::kGp16      : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt16)  ? RegType::kGp16      : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt32)   ? RegType::kGp32      : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt32)  ? RegType::kGp32      : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kIntPtr)  ? RegType::kGp32      : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUIntPtr) ? RegType::kGp32      : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat32) ? RegType::kVec128    : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat64) ? RegType::kVec128    : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMask8)   ? RegType::kMask      : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMask16)  ? RegType::kMask      : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMask32)  ? RegType::kMask      : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMask64)  ? RegType::kMask      : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMmx32)   ? RegType::kX86_Mm    : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMmx64)   ? RegType::kX86_Mm    : RegType::kNone)
  {{ ASMJIT_LOOKUP_TABLE_32(V, 0) }},
  #undef V
  {
    ArchTypeNameId::kDB,
    ArchTypeNameId::kDW,
    ArchTypeNameId::kDD,
    ArchTypeNameId::kDQ
  }
};
static const constexpr ArchTraits x64_arch_traits = {
  Gp::kIdSp, Gp::kIdBp, 0xFF, 0xFF,
  { 0, 0, 0 },
  1,
  0x7FFFFFFFu, 0x7FFFFFFFu,
  0u | (1u << uint32_t(RegType::kGp8Lo  ))
     | (1u << uint32_t(RegType::kGp8Hi  ))
     | (1u << uint32_t(RegType::kGp16   ))
     | (1u << uint32_t(RegType::kGp32   ))
     | (1u << uint32_t(RegType::kGp64   ))
     | (1u << uint32_t(RegType::kVec128 ))
     | (1u << uint32_t(RegType::kVec256 ))
     | (1u << uint32_t(RegType::kVec512 ))
     | (1u << uint32_t(RegType::kMask   ))
     | (1u << uint32_t(RegType::kTile   ))
     | (1u << uint32_t(RegType::kSegment))
     | (1u << uint32_t(RegType::kControl))
     | (1u << uint32_t(RegType::kDebug  ))
     | (1u << uint32_t(RegType::kX86_Mm ))
     | (1u << uint32_t(RegType::kX86_St ))
     | (1u << uint32_t(RegType::kX86_Bnd))
     | (1u << uint32_t(RegType::kPC     )),
  {{
    InstHints::kRegSwap | InstHints::kPushPop,
    InstHints::kNoHints,
    InstHints::kNoHints,
    InstHints::kNoHints
  }},
  #define V(index) (index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt8)    ? RegType::kGp8Lo  : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt8)   ? RegType::kGp8Lo  : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt16)   ? RegType::kGp16   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt16)  ? RegType::kGp16   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt32)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt32)  ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt64)   ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt64)  ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kIntPtr)  ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUIntPtr) ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat32) ? RegType::kVec128 : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat64) ? RegType::kVec128 : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMask8)   ? RegType::kMask   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMask16)  ? RegType::kMask   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMask32)  ? RegType::kMask   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMask64)  ? RegType::kMask   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMmx32)   ? RegType::kX86_Mm : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kMmx64)   ? RegType::kX86_Mm : RegType::kNone)
  {{ ASMJIT_LOOKUP_TABLE_32(V, 0) }},
  #undef V
  {
    ArchTypeNameId::kDB,
    ArchTypeNameId::kDW,
    ArchTypeNameId::kDD,
    ArchTypeNameId::kDQ
  }
};
ASMJIT_END_SUB_NAMESPACE
#endif
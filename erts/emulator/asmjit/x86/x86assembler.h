#ifndef ASMJIT_X86_X86ASSEMBLER_H_INCLUDED
#define ASMJIT_X86_X86ASSEMBLER_H_INCLUDED
#include <asmjit/core/assembler.h>
#include <asmjit/x86/x86emitter.h>
#include <asmjit/x86/x86operand.h>
ASMJIT_BEGIN_SUB_NAMESPACE(x86)
class ASMJIT_VIRTAPI Assembler
  : public BaseAssembler,
    public EmitterImplicitT<Assembler> {
public:
  ASMJIT_NONCOPYABLE(Assembler)
  using Base = BaseAssembler;
  ASMJIT_API explicit Assembler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Assembler() noexcept override;
  ASMJIT_INLINE_NODEBUG uint32_t _address_override_mask() const noexcept { return _private_data; }
  ASMJIT_INLINE_NODEBUG void _set_address_override_mask(uint32_t m) noexcept { _private_data = m; }
  ASMJIT_API Error _emit(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext) override;
  ASMJIT_API Error align(AlignMode align_mode, uint32_t alignment) override;
  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
};
ASMJIT_END_SUB_NAMESPACE
#endif
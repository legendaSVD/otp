#ifndef ASMJIT_X86_X86BUILDER_H_INCLUDED
#define ASMJIT_X86_X86BUILDER_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_BUILDER
#include <asmjit/core/builder.h>
#include <asmjit/x86/x86emitter.h>
ASMJIT_BEGIN_SUB_NAMESPACE(x86)
class ASMJIT_VIRTAPI Builder
  : public BaseBuilder,
    public EmitterImplicitT<Builder> {
public:
  ASMJIT_NONCOPYABLE(Builder)
  using Base = BaseBuilder;
  ASMJIT_API explicit Builder(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Builder() noexcept override;
  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
  ASMJIT_API Error finalize() override;
};
ASMJIT_END_SUB_NAMESPACE
#endif
#endif
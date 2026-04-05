#ifndef ASMJIT_X86_X86COMPILER_H_INCLUDED
#define ASMJIT_X86_X86COMPILER_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER
#include <asmjit/core/compiler.h>
#include <asmjit/core/type.h>
#include <asmjit/x86/x86emitter.h>
ASMJIT_BEGIN_SUB_NAMESPACE(x86)
class ASMJIT_VIRTAPI Compiler
  : public BaseCompiler,
    public EmitterExplicitT<Compiler> {
public:
  ASMJIT_NONCOPYABLE(Compiler)
  using Base = BaseCompiler;
  ASMJIT_API explicit Compiler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Compiler() noexcept override;
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp(TypeId type_id, Args&&... args) { return new_reg<Gp>(type_id, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec(TypeId type_id, Args&&... args) { return new_reg<Vec>(type_id, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_k(TypeId type_id, Args&&... args) { return new_reg<KReg>(type_id, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp8(Args&&... args) { return new_reg<Gp>(TypeId::kUInt8, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp16(Args&&... args) { return new_reg<Gp>(TypeId::kUInt16, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp32(Args&&... args) { return new_reg<Gp>(TypeId::kUInt32, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp64(Args&&... args) { return new_reg<Gp>(TypeId::kUInt64, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gpz(Args&&... args) { return new_reg<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp_ptr(Args&&... args) { return new_reg<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x4, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f32x1(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x1, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f64x1(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x1, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f32x4(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x4, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f64x2(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x2, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec256(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x8, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec256_f32x8(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x8, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec256_f64x4(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x4, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec512(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x16, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec512_f32x16(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x16, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec512_f64x8(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x8, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_xmm(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x4, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_xmm_ss(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x1, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_xmm_sd(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x1, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_xmm_ps(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x4, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_xmm_pd(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x2, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_ymm(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x8, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_ymm_ps(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x8, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_ymm_pd(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x4, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_zmm(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x16, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_zmm_ps(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x16, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_zmm_pd(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x8, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Mm new_mm(Args&&... args) { return new_reg<Mm>(TypeId::kMmx64, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_k8(Args&&... args) { return new_reg<KReg>(TypeId::kMask8, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_k16(Args&&... args) { return new_reg<KReg>(TypeId::kMask16, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_k32(Args&&... args) { return new_reg<KReg>(TypeId::kMask32, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_k64(Args&&... args) { return new_reg<KReg>(TypeId::kMask64, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_kb(Args&&... args) { return new_reg<KReg>(TypeId::kMask8, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_kw(Args&&... args) { return new_reg<KReg>(TypeId::kMask16, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_kd(Args&&... args) { return new_reg<KReg>(TypeId::kMask32, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_kq(Args&&... args) { return new_reg<KReg>(TypeId::kMask64, std::forward<Args>(args)...); }
  ASMJIT_INLINE_NODEBUG Mem new_stack(uint32_t size, uint32_t alignment, const char* name = nullptr) {
    Mem m(Globals::NoInit);
    _new_stack(Out<BaseMem>{m}, size, alignment, name);
    return m;
  }
  ASMJIT_INLINE_NODEBUG Mem new_const(ConstPoolScope scope, const void* data, size_t size) {
    Mem m(Globals::NoInit);
    _new_const(Out<BaseMem>(m), scope, data, size);
    return m;
  }
  ASMJIT_INLINE_NODEBUG Mem new_byte_const(ConstPoolScope scope, uint8_t val) noexcept { return new_const(scope, &val, 1); }
  ASMJIT_INLINE_NODEBUG Mem new_word_const(ConstPoolScope scope, uint16_t val) noexcept { return new_const(scope, &val, 2); }
  ASMJIT_INLINE_NODEBUG Mem new_dword_const(ConstPoolScope scope, uint32_t val) noexcept { return new_const(scope, &val, 4); }
  ASMJIT_INLINE_NODEBUG Mem new_qword_const(ConstPoolScope scope, uint64_t val) noexcept { return new_const(scope, &val, 8); }
  ASMJIT_INLINE_NODEBUG Mem new_int16_const(ConstPoolScope scope, int16_t val) noexcept { return new_const(scope, &val, 2); }
  ASMJIT_INLINE_NODEBUG Mem new_uint16_const(ConstPoolScope scope, uint16_t val) noexcept { return new_const(scope, &val, 2); }
  ASMJIT_INLINE_NODEBUG Mem new_int32_const(ConstPoolScope scope, int32_t val) noexcept { return new_const(scope, &val, 4); }
  ASMJIT_INLINE_NODEBUG Mem new_uint32_const(ConstPoolScope scope, uint32_t val) noexcept { return new_const(scope, &val, 4); }
  ASMJIT_INLINE_NODEBUG Mem new_int64_const(ConstPoolScope scope, int64_t val) noexcept { return new_const(scope, &val, 8); }
  ASMJIT_INLINE_NODEBUG Mem new_uint64_const(ConstPoolScope scope, uint64_t val) noexcept { return new_const(scope, &val, 8); }
  ASMJIT_INLINE_NODEBUG Mem new_float_const(ConstPoolScope scope, float val) noexcept { return new_const(scope, &val, 4); }
  ASMJIT_INLINE_NODEBUG Mem new_double_const(ConstPoolScope scope, double val) noexcept { return new_const(scope, &val, 8); }
  ASMJIT_INLINE_NODEBUG Compiler& unfollow() noexcept { add_inst_options(InstOptions::kUnfollow); return *this; }
  ASMJIT_INLINE_NODEBUG Compiler& overwrite() noexcept { add_inst_options(InstOptions::kOverwrite); return *this; }
  ASMJIT_INLINE_NODEBUG Error invoke_(Out<InvokeNode*> out, const Operand_& target, const FuncSignature& signature) {
    return add_invoke_node(out, Inst::kIdCall, target, signature);
  }
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Gp& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Mem& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Label& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Imm& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, uint64_t target, const FuncSignature& signature) { return invoke_(out, Imm(int64_t(target)), signature); }
  ASMJIT_INLINE_NODEBUG Error ret() { return add_ret(Operand(), Operand()); }
  ASMJIT_INLINE_NODEBUG Error ret(const Reg& o0) { return add_ret(o0, Operand()); }
  ASMJIT_INLINE_NODEBUG Error ret(const Reg& o0, const Reg& o1) { return add_ret(o0, o1); }
  using EmitterExplicitT<Compiler>::jmp;
  ASMJIT_INLINE_NODEBUG Error jmp(const Reg& target, JumpAnnotation* annotation) { return emit_annotated_jump(Inst::kIdJmp, target, annotation); }
  ASMJIT_INLINE_NODEBUG Error jmp(const BaseMem& target, JumpAnnotation* annotation) { return emit_annotated_jump(Inst::kIdJmp, target, annotation); }
  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_reinit(CodeHolder& code) noexcept override;
  ASMJIT_API Error finalize() override;
};
ASMJIT_END_SUB_NAMESPACE
#endif
#endif
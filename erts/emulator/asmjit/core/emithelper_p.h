#ifndef ASMJIT_CORE_EMITHELPER_P_H_INCLUDED
#define ASMJIT_CORE_EMITHELPER_P_H_INCLUDED
#include <asmjit/core/emitter.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
ASMJIT_BEGIN_NAMESPACE
class BaseEmitHelper {
protected:
  BaseEmitter* _emitter;
public:
  ASMJIT_INLINE_NODEBUG explicit BaseEmitHelper(BaseEmitter* emitter = nullptr) noexcept
    : _emitter(emitter) {}
  ASMJIT_INLINE_NODEBUG virtual ~BaseEmitHelper() noexcept = default;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseEmitter* emitter() const noexcept { return _emitter; }
  virtual Error emit_reg_move(
    const Operand_& dst_,
    const Operand_& src_, TypeId type_id, const char* comment = nullptr);
  virtual Error emit_reg_swap(
    const Reg& a,
    const Reg& b, const char* comment = nullptr);
  virtual Error emit_arg_move(
    const Reg& dst_, TypeId dst_type_id,
    const Operand_& src_, TypeId src_type_id, const char* comment = nullptr);
  Error emit_args_assignment(const FuncFrame& frame, const FuncArgsAssignment& args);
};
ASMJIT_END_NAMESPACE
#endif
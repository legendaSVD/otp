#ifndef ASMJIT_CORE_COMPILER_H_INCLUDED
#define ASMJIT_CORE_COMPILER_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER
#include <asmjit/core/assembler.h>
#include <asmjit/core/builder.h>
#include <asmjit/core/constpool.h>
#include <asmjit/core/compilerdefs.h>
#include <asmjit/core/func.h>
#include <asmjit/core/inst.h>
#include <asmjit/core/operand.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenavector.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
class JumpAnnotation;
class JumpNode;
class FuncNode;
class FuncRetNode;
class InvokeNode;
class ASMJIT_VIRTAPI BaseCompiler : public BaseBuilder {
public:
  ASMJIT_NONCOPYABLE(BaseCompiler)
  using Base = BaseBuilder;
  FuncNode* _func;
  ArenaVector<VirtReg*> _virt_regs;
  ArenaVector<JumpAnnotation*> _jump_annotations;
  ConstPoolNode* _const_pools[2];
  ASMJIT_API BaseCompiler() noexcept;
  ASMJIT_API ~BaseCompiler() noexcept override;
  template<typename PassT, typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE PassT* new_pass(Args&&... args) noexcept { return _builder_arena.new_oneshot<PassT>(*this, std::forward<Args>(args)...); }
  template<typename T, typename... Args>
  ASMJIT_INLINE Error add_pass(Args&&... args) { return _add_pass(new_pass<T, Args...>(std::forward<Args>(args)...)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncNode* func() const noexcept { return _func; }
  ASMJIT_API Error new_func_node(Out<FuncNode*> out, const FuncSignature& signature);
  ASMJIT_API Error add_func_node(Out<FuncNode*> out, const FuncSignature& signature);
  ASMJIT_API Error new_func_ret_node(Out<FuncRetNode*> out, const Operand_& o0, const Operand_& o1);
  ASMJIT_API Error add_func_ret_node(Out<FuncRetNode*> out, const Operand_& o0, const Operand_& o1);
  ASMJIT_INLINE FuncNode* new_func(const FuncSignature& signature) {
    FuncNode* node;
    new_func_node(Out(node), signature);
    return node;
  }
  ASMJIT_INLINE FuncNode* add_func(const FuncSignature& signature) {
    FuncNode* node;
    add_func_node(Out(node), signature);
    return node;
  }
  ASMJIT_API FuncNode* add_func(FuncNode* ASMJIT_NONNULL(func));
  ASMJIT_API Error end_func();
  ASMJIT_INLINE Error add_ret(const Operand_& o0, const Operand_& o1) {
    FuncRetNode* node;
    return add_func_ret_node(Out(node), o0, o1);
  }
  ASMJIT_API Error new_invoke_node(Out<InvokeNode*> out, InstId inst_id, const Operand_& o0, const FuncSignature& signature);
  ASMJIT_API Error add_invoke_node(Out<InvokeNode*> out, InstId inst_id, const Operand_& o0, const FuncSignature& signature);
  ASMJIT_API Error new_virt_reg(Out<VirtReg*> out, TypeId type_id, OperandSignature signature, const char* name);
  ASMJIT_API Error _new_reg_with_name(Out<Reg> out, TypeId type_id, const char* name);
  ASMJIT_API Error _new_reg_with_name(Out<Reg> out, const Reg& ref, const char* name);
  ASMJIT_API Error _new_reg_with_vfmt(Out<Reg> out, TypeId type_id, const char* fmt, ...);
  ASMJIT_API Error _new_reg_with_vfmt(Out<Reg> out, const Reg& ref, const char* fmt, ...);
  template<typename RegT>
  ASMJIT_INLINE Error _new_reg(Out<RegT> out, TypeId type_id) {
    return _new_reg_with_name(Out<Reg>(out.value()), type_id, nullptr);
  }
  template<typename RegT, typename... Args>
  ASMJIT_INLINE Error _new_reg(Out<RegT> out, TypeId type_id, const char* name_or_fmt, Args&&... args) {
#ifndef ASMJIT_NO_LOGGING
    if constexpr (sizeof...(args) == 0u) {
      return _new_reg_with_name(Out<Reg>(out.value()), type_id, name_or_fmt);
    }
    else {
      return _new_reg_with_vfmt(Out<Reg>(out.value()), type_id, name_or_fmt, std::forward<Args>(args)...);
    }
#else
    Support::maybe_unused(name_or_fmt, std::forward<Args>(args)...);
    return _new_reg_with_name(Out<Reg>(out.value()), type_id, nullptr);
#endif
  }
  template<typename RegT>
  ASMJIT_INLINE Error _new_reg(Out<RegT> out, const Reg& ref) {
    return _new_reg_with_name(Out<Reg>(out.value()), ref, nullptr);
  }
  template<typename RegT, typename... Args>
  ASMJIT_INLINE Error _new_reg(Out<RegT> out, const Reg& ref, const char* name_or_fmt, Args&&... args) {
#ifndef ASMJIT_NO_LOGGING
    if constexpr (sizeof...(args) == 0u) {
      return _new_reg_with_name(Out<Reg>(out.value()), ref, name_or_fmt);
    }
    else {
      return _new_reg_with_vfmt(Out<Reg>(out.value()), ref, name_or_fmt, std::forward<Args>(args)...);
    }
#else
    Support::maybe_unused(name_or_fmt, std::forward<Args>(args)...);
    return _new_reg_with_name(Out<Reg>(out.value()), ref, nullptr);
#endif
  }
  template<typename RegT, typename... Args>
  ASMJIT_INLINE_NODEBUG RegT new_reg(TypeId type_id, Args&&... args) {
    RegT reg(Globals::NoInit);
    (void)_new_reg<RegT>(Out(reg), type_id, std::forward<Args>(args)...);
    return reg;
  }
  template<typename RegT, typename... Args>
  ASMJIT_INLINE_NODEBUG RegT new_similar_reg(const RegT& ref, Args&&... args) {
    RegT reg(Globals::NoInit);
    (void)_new_reg<RegT>(Out(reg), ref, std::forward<Args>(args)...);
    return reg;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_virt_id_valid(uint32_t virt_id) const noexcept {
    uint32_t index = Operand::virt_id_to_index(virt_id);
    return index < _virt_regs.size();
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_virt_reg_valid(const Reg& reg) const noexcept {
    return is_virt_id_valid(reg.id());
  }
  [[nodiscard]]
  ASMJIT_INLINE VirtReg* virt_reg_by_id(uint32_t virt_id) const noexcept {
    ASMJIT_ASSERT(is_virt_id_valid(virt_id));
    return _virt_regs[Operand::virt_id_to_index(virt_id)];
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG VirtReg* virt_reg_by_reg(const Reg& reg) const noexcept { return virt_reg_by_id(reg.id()); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG VirtReg* virt_reg_by_index(uint32_t index) const noexcept { return _virt_regs[index]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<VirtReg*> virt_regs() const noexcept { return _virt_regs.as_span(); }
  ASMJIT_API Error _new_stack(Out<BaseMem> out, uint32_t size, uint32_t alignment, const char* name = nullptr);
  ASMJIT_API Error set_stack_size(uint32_t virt_id, uint32_t new_size, uint32_t new_alignment = 0);
  ASMJIT_INLINE_NODEBUG Error set_stack_size(const BaseMem& mem, uint32_t new_size, uint32_t new_alignment = 0) {
    return set_stack_size(mem.id(), new_size, new_alignment);
  }
  ASMJIT_API Error _new_const(Out<BaseMem> out, ConstPoolScope scope, const void* data, size_t size);
  ASMJIT_API void rename(const Reg& reg, const char* fmt, ...);
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<JumpAnnotation*> jump_annotations() const noexcept { return _jump_annotations.as_span(); }
  ASMJIT_API Error new_jump_node(Out<JumpNode*> out, InstId inst_id, InstOptions inst_options, const Operand_& o0, JumpAnnotation* annotation);
  ASMJIT_API Error emit_annotated_jump(InstId inst_id, const Operand_& o0, JumpAnnotation* annotation);
  [[nodiscard]]
  ASMJIT_API JumpAnnotation* new_jump_annotation();
  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_reinit(CodeHolder& code) noexcept override;
};
class JumpAnnotation {
public:
  ASMJIT_NONCOPYABLE(JumpAnnotation)
  BaseCompiler* _compiler;
  uint32_t _annotation_id;
  ArenaVector<uint32_t> _label_ids;
  ASMJIT_INLINE_NODEBUG JumpAnnotation(BaseCompiler* ASMJIT_NONNULL(compiler), uint32_t annotation_id) noexcept
    : _compiler(compiler),
      _annotation_id(annotation_id) {}
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseCompiler* compiler() const noexcept { return _compiler; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t annotation_id() const noexcept { return _annotation_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<uint32_t> label_ids() const noexcept { return _label_ids.as_span(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_label(const Label& label) const noexcept { return has_label_id(label.id()); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_label_id(uint32_t label_id) const noexcept { return _label_ids.contains(label_id); }
  ASMJIT_INLINE_NODEBUG Error add_label(const Label& label) noexcept { return add_label_id(label.id()); }
  ASMJIT_INLINE_NODEBUG Error add_label_id(uint32_t label_id) noexcept { return _label_ids.append(_compiler->_builder_arena, label_id); }
};
class JumpNode : public InstNodeWithOperands<InstNode::kBaseOpCapacity> {
public:
  ASMJIT_NONCOPYABLE(JumpNode)
  JumpAnnotation* _annotation;
  inline JumpNode(InstId inst_id, InstOptions options, uint32_t op_count, JumpAnnotation* annotation) noexcept
    : InstNodeWithOperands(inst_id, options, op_count),
      _annotation(annotation) {
    _set_type(NodeType::kJump);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_annotation() const noexcept { return _annotation != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG JumpAnnotation* annotation() const noexcept { return _annotation; }
  ASMJIT_INLINE_NODEBUG void set_annotation(JumpAnnotation* annotation) noexcept { _annotation = annotation; }
};
class FuncNode : public LabelNode {
public:
  ASMJIT_NONCOPYABLE(FuncNode)
  struct ArgPack {
    RegOnly _data[Globals::kMaxValuePack];
    ASMJIT_INLINE void reset() noexcept {
      for (RegOnly& v : _data) {
        v.reset();
      }
    }
    ASMJIT_INLINE RegOnly& operator[](size_t value_index) noexcept { return _data[value_index]; }
    ASMJIT_INLINE const RegOnly& operator[](size_t value_index) const noexcept { return _data[value_index]; }
  };
  FuncDetail _func_detail;
  FuncFrame _frame;
  LabelNode* _exit_node;
  SentinelNode* _end;
  ArgPack* _args;
  inline explicit FuncNode(uint32_t label_id = Globals::kInvalidId) noexcept
    : LabelNode(label_id),
      _func_detail(),
      _frame(),
      _exit_node(nullptr),
      _end(nullptr),
      _args(nullptr) {
    _set_type(NodeType::kFunc);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelNode* exit_node() const noexcept { return _exit_node; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label exit_label() const noexcept { return _exit_node->label(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SentinelNode* end_node() const noexcept { return _end; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncDetail& detail() noexcept { return _func_detail; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncDetail& detail() const noexcept { return _func_detail; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncFrame& frame() noexcept { return _frame; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncFrame& frame() const noexcept { return _frame; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncAttributes attributes() const noexcept { return _frame.attributes(); }
  ASMJIT_INLINE_NODEBUG void add_attributes(FuncAttributes attrs) noexcept { _frame.add_attributes(attrs); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t arg_count() const noexcept { return _func_detail.arg_count(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ArgPack* arg_packs() const noexcept { return _args; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_ret() const noexcept { return _func_detail.has_ret(); }
  [[nodiscard]]
  inline ArgPack& arg_pack(size_t arg_index) const noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    return _args[arg_index];
  }
  inline void set_arg(size_t arg_index, const Reg& virt_reg) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][0].init(virt_reg);
  }
  inline void set_arg(size_t arg_index, const RegOnly& virt_reg) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][0].init(virt_reg);
  }
  inline void set_arg(size_t arg_index, size_t value_index, const Reg& virt_reg) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][value_index].init(virt_reg);
  }
  inline void set_arg(size_t arg_index, size_t value_index, const RegOnly& virt_reg) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][value_index].init(virt_reg);
  }
  inline void reset_arg(size_t arg_index) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index].reset();
  }
  inline void reset_arg(size_t arg_index, size_t value_index) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][value_index].reset();
  }
};
class FuncRetNode : public InstNodeWithOperands<InstNode::kBaseOpCapacity> {
public:
  ASMJIT_NONCOPYABLE(FuncRetNode)
  inline FuncRetNode() noexcept
    : InstNodeWithOperands(BaseInst::kIdAbstract, InstOptions::kNone, 0) {
    _node_type = NodeType::kFuncRet;
  }
};
class InvokeNode : public InstNodeWithOperands<InstNode::kBaseOpCapacity> {
public:
  ASMJIT_NONCOPYABLE(InvokeNode)
  struct OperandPack {
    Operand_ _data[Globals::kMaxValuePack];
    ASMJIT_INLINE void reset() noexcept {
      for (Operand_& op : _data) {
        op.reset();
      }
    }
    [[nodiscard]]
    ASMJIT_INLINE Operand& operator[](size_t value_index) noexcept {
      ASMJIT_ASSERT(value_index < Globals::kMaxValuePack);
      return _data[value_index].as<Operand>();
    }
    [[nodiscard]]
    ASMJIT_INLINE const Operand& operator[](size_t value_index) const noexcept {
      ASMJIT_ASSERT(value_index < Globals::kMaxValuePack);
      return _data[value_index].as<Operand>();
    }
  };
  FuncDetail _func_detail;
  OperandPack _rets;
  OperandPack* _args;
  inline InvokeNode(InstId inst_id, InstOptions options) noexcept
    : InstNodeWithOperands(inst_id, options, 0),
      _func_detail(),
      _args(nullptr) {
    _set_type(NodeType::kInvoke);
    _reset_ops();
    _rets.reset();
    _add_flags(NodeFlags::kIsRemovable);
  }
  [[nodiscard]]
  inline Error init(const FuncSignature& signature, const Environment& environment) noexcept {
    return _func_detail.init(signature, environment);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncDetail& detail() noexcept { return _func_detail; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncDetail& detail() const noexcept { return _func_detail; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Operand& target() noexcept { return op(0); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Operand& target() const noexcept { return op(0); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_ret() const noexcept { return _func_detail.has_ret(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t arg_count() const noexcept { return _func_detail.arg_count(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OperandPack& ret_pack() noexcept { return _rets; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const OperandPack& ret_pack() const noexcept { return _rets; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Operand& ret(size_t value_index = 0) noexcept { return _rets[value_index]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Operand& ret(size_t value_index = 0) const noexcept { return _rets[value_index]; }
  [[nodiscard]]
  inline OperandPack& arg_pack(size_t arg_index) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    return _args[arg_index];
  }
  [[nodiscard]]
  inline const OperandPack& arg_pack(size_t arg_index) const noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    return _args[arg_index];
  }
  [[nodiscard]]
  inline Operand& arg(size_t arg_index, size_t value_index) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    return _args[arg_index][value_index];
  }
  [[nodiscard]]
  inline const Operand& arg(size_t arg_index, size_t value_index) const noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    return _args[arg_index][value_index];
  }
  inline void _set_ret(size_t value_index, const Operand_& op) noexcept { _rets[value_index] = op; }
  inline void _set_arg(size_t arg_index, size_t value_index, const Operand_& op) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][value_index] = op;
  }
  ASMJIT_INLINE_NODEBUG void set_ret(size_t value_index, const Reg& reg) noexcept { _set_ret(value_index, reg); }
  ASMJIT_INLINE_NODEBUG void set_arg(size_t arg_index, const Reg& reg) noexcept { _set_arg(arg_index, 0, reg); }
  ASMJIT_INLINE_NODEBUG void set_arg(size_t arg_index, const Imm& imm) noexcept { _set_arg(arg_index, 0, imm); }
  ASMJIT_INLINE_NODEBUG void set_arg(size_t arg_index, size_t value_index, const Reg& reg) noexcept { _set_arg(arg_index, value_index, reg); }
  ASMJIT_INLINE_NODEBUG void set_arg(size_t arg_index, size_t value_index, const Imm& imm) noexcept { _set_arg(arg_index, value_index, imm); }
};
class ASMJIT_VIRTAPI FuncPass : public Pass {
public:
  ASMJIT_NONCOPYABLE(FuncPass)
  using Base = Pass;
  ASMJIT_API FuncPass(BaseCompiler& cc, const char* name) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseCompiler& cc() const noexcept { return static_cast<BaseCompiler&>(_cb); }
  ASMJIT_API Error run(Arena& arena, Logger* logger) override;
  ASMJIT_API virtual Error run_on_function(Arena& arena, Logger* logger, FuncNode* func);
};
ASMJIT_END_NAMESPACE
#endif
#endif
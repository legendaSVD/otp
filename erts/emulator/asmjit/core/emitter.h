#ifndef ASMJIT_CORE_EMITTER_H_INCLUDED
#define ASMJIT_CORE_EMITTER_H_INCLUDED
#include <asmjit/core/archtraits.h>
#include <asmjit/core/codeholder.h>
#include <asmjit/core/formatter.h>
#include <asmjit/core/inst.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
ASMJIT_BEGIN_NAMESPACE
class ConstPool;
class FuncFrame;
class FuncArgsAssignment;
enum class AlignMode : uint8_t {
  kCode = 0,
  kData = 1,
  kZero = 2,
  kMaxValue = kZero
};
enum class EmitterType : uint8_t {
  kNone = 0,
  kAssembler = 1,
  kBuilder = 2,
  kCompiler = 3,
  kMaxValue = kCompiler
};
enum class EmitterFlags : uint8_t {
  kNone = 0u,
  kAttached = 0x01u,
  kLogComments = 0x08u,
  kOwnLogger = 0x10u,
  kOwnErrorHandler = 0x20u,
  kFinalized = 0x40u,
  kDestroyed = 0x80u
};
ASMJIT_DEFINE_ENUM_FLAGS(EmitterFlags)
enum class EncodingOptions : uint32_t {
  kNone = 0,
  kOptimizeForSize = 0x00000001u,
  kOptimizedAlign = 0x00000002u,
  kPredictedJumps = 0x00000010u
};
ASMJIT_DEFINE_ENUM_FLAGS(EncodingOptions)
enum class DiagnosticOptions : uint32_t {
  kNone = 0,
  kValidateAssembler = 0x00000001u,
  kValidateIntermediate = 0x00000002u,
  kRAAnnotate = 0x00000080u,
  kRADebugCFG = 0x00000100u,
  kRADebugLiveness = 0x00000200u,
  kRADebugAssignment = 0x00000400u,
  kRADebugUnreachable = 0x00000800u,
  kRADebugAll = 0x0000FF00u
};
ASMJIT_DEFINE_ENUM_FLAGS(DiagnosticOptions)
class ASMJIT_VIRTAPI BaseEmitter {
public:
  ASMJIT_BASE_CLASS(BaseEmitter)
  ASMJIT_NONCOPYABLE(BaseEmitter)
  struct State {
    InstOptions options;
    RegOnly extra_reg;
    const char* comment;
  };
  struct Funcs {
    using EmitProlog = Error (ASMJIT_CDECL*)(BaseEmitter* emitter, const FuncFrame& frame);
    using EmitEpilog = Error (ASMJIT_CDECL*)(BaseEmitter* emitter, const FuncFrame& frame);
    using EmitArgsAssignment = Error (ASMJIT_CDECL*)(BaseEmitter* emitter, const FuncFrame& frame, const FuncArgsAssignment& args);
    using FormatInstruction = Error (ASMJIT_CDECL*)(
      String& sb,
      FormatFlags format_flags,
      const BaseEmitter* emitter,
      Arch arch,
      const BaseInst& inst, Span<const Operand_> operands) noexcept;
    using ValidateFunc = Error (ASMJIT_CDECL*)(const BaseInst& inst, const Operand_* operands, size_t op_count, ValidationFlags validation_flags) noexcept;
    EmitProlog emit_prolog;
    EmitEpilog emit_epilog;
    EmitArgsAssignment emit_args_assignment;
    FormatInstruction format_instruction;
    ValidateFunc validate;
    ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = Funcs{}; }
  };
  EmitterType _emitter_type = EmitterType::kNone;
  EmitterFlags _emitter_flags = EmitterFlags::kNone;
  uint8_t _instruction_alignment = 0u;
  ValidationFlags _validation_flags = ValidationFlags::kNone;
  DiagnosticOptions _diagnostic_options = DiagnosticOptions::kNone;
  EncodingOptions _encoding_options = EncodingOptions::kNone;
  InstOptions _forced_inst_options = InstOptions::kReserved;
  uint64_t _arch_mask = 0;
  CodeHolder* _code = nullptr;
  Logger* _logger = nullptr;
  ErrorHandler* _error_handler = nullptr;
  Environment _environment {};
  OperandSignature _gp_signature {};
  uint32_t _private_data = 0;
  InstOptions _inst_options = InstOptions::kNone;
  RegOnly _extra_reg {};
  const char* _inline_comment = nullptr;
  Funcs _funcs {};
  BaseEmitter* _attached_prev = nullptr;
  BaseEmitter* _attached_next = nullptr;
  ASMJIT_API explicit BaseEmitter(EmitterType emitter_type) noexcept;
  ASMJIT_API virtual ~BaseEmitter() noexcept;
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* as() noexcept { return reinterpret_cast<T*>(this); }
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const T* as() const noexcept { return reinterpret_cast<const T*>(this); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG EmitterType emitter_type() const noexcept { return _emitter_type; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG EmitterFlags emitter_flags() const noexcept { return _emitter_flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_assembler() const noexcept { return _emitter_type == EmitterType::kAssembler; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_builder() const noexcept { return uint32_t(_emitter_type) >= uint32_t(EmitterType::kBuilder); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_compiler() const noexcept { return _emitter_type == EmitterType::kCompiler; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_emitter_flag(EmitterFlags flag) const noexcept { return Support::test(_emitter_flags, flag); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_finalized() const noexcept { return has_emitter_flag(EmitterFlags::kFinalized); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_destroyed() const noexcept { return has_emitter_flag(EmitterFlags::kDestroyed); }
  ASMJIT_INLINE_NODEBUG void _add_emitter_flags(EmitterFlags flags) noexcept { _emitter_flags |= flags; }
  ASMJIT_INLINE_NODEBUG void _clear_emitter_flags(EmitterFlags flags) noexcept { _emitter_flags &= _emitter_flags & ~flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CodeHolder* code() const noexcept { return _code; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Environment& environment() const noexcept { return _environment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_32bit() const noexcept { return environment().is_32bit(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_64bit() const noexcept { return environment().is_64bit(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return environment().arch(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SubArch sub_arch() const noexcept { return environment().sub_arch(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t register_size() const noexcept { return environment().register_size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OperandSignature gp_signature() const noexcept { return _gp_signature; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t instruction_alignment() const noexcept { return _instruction_alignment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_initialized() const noexcept { return _code != nullptr; }
  ASMJIT_API virtual Error finalize();
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_logger() const noexcept { return _logger != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_own_logger() const noexcept { return has_emitter_flag(EmitterFlags::kOwnLogger); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Logger* logger() const noexcept { return _logger; }
  ASMJIT_API void set_logger(Logger* logger) noexcept;
  ASMJIT_INLINE_NODEBUG void reset_logger() noexcept { return set_logger(nullptr); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_error_handler() const noexcept { return _error_handler != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_own_error_handler() const noexcept { return has_emitter_flag(EmitterFlags::kOwnErrorHandler); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ErrorHandler* error_handler() const noexcept { return _error_handler; }
  ASMJIT_API void set_error_handler(ErrorHandler* error_handler) noexcept;
  ASMJIT_INLINE_NODEBUG void reset_error_handler() noexcept { set_error_handler(nullptr); }
  ASMJIT_API Error _report_error(Error err, const char* message = nullptr);
  ASMJIT_INLINE Error report_error(Error err, const char* message = nullptr) {
    Error e = _report_error(err, message);
    ASMJIT_ASSUME(e == err);
    ASMJIT_ASSUME(e != Error::kOk);
    return e;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG EncodingOptions encoding_options() const noexcept { return _encoding_options; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_encoding_option(EncodingOptions option) const noexcept { return Support::test(_encoding_options, option); }
  ASMJIT_INLINE_NODEBUG void add_encoding_options(EncodingOptions options) noexcept { _encoding_options |= options; }
  ASMJIT_INLINE_NODEBUG void clear_encoding_options(EncodingOptions options) noexcept { _encoding_options &= ~options; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG DiagnosticOptions diagnostic_options() const noexcept { return _diagnostic_options; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_diagnostic_option(DiagnosticOptions option) const noexcept { return Support::test(_diagnostic_options, option); }
  ASMJIT_API void add_diagnostic_options(DiagnosticOptions options) noexcept;
  ASMJIT_API void clear_diagnostic_options(DiagnosticOptions options) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstOptions forced_inst_options() const noexcept { return _forced_inst_options; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstOptions inst_options() const noexcept { return _inst_options; }
  ASMJIT_INLINE_NODEBUG void set_inst_options(InstOptions options) noexcept { _inst_options = options; }
  ASMJIT_INLINE_NODEBUG void add_inst_options(InstOptions options) noexcept { _inst_options |= options; }
  ASMJIT_INLINE_NODEBUG void reset_inst_options() noexcept { _inst_options = InstOptions::kNone; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_extra_reg() const noexcept { return _extra_reg.is_reg(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegOnly& extra_reg() const noexcept { return _extra_reg; }
  ASMJIT_INLINE_NODEBUG void set_extra_reg(const Reg& reg) noexcept { _extra_reg.init(reg); }
  ASMJIT_INLINE_NODEBUG void set_extra_reg(const RegOnly& reg) noexcept { _extra_reg.init(reg); }
  ASMJIT_INLINE_NODEBUG void reset_extra_reg() noexcept { _extra_reg.reset(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* inline_comment() const noexcept { return _inline_comment; }
  ASMJIT_INLINE_NODEBUG void set_inline_comment(const char* s) noexcept { _inline_comment = s; }
  ASMJIT_INLINE_NODEBUG void reset_inline_comment() noexcept { _inline_comment = nullptr; }
  ASMJIT_INLINE_NODEBUG void reset_state() noexcept {
    reset_inst_options();
    reset_extra_reg();
    reset_inline_comment();
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG State _grab_state() noexcept {
    State s{_inst_options | _forced_inst_options, _extra_reg, _inline_comment};
    reset_state();
    return s;
  }
  ASMJIT_API virtual Error section(Section* section);
  [[nodiscard]]
  ASMJIT_API virtual Label new_label();
  [[nodiscard]]
  ASMJIT_API virtual Label new_named_label(const char* name, size_t name_size = SIZE_MAX, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId);
  [[nodiscard]]
  ASMJIT_INLINE Label new_named_label(Span<const char> name, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId) {
    return new_named_label(name.data(), name.size(), type, parent_id);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label new_anonymous_label(const char* name, size_t name_size = SIZE_MAX) {
    return new_named_label(name, name_size, LabelType::kAnonymous);
  }
  [[nodiscard]]
  ASMJIT_INLINE Label new_anonymous_label(Span<const char> name) { return new_anonymous_label(name.data(), name.size()); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label new_external_label(const char* name, size_t name_size = SIZE_MAX) {
    return new_named_label(name, name_size, LabelType::kExternal);
  }
  [[nodiscard]]
  ASMJIT_INLINE Label new_external_label(Span<const char> name) { return new_external_label(name.data(), name.size()); }
  [[nodiscard]]
  ASMJIT_API Label label_by_name(const char* name, size_t name_size = SIZE_MAX, uint32_t parent_id = Globals::kInvalidId) noexcept;
  [[nodiscard]]
  ASMJIT_API Label label_by_name(Span<const char> name, uint32_t parent_id = Globals::kInvalidId) noexcept {
    return label_by_name(name.data(), name.size(), parent_id);
  }
  ASMJIT_API virtual Error bind(const Label& label);
  [[nodiscard]]
  ASMJIT_API bool is_label_valid(uint32_t label_id) const noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label_valid(const Label& label) const noexcept { return is_label_valid(label.id()); }
  ASMJIT_API Error _emitI(InstId inst_id);
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0);
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0, const Operand_& o1);
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2);
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3);
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4);
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4, const Operand_& o5);
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Error emit(InstId inst_id, Args&&... operands) {
    return _emitI(inst_id, Support::ForwardOp<Args>::forward(operands)...);
  }
  ASMJIT_INLINE_NODEBUG Error emit_op_array(InstId inst_id, const Operand_* operands, size_t op_count) {
    return _emit_op_array(inst_id, operands, op_count);
  }
  ASMJIT_INLINE Error emit_inst(const BaseInst& inst, const Operand_* operands, size_t op_count) {
    set_inst_options(inst.options());
    set_extra_reg(inst.extra_reg());
    return _emit_op_array(inst.inst_id(), operands, op_count);
  }
  ASMJIT_API virtual Error _emit(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext);
  ASMJIT_API virtual Error _emit_op_array(InstId inst_id, const Operand_* operands, size_t op_count);
  ASMJIT_API Error emit_prolog(const FuncFrame& frame);
  ASMJIT_API Error emit_epilog(const FuncFrame& frame);
  ASMJIT_API Error emit_args_assignment(const FuncFrame& frame, const FuncArgsAssignment& args);
  ASMJIT_API virtual Error align(AlignMode align_mode, uint32_t alignment);
  ASMJIT_API virtual Error embed(const void* data, size_t data_size);
  ASMJIT_API virtual Error embed_data_array(TypeId type_id, const void* data, size_t item_count, size_t repeat_count = 1);
  ASMJIT_INLINE_NODEBUG Error embed_int8(int8_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kInt8, &value, 1, repeat_count); }
  ASMJIT_INLINE_NODEBUG Error embed_uint8(uint8_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kUInt8, &value, 1, repeat_count); }
  ASMJIT_INLINE_NODEBUG Error embed_int16(int16_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kInt16, &value, 1, repeat_count); }
  ASMJIT_INLINE_NODEBUG Error embed_uint16(uint16_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kUInt16, &value, 1, repeat_count); }
  ASMJIT_INLINE_NODEBUG Error embed_int32(int32_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kInt32, &value, 1, repeat_count); }
  ASMJIT_INLINE_NODEBUG Error embed_uint32(uint32_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kUInt32, &value, 1, repeat_count); }
  ASMJIT_INLINE_NODEBUG Error embed_int64(int64_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kInt64, &value, 1, repeat_count); }
  ASMJIT_INLINE_NODEBUG Error embed_uint64(uint64_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kUInt64, &value, 1, repeat_count); }
  ASMJIT_INLINE_NODEBUG Error embed_float(float value, size_t repeat_count = 1) { return embed_data_array(TypeId(TypeUtils::TypeIdOfT<float>::kTypeId), &value, 1, repeat_count); }
  ASMJIT_INLINE_NODEBUG Error embed_double(double value, size_t repeat_count = 1) { return embed_data_array(TypeId(TypeUtils::TypeIdOfT<double>::kTypeId), &value, 1, repeat_count); }
  ASMJIT_API virtual Error embed_const_pool(const Label& label, const ConstPool& pool);
  ASMJIT_API virtual Error embed_label(const Label& label, size_t data_size = 0);
  ASMJIT_API virtual Error embed_label_delta(const Label& label, const Label& base, size_t data_size = 0);
  ASMJIT_API virtual Error comment(const char* data, size_t size = SIZE_MAX);
  ASMJIT_INLINE Error comment(Span<const char> data) {
    return comment(data.data(), data.size());
  }
  ASMJIT_API Error commentf(const char* fmt, ...);
  ASMJIT_API Error commentv(const char* fmt, va_list ap);
  ASMJIT_API virtual Error on_attach(CodeHolder& code) noexcept;
  ASMJIT_API virtual Error on_detach(CodeHolder& code) noexcept;
  ASMJIT_API virtual Error on_reinit(CodeHolder& code) noexcept;
  ASMJIT_API virtual void on_settings_updated() noexcept;
};
ASMJIT_END_NAMESPACE
#endif
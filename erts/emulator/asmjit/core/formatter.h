#ifndef ASMJIT_CORE_FORMATTER_H_INCLUDED
#define ASMJIT_CORE_FORMATTER_H_INCLUDED
#include <asmjit/core/globals.h>
#include <asmjit/core/inst.h>
#include <asmjit/core/string.h>
#include <asmjit/support/span.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
class BaseBuilder;
class BaseEmitter;
class BaseNode;
struct Operand_;
enum class FormatFlags : uint32_t {
  kNone = 0u,
  kMachineCode = 0x00000001u,
  kShowAliases = 0x00000008u,
  kExplainImms = 0x00000010u,
  kHexImms = 0x00000020u,
  kHexOffsets = 0x00000040u,
  kRegCasts = 0x00000100u,
  kPositions = 0x00000200u,
  kRegType = 0x00000400u
};
ASMJIT_DEFINE_ENUM_FLAGS(FormatFlags)
enum class FormatIndentationGroup : uint32_t {
  kCode = 0u,
  kLabel = 1u,
  kComment = 2u,
  kReserved = 3u,
  kMaxValue = kReserved
};
enum class FormatPaddingGroup : uint32_t {
  kRegularLine = 0,
  kMachineCode = 1,
  kMaxValue = kMachineCode
};
class FormatOptions {
public:
  FormatFlags _flags = FormatFlags::kNone;
  Support::Array<uint8_t, uint32_t(FormatIndentationGroup::kMaxValue) + 1> _indentation {};
  Support::Array<uint16_t, uint32_t(FormatPaddingGroup::kMaxValue) + 1> _padding {};
  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _flags = FormatFlags::kNone;
    _indentation.fill(uint8_t(0));
    _padding.fill(uint16_t(0));
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FormatFlags flags() const noexcept { return _flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(FormatFlags flag) const noexcept { return Support::test(_flags, flag); }
  ASMJIT_INLINE_NODEBUG void set_flags(FormatFlags flags) noexcept { _flags = flags; }
  ASMJIT_INLINE_NODEBUG void add_flags(FormatFlags flags) noexcept { _flags |= flags; }
  ASMJIT_INLINE_NODEBUG void clear_flags(FormatFlags flags) noexcept { _flags &= ~flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t indentation(FormatIndentationGroup group) const noexcept { return _indentation[group]; }
  ASMJIT_INLINE_NODEBUG void set_indentation(FormatIndentationGroup group, uint32_t n) noexcept { _indentation[group] = uint8_t(n); }
  ASMJIT_INLINE_NODEBUG void reset_indentation(FormatIndentationGroup group) noexcept { _indentation[group] = uint8_t(0); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t padding(FormatPaddingGroup group) const noexcept { return _padding[group]; }
  ASMJIT_INLINE_NODEBUG void set_padding(FormatPaddingGroup group, size_t n) noexcept { _padding[group] = uint16_t(n); }
  ASMJIT_INLINE_NODEBUG void reset_padding(FormatPaddingGroup group) noexcept { _padding[group] = uint16_t(0); }
};
namespace Formatter {
#ifndef ASMJIT_NO_LOGGING
ASMJIT_API Error format_type_id(
  String& sb,
  TypeId type_id) noexcept;
ASMJIT_API Error format_feature(
  String& sb,
  Arch arch,
  uint32_t feature_id) noexcept;
ASMJIT_API Error format_register(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  RegType reg_type,
  uint32_t reg_id) noexcept;
ASMJIT_API Error format_label(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  uint32_t label_id) noexcept;
ASMJIT_API Error format_operand(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  const Operand_& op) noexcept;
ASMJIT_API Error format_data_type(
  String& sb,
  FormatFlags format_flags,
  Arch arch,
  TypeId type_id) noexcept;
ASMJIT_API Error format_data(
  String& sb,
  FormatFlags format_flags,
  Arch arch,
  TypeId type_id, const void* data, size_t item_count, size_t repeat_count = 1) noexcept;
ASMJIT_API Error format_instruction(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  const BaseInst& inst, Span<const Operand_> operands) noexcept;
#ifndef ASMJIT_NO_BUILDER
ASMJIT_API Error format_node(
  String& sb,
  const FormatOptions& format_options,
  const BaseBuilder* builder,
  const BaseNode* node) noexcept;
ASMJIT_API Error format_node_list(
  String& sb,
  const FormatOptions& format_options,
  const BaseBuilder* builder) noexcept;
ASMJIT_API Error format_node_list(
  String& sb,
  const FormatOptions& format_options,
  const BaseBuilder* builder,
  const BaseNode* begin,
  const BaseNode* end) noexcept;
#endif
#endif
}
ASMJIT_END_NAMESPACE
#endif
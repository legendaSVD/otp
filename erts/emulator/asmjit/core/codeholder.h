#ifndef ASMJIT_CORE_CODEHOLDER_H_INCLUDED
#define ASMJIT_CORE_CODEHOLDER_H_INCLUDED
#include <asmjit/core/archtraits.h>
#include <asmjit/core/codebuffer.h>
#include <asmjit/core/errorhandler.h>
#include <asmjit/core/fixup.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/string.h>
#include <asmjit/core/target.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenahash.h>
#include <asmjit/support/arenapool.h>
#include <asmjit/support/arenastring.h>
#include <asmjit/support/arenatree.h>
#include <asmjit/support/arenavector.h>
#include <asmjit/support/span.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
class BaseEmitter;
class CodeHolder;
class LabelEntry;
class Logger;
enum class ExpressionOpType : uint8_t {
  kAdd = 0,
  kSub = 1,
  kMul = 2,
  kSll = 3,
  kSrl = 4,
  kSra = 5
};
enum class ExpressionValueType : uint8_t {
  kNone = 0,
  kConstant = 1,
  kLabel = 2,
  kExpression = 3
};
struct Expression {
  union Value {
    uint64_t constant;
    Expression* expression;
    uint32_t label_id;
  };
  ExpressionOpType op_type;
  ExpressionValueType value_type[2];
  uint8_t reserved[5];
  Value value[2];
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = Expression{}; }
  ASMJIT_INLINE_NODEBUG void set_value_as_constant(size_t index, uint64_t constant) noexcept {
    value_type[index] = ExpressionValueType::kConstant;
    value[index].constant = constant;
  }
  ASMJIT_INLINE_NODEBUG void set_value_as_label_id(size_t index, uint32_t label_id) noexcept {
    value_type[index] = ExpressionValueType::kLabel;
    value[index].label_id = label_id;
  }
  ASMJIT_INLINE_NODEBUG void set_value_as_expression(size_t index, Expression* expression) noexcept {
    value_type[index] = ExpressionValueType::kExpression;
    value[index].expression = expression;
  }
};
enum class RelocType : uint32_t {
  kNone = 0,
  kExpression = 1,
  kSectionRelative = 2,
  kAbsToAbs = 3,
  kRelToAbs = 4,
  kAbsToRel = 5,
  kX64AddressEntry = 6
};
enum class LabelType : uint8_t {
  kAnonymous = 0u,
  kLocal = 1u,
  kGlobal = 2u,
  kExternal = 3u,
  kMaxValue = kExternal
};
enum class LabelFlags : uint8_t {
  kNone = 0x00u,
  kHasOwnExtraData = 0x01u,
  kHasName = 0x02u,
  kHasParent = 0x04u
};
ASMJIT_DEFINE_ENUM_FLAGS(LabelFlags)
enum class SectionFlags : uint32_t {
  kNone = 0,
  kExecutable = 0x0001u,
  kReadOnly = 0x0002u,
  kZeroInitialized = 0x0004u,
  kComment = 0x0008u,
  kBuiltIn = 0x4000u,
  kImplicit = 0x8000u
};
ASMJIT_DEFINE_ENUM_FLAGS(SectionFlags)
enum class CopySectionFlags : uint32_t {
  kNone = 0,
  kPadSectionBuffer = 0x00000001u,
  kPadTargetBuffer = 0x00000002u
};
ASMJIT_DEFINE_ENUM_FLAGS(CopySectionFlags)
class SectionOrLabelEntryExtraHeader {
public:
  uint32_t _section_id;
  LabelType _internal_label_type;
  LabelFlags _internal_label_flags;
  uint16_t _internal_uint16_data;
};
class Section : public SectionOrLabelEntryExtraHeader {
public:
  uint32_t _alignment;
  int32_t _order;
  uint64_t _offset;
  uint64_t _virtual_size;
  FixedString<Globals::kMaxSectionNameSize + 1> _name;
  CodeBuffer _buffer;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t section_id() const noexcept { return _section_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return _name.str; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* data() noexcept { return _buffer.data(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint8_t* data() const noexcept { return _buffer.data(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SectionFlags flags() const noexcept { return SectionFlags(_internal_uint16_data); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(SectionFlags flag) const noexcept { return Support::test(_internal_uint16_data, uint32_t(flag)); }
  ASMJIT_INLINE_NODEBUG void assign_flags(SectionFlags flags) noexcept { _internal_uint16_data = uint16_t(flags); }
  ASMJIT_INLINE_NODEBUG void add_flags(SectionFlags flags) noexcept { _internal_uint16_data = uint16_t(_internal_uint16_data | uint32_t(flags)); }
  ASMJIT_INLINE_NODEBUG void clear_flags(SectionFlags flags) noexcept { _internal_uint16_data = uint16_t(_internal_uint16_data | ~uint32_t(flags)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t alignment() const noexcept { return _alignment; }
  ASMJIT_INLINE_NODEBUG void set_alignment(uint32_t alignment) noexcept { _alignment = alignment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG int32_t order() const noexcept { return _order; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t offset() const noexcept { return _offset; }
  ASMJIT_INLINE_NODEBUG void set_offset(uint64_t offset) noexcept { _offset = offset; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t virtual_size() const noexcept { return _virtual_size; }
  ASMJIT_INLINE_NODEBUG void set_virtual_size(uint64_t virtual_size) noexcept { _virtual_size = virtual_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t buffer_size() const noexcept { return _buffer.size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t real_size() const noexcept { return Support::max<uint64_t>(virtual_size(), buffer_size()); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CodeBuffer& buffer() noexcept { return _buffer; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CodeBuffer& buffer() const noexcept { return _buffer; }
};
class AddressTableEntry : public ArenaTreeNodeT<AddressTableEntry> {
public:
  ASMJIT_NONCOPYABLE(AddressTableEntry)
  uint64_t _address;
  uint32_t _slot;
  ASMJIT_INLINE_NODEBUG explicit AddressTableEntry(uint64_t address) noexcept
    : _address(address),
      _slot(0xFFFFFFFFu) {}
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t address() const noexcept { return _address; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t slot() const noexcept { return _slot; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_assigned_slot() const noexcept { return _slot != 0xFFFFFFFFu; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator<(const AddressTableEntry& other) const noexcept { return _address < other._address; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator>(const AddressTableEntry& other) const noexcept { return _address > other._address; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator<(uint64_t query_address) const noexcept { return _address < query_address; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator>(uint64_t query_address) const noexcept { return _address > query_address; }
};
struct RelocEntry {
  uint32_t _id;
  RelocType _reloc_type;
  OffsetFormat _format;
  uint32_t _source_section_id;
  uint32_t _target_section_id;
  uint64_t _source_offset;
  uint64_t _payload;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t id() const noexcept { return _id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RelocType reloc_type() const noexcept { return _reloc_type; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const OffsetFormat& format() const noexcept { return _format; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t source_section_id() const noexcept { return _source_section_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t target_section_id() const noexcept { return _target_section_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t source_offset() const noexcept { return _source_offset; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t payload() const noexcept { return _payload; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Expression* payload_as_expression() const noexcept {
    return reinterpret_cast<Expression*>(uintptr_t(_payload));
  }
};
class LabelEntry {
public:
  struct ExtraData : public SectionOrLabelEntryExtraHeader {
    uint32_t _parent_id;
    uint32_t _name_size;
    ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return Support::offset_ptr<char>(this, sizeof(ExtraData)); }
  };
  SectionOrLabelEntryExtraHeader* _object_data;
  uint64_t _offset_or_fixups;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelType label_type() const noexcept { return _object_data->_internal_label_type; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelFlags label_flags() const noexcept { return _object_data->_internal_label_flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_label_flag(LabelFlags flag) const noexcept { return Support::test(_object_data->_internal_label_flags, flag); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool _has_own_extra_data() const noexcept { return has_label_flag(LabelFlags::kHasOwnExtraData); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_name() const noexcept { return has_label_flag(LabelFlags::kHasName); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_parent() const noexcept { return has_label_flag(LabelFlags::kHasParent); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_bound() const noexcept { return _object_data->_section_id != Globals::kInvalidId; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_bound_to(const Section* section) const noexcept { return _object_data->_section_id == section->section_id(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_bound_to(uint32_t section_id) const noexcept { return _object_data->_section_id == section_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t section_id() const noexcept { return _object_data->_section_id; }
  [[nodiscard]]
  ASMJIT_INLINE ExtraData* _own_extra_data() const noexcept {
    ASMJIT_ASSERT(_has_own_extra_data());
    return static_cast<ExtraData*>(_object_data);
  }
  [[nodiscard]]
  ASMJIT_INLINE uint32_t parent_id() const noexcept {
    return _has_own_extra_data() ? _own_extra_data()->_parent_id : Globals::kInvalidId;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept {
    return has_name() ? _own_extra_data()->name() : nullptr;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t name_size() const noexcept {
    return has_name() ? _own_extra_data()->_name_size : uint32_t(0);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_fixups() const noexcept {
    return Support::bool_and(!is_bound(), _offset_or_fixups != 0u);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Fixup* _get_fixups() const noexcept { return reinterpret_cast<Fixup*>(uintptr_t(_offset_or_fixups)); }
  ASMJIT_INLINE_NODEBUG void _set_fixups(Fixup* first) noexcept { _offset_or_fixups = reinterpret_cast<uintptr_t>(first); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Fixup* unresolved_fixups() const noexcept { return !is_bound() ? _get_fixups() : nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE uint64_t offset() const noexcept {
    ASMJIT_ASSERT(is_bound());
    return _offset_or_fixups;
  }
};
class CodeHolder {
public:
  ASMJIT_NONCOPYABLE(CodeHolder)
  struct NamedLabelExtraData : public ArenaHashNode {
    LabelEntry::ExtraData extra_data;
    ASMJIT_INLINE_NODEBUG uint32_t label_id() const noexcept { return _custom_data; }
  };
  struct RelocationSummary {
    size_t code_size_reduction;
  };
  Environment _environment;
  CpuFeatures _cpu_features;
  uint64_t _base_address;
  Logger* _logger;
  ErrorHandler* _error_handler;
  Arena _arena;
  BaseEmitter* _attached_first;
  BaseEmitter* _attached_last;
  ArenaVector<Section*> _sections;
  ArenaVector<Section*> _sections_by_order;
  ArenaVector<LabelEntry> _label_entries;
  ArenaVector<RelocEntry*> _relocations;
  ArenaHash<NamedLabelExtraData> _named_labels;
  Fixup* _fixups;
  ArenaPool<Fixup> _fixup_data_pool;
  size_t _unresolved_fixup_count;
  Section _text_section;
  Section* _address_table_section;
  ArenaTree<AddressTableEntry> _address_table_entries;
  ASMJIT_API explicit CodeHolder(Span<uint8_t> static_arena_memory = Span<uint8_t>{}) noexcept;
  ASMJIT_API ~CodeHolder() noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_initialized() const noexcept { return _environment.is_initialized(); }
  ASMJIT_API Error init(const Environment& environment, uint64_t base_address = Globals::kNoBaseAddress) noexcept;
  ASMJIT_API Error init(const Environment& environment, const CpuFeatures& cpu_features, uint64_t base_address = Globals::kNoBaseAddress) noexcept;
  ASMJIT_API Error reinit() noexcept;
  ASMJIT_API void reset(ResetPolicy reset_policy = ResetPolicy::kSoft) noexcept;
  ASMJIT_API Error attach(BaseEmitter* emitter) noexcept;
  ASMJIT_API Error detach(BaseEmitter* emitter) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arena& arena() const noexcept { return const_cast<Arena&>(_arena); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Environment& environment() const noexcept { return _environment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return environment().arch(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SubArch sub_arch() const noexcept { return environment().sub_arch(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CpuFeatures& cpu_features() const noexcept { return _cpu_features; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_base_address() const noexcept { return _base_address != Globals::kNoBaseAddress; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t base_address() const noexcept { return _base_address; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseEmitter* attached_first() noexcept { return _attached_first; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseEmitter* attached_last() noexcept { return _attached_last; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const BaseEmitter* attached_first() const noexcept { return _attached_first; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const BaseEmitter* attached_last() const noexcept { return _attached_last; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Logger* logger() const noexcept { return _logger; }
  ASMJIT_API void set_logger(Logger* logger) noexcept;
  ASMJIT_INLINE_NODEBUG void reset_logger() noexcept { set_logger(nullptr); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_error_handler() const noexcept { return _error_handler != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ErrorHandler* error_handler() const noexcept { return _error_handler; }
  ASMJIT_API void set_error_handler(ErrorHandler* error_handler) noexcept;
  ASMJIT_INLINE_NODEBUG void reset_error_handler() noexcept { set_error_handler(nullptr); }
  ASMJIT_API Error grow_buffer(CodeBuffer* cb, size_t n) noexcept;
  ASMJIT_API Error reserve_buffer(CodeBuffer* cb, size_t n) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<Section*> sections() const noexcept { return _sections.as_span(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<Section*> sections_by_order() const noexcept { return _sections_by_order.as_span(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t section_count() const noexcept { return _sections.size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_section_valid(uint32_t section_id) const noexcept { return section_id < _sections.size(); }
  ASMJIT_API Error new_section(Out<Section*> section_out, const char* name, size_t name_size = SIZE_MAX, SectionFlags flags = SectionFlags::kNone, uint32_t alignment = 1, int32_t order = 0) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Section* section_by_id(uint32_t section_id) const noexcept { return _sections[section_id]; }
  [[nodiscard]]
  ASMJIT_API Section* section_by_name(const char* name, size_t name_size = SIZE_MAX) const noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Section* text_section() const noexcept { return _sections[0]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_address_table_section() const noexcept { return _address_table_section != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Section* address_table_section() const noexcept { return _address_table_section; }
  [[nodiscard]]
  ASMJIT_API Section* ensure_address_table_section() noexcept;
  ASMJIT_API Error add_address_to_address_table(uint64_t address) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<LabelEntry> label_entries() const noexcept { return _label_entries.as_span(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t label_count() const noexcept { return _label_entries.size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label_valid(uint32_t label_id) const noexcept {
    return label_id < _label_entries.size();
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label_valid(const Label& label) const noexcept {
    return is_label_valid(label.id());
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label_bound(uint32_t label_id) const noexcept {
    return is_label_valid(label_id) && _label_entries[label_id].is_bound();
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label_bound(const Label& label) const noexcept {
    return is_label_bound(label.id());
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelEntry& label_entry_of(uint32_t label_id) noexcept {
    return _label_entries[label_id];
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const LabelEntry& label_entry_of(uint32_t label_id) const noexcept {
    return _label_entries[label_id];
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelEntry& label_entry_of(const Label& label) noexcept {
    return label_entry_of(label.id());
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const LabelEntry& label_entry_of(const Label& label) const noexcept {
    return label_entry_of(label.id());
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t label_offset(uint32_t label_id) const noexcept {
    ASMJIT_ASSERT(is_label_valid(label_id));
    return _label_entries[label_id].offset();
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t label_offset(const Label& label) const noexcept {
    return label_offset(label.id());
  }
  [[nodiscard]]
  inline uint64_t label_offset_from_base(uint32_t label_id) const noexcept {
    ASMJIT_ASSERT(is_label_valid(label_id));
    const LabelEntry& le = _label_entries[label_id];
    return (le.is_bound() ? _sections[le.section_id()]->offset() : uint64_t(0)) + le.offset();
  }
  [[nodiscard]]
  inline uint64_t label_offset_from_base(const Label& label) const noexcept {
    return label_offset_from_base(label.id());
  }
  [[nodiscard]]
  ASMJIT_API Error new_label_id(Out<uint32_t> label_id_out) noexcept;
  [[nodiscard]]
  ASMJIT_API Error new_named_label_id(Out<uint32_t> label_id_out, const char* name, size_t name_size, LabelType type, uint32_t parent_id = Globals::kInvalidId) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label label_by_name(const char* name, size_t name_size = SIZE_MAX, uint32_t parent_id = Globals::kInvalidId) noexcept {
    return Label(label_id_by_name(name, name_size, parent_id));
  }
  [[nodiscard]]
  ASMJIT_API Label label_by_name(Span<const char> name, uint32_t parent_id = Globals::kInvalidId) noexcept {
    return label_by_name(name.data(), name.size(), parent_id);
  }
  [[nodiscard]]
  ASMJIT_API uint32_t label_id_by_name(const char* name, size_t name_size = SIZE_MAX, uint32_t parent_id = Globals::kInvalidId) noexcept;
  [[nodiscard]]
  ASMJIT_API uint32_t label_id_by_name(Span<const char> name, uint32_t parent_id = Globals::kInvalidId) noexcept {
    return label_id_by_name(name.data(), name.size(), parent_id);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_unresolved_fixups() const noexcept { return _unresolved_fixup_count != 0u; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t unresolved_fixup_count() const noexcept { return _unresolved_fixup_count; }
  [[nodiscard]]
  ASMJIT_API Fixup* new_fixup(LabelEntry& le, uint32_t section_id, size_t offset, intptr_t rel, const OffsetFormat& format) noexcept;
  ASMJIT_API Error resolve_cross_section_fixups() noexcept;
  ASMJIT_API Error bind_label(const Label& label, uint32_t section_id, uint64_t offset) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_reloc_entries() const noexcept { return !_relocations.is_empty(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<RelocEntry*> reloc_entries() const noexcept { return _relocations.as_span(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RelocEntry* reloc_entry_of(uint32_t id) const noexcept { return _relocations[id]; }
  [[nodiscard]]
  ASMJIT_API Error new_reloc_entry(Out<RelocEntry*> dst, RelocType reloc_type) noexcept;
  ASMJIT_API Error flatten() noexcept;
  [[nodiscard]]
  ASMJIT_API size_t code_size() const noexcept;
  ASMJIT_API Error relocate_to_base(uint64_t base_address, RelocationSummary* summary_out = nullptr) noexcept;
  ASMJIT_API Error copy_section_data(void* dst, size_t dst_size, uint32_t section_id, CopySectionFlags copy_flags = CopySectionFlags::kNone) noexcept;
  ASMJIT_API Error copy_flattened_data(void* dst, size_t dst_size, CopySectionFlags copy_flags = CopySectionFlags::kNone) noexcept;
};
ASMJIT_END_NAMESPACE
#endif
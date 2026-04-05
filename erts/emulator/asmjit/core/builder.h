#ifndef ASMJIT_CORE_BUILDER_H_INCLUDED
#define ASMJIT_CORE_BUILDER_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_BUILDER
#include <asmjit/core/assembler.h>
#include <asmjit/core/codeholder.h>
#include <asmjit/core/constpool.h>
#include <asmjit/core/formatter.h>
#include <asmjit/core/inst.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/string.h>
#include <asmjit/core/type.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenavector.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
class BaseBuilder;
class Pass;
class BaseNode;
class InstNode;
class SectionNode;
class LabelNode;
class AlignNode;
class EmbedDataNode;
class EmbedLabelNode;
class ConstPoolNode;
class CommentNode;
class SentinelNode;
class LabelDeltaNode;
enum class NodeType : uint8_t {
  kNone = 0,
  kInst = 1,
  kSection = 2,
  kLabel = 3,
  kAlign = 4,
  kEmbedData = 5,
  kEmbedLabel = 6,
  kEmbedLabelDelta = 7,
  kConstPool = 8,
  kComment = 9,
  kSentinel = 10,
  kJump = 15,
  kFunc = 16,
  kFuncRet = 17,
  kInvoke = 18,
  kUser = 32
};
enum class NodeFlags : uint8_t {
  kNone = 0,
  kIsCode = 0x01u,
  kIsData = 0x02u,
  kIsInformative = 0x04u,
  kIsRemovable = 0x08u,
  kHasNoEffect = 0x10u,
  kActsAsInst = 0x20u,
  kActsAsLabel = 0x40u,
  kIsActive = 0x80u
};
ASMJIT_DEFINE_ENUM_FLAGS(NodeFlags)
enum class NodePosition : uint32_t {};
enum class SentinelType : uint8_t {
  kUnknown = 0u,
  kFuncEnd = 1u
};
class NodeList {
public:
  BaseNode* _first = nullptr;
  BaseNode* _last = nullptr;
  ASMJIT_INLINE_NODEBUG NodeList() noexcept {}
  ASMJIT_INLINE_NODEBUG NodeList(BaseNode* first, BaseNode* last) noexcept
    : _first(first),
      _last(last) {}
  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _first = nullptr;
    _last = nullptr;
  }
  ASMJIT_INLINE_NODEBUG void reset(BaseNode* first, BaseNode* last) noexcept {
    _first = first;
    _last = last;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _first == nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* first() const noexcept { return _first; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* last() const noexcept { return _last; }
};
class ASMJIT_VIRTAPI BaseBuilder : public BaseEmitter {
public:
  ASMJIT_NONCOPYABLE(BaseBuilder)
  using Base = BaseEmitter;
  Arena _builder_arena;
  Arena _pass_arena;
  ArenaVector<Pass*> _passes;
  ArenaVector<SectionNode*> _section_nodes;
  ArenaVector<LabelNode*> _label_nodes;
  BaseNode* _cursor = nullptr;
  NodeList _node_list;
  bool _dirty_section_links = false;
  ASMJIT_API BaseBuilder() noexcept;
  ASMJIT_API ~BaseBuilder() noexcept override;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeList node_list() const noexcept { return _node_list; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* first_node() const noexcept { return _node_list.first(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* last_node() const noexcept { return _node_list.last(); }
  template<typename T, typename... Args>
  ASMJIT_INLINE Error new_node_with_size_t(Out<T*> out, size_t size, Args&&... args) {
    ASMJIT_ASSERT(Support::is_aligned(size, Arena::kAlignment));
    void* ptr =_builder_arena.alloc_oneshot(size);
    if (ASMJIT_UNLIKELY(!ptr)) {
      return report_error(make_error(Error::kOutOfMemory));
    }
    out = new(Support::PlacementNew{ptr}) T(std::forward<Args>(args)...);
    return Error::kOk;
  }
  template<typename T, typename... Args>
  ASMJIT_INLINE Error new_node_t(Out<T*> out, Args&&... args) {
    void* ptr = _builder_arena.alloc_oneshot(Arena::aligned_size_of<T>());
    if (ASMJIT_UNLIKELY(!ptr)) {
      return report_error(make_error(Error::kOutOfMemory));
    }
    out = new(Support::PlacementNew{ptr}) T(std::forward<Args>(args)...);
    return Error::kOk;
  }
  ASMJIT_API Error new_inst_node(Out<InstNode*> out, InstId inst_id, InstOptions inst_options, uint32_t op_count);
  ASMJIT_API Error new_label_node(Out<LabelNode*> out);
  ASMJIT_API Error new_align_node(Out<AlignNode*> out, AlignMode align_mode, uint32_t alignment);
  ASMJIT_API Error new_embed_data_node(Out<EmbedDataNode*> out, TypeId type_id, const void* data, size_t item_count, size_t repeat_count = 1);
  ASMJIT_API Error new_const_pool_node(Out<ConstPoolNode*> out);
  ASMJIT_API Error new_comment_node(Out<CommentNode*> out, const char* data, size_t size);
  ASMJIT_API BaseNode* add_node(BaseNode* ASMJIT_NONNULL(node)) noexcept;
  ASMJIT_API BaseNode* add_after(BaseNode* ASMJIT_NONNULL(node), BaseNode* ASMJIT_NONNULL(ref)) noexcept;
  ASMJIT_API BaseNode* add_before(BaseNode* ASMJIT_NONNULL(node), BaseNode* ASMJIT_NONNULL(ref)) noexcept;
  ASMJIT_API BaseNode* remove_node(BaseNode* ASMJIT_NONNULL(node)) noexcept;
  ASMJIT_API void remove_nodes(BaseNode* first, BaseNode* last) noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* cursor() const noexcept { return _cursor; }
  ASMJIT_INLINE_NODEBUG BaseNode* set_cursor(BaseNode* node) noexcept {
    BaseNode* old = _cursor;
    _cursor = node;
    return old;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<SectionNode*> section_nodes() const noexcept {
    return _section_nodes.as_span();
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_registered_section_node(uint32_t section_id) const noexcept {
    return section_id < _section_nodes.size() && _section_nodes[section_id] != nullptr;
  }
  ASMJIT_API Error section_node_of(Out<SectionNode*> out, uint32_t section_id);
  ASMJIT_API Error section(Section* ASMJIT_NONNULL(section)) override;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_dirty_section_links() const noexcept { return _dirty_section_links; }
  ASMJIT_API void update_section_links() noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<LabelNode*> label_nodes() const noexcept { return _label_nodes.as_span(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_registered_label_node(uint32_t label_id) const noexcept {
    return label_id < _label_nodes.size() && _label_nodes[label_id] != nullptr;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_registered_label_node(const Label& label) const noexcept {
    return has_registered_label_node(label.id());
  }
  ASMJIT_API Error label_node_of(Out<LabelNode*> out, uint32_t label_id);
  ASMJIT_INLINE_NODEBUG Error label_node_of(Out<LabelNode*> out, const Label& label) {
    return label_node_of(out, label.id());
  }
  [[nodiscard]]
  ASMJIT_API Error register_label_node(LabelNode* ASMJIT_NONNULL(node));
  [[nodiscard]]
  ASMJIT_API Label new_label() override;
  [[nodiscard]]
  ASMJIT_API Label new_named_label(const char* name, size_t name_size = SIZE_MAX, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId) override;
  [[nodiscard]]
  ASMJIT_INLINE Label new_named_label(Span<const char> name, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId) {
    return new_named_label(name.data(), name.size(), type, parent_id);
  }
  ASMJIT_API Error bind(const Label& label) override;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<Pass*> passes() const noexcept { return _passes.as_span(); }
  template<typename PassT, typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE PassT* new_pass(Args&&... args) noexcept { return _builder_arena.new_oneshot<PassT>(*this, std::forward<Args>(args)...); }
  template<typename T, typename... Args>
  ASMJIT_INLINE Error add_pass(Args&&... args) { return _add_pass(new_pass<T, Args...>(std::forward<Args>(args)...)); }
  [[nodiscard]]
  ASMJIT_API Pass* pass_by_name(const char* name) const noexcept;
  ASMJIT_API Error _add_pass(Pass* pass) noexcept;
  ASMJIT_API Error run_passes();
  ASMJIT_API Error _emit(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext) override;
  ASMJIT_API Error align(AlignMode align_mode, uint32_t alignment) override;
  ASMJIT_API Error embed(const void* data, size_t data_size) override;
  ASMJIT_API Error embed_data_array(TypeId type_id, const void* data, size_t count, size_t repeat = 1) override;
  ASMJIT_API Error embed_const_pool(const Label& label, const ConstPool& pool) override;
  ASMJIT_API Error embed_label(const Label& label, size_t data_size = 0) override;
  ASMJIT_API Error embed_label_delta(const Label& label, const Label& base, size_t data_size = 0) override;
  ASMJIT_API Error comment(const char* data, size_t size = SIZE_MAX) override;
  ASMJIT_INLINE Error comment(Span<const char> data) { return comment(data.data(), data.size()); }
  ASMJIT_API Error serialize_to(BaseEmitter* dst);
  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_reinit(CodeHolder& code) noexcept override;
};
class BaseNode {
public:
  ASMJIT_NONCOPYABLE(BaseNode)
  union {
    struct {
      BaseNode* _prev;
      BaseNode* _next;
    };
    BaseNode* _links[2];
  };
  NodeType _node_type;
  NodeFlags _node_flags;
  struct AnyData {
    uint8_t _reserved_0;
    uint8_t _reserved_1;
  };
  struct AlignData {
    AlignMode _align_mode;
    uint8_t _reserved;
  };
  struct InstData {
    uint8_t _op_count;
    uint8_t _op_capacity;
  };
  struct EmbedData {
    TypeId _type_id;
    uint8_t _type_size;
  };
  struct SentinelData {
    SentinelType _sentinel_type;
    uint8_t _reserved_1;
  };
  union {
    AnyData _any;
    AlignData _align_data;
    InstData _inst;
    EmbedData _embed;
    SentinelData _sentinel;
  };
  NodePosition _position;
#if !defined(ASMJIT_NO_NODE_USERDATA)
  union {
    uint64_t _user_data_u64;
    void* _user_data_ptr;
  };
#endif
  void* _pass_data;
  const char* _inline_comment;
  ASMJIT_INLINE_NODEBUG explicit BaseNode(NodeType node_type, NodeFlags node_flags = NodeFlags::kNone) noexcept {
    _prev = nullptr;
    _next = nullptr;
    _node_type = node_type;
    _node_flags = node_flags;
    _any._reserved_0 = 0;
    _any._reserved_1 = 0;
    _position = NodePosition(0);
#if !defined(ASMJIT_NO_NODE_USERDATA)
    _user_data_u64 = 0;
#endif
    _pass_data = nullptr;
    _inline_comment = nullptr;
  }
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* as() noexcept { return static_cast<T*>(this); }
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const T* as() const noexcept { return static_cast<const T*>(this); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* prev() const noexcept { return _prev; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* next() const noexcept { return _next; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeType type() const noexcept { return _node_type; }
  ASMJIT_INLINE_NODEBUG void _set_type(NodeType type) noexcept { _node_type = type; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_inst() const noexcept { return has_flag(NodeFlags::kActsAsInst); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_section() const noexcept { return type() == NodeType::kSection; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label() const noexcept { return has_flag(NodeFlags::kActsAsLabel); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_align() const noexcept { return type() == NodeType::kAlign; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_embed_data() const noexcept { return type() == NodeType::kEmbedData; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_embed_label() const noexcept { return type() == NodeType::kEmbedLabel; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_embed_label_delta() const noexcept { return type() == NodeType::kEmbedLabelDelta; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_const_pool() const noexcept { return type() == NodeType::kConstPool; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_comment() const noexcept { return type() == NodeType::kComment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_sentinel() const noexcept { return type() == NodeType::kSentinel; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_func() const noexcept { return type() == NodeType::kFunc; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_func_ret() const noexcept { return type() == NodeType::kFuncRet; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_invoke() const noexcept { return type() == NodeType::kInvoke; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeFlags flags() const noexcept { return _node_flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(NodeFlags flag) const noexcept { return Support::test(_node_flags, flag); }
  ASMJIT_INLINE_NODEBUG void _assign_flags(NodeFlags flags) noexcept { _node_flags = flags; }
  ASMJIT_INLINE_NODEBUG void _add_flags(NodeFlags flags) noexcept { _node_flags |= flags; }
  ASMJIT_INLINE_NODEBUG void _clear_flags(NodeFlags flags) noexcept { _node_flags &= ~flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_code() const noexcept { return has_flag(NodeFlags::kIsCode); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_data() const noexcept { return has_flag(NodeFlags::kIsData); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_informative() const noexcept { return has_flag(NodeFlags::kIsInformative); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_removable() const noexcept { return has_flag(NodeFlags::kIsRemovable); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_no_effect() const noexcept { return has_flag(NodeFlags::kHasNoEffect); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_active() const noexcept { return has_flag(NodeFlags::kIsActive); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_position() const noexcept { return _position != NodePosition(0); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodePosition position() const noexcept { return _position; }
  ASMJIT_INLINE_NODEBUG void set_position(NodePosition position) noexcept { _position = position; }
#if !defined(ASMJIT_NO_NODE_USERDATA)
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* user_data_as_ptr() const noexcept { return static_cast<T*>(_user_data_ptr); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG int64_t user_data_as_int64() const noexcept { return int64_t(_user_data_u64); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t user_data_as_uint64() const noexcept { return _user_data_u64; }
  template<typename T>
  ASMJIT_INLINE_NODEBUG void set_user_data_as_ptr(T* data) noexcept { _user_data_ptr = static_cast<void*>(data); }
  ASMJIT_INLINE_NODEBUG void set_user_data_as_int64(int64_t value) noexcept { _user_data_u64 = uint64_t(value); }
  ASMJIT_INLINE_NODEBUG void set_user_data_as_uint64(uint64_t value) noexcept { _user_data_u64 = value; }
  ASMJIT_INLINE_NODEBUG void reset_user_data() noexcept { _user_data_u64 = 0; }
#endif
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_pass_data() const noexcept { return _pass_data != nullptr; }
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* pass_data() const noexcept { return (T*)_pass_data; }
  template<typename T>
  ASMJIT_INLINE_NODEBUG void set_pass_data(T* data) noexcept { _pass_data = (void*)data; }
  ASMJIT_INLINE_NODEBUG void reset_pass_data() noexcept { _pass_data = nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_inline_comment() const noexcept { return _inline_comment != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* inline_comment() const noexcept { return _inline_comment; }
  ASMJIT_INLINE_NODEBUG void set_inline_comment(const char* s) noexcept { _inline_comment = s; }
  ASMJIT_INLINE_NODEBUG void reset_inline_comment() noexcept { _inline_comment = nullptr; }
};
class InstNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(InstNode)
  static inline constexpr uint32_t kBaseOpCapacity = 3u;
  static inline constexpr uint32_t kFullOpCapacity = Globals::kMaxOpCount;
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR uint32_t capacity_of_op_count(uint32_t op_count) noexcept {
    return op_count <= kBaseOpCapacity ? kBaseOpCapacity : kFullOpCapacity;
  }
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR size_t node_size_of_op_capacity(uint32_t op_capacity) noexcept {
    return Arena::aligned_size(sizeof(InstNode) + op_capacity * sizeof(Operand));
  }
  BaseInst _base_inst;
  ASMJIT_INLINE_NODEBUG InstNode(InstId inst_id, InstOptions options, uint32_t op_count, uint32_t op_capacity = kBaseOpCapacity) noexcept
    : BaseNode(NodeType::kInst, NodeFlags::kIsCode | NodeFlags::kIsRemovable | NodeFlags::kActsAsInst),
      _base_inst(inst_id, options) {
    _inst._op_capacity = uint8_t(op_capacity);
    _inst._op_count = uint8_t(op_count);
  }
  ASMJIT_INLINE_NODEBUG void _reset_ops() noexcept {
    _base_inst.reset_extra_reg();
    reset_op_range(0, op_capacity());
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseInst& baseInst() noexcept { return _base_inst; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const BaseInst& baseInst() const noexcept { return _base_inst; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstId inst_id() const noexcept { return _base_inst.inst_id(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstId real_id() const noexcept { return _base_inst.real_id(); }
  ASMJIT_INLINE_NODEBUG void set_inst_id(InstId id) noexcept { _base_inst.set_inst_id(id); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstOptions options() const noexcept { return _base_inst.options(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_option(InstOptions option) const noexcept { return _base_inst.has_option(option); }
  ASMJIT_INLINE_NODEBUG void set_options(InstOptions options) noexcept { _base_inst.set_options(options); }
  ASMJIT_INLINE_NODEBUG void add_options(InstOptions options) noexcept { _base_inst.add_options(options); }
  ASMJIT_INLINE_NODEBUG void clear_options(InstOptions options) noexcept { _base_inst.clear_options(options); }
  ASMJIT_INLINE_NODEBUG void reset_options() noexcept { _base_inst.reset_options(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_extra_reg() const noexcept { return _base_inst.has_extra_reg(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegOnly& extra_reg() noexcept { return _base_inst.extra_reg(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegOnly& extra_reg() const noexcept { return _base_inst.extra_reg(); }
  ASMJIT_INLINE_NODEBUG void set_extra_reg(const Reg& reg) noexcept { _base_inst.set_extra_reg(reg); }
  ASMJIT_INLINE_NODEBUG void set_extra_reg(const RegOnly& reg) noexcept { _base_inst.set_extra_reg(reg); }
  ASMJIT_INLINE_NODEBUG void reset_extra_reg() noexcept { _base_inst.reset_extra_reg(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<Operand> operands() noexcept { return Span<Operand>(operands_data(), _inst._op_count); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<const Operand> operands() const noexcept { return Span<const Operand>(operands_data(), _inst._op_count); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Operand* operands_data() noexcept { return Support::offset_ptr<Operand>(this, sizeof(InstNode)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Operand* operands_data() const noexcept { return Support::offset_ptr<Operand>(this, sizeof(InstNode)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t op_count() const noexcept { return _inst._op_count; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t op_capacity() const noexcept { return _inst._op_capacity; }
  ASMJIT_INLINE_NODEBUG void set_op_count(size_t op_count) noexcept { _inst._op_count = uint8_t(op_count); }
  [[nodiscard]]
  inline Operand& op(size_t index) noexcept {
    ASMJIT_ASSERT(index < op_capacity());
    Operand* ops = operands_data();
    return ops[index].as<Operand>();
  }
  [[nodiscard]]
  inline const Operand& op(size_t index) const noexcept {
    ASMJIT_ASSERT(index < op_capacity());
    const Operand* ops = operands_data();
    return ops[index].as<Operand>();
  }
  inline void set_op(size_t index, const Operand_& op) noexcept {
    ASMJIT_ASSERT(index < op_capacity());
    Operand* ops = operands_data();
    ops[index].copy_from(op);
  }
  inline void reset_op(size_t index) noexcept {
    ASMJIT_ASSERT(index < op_capacity());
    Operand* ops = operands_data();
    ops[index].reset();
  }
  inline void reset_op_range(size_t start, size_t end) noexcept {
    Operand* ops = operands_data();
    for (size_t i = start; i < end; i++)
      ops[i].reset();
  }
  [[nodiscard]]
  inline bool has_op_type(OperandType op_type) const noexcept {
    for (const Operand& op : operands()) {
      if (op.op_type() == op_type) {
        return true;
      }
    }
    return false;
  }
  [[nodiscard]]
  inline bool has_reg_op() const noexcept { return has_op_type(OperandType::kReg); }
  [[nodiscard]]
  inline bool has_mem_op() const noexcept { return has_op_type(OperandType::kMem); }
  [[nodiscard]]
  inline bool has_imm_op() const noexcept { return has_op_type(OperandType::kImm); }
  [[nodiscard]]
  inline bool has_label_op() const noexcept { return has_op_type(OperandType::kLabel); }
  [[nodiscard]]
  inline size_t index_of_op_type(OperandType op_type) const noexcept {
    Span<const Operand> ops = operands();
    for (size_t i = 0u; i < ops.size(); i++) {
      if (ops[i].op_type() == op_type)
        return i;
    }
    return SIZE_MAX;
  }
  [[nodiscard]]
  inline size_t index_of_mem_op() const noexcept { return index_of_op_type(OperandType::kMem); }
  [[nodiscard]]
  inline size_t index_of_imm_op() const noexcept { return index_of_op_type(OperandType::kImm); }
  [[nodiscard]]
  inline size_t index_of_label_op() const noexcept { return index_of_op_type(OperandType::kLabel); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t* _get_rewrite_array() noexcept { return &_base_inst._extra_reg._id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint32_t* _get_rewrite_array() const noexcept { return &_base_inst._extra_reg._id; }
  static inline constexpr uint32_t kMaxRewriteId = 26 - 1;
  [[nodiscard]]
  inline uint32_t _get_rewrite_index(const uint32_t* id) const noexcept {
    const uint32_t* array = _get_rewrite_array();
    ASMJIT_ASSERT(array <= id);
    size_t index = (size_t)(id - array);
    ASMJIT_ASSERT(index <= kMaxRewriteId);
    return uint32_t(index);
  }
  inline void _rewrite_id_at_index(uint32_t index, uint32_t id) noexcept {
    ASMJIT_ASSERT(index <= kMaxRewriteId);
    uint32_t* array = _get_rewrite_array();
    array[index] = id;
  }
};
template<uint32_t kN>
class InstNodeWithOperands : public InstNode {
public:
  Operand_ _operands[kN];
  ASMJIT_INLINE_NODEBUG InstNodeWithOperands(InstId inst_id, InstOptions options, uint32_t op_count) noexcept
    : InstNode(inst_id, options, op_count, kN) {}
};
class SectionNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(SectionNode)
  uint32_t _section_id;
  SectionNode* _next_section;
  ASMJIT_INLINE_NODEBUG explicit SectionNode(uint32_t section_id = 0) noexcept
    : BaseNode(NodeType::kSection, NodeFlags::kHasNoEffect),
      _section_id(section_id),
      _next_section(nullptr) {}
  ASMJIT_INLINE_NODEBUG uint32_t section_id() const noexcept { return _section_id; }
};
class LabelNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(LabelNode)
  uint32_t _label_id;
  ASMJIT_INLINE_NODEBUG explicit LabelNode(uint32_t label_id = Globals::kInvalidId) noexcept
    : BaseNode(NodeType::kLabel, NodeFlags::kHasNoEffect | NodeFlags::kActsAsLabel),
      _label_id(label_id) {}
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label label() const noexcept { return Label(_label_id); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t label_id() const noexcept { return _label_id; }
};
class AlignNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(AlignNode)
  uint32_t _alignment;
  ASMJIT_INLINE_NODEBUG AlignNode(AlignMode align_mode, uint32_t alignment) noexcept
    : BaseNode(NodeType::kAlign, NodeFlags::kIsCode | NodeFlags::kHasNoEffect) {
    _align_data._align_mode = align_mode;
    _alignment = alignment;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG AlignMode align_mode() const noexcept { return _align_data._align_mode; }
  ASMJIT_INLINE_NODEBUG void set_align_mode(AlignMode align_mode) noexcept { _align_data._align_mode = align_mode; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t alignment() const noexcept { return _alignment; }
  ASMJIT_INLINE_NODEBUG void set_alignment(uint32_t alignment) noexcept { _alignment = alignment; }
};
class EmbedDataNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(EmbedDataNode)
  size_t _item_count;
  size_t _repeat_count;
  ASMJIT_INLINE_NODEBUG EmbedDataNode(TypeId type_id, uint8_t type_size, size_t item_count, size_t repeat_count) noexcept
    : BaseNode(NodeType::kEmbedData, NodeFlags::kIsData),
      _item_count(item_count),
      _repeat_count(repeat_count) {
    _embed._type_id = type_id;
    _embed._type_size = type_size;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG TypeId type_id() const noexcept { return _embed._type_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t type_size() const noexcept { return _embed._type_size; }
  template<typename T = uint8_t>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* data() noexcept { return Support::offset_ptr<T>(this, sizeof(EmbedDataNode)); }
  template<typename T = uint8_t>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint8_t* data() const noexcept { return Support::offset_ptr<T>(this, sizeof(EmbedDataNode)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t item_count() const noexcept { return _item_count; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t repeat_count() const noexcept { return _repeat_count; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t data_size() const noexcept { return size_t(type_size()) * _item_count; }
};
class EmbedLabelNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(EmbedLabelNode)
  uint32_t _label_id;
  uint32_t _data_size;
  ASMJIT_INLINE_NODEBUG EmbedLabelNode(uint32_t label_id = 0, uint32_t data_size = 0) noexcept
    : BaseNode(NodeType::kEmbedLabel, NodeFlags::kIsData),
      _label_id(label_id),
      _data_size(data_size) {}
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label label() const noexcept { return Label(_label_id); }
  ASMJIT_INLINE_NODEBUG void set_label(const Label& label) noexcept { set_label_id(label.id()); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t label_id() const noexcept { return _label_id; }
  ASMJIT_INLINE_NODEBUG void set_label_id(uint32_t label_id) noexcept { _label_id = label_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t data_size() const noexcept { return _data_size; }
  ASMJIT_INLINE_NODEBUG void set_data_size(uint32_t data_size) noexcept { _data_size = data_size; }
};
class EmbedLabelDeltaNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(EmbedLabelDeltaNode)
  uint32_t _label_id;
  uint32_t _base_label_id;
  uint32_t _data_size;
  ASMJIT_INLINE_NODEBUG EmbedLabelDeltaNode(uint32_t label_id = 0, uint32_t base_label_id = 0, uint32_t data_size = 0) noexcept
    : BaseNode(NodeType::kEmbedLabelDelta, NodeFlags::kIsData),
      _label_id(label_id),
      _base_label_id(base_label_id),
      _data_size(data_size) {}
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label label() const noexcept { return Label(_label_id); }
  ASMJIT_INLINE_NODEBUG void set_label(const Label& label) noexcept { set_label_id(label.id()); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t label_id() const noexcept { return _label_id; }
  ASMJIT_INLINE_NODEBUG void set_label_id(uint32_t label_id) noexcept { _label_id = label_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label base_babel() const noexcept { return Label(_base_label_id); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t base_label_id() const noexcept { return _base_label_id; }
  ASMJIT_INLINE_NODEBUG void set_base_label(const Label& base_babel) noexcept { set_base_label_id(base_babel.id()); }
  ASMJIT_INLINE_NODEBUG void set_base_label_id(uint32_t base_label_id) noexcept { _base_label_id = base_label_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t data_size() const noexcept { return _data_size; }
  ASMJIT_INLINE_NODEBUG void set_data_size(uint32_t data_size) noexcept { _data_size = data_size; }
};
class ConstPoolNode : public LabelNode {
public:
  ASMJIT_NONCOPYABLE(ConstPoolNode)
  ConstPool _const_pool;
  ASMJIT_INLINE_NODEBUG ConstPoolNode(Arena& arena, uint32_t id = 0) noexcept
    : LabelNode(id),
      _const_pool(arena) {
    _set_type(NodeType::kConstPool);
    _add_flags(NodeFlags::kIsData);
    _clear_flags(NodeFlags::kIsCode | NodeFlags::kHasNoEffect);
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _const_pool.is_empty(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _const_pool.size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t alignment() const noexcept { return _const_pool.alignment(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ConstPool& const_pool() noexcept { return _const_pool; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ConstPool& const_pool() const noexcept { return _const_pool; }
  ASMJIT_INLINE_NODEBUG Error add(const void* data, size_t size, Out<size_t> offset_out) noexcept {
    return _const_pool.add(data, size, offset_out);
  }
};
class CommentNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(CommentNode)
  ASMJIT_INLINE_NODEBUG CommentNode(const char* comment) noexcept
    : BaseNode(NodeType::kComment, NodeFlags::kIsInformative | NodeFlags::kHasNoEffect | NodeFlags::kIsRemovable) {
    _inline_comment = comment;
  }
};
class SentinelNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(SentinelNode)
  ASMJIT_INLINE_NODEBUG SentinelNode(SentinelType sentinel_type = SentinelType::kUnknown) noexcept
    : BaseNode(NodeType::kSentinel, NodeFlags::kIsInformative | NodeFlags::kHasNoEffect) {
    _sentinel._sentinel_type = sentinel_type;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SentinelType sentinel_type() const noexcept {
    return _sentinel._sentinel_type;
  }
  ASMJIT_INLINE_NODEBUG void set_sentinel_type(SentinelType type) noexcept {
    _sentinel._sentinel_type = type;
  }
};
class ASMJIT_VIRTAPI Pass {
public:
  ASMJIT_BASE_CLASS(Pass)
  ASMJIT_NONCOPYABLE(Pass)
  BaseBuilder& _cb;
  const char* _name {};
  ASMJIT_API Pass(BaseBuilder& cb, const char* name) noexcept;
  ASMJIT_API virtual ~Pass() noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseBuilder& cb() const noexcept { return _cb; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return _name; }
  ASMJIT_API virtual Error run(Arena& arena, Logger* logger);
};
ASMJIT_END_NAMESPACE
#endif
#endif
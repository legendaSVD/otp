#ifndef ASMJIT_CORE_CONSTPOOL_H_INCLUDED
#define ASMJIT_CORE_CONSTPOOL_H_INCLUDED
#include <asmjit/support/arena.h>
#include <asmjit/support/arenatree.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
enum class ConstPoolScope : uint32_t {
  kLocal = 0,
  kGlobal = 1,
  kMaxValue = kGlobal
};
class ConstPool {
public:
  ASMJIT_NONCOPYABLE(ConstPool)
  enum Index : uint32_t {
    kIndex1 = 0,
    kIndex2 = 1,
    kIndex4 = 2,
    kIndex8 = 3,
    kIndex16 = 4,
    kIndex32 = 5,
    kIndex64 = 6,
    kIndexCount = 7
  };
  struct Gap {
    Gap* _next;
    size_t _offset;
    size_t _size;
  };
  class Node : public ArenaTreeNodeT<Node> {
  public:
    ASMJIT_NONCOPYABLE(Node)
    uint32_t _shared : 1;
    uint32_t _offset;
    ASMJIT_INLINE_NODEBUG Node(size_t offset, bool shared) noexcept
      : ArenaTreeNodeT<Node>(),
        _shared(shared),
        _offset(uint32_t(offset)) {}
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG void* data() noexcept { return Support::offset_ptr<void>(this, sizeof(*this)); }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG const void* data() const noexcept { return Support::offset_ptr<void>(this, sizeof(*this)); }
  };
  class Compare {
  public:
    size_t _data_size;
    ASMJIT_INLINE_NODEBUG Compare(size_t data_size) noexcept
      : _data_size(data_size) {}
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG int operator()(const Node& a, const Node& b) const noexcept {
      return ::memcmp(a.data(), b.data(), _data_size);
    }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG int operator()(const Node& a, const void* data) const noexcept {
      return ::memcmp(a.data(), data, _data_size);
    }
  };
  struct Tree {
    ArenaTree<Node> _tree;
    size_t _size;
    size_t _data_size;
    ASMJIT_INLINE_NODEBUG explicit Tree(size_t data_size = 0) noexcept
      : _tree(),
        _size(0),
        _data_size(data_size) {}
    ASMJIT_INLINE_NODEBUG void reset() noexcept {
      _tree.reset();
      _size = 0;
    }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }
    inline void set_data_size(size_t data_size) noexcept {
      ASMJIT_ASSERT(is_empty());
      _data_size = data_size;
    }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG Node* get(const void* data) noexcept {
      Compare cmp(_data_size);
      return _tree.get(data, cmp);
    }
    ASMJIT_INLINE_NODEBUG void insert(Node* node) noexcept {
      Compare cmp(_data_size);
      _tree.insert(node, cmp);
      _size++;
    }
    template<typename Visitor>
    inline void for_each(Visitor& visitor) const noexcept {
      Node* node = _tree.root();
      if (!node) return;
      Node* stack[Globals::kMaxTreeHeight];
      size_t top = 0;
      for (;;) {
        Node* left = node->left();
        if (left != nullptr) {
          ASMJIT_ASSERT(top != Globals::kMaxTreeHeight);
          stack[top++] = node;
          node = left;
          continue;
        }
        for (;;) {
          visitor(node);
          node = node->right();
          if (node != nullptr)
            break;
          if (top == 0)
            return;
          node = stack[--top];
        }
      }
    }
    [[nodiscard]]
    static inline Node* new_node_t(Arena& arena, const void* data, size_t size, size_t offset, bool shared) noexcept {
      Node* node = arena.alloc_oneshot<Node>(Arena::aligned_size(sizeof(Node) + size));
      if (ASMJIT_UNLIKELY(!node)) {
        return nullptr;
      }
      node = new(Support::PlacementNew{node}) Node(offset, shared);
      memcpy(node->data(), data, size);
      return node;
    }
  };
  Arena& _arena;
  Tree _tree[kIndexCount];
  Gap* _gaps[kIndexCount];
  Gap* _gap_pool;
  size_t _size;
  size_t _alignment;
  size_t _min_item_size;
  ASMJIT_API explicit ConstPool(Arena& arena) noexcept;
  ASMJIT_API ~ConstPool() noexcept;
  ASMJIT_API void reset() noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t alignment() const noexcept { return _alignment; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t min_item_size() const noexcept { return _min_item_size; }
  ASMJIT_API Error add(const void* data, size_t size, Out<size_t> offset_out) noexcept;
  ASMJIT_API void fill(void* dst) const noexcept;
};
ASMJIT_END_NAMESPACE
#endif
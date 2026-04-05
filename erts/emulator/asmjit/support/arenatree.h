#ifndef ASMJIT_SUPPORT_ARENATREE_H_INCLUDED
#define ASMJIT_SUPPORT_ARENATREE_H_INCLUDED
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
class ArenaTreeNode {
public:
  ASMJIT_NONCOPYABLE(ArenaTreeNode)
  static inline constexpr uintptr_t kRedMask = 0x1;
  static inline constexpr uintptr_t kPtrMask = ~kRedMask;
  uintptr_t _tree_nodes[2] {};
  ASMJIT_INLINE_NODEBUG ArenaTreeNode() noexcept {}
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_red() const noexcept { return static_cast<bool>(_tree_nodes[0] & kRedMask); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_child(size_t i) const noexcept { return _tree_nodes[i] > kRedMask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_left() const noexcept { return _tree_nodes[0] > kRedMask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_right() const noexcept { return _tree_nodes[1] != 0; }
  template<typename T = ArenaTreeNode>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* child(size_t i) const noexcept { return static_cast<T*>(_get_child(i)); }
  template<typename T = ArenaTreeNode>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* left() const noexcept { return static_cast<T*>(_get_left()); }
  template<typename T = ArenaTreeNode>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* right() const noexcept { return static_cast<T*>(_get_right()); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ArenaTreeNode* _get_child(size_t i) const noexcept { return (ArenaTreeNode*)(_tree_nodes[i] & kPtrMask); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ArenaTreeNode* _get_left() const noexcept { return (ArenaTreeNode*)(_tree_nodes[0] & kPtrMask); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ArenaTreeNode* _get_right() const noexcept { return (ArenaTreeNode*)(_tree_nodes[1]); }
  ASMJIT_INLINE_NODEBUG void _set_child(size_t i, ArenaTreeNode* node) noexcept { _tree_nodes[i] = (_tree_nodes[i] & kRedMask) | (uintptr_t)node; }
  ASMJIT_INLINE_NODEBUG void _set_left(ArenaTreeNode* node) noexcept { _tree_nodes[0] = (_tree_nodes[0] & kRedMask) | (uintptr_t)node; }
  ASMJIT_INLINE_NODEBUG void _set_right(ArenaTreeNode* node) noexcept { _tree_nodes[1] = (uintptr_t)node; }
  ASMJIT_INLINE_NODEBUG void _make_red() noexcept { _tree_nodes[0] |= kRedMask; }
  ASMJIT_INLINE_NODEBUG void _make_black() noexcept { _tree_nodes[0] &= kPtrMask; }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool _is_valid_red(ArenaTreeNode* node) noexcept { return node && node->is_red(); }
};
template<typename NodeT>
class ArenaTreeNodeT : public ArenaTreeNode {
public:
  ASMJIT_NONCOPYABLE(ArenaTreeNodeT)
  ASMJIT_INLINE_NODEBUG ArenaTreeNodeT() noexcept
    : ArenaTreeNode() {}
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeT* child(size_t i) const noexcept { return static_cast<NodeT*>(_get_child(i)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeT* left() const noexcept { return static_cast<NodeT*>(_get_left()); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeT* right() const noexcept { return static_cast<NodeT*>(_get_right()); }
};
template<typename NodeT>
class ArenaTree {
public:
  ASMJIT_NONCOPYABLE(ArenaTree)
  using Node = NodeT;
  NodeT* _root {};
  ASMJIT_INLINE_NODEBUG ArenaTree() noexcept {}
  ASMJIT_INLINE_NODEBUG ArenaTree(ArenaTree&& other) noexcept
    : _root(other._root) {}
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _root = nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _root == nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeT* root() const noexcept { return static_cast<NodeT*>(_root); }
  ASMJIT_INLINE_NODEBUG void swap(ArenaTree& other) noexcept {
    std::swap(_root, other._root);
  }
  template<typename CompareT = Support::Compare<Support::SortOrder::kAscending>>
  void insert(NodeT* ASMJIT_NONNULL(node), const CompareT& cmp = CompareT()) noexcept {
    ASMJIT_ASSERT(!node->has_left());
    ASMJIT_ASSERT(!node->has_right());
    ASMJIT_ASSERT(!node->is_red());
    if (!_root) {
      _root = node;
      return;
    }
    ArenaTreeNode head;
    head._set_right(_root);
    ArenaTreeNode* g = nullptr;
    ArenaTreeNode* p = nullptr;
    ArenaTreeNode* t = &head;
    ArenaTreeNode* q = _root;
    size_t dir = 0;
    size_t last = 0;
    node->_make_red();
    for (;;) {
      if (!q) {
        q = node;
        p->_set_child(dir, node);
      }
      else if (_is_valid_red(q->_get_left()) && _is_valid_red(q->_get_right())) {
        q->_make_red();
        q->_get_left()->_make_black();
        q->_get_right()->_make_black();
      }
      if (_is_valid_red(q) && _is_valid_red(p)) {
        ASMJIT_ASSUME(g != nullptr);
        ASMJIT_ASSUME(p != nullptr);
        t->_set_child(t->_get_right() == g,
                     q == p->_get_child(last) ? _single_rotate(g, !last) : _double_rotate(g, !last));
      }
      if (q == node) {
        break;
      }
      last = dir;
      dir = cmp(*static_cast<NodeT*>(q), *static_cast<NodeT*>(node)) < 0;
      if (g) {
        t = g;
      }
      g = p;
      p = q;
      q = q->_get_child(dir);
    }
    _root = static_cast<NodeT*>(head._get_right());
    _root->_make_black();
  }
  template<typename CompareT = Support::Compare<Support::SortOrder::kAscending>>
  void remove(ArenaTreeNode* ASMJIT_NONNULL(node), const CompareT& cmp = CompareT()) noexcept {
    ArenaTreeNode head;
    head._set_right(_root);
    ArenaTreeNode* g = nullptr;
    ArenaTreeNode* p = nullptr;
    ArenaTreeNode* q = &head;
    ArenaTreeNode* f  = nullptr;
    ArenaTreeNode* gf = nullptr;
    size_t dir = 1;
    while (q->has_child(dir)) {
      size_t last = dir;
      g = p;
      p = q;
      q = q->_get_child(dir);
      dir = cmp(*static_cast<NodeT*>(q), *static_cast<NodeT*>(node)) < 0;
      if (q == node) {
        f = q;
        gf = g;
      }
      if (!_is_valid_red(q) && !_is_valid_red(q->_get_child(dir))) {
        if (_is_valid_red(q->_get_child(!dir))) {
          ArenaTreeNode* child = _single_rotate(q, dir);
          p->_set_child(last, child);
          p = child;
        }
        else if (!_is_valid_red(q->_get_child(!dir)) && p->_get_child(!last)) {
          ArenaTreeNode* s = p->_get_child(!last);
          if (!_is_valid_red(s->_get_child(!last)) && !_is_valid_red(s->_get_child(last))) {
            p->_make_black();
            s->_make_red();
            q->_make_red();
          }
          else {
            ASMJIT_ASSUME(g != nullptr);
            ASMJIT_ASSUME(s != nullptr);
            size_t dir2 = g->_get_right() == p;
            ArenaTreeNode* child = g->_get_child(dir2);
            if (_is_valid_red(s->_get_child(last))) {
              child = _double_rotate(p, last);
              g->_set_child(dir2, child);
            }
            else if (_is_valid_red(s->_get_child(!last))) {
              child = _single_rotate(p, last);
              g->_set_child(dir2, child);
            }
            q->_make_red();
            child->_make_red();
            child->_get_left()->_make_black();
            child->_get_right()->_make_black();
          }
        }
      }
    }
    ASMJIT_ASSERT(f != nullptr);
    ASMJIT_ASSERT(f != &head);
    ASMJIT_ASSERT(q != &head);
    p->_set_child(p->_get_right() == q,
                  q->_get_child(q->_get_left() == nullptr));
    if (f != q) {
      ASMJIT_ASSERT(f != &head);
      ASMJIT_ASSERT(f != gf);
      ArenaTreeNode* n = gf ? gf : &head;
      dir = (n == &head) ? 1  : cmp(*static_cast<NodeT*>(n), *static_cast<NodeT*>(node)) < 0;
      for (;;) {
        if (n->_get_child(dir) == f) {
          n->_set_child(dir, q);
          q->_tree_nodes[0] = f->_tree_nodes[0];
          q->_tree_nodes[1] = f->_tree_nodes[1];
          break;
        }
        n = n->_get_child(dir);
        ASMJIT_ASSERT(n != nullptr);
        dir = cmp(*static_cast<NodeT*>(n), *static_cast<NodeT*>(node)) < 0;
      }
    }
    _root = static_cast<NodeT*>(head._get_right());
    if (_root) {
      _root->_make_black();
    }
  }
  template<typename KeyT, typename CompareT = Support::Compare<Support::SortOrder::kAscending>>
  [[nodiscard]]
  inline NodeT* get(const KeyT& key, const CompareT& cmp = CompareT()) const noexcept {
    ArenaTreeNode* node = _root;
    while (node) {
      auto result = cmp(*static_cast<const NodeT*>(node), key);
      if (result == 0) {
        break;
      }
      node = node->_get_child(result < 0);
    }
    return static_cast<NodeT*>(node);
  }
  static inline bool _is_valid_red(ArenaTreeNode* node) noexcept { return ArenaTreeNode::_is_valid_red(node); }
  static inline ArenaTreeNode* _single_rotate(ArenaTreeNode* ASMJIT_NONNULL(root), size_t dir) noexcept {
    ArenaTreeNode* save = root->_get_child(!dir);
    ASMJIT_ASSUME(save != nullptr);
    ArenaTreeNode* save_child = save->_get_child(dir);
    root->_set_child(!dir, save_child);
    save->_set_child( dir, root);
    root->_make_red();
    save->_make_black();
    return save;
  }
  static inline ArenaTreeNode* _double_rotate(ArenaTreeNode* ASMJIT_NONNULL(root), size_t dir) noexcept {
    ArenaTreeNode* child = root->_get_child(!dir);
    ASMJIT_ASSUME(child != nullptr);
    root->_set_child(!dir, _single_rotate(child, !dir));
    return _single_rotate(root, dir);
  }
};
ASMJIT_END_NAMESPACE
#endif
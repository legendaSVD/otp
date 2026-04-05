#ifndef ASMJIT_SUPPORT_ARENAVECTOR_H_INCLUDED
#define ASMJIT_SUPPORT_ARENAVECTOR_H_INCLUDED
#include <asmjit/support/arena.h>
#include <asmjit/support/span.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
class ArenaVectorBase {
public:
  ASMJIT_NONCOPYABLE(ArenaVectorBase)
  template<bool IsPowerOf2>
  struct ItemSize { uint32_t n; };
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  void* _data {};
  uint32_t _size {};
  uint32_t _capacity {};
protected:
  ASMJIT_INLINE_NODEBUG ArenaVectorBase() noexcept {}
  ASMJIT_INLINE_NODEBUG ArenaVectorBase(ArenaVectorBase&& other) noexcept
    : _data(other._data),
      _size(other._size),
      _capacity(other._capacity) { other.reset(); }
  ASMJIT_INLINE void _release(Arena& arena, uint32_t item_byte_size) noexcept {
    if (_data != nullptr) {
      arena.free_reusable(_data, _capacity * item_byte_size);
      reset();
    }
  }
  ASMJIT_INLINE_NODEBUG void _move_from(ArenaVectorBase&& other) noexcept {
    void* data = other._data;
    uint32_t size = other._size;
    uint32_t capacity = other._capacity;
    other._data = nullptr;
    other._size = 0u;
    other._capacity = 0u;
    _data = data;
    _size = size;
    _capacity = capacity;
  }
  ASMJIT_API Error _reserve_fit(Arena& arena, size_t n, ItemSize<false> byte_size) noexcept;
  ASMJIT_API Error _reserve_fit(Arena& arena, size_t n, ItemSize<true> log2_size) noexcept;
  ASMJIT_API Error _reserve_grow(Arena& arena, size_t n, ItemSize<false> byte_size) noexcept;
  ASMJIT_API Error _reserve_grow(Arena& arena, size_t n, ItemSize<true> log2_size) noexcept;
  ASMJIT_API Error _reserve_additional(Arena& arena, size_t n, ItemSize<false> byte_size) noexcept;
  ASMJIT_API Error _reserve_additional(Arena& arena, size_t n, ItemSize<true> log2_size) noexcept;
  ASMJIT_API Error _resize_fit(Arena& arena, size_t n, ItemSize<false> byte_size) noexcept;
  ASMJIT_API Error _resize_fit(Arena& arena, size_t n, ItemSize<true> log2_size) noexcept;
  ASMJIT_API Error _resize_grow(Arena& arena, size_t n, ItemSize<false> byte_size) noexcept;
  ASMJIT_API Error _resize_grow(Arena& arena, size_t n, ItemSize<true> log2_size) noexcept;
  ASMJIT_INLINE_NODEBUG void _swap(ArenaVectorBase& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
  }
public:
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t capacity() const noexcept { return _capacity; }
  ASMJIT_INLINE_NODEBUG void clear() noexcept { _size = 0u; }
  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _data = nullptr;
    _size = 0;
    _capacity = 0;
  }
  ASMJIT_INLINE_NODEBUG void truncate(size_t n) noexcept {
    _size = uint32_t(Support::min<size_t>(_size, n));
  }
  ASMJIT_INLINE void _set_size(size_t n) noexcept {
    ASMJIT_ASSERT(n <= _capacity);
    _size = uint32_t(n);
  }
};
template <typename T>
class ArenaVector : public ArenaVectorBase {
public:
  ASMJIT_NONCOPYABLE(ArenaVector)
  static inline constexpr bool ItemSizeIsPowerOf2 = Support::is_power_of_2(sizeof(T));
  static inline constexpr ItemSize<ItemSizeIsPowerOf2> item_size_ { ItemSizeIsPowerOf2 ? uint32_t(Support::ctz_t(sizeof(T))) : uint32_t(sizeof(T)) };
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using iterator = T*;
  using const_iterator = const T*;
  ASMJIT_INLINE_NODEBUG ArenaVector() noexcept : ArenaVectorBase() {}
  ASMJIT_INLINE_NODEBUG ArenaVector(ArenaVector&& other) noexcept : ArenaVectorBase(std::move(other)) {}
  ASMJIT_INLINE_NODEBUG ArenaVector& operator=(ArenaVector&& other) noexcept {
    _move_from(other);
    return *this;
  }
  [[nodiscard]]
  ASMJIT_INLINE T& operator[](size_t i) noexcept {
    ASMJIT_ASSERT(i < _size);
    return data()[i];
  }
  [[nodiscard]]
  ASMJIT_INLINE const T& operator[](size_t i) const noexcept {
    ASMJIT_ASSERT(i < _size);
    return data()[i];
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG operator Span<T>() const noexcept { return Span<T>(static_cast<T*>(_data), _size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<T> as_span() const noexcept { return Span<T>(static_cast<T*>(_data), _size); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* data() noexcept { return static_cast<T*>(_data); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const T* data() const noexcept { return static_cast<const T*>(_data); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const T* cdata() const noexcept { return static_cast<const T*>(_data); }
  [[nodiscard]]
  ASMJIT_INLINE const T& at(size_t i) const noexcept {
    ASMJIT_ASSERT(i < _size);
    return data()[i];
  }
  ASMJIT_INLINE void _set_end(T* p) noexcept {
    ASMJIT_ASSERT(p >= data() && p <= data() + _capacity);
    _set_size(size_t(p - data()));
  }
  [[nodiscard]]
  ASMJIT_INLINE T& first() noexcept { return operator[](0); }
  [[nodiscard]]
  ASMJIT_INLINE const T& first() const noexcept { return operator[](0); }
  [[nodiscard]]
  ASMJIT_INLINE T& last() noexcept { return operator[](_size - 1); }
  [[nodiscard]]
  ASMJIT_INLINE const T& last() const noexcept { return operator[](_size - 1); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG iterator begin() noexcept { return iterator(data()); };
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_iterator begin() const noexcept { return const_iterator(data()); };
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG iterator end() noexcept { return iterator(data() + _size); };
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_iterator end() const noexcept { return const_iterator(data() + _size); };
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_iterator cbegin() const noexcept { return const_iterator(data()); };
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_iterator cend() const noexcept { return const_iterator(data() + _size); };
  ASMJIT_INLINE_NODEBUG SpanForwardIteratorAdaptor<T> iterate() const noexcept {
    T* p = static_cast<T*>(_data);
    return SpanForwardIteratorAdaptor<T>{p, p + _size};
  }
  ASMJIT_INLINE_NODEBUG SpanReverseIteratorAdaptor<T> iterate_reverse() const noexcept {
    T* p = static_cast<T*>(_data);
    return SpanReverseIteratorAdaptor<T>{p, p + _size};
  }
  ASMJIT_INLINE void swap(ArenaVector<T>& other) noexcept { _swap(other); }
  ASMJIT_INLINE Error prepend(Arena& arena, const T& item) noexcept {
    ASMJIT_PROPAGATE(reserve_additional(arena));
    memmove(static_cast<void*>(static_cast<T*>(_data) + 1),
            static_cast<const void*>(_data),
            size_t(_size) * sizeof(T));
    memcpy(static_cast<void*>(_data),
           static_cast<const void*>(&item),
           sizeof(T));
    _size++;
    return Error::kOk;
  }
  ASMJIT_INLINE Error insert(Arena& arena, size_t index, const T& item) noexcept {
    ASMJIT_ASSERT(index <= _size);
    ASMJIT_PROPAGATE(reserve_additional(arena));
    T* dst = static_cast<T*>(_data) + index;
    memmove(static_cast<void*>(dst + 1),
            static_cast<const void*>(dst),
            size_t(_size - index) * sizeof(T));
    memcpy(static_cast<void*>(dst),
           static_cast<const void*>(&item),
           sizeof(T));
    _size++;
    return Error::kOk;
  }
  ASMJIT_INLINE Error append(Arena& arena, const T& item) noexcept {
    ASMJIT_PROPAGATE(reserve_additional(arena));
    memcpy(static_cast<void*>(static_cast<T*>(_data) + _size),
           static_cast<const void*>(&item),
           sizeof(T));
    _size++;
    return Error::kOk;
  }
  ASMJIT_INLINE Error concat(Arena& arena, const ArenaVector<T>& other) noexcept {
    uint32_t size = other._size;
    if (_capacity - _size < size) {
      ASMJIT_PROPAGATE(reserve_additional(arena, size));
    }
    if (size) {
      memcpy(static_cast<void*>(static_cast<T*>(_data) + _size),
             static_cast<const void*>(other._data),
             size_t(size) * sizeof(T));
      _size += size;
    }
    return Error::kOk;
  }
  ASMJIT_INLINE void assign_unchecked(const ArenaVector<T>& other) noexcept {
    uint32_t size = other._size;
    ASMJIT_ASSERT(_capacity >= other._size);
    if (size) {
      memcpy(_data, other._data, size_t(size) * sizeof(T));
    }
    _size = size;
  }
  ASMJIT_INLINE void prepend_unchecked(const T& item) noexcept {
    ASMJIT_ASSERT(_size < _capacity);
    T* data = static_cast<T*>(_data);
    if (_size) {
      memmove(static_cast<void*>(data + 1),
              static_cast<const void*>(data),
              size_t(_size) * sizeof(T));
    }
    memcpy(static_cast<void*>(data),
           static_cast<const void*>(&item),
           sizeof(T));
    _size++;
  }
  ASMJIT_INLINE void append_unchecked(const T& item) noexcept {
    ASMJIT_ASSERT(_size < _capacity);
    memcpy(static_cast<void*>(static_cast<T*>(_data) + _size),
           static_cast<const void*>(&item),
           sizeof(T));
    _size++;
  }
  ASMJIT_INLINE void insert_unchecked(size_t index, const T& item) noexcept {
    ASMJIT_ASSERT(_size < _capacity);
    ASMJIT_ASSERT(index <= _size);
    T* dst = static_cast<T*>(_data) + index;
    memmove(static_cast<void*>(dst + 1),
            static_cast<const void*>(dst),
            size_t(_size - index) * sizeof(T));
    memcpy(static_cast<void*>(dst),
           static_cast<const void*>(&item),
           sizeof(T));
    _size++;
  }
  ASMJIT_INLINE void concat_unchecked(const ArenaVector<T>& other) noexcept {
    uint32_t size = other._size;
    ASMJIT_ASSERT(_capacity - _size >= size);
    if (size) {
      memcpy(static_cast<void*>(static_cast<T*>(_data) + _size),
             static_cast<const void*>(other._data),
             size_t(size) * sizeof(T));
      _size += size;
    }
  }
  ASMJIT_INLINE void remove_at(size_t i) noexcept {
    ASMJIT_ASSERT(i < _size);
    T* data = static_cast<T*>(_data) + i;
    size_t size = --_size - i;
    if (size) {
      memmove(static_cast<void*>(data),
              static_cast<const void*>(data + 1),
              size_t(size) * sizeof(T));
    }
  }
  [[nodiscard]]
  ASMJIT_INLINE T pop() noexcept {
    ASMJIT_ASSERT(_size > 0);
    uint32_t index = --_size;
    return data()[index];
  }
  template<typename CompareT = Support::Compare<Support::SortOrder::kAscending>>
  ASMJIT_INLINE void sort(const CompareT& cmp = CompareT()) noexcept {
    Support::sort<T, CompareT>(data(), size(), cmp);
  }
  template<typename Value>
  ASMJIT_INLINE bool contains(Value&& value) const noexcept {
    return as_span().contains(std::forward<Value>(value));
  }
  template<typename Value>
  ASMJIT_INLINE size_t index_of(Value&& value) const noexcept {
    return as_span().index_of(std::forward<Value>(value));
  }
  template<typename Value>
  ASMJIT_INLINE size_t last_index_of(Value&& value) const noexcept {
    return as_span().index_of(std::forward<Value>(value));
  }
  ASMJIT_INLINE Error _reserve_fit(Arena& arena, size_t n) noexcept {
    return ArenaVectorBase::_reserve_fit(arena, n, item_size_);
  }
  ASMJIT_INLINE Error _reserve_grow(Arena& arena, size_t n) noexcept {
    return ArenaVectorBase::_reserve_grow(arena, n, item_size_);
  }
  ASMJIT_INLINE Error _resize_fit(Arena& arena, size_t n) noexcept {
    return ArenaVectorBase::_resize_fit(arena, n, item_size_);
  }
  ASMJIT_INLINE Error _resize_grow(Arena& arena, size_t n) noexcept {
    return ArenaVectorBase::_resize_grow(arena, n, item_size_);
  }
  ASMJIT_INLINE Error _reserve_additional(Arena& arena, size_t n) noexcept {
    return ArenaVectorBase::_reserve_additional(arena, n, item_size_);
  }
  ASMJIT_INLINE void release(Arena& arena) noexcept {
    _release(arena, sizeof(T));
  }
  [[nodiscard]]
  ASMJIT_INLINE Error reserve_fit(Arena& arena, size_t n) noexcept {
    if (ASMJIT_UNLIKELY(n > _capacity)) {
      return _reserve_fit(arena, n);
    }
    else {
      return Error::kOk;
    }
  }
  [[nodiscard]]
  ASMJIT_INLINE Error reserve_grow(Arena& arena, size_t n) noexcept {
    if (ASMJIT_UNLIKELY(n > _capacity)) {
      return _reserve_grow(arena, n);
    }
    else {
      return Error::kOk;
    }
  }
  [[nodiscard]]
  ASMJIT_INLINE Error reserve_additional(Arena& arena) noexcept {
    if (ASMJIT_UNLIKELY(_size == _capacity)) {
      return _reserve_additional(arena, 1u);
    }
    else {
      return Error::kOk;
    }
  }
  [[nodiscard]]
  ASMJIT_INLINE Error reserve_additional(Arena& arena, size_t n) noexcept {
    if (ASMJIT_UNLIKELY(_capacity - _size < n)) {
      return _reserve_additional(arena, n);
    }
    else {
      return Error::kOk;
    }
  }
  [[nodiscard]]
  ASMJIT_INLINE Error resize_fit(Arena& arena, size_t n) noexcept {
    return _resize_fit(arena, n);
  }
  [[nodiscard]]
  ASMJIT_INLINE Error resize_grow(Arena& arena, size_t n) noexcept {
    return _resize_grow(arena, n);
  }
};
ASMJIT_END_NAMESPACE
#endif
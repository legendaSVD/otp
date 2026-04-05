#ifndef ASMJIT_SUPPORT_SPAN_H_INCLUDED
#define ASMJIT_SUPPORT_SPAN_H_INCLUDED
#include <asmjit/core/globals.h>
ASMJIT_BEGIN_NAMESPACE
template<typename T>
class SpanForwardIterator {
public:
  T* ptr {};
  ASMJIT_INLINE_CONSTEXPR bool operator==(const T* other) const noexcept { return ptr == other; }
  ASMJIT_INLINE_CONSTEXPR bool operator==(const SpanForwardIterator& other) const noexcept { return ptr == other.ptr; }
  ASMJIT_INLINE_CONSTEXPR bool operator!=(const T* other) const noexcept { return ptr != other; }
  ASMJIT_INLINE_CONSTEXPR bool operator!=(const SpanForwardIterator& other) const noexcept { return ptr != other.ptr; }
  ASMJIT_INLINE_CONSTEXPR SpanForwardIterator& operator++() noexcept { ptr++; return *this; }
  ASMJIT_INLINE_CONSTEXPR SpanForwardIterator operator++(int) noexcept { SpanForwardIterator prev(*this); ptr++; return prev; }
  ASMJIT_INLINE_CONSTEXPR T& operator*() const noexcept { return *ptr; }
  ASMJIT_INLINE_CONSTEXPR T* operator->() const noexcept { return ptr; }
  ASMJIT_INLINE_CONSTEXPR operator T*() const noexcept { return ptr; }
};
template<typename T>
class SpanReverseIterator {
public:
  T* ptr {};
  ASMJIT_INLINE_CONSTEXPR bool operator==(const T* other) const noexcept { return ptr == other; }
  ASMJIT_INLINE_CONSTEXPR bool operator==(const SpanReverseIterator& other) const noexcept { return ptr == other.ptr; }
  ASMJIT_INLINE_CONSTEXPR bool operator!=(const T* other) const noexcept { return ptr != other; }
  ASMJIT_INLINE_CONSTEXPR bool operator!=(const SpanReverseIterator& other) const noexcept { return ptr != other.ptr; }
  ASMJIT_INLINE_CONSTEXPR SpanReverseIterator& operator++() noexcept { ptr--; return *this; }
  ASMJIT_INLINE_CONSTEXPR SpanReverseIterator operator++(int) noexcept { SpanReverseIterator prev(*this); ptr--; return prev; }
  ASMJIT_INLINE_CONSTEXPR T& operator*() const noexcept { return ptr[-1]; }
  ASMJIT_INLINE_CONSTEXPR T* operator->() const noexcept { return &ptr[-1]; }
  ASMJIT_INLINE_CONSTEXPR operator T*() const noexcept { return &ptr[-1]; }
};
template<typename T>
class SpanForwardIteratorAdaptor {
public:
  using iterator = SpanForwardIterator<T>;
  T* _begin {};
  T* _end {};
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR iterator begin() const noexcept { return iterator{_begin}; };
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR iterator end() const noexcept { return iterator{_end}; };
};
template<typename T>
class SpanReverseIteratorAdaptor {
public:
  using iterator = SpanReverseIterator<T>;
  T* _begin {};
  T* _end {};
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR iterator begin() const noexcept { return iterator{_end}; };
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR iterator end() const noexcept { return iterator{_begin}; };
};
template<typename T>
struct Span {
  T* _data {};
  size_t _size {};
  ASMJIT_INLINE_CONSTEXPR Span() noexcept = default;
  template<typename Other>
  ASMJIT_INLINE_CONSTEXPR Span(Span<Other> other) noexcept
    : _data(other.data()),
      _size(other.size()) {}
  ASMJIT_INLINE_CONSTEXPR Span(T* data, size_t size) noexcept
    : _data(data),
      _size(size) {}
  template<size_t N>
  static ASMJIT_INLINE_CONSTEXPR Span<T> from_array(T(&array)[N]) noexcept {
    return Span<T>(array, N);
  }
  template<typename OtherT>
  ASMJIT_INLINE_CONSTEXPR T& operator=(Span<OtherT> other) noexcept {
    _data = other._data;
    _size = other._size;
    return *this;
  }
  template<typename OtherT>
  [[nodiscard]]
  ASMJIT_INLINE bool operator==(const Span<OtherT>& other) noexcept { return equals(other); }
  template<typename OtherT>
  [[nodiscard]]
  ASMJIT_INLINE bool operator!=(const Span<OtherT>& other) noexcept { return !equals(other); }
  [[nodiscard]]
  ASMJIT_INLINE T& operator[](size_t index) noexcept {
    ASMJIT_ASSERT(index < _size);
    return _data[index];
  }
  [[nodiscard]]
  ASMJIT_INLINE const T& operator[](size_t index) const noexcept {
    ASMJIT_ASSERT(index < _size);
    return _data[index];
  }
  template<typename OtherT>
  [[nodiscard]]
  ASMJIT_INLINE bool equals(Span<OtherT> other) const noexcept {
    size_t size = _size;
    if (size != other.size()) {
      return false;
    }
    for (size_t i = 0u; i < size; i++) {
      if (_data[i] != other._data[i]) {
        return false;
      }
    }
    return true;
  }
  ASMJIT_INLINE void swap(Span<T>& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
  }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR T* data() noexcept { return _data; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T* data() const noexcept { return _data; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T* cdata() const noexcept { return _data; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR size_t size() const noexcept { return _size; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_empty() const noexcept { return _size == 0u; }
  [[nodiscard]]
  ASMJIT_INLINE T& first() noexcept { return operator[](0u); }
  [[nodiscard]]
  ASMJIT_INLINE const T& first() const noexcept { return operator[](0u); }
  [[nodiscard]]
  ASMJIT_INLINE T& last() noexcept { return operator[](_size - 1u); }
  [[nodiscard]]
  ASMJIT_INLINE const T& last() const noexcept { return operator[](_size - 1u); }
  template<typename Value>
  [[nodiscard]]
  ASMJIT_INLINE bool contains(Value&& value) const noexcept {
    size_t size = _size;
    for (size_t i = 0u; i < size; i++) {
      if (_data[i] == value) {
        return true;
      }
    }
    return false;
  }
  template<typename Value>
  [[nodiscard]]
  ASMJIT_INLINE size_t index_of(Value&& value) const noexcept {
    size_t size = _size;
    for (size_t i = 0u; i < size; i++) {
      if (_data[i] == value) {
        return i;
      }
    }
    return SIZE_MAX;
  }
  template<typename Value>
  [[nodiscard]]
  ASMJIT_INLINE size_t last_index_of(Value&& value) const noexcept {
    size_t i = _size;
    while (--i != SIZE_MAX) {
      if (_data[i] == value) {
        break;
      }
    }
    return i;
  }
  ASMJIT_INLINE_NODEBUG T* begin() const noexcept { return _data; }
  ASMJIT_INLINE_NODEBUG T* end() const noexcept { return _data + _size; }
  ASMJIT_INLINE_NODEBUG SpanForwardIteratorAdaptor<T> iterate() const noexcept { return SpanForwardIteratorAdaptor<T>{begin(), end()}; }
  ASMJIT_INLINE_NODEBUG SpanReverseIteratorAdaptor<T> iterate_reverse() const noexcept { return SpanReverseIteratorAdaptor<T>{begin(), end()}; }
};
ASMJIT_END_NAMESPACE
#endif
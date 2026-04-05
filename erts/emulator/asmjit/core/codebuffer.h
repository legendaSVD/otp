#ifndef ASMJIT_CORE_CODEBUFFER_H_INCLUDED
#define ASMJIT_CORE_CODEBUFFER_H_INCLUDED
#include <asmjit/core/globals.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
enum class CodeBufferFlags : uint32_t {
  kNone = 0,
  kIsExternal = 0x00000001u,
  kIsFixed = 0x00000002u
};
ASMJIT_DEFINE_ENUM_FLAGS(CodeBufferFlags)
struct CodeBuffer {
  uint8_t* _data;
  size_t _size;
  size_t _capacity;
  CodeBufferFlags _flags;
  [[nodiscard]]
  inline uint8_t& operator[](size_t index) noexcept {
    ASMJIT_ASSERT(index < _size);
    return _data[index];
  }
  [[nodiscard]]
  inline const uint8_t& operator[](size_t index) const noexcept {
    ASMJIT_ASSERT(index < _size);
    return _data[index];
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CodeBufferFlags flags() const noexcept { return _flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(CodeBufferFlags flag) const noexcept { return Support::test(_flags, flag); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_fixed() const noexcept { return has_flag(CodeBufferFlags::kIsFixed); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_external() const noexcept { return has_flag(CodeBufferFlags::kIsExternal); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_allocated() const noexcept { return _data != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return !_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t capacity() const noexcept { return _capacity; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* data() noexcept { return _data; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint8_t* data() const noexcept { return _data; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* begin() noexcept { return _data; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint8_t* begin() const noexcept { return _data; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* end() noexcept { return _data + _size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint8_t* end() const noexcept { return _data + _size; }
};
ASMJIT_END_NAMESPACE
#endif
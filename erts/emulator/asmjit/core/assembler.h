#ifndef ASMJIT_CORE_ASSEMBLER_H_INCLUDED
#define ASMJIT_CORE_ASSEMBLER_H_INCLUDED
#include <asmjit/core/codeholder.h>
#include <asmjit/core/emitter.h>
#include <asmjit/core/operand.h>
ASMJIT_BEGIN_NAMESPACE
class ASMJIT_VIRTAPI BaseAssembler : public BaseEmitter {
public:
  ASMJIT_NONCOPYABLE(BaseAssembler)
  using Base = BaseEmitter;
  Section* _section = nullptr;
  uint8_t* _buffer_data = nullptr;
  uint8_t* _buffer_end = nullptr;
  uint8_t* _buffer_ptr = nullptr;
  ASMJIT_API BaseAssembler() noexcept;
  ASMJIT_API ~BaseAssembler() noexcept override;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t buffer_capacity() const noexcept { return (size_t)(_buffer_end - _buffer_data); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t remaining_space() const noexcept { return (size_t)(_buffer_end - _buffer_ptr); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t offset() const noexcept { return (size_t)(_buffer_ptr - _buffer_data); }
  ASMJIT_API Error set_offset(size_t offset);
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* buffer_data() const noexcept { return _buffer_data; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* buffer_end() const noexcept { return _buffer_end; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* buffer_ptr() const noexcept { return _buffer_ptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Section* current_section() const noexcept { return _section; }
  ASMJIT_API Error section(Section* section) override;
  ASMJIT_API Label new_label() override;
  ASMJIT_API Label new_named_label(const char* name, size_t name_size = SIZE_MAX, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId) override;
  ASMJIT_API Error bind(const Label& label) override;
  ASMJIT_API Error embed(const void* data, size_t data_size) override;
  ASMJIT_API Error embed_data_array(TypeId type_id, const void* data, size_t item_count, size_t repeat_count = 1) override;
  ASMJIT_API Error embed_const_pool(const Label& label, const ConstPool& pool) override;
  ASMJIT_API Error embed_label(const Label& label, size_t data_size = 0) override;
  ASMJIT_API Error embed_label_delta(const Label& label, const Label& base, size_t data_size = 0) override;
  ASMJIT_API Error comment(const char* data, size_t size = SIZE_MAX) override;
  ASMJIT_INLINE Error comment(Span<const char> data) { return comment(data.data(), data.size()); }
  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_reinit(CodeHolder& code) noexcept override;
};
ASMJIT_END_NAMESPACE
#endif
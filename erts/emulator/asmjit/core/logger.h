#ifndef ASMJIT_CORE_LOGGING_H_INCLUDED
#define ASMJIT_CORE_LOGGING_H_INCLUDED
#include <asmjit/core/inst.h>
#include <asmjit/core/string.h>
#include <asmjit/core/formatter.h>
#ifndef ASMJIT_NO_LOGGING
ASMJIT_BEGIN_NAMESPACE
class ASMJIT_VIRTAPI Logger {
public:
  ASMJIT_BASE_CLASS(Logger)
  ASMJIT_NONCOPYABLE(Logger)
  FormatOptions _options;
  ASMJIT_API Logger() noexcept;
  ASMJIT_API virtual ~Logger() noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FormatOptions& options() noexcept { return _options; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FormatOptions& options() const noexcept { return _options; }
  ASMJIT_INLINE_NODEBUG void set_options(const FormatOptions& options) noexcept { _options = options; }
  ASMJIT_INLINE_NODEBUG void reset_options() noexcept { _options.reset(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FormatFlags flags() const noexcept { return _options.flags(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(FormatFlags flag) const noexcept { return _options.has_flag(flag); }
  ASMJIT_INLINE_NODEBUG void set_flags(FormatFlags flags) noexcept { _options.set_flags(flags); }
  ASMJIT_INLINE_NODEBUG void add_flags(FormatFlags flags) noexcept { _options.add_flags(flags); }
  ASMJIT_INLINE_NODEBUG void clear_flags(FormatFlags flags) noexcept { _options.clear_flags(flags); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t indentation(FormatIndentationGroup type) const noexcept { return _options.indentation(type); }
  ASMJIT_INLINE_NODEBUG void set_indentation(FormatIndentationGroup type, uint32_t n) noexcept { _options.set_indentation(type, n); }
  ASMJIT_INLINE_NODEBUG void reset_indentation(FormatIndentationGroup type) noexcept { _options.reset_indentation(type); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t padding(FormatPaddingGroup type) const noexcept { return _options.padding(type); }
  ASMJIT_INLINE_NODEBUG void set_padding(FormatPaddingGroup type, uint32_t n) noexcept { _options.set_padding(type, n); }
  ASMJIT_INLINE_NODEBUG void reset_padding(FormatPaddingGroup type) noexcept { _options.reset_padding(type); }
  ASMJIT_API virtual Error _log(const char* data, size_t size) noexcept;
  ASMJIT_INLINE_NODEBUG Error log(const char* data, size_t size = SIZE_MAX) noexcept { return _log(data, size); }
  ASMJIT_INLINE_NODEBUG Error log(const String& str) noexcept { return _log(str.data(), str.size()); }
  ASMJIT_API Error logf(const char* fmt, ...) noexcept;
  ASMJIT_API Error logv(const char* fmt, va_list ap) noexcept;
};
class ASMJIT_VIRTAPI FileLogger : public Logger {
public:
  ASMJIT_NONCOPYABLE(FileLogger)
  FILE* _file;
  ASMJIT_API FileLogger(FILE* file = nullptr) noexcept;
  ASMJIT_API ~FileLogger() noexcept override;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FILE* file() const noexcept { return _file; }
  ASMJIT_INLINE_NODEBUG void set_file(FILE* file) noexcept { _file = file; }
  ASMJIT_API Error _log(const char* data, size_t size = SIZE_MAX) noexcept override;
};
class ASMJIT_VIRTAPI StringLogger : public Logger {
public:
  ASMJIT_NONCOPYABLE(StringLogger)
  String _content;
  ASMJIT_API StringLogger() noexcept;
  ASMJIT_API ~StringLogger() noexcept override;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG String& content() noexcept { return _content; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const String& content() const noexcept { return _content; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* data() const noexcept { return _content.data(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t data_size() const noexcept { return _content.size(); }
  ASMJIT_INLINE_NODEBUG void clear() noexcept { _content.clear(); }
  ASMJIT_API Error _log(const char* data, size_t size = SIZE_MAX) noexcept override;
};
ASMJIT_END_NAMESPACE
#endif
#endif
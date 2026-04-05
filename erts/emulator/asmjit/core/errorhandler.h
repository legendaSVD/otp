#ifndef ASMJIT_CORE_ERRORHANDLER_H_INCLUDED
#define ASMJIT_CORE_ERRORHANDLER_H_INCLUDED
#include <asmjit/core/globals.h>
ASMJIT_BEGIN_NAMESPACE
class BaseEmitter;
class ASMJIT_VIRTAPI ErrorHandler {
public:
  ASMJIT_BASE_CLASS(ErrorHandler)
  ASMJIT_API ErrorHandler() noexcept;
  ASMJIT_API virtual ~ErrorHandler() noexcept;
  ASMJIT_API virtual void handle_error(Error err, const char* message, BaseEmitter* origin);
};
ASMJIT_END_NAMESPACE
#endif
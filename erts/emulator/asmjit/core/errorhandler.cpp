#include <asmjit/core/api-build_p.h>
#include <asmjit/core/errorhandler.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
ErrorHandler::ErrorHandler() noexcept {}
ErrorHandler::~ErrorHandler() noexcept {}
void ErrorHandler::handle_error(Error err, const char* message, BaseEmitter* origin) {
  Support::maybe_unused(err, message, origin);
}
ASMJIT_END_NAMESPACE
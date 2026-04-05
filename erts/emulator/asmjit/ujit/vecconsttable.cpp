#include <asmjit/core/api-build_p.h>
#include <asmjit/ujit/vecconsttable.h>
#if !defined(ASMJIT_NO_UJIT)
ASMJIT_BEGIN_SUB_NAMESPACE(ujit)
static constexpr VecConstTable vec_const_table_;
const VecConstTable vec_const_table = vec_const_table_;
ASMJIT_END_SUB_NAMESPACE
#endif
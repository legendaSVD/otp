#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "erl_lock_flags.h"
const char *erts_lock_flags_get_type_name(erts_lock_flags_t flags) {
    switch(flags & ERTS_LOCK_FLAGS_MASK_TYPE) {
        case ERTS_LOCK_FLAGS_TYPE_PROCLOCK:
            return "proclock";
        case ERTS_LOCK_FLAGS_TYPE_MUTEX:
            if(flags & ERTS_LOCK_FLAGS_PROPERTY_READ_WRITE) {
                return "rw_mutex";
            }
            return "mutex";
        case ERTS_LOCK_FLAGS_TYPE_SPINLOCK:
            if(flags & ERTS_LOCK_FLAGS_PROPERTY_READ_WRITE) {
                return "rw_spinlock";
            }
            return "spinlock";
        default:
            return "garbage";
    }
}
const char *erts_lock_options_get_short_desc(erts_lock_options_t options) {
    switch(options) {
        case ERTS_LOCK_OPTIONS_RDWR:
            return "rw";
        case ERTS_LOCK_OPTIONS_READ:
            return "r";
        case ERTS_LOCK_OPTIONS_WRITE:
            return "w";
        default:
            return "none";
    }
}
typedef int posix_errno_t;
enum efile_modes_t {
    EFILE_MODE_READ = (1 << 0),
    EFILE_MODE_WRITE = (1 << 1),
    EFILE_MODE_APPEND = (1 << 2),
    EFILE_MODE_EXCLUSIVE = (1 << 3),
    EFILE_MODE_SYNC = (1 << 4),
    EFILE_MODE_SKIP_TYPE_CHECK = (1 << 5),
    EFILE_MODE_DIRECTORY = (1 << 7),
    EFILE_MODE_FROM_ALREADY_OPEN_FD = (1 << 8),
    EFILE_MODE_READ_WRITE = EFILE_MODE_READ | EFILE_MODE_WRITE
};
enum efile_access_t {
    EFILE_ACCESS_NONE = 0,
    EFILE_ACCESS_READ = 1,
    EFILE_ACCESS_WRITE = 2,
    EFILE_ACCESS_READ_WRITE = EFILE_ACCESS_READ | EFILE_ACCESS_WRITE
};
enum efile_seek_t {
    EFILE_SEEK_BOF,
    EFILE_SEEK_CUR,
    EFILE_SEEK_EOF
};
enum efile_filetype_t {
    EFILE_FILETYPE_DEVICE,
    EFILE_FILETYPE_DIRECTORY,
    EFILE_FILETYPE_REGULAR,
    EFILE_FILETYPE_SYMLINK,
    EFILE_FILETYPE_OTHER
};
enum efile_advise_t {
    EFILE_ADVISE_NORMAL,
    EFILE_ADVISE_RANDOM,
    EFILE_ADVISE_SEQUENTIAL,
    EFILE_ADVISE_WILL_NEED,
    EFILE_ADVISE_DONT_NEED,
    EFILE_ADVISE_NO_REUSE
};
enum efile_state_t {
    EFILE_STATE_IDLE = 0,
    EFILE_STATE_BUSY = 1,
    EFILE_STATE_CLOSE_PENDING = 2,
    EFILE_STATE_CLOSED = 3
};
typedef struct {
    Sint64 size;
    Uint32 type;
    Uint32 access;
    Uint32 mode;
    Uint32 links;
    Uint32 major_device;
    Uint32 minor_device;
    Uint32 inode;
    Uint32 uid;
    Uint32 gid;
    Sint64 a_time;
    Sint64 m_time;
    Sint64 c_time;
} efile_fileinfo_t;
#define EFILE_MIN_FILETIME (-2145916800LL)
#define EFILE_MAX_FILETIME (253402300799LL)
#define EFILE_INIT_RESOURCE(__d, __modes) do { \
        erts_atomic32_init_acqb(&(__d)->state, EFILE_STATE_IDLE); \
        (__d)->posix_errno = 0; \
        (__d)->modes = __modes; \
    } while(0)
typedef struct {
    erts_atomic32_t state;
    posix_errno_t posix_errno;
    enum efile_modes_t modes;
    ErlNifMonitor monitor;
} efile_data_t;
typedef ErlNifBinary efile_path_t;
posix_errno_t efile_marshal_path(ErlNifEnv *env, ERL_NIF_TERM path, efile_path_t *result);
ERL_NIF_TERM efile_get_handle(ErlNifEnv *env, efile_data_t *d);
posix_errno_t efile_dup_handle(ErlNifEnv *env, efile_data_t *d, ErlNifEvent *handle);
Sint64 efile_readv(efile_data_t *d, SysIOVec *iov, int iovlen);
Sint64 efile_writev(efile_data_t *d, SysIOVec *iov, int iovlen);
Sint64 efile_preadv(efile_data_t *d, Sint64 offset, SysIOVec *iov, int iovlen);
Sint64 efile_pwritev(efile_data_t *d, Sint64 offset, SysIOVec *iov, int iovlen);
int efile_seek(efile_data_t *d, enum efile_seek_t seek, Sint64 offset, Sint64 *new_position);
int efile_sync(efile_data_t *d, int data_only);
int efile_advise(efile_data_t *d, Sint64 offset, Sint64 length, enum efile_advise_t advise);
int efile_allocate(efile_data_t *d, Sint64 offset, Sint64 length);
int efile_truncate(efile_data_t *d);
posix_errno_t efile_open(const efile_path_t *path, enum efile_modes_t modes,
        ErlNifResourceType *nif_type, efile_data_t **d);
posix_errno_t efile_from_fd(int fd,
                            ErlNifResourceType *nif_type,
                            efile_data_t **d);
int efile_close(efile_data_t *d, posix_errno_t *error);
posix_errno_t efile_read_info(const efile_path_t *path, int follow_link, efile_fileinfo_t *result);
posix_errno_t efile_read_handle_info(efile_data_t *d, efile_fileinfo_t *result);
posix_errno_t efile_set_time(const efile_path_t *path, Sint64 a_time, Sint64 m_time, Sint64 c_time);
posix_errno_t efile_set_permissions(const efile_path_t *path, Uint32 permissions);
posix_errno_t efile_set_owner(const efile_path_t *path, Sint32 owner, Sint32 group);
posix_errno_t efile_read_link(ErlNifEnv *env, const efile_path_t *path, ERL_NIF_TERM *result);
posix_errno_t efile_list_dir(ErlNifEnv *env, const efile_path_t *path, ERL_NIF_TERM *result);
posix_errno_t efile_rename(const efile_path_t *old_path, const efile_path_t *new_path);
posix_errno_t efile_make_hard_link(const efile_path_t *existing_path, const efile_path_t *new_path);
posix_errno_t efile_make_soft_link(const efile_path_t *existing_path, const efile_path_t *new_path);
posix_errno_t efile_make_dir(const efile_path_t *path);
posix_errno_t efile_del_file(const efile_path_t *path);
posix_errno_t efile_del_dir(const efile_path_t *path);
posix_errno_t efile_get_cwd(ErlNifEnv *env, ERL_NIF_TERM *result);
posix_errno_t efile_set_cwd(const efile_path_t *path);
posix_errno_t efile_get_device_cwd(ErlNifEnv *env, int device_index, ERL_NIF_TERM *result);
posix_errno_t efile_altname(ErlNifEnv *env, const efile_path_t *path, ERL_NIF_TERM *result);
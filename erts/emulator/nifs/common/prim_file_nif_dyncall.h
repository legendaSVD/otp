#ifndef PRIM_FILE_NIF_DYNCALL_H
#define PRIM_FILE_NIF_DYNCALL_H
#include "erl_nif.h"
enum prim_file_nif_dyncall_op {
    prim_file_nif_dyncall_dup,
};
struct prim_file_nif_dyncall {
    enum prim_file_nif_dyncall_op op;
    int result;
};
struct prim_file_nif_dyncall_dup {
    enum prim_file_nif_dyncall_op op;
    int result;
    ErlNifEvent handle;
};
#endif
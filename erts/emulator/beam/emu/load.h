#ifndef _EMU_LOAD_H
#define _EMU_LOAD_H
typedef struct {
    Uint pos;
    Uint offset;
    int packed;
} LabelPatch;
typedef struct {
    Uint value;
    Uint looprec_targeted;
    LabelPatch* patches;
    Uint num_patches;
    Uint num_allocated;
} Label;
typedef struct {
    Eterm term;
    ErlHeapFragment* heap_frags;
} Literal;
typedef struct literal_patch {
    Uint pos;
    struct literal_patch *next;
} LiteralPatch;
typedef struct string_patch {
    Uint pos;
    struct string_patch *next;
} StringPatch;
typedef struct lambda_patch {
    Uint pos;
    struct lambda_patch *next;
} LambdaPatch;
typedef struct {
    int pos;
    int loc;
} LineInstr;
struct LoaderState_ {
    Eterm group_leader;
    Eterm module;
    Eterm function;
    unsigned arity;
    int specific_op;
    BeamCodeHeader* code_hdr;
    BeamInstr* codev;
    int        codev_size;
    int ci;
    Label* labels;
    unsigned loaded_size;
    int may_load_nif;
    int on_load;
    BeamOp* genop;
    BifEntry **bif_imports;
    BeamInstr catches;
    BeamInstr *import_patches;
    LambdaPatch* lambda_patches;
    LiteralPatch* literal_patches;
    StringPatch* string_patches;
    LineInstr* line_instr;
    unsigned int current_li;
    unsigned int* func_line;
    SWord *lambda_literals;
    int otp_20_or_higher;
    Uint last_func_start;
    int function_number;
    int last_label;
    BeamOpAllocator op_allocator;
    BeamFile beam;
};
#endif
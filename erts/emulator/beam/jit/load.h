#ifndef _ASM_LOAD_H
#define _ASM_LOAD_H
typedef struct {
    Uint value;
    int looprec_targeted;
    int lambda_index;
} Label;
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
    const BeamCodeHeader *code_hdr;
    BeamCodeHeader *load_hdr;
    int codev_size;
    int ci;
    Label *labels;
    unsigned loaded_size;
    int may_load_nif;
    const ErtsCodeInfo *on_load;
    unsigned max_opcode;
    BeamOp *genop;
    BifEntry **bif_imports;
    LineInstr *line_instr;
    unsigned int current_li;
    unsigned int *func_line;
    void *coverage;
    byte *line_coverage_valid;
    unsigned int current_index;
    unsigned int *loc_index_to_cover_id;
    SWord *lambda_literals;
    void *ba;
    const void *executable_region;
    void *writable_region;
    int function_number;
    int last_label;
    BeamOpAllocator op_allocator;
    BeamFile beam;
};
#endif
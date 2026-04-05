#ifndef _BEAM_FILE_H
#define _BEAM_FILE_H
#define ERTS_BEAM_MAX_OPARGS 8
#include "sys.h"
#include "atom.h"
#include "beam_types.h"
#define CHECKSUM_SIZE 16
#define MakeIffId(a, b, c, d) \
  (((Uint) (a) << 24) | ((Uint) (b) << 16) | ((Uint) (c) << 8) | (Uint) (d))
typedef struct {
    Sint32 form_id;
    Sint32 size;
    const byte *data;
} IFF_File;
typedef struct {
    Uint32 iff_id;
    Sint32 size;
    const byte *data;
} IFF_Chunk;
int iff_init(const byte *data, size_t size, IFF_File *iff);
int iff_read_chunk(IFF_File *iff, Uint id, IFF_Chunk *chunk);
typedef struct {
    Sint32 count;
    Eterm *entries;
} BeamFile_AtomTable;
typedef struct {
    Sint32 function_count;
    Sint32 label_count;
    Sint32 max_opcode;
    const byte *data;
    Sint32 size;
} BeamFile_Code;
typedef struct {
    Eterm function;
    int arity;
    Sint32 label;
} BeamFile_ExportEntry;
typedef struct {
    Sint32 count;
    BeamFile_ExportEntry *entries;
} BeamFile_ExportTable;
typedef struct {
    Eterm module;
    Eterm function;
    int arity;
} BeamFile_ImportEntry;
typedef struct {
    Sint32 count;
    BeamFile_ImportEntry *entries;
} BeamFile_ImportTable;
typedef struct {
    Sint32 index;
    Sint32 old_uniq;
    Eterm function;
    Sint32 num_free;
    Sint32 arity;
    Sint32 label;
} BeamFile_LambdaEntry;
typedef struct {
    Sint count;
    BeamFile_LambdaEntry *entries;
} BeamFile_LambdaTable;
typedef struct {
    Sint32 location;
    Sint32 name_index;
} BeamFile_LineEntry;
typedef struct {
    Sint32 instruction_count;
    Sint32 flags;
    Sint32 name_count;
    Sint *names;
    Sint32 location_size;
    Sint32 item_count;
    BeamFile_LineEntry *items;
} BeamFile_LineTable;
enum beamfile_line_flags {
    BEAMFILE_EXECUTABLE_LINE = 1,
    BEAMFILE_FORCE_LINE_COUNTERS = 2
};
typedef struct {
    struct erl_heap_fragment *heap_fragments;
    Eterm value;
} BeamFile_LiteralEntry;
typedef struct {
    Sint heap_size;
    Sint32 allocated;
    Sint32 count;
    BeamFile_LiteralEntry *entries;
} BeamFile_LiteralTable;
typedef struct {
    Sint32 count;
    char fallback;
    BeamType *entries;
} BeamFile_TypeTable;
#define BEAMFILE_DEBUG_INFO_ENTRY_FRAME_SIZE 0
#define BEAMFILE_DEBUG_INFO_ENTRY_VAR_MAPPINGS 1
#define BEAMFILE_DEBUG_INFO_ENTRY_CALLS 2
#define BEAMFILE_FRAMESIZE_ENTRY (-2)
#define BEAMFILE_FRAMESIZE_NONE (-1)
typedef struct {
    Uint32 location_index;
    Sint32 frame_size;
    Sint32 num_vars;
    Sint32 num_calls_terms;
    Eterm *first;
} BeamFile_DebugItem;
typedef struct {
    Sint32 item_count;
    Sint32 term_count;
    BeamFile_DebugItem *items;
    Eterm *terms;
    byte *is_literal;
} BeamFile_DebugTable;
typedef struct {
    Eterm name;
    Sint def_literal;
    Sint32 num_fields;
} BeamFile_Record;
typedef struct {
    Sint32 record_count;
    Sint32 total_field_count;
    BeamFile_Record *records;
} BeamFile_RecordTable;
typedef struct {
    IFF_File iff;
    Eterm module;
    byte checksum[CHECKSUM_SIZE];
    BeamFile_AtomTable atoms;
    BeamFile_ImportTable imports;
    BeamFile_ExportTable exports;
#ifdef BEAMASM
    BeamFile_ExportTable locals;
#endif
    BeamFile_LambdaTable lambdas;
    BeamFile_LineTable lines;
    BeamFile_TypeTable types;
    BeamFile_DebugTable debug;
    BeamFile_RecordTable record;
    BeamFile_LiteralTable static_literals;
    BeamFile_LiteralTable dynamic_literals;
    BeamFile_Code code;
    struct {
        const byte *data;
        Sint32 size;
    } attributes, compile_info, strings;
} BeamFile;
enum beamfile_read_result {
    BEAMFILE_READ_SUCCESS,
    BEAMFILE_READ_CORRUPT_FILE_HEADER,
    BEAMFILE_READ_MISSING_ATOM_TABLE,
    BEAMFILE_READ_OBSOLETE_ATOM_TABLE,
    BEAMFILE_READ_CORRUPT_ATOM_TABLE,
    BEAMFILE_READ_MISSING_CODE_CHUNK,
    BEAMFILE_READ_CORRUPT_CODE_CHUNK,
    BEAMFILE_READ_MISSING_EXPORT_TABLE,
    BEAMFILE_READ_CORRUPT_EXPORT_TABLE,
    BEAMFILE_READ_MISSING_IMPORT_TABLE,
    BEAMFILE_READ_CORRUPT_IMPORT_TABLE,
    BEAMFILE_READ_CORRUPT_LOCALS_TABLE,
    BEAMFILE_READ_CORRUPT_LAMBDA_TABLE,
    BEAMFILE_READ_CORRUPT_LINE_TABLE,
    BEAMFILE_READ_CORRUPT_LITERAL_TABLE,
    BEAMFILE_READ_CORRUPT_TYPE_TABLE,
    BEAMFILE_READ_CORRUPT_DEBUG_TABLE,
    BEAMFILE_READ_CORRUPT_RECORD_TABLE,
};
typedef struct {
    UWord type;
    UWord val;
} BeamOpArg;
typedef struct beamop {
    int op;
    int arity;
    BeamOpArg def_args[ERTS_BEAM_MAX_OPARGS];
    BeamOpArg *a;
    struct beamop *next;
} BeamOp;
typedef struct beamop_block {
    BeamOp ops[32];
    struct beamop_block* next;
} BeamOpBlock;
typedef struct {
    BeamOpBlock *beamop_blocks;
    BeamOp *free;
} BeamOpAllocator;
#include "erl_process.h"
#include "erl_message.h"
void beamfile_init(void);
enum beamfile_read_result
beamfile_read(const byte *data, size_t size, BeamFile *beam);
void beamfile_free(BeamFile *beam);
Sint beamfile_add_literal(BeamFile *beam, Eterm term, int deduplicate);
Eterm beamfile_get_literal(const BeamFile *beam, Sint index);
void beamfile_move_literals(BeamFile *beam, Eterm **hpp, ErlOffHeap *oh);
void beamopallocator_init(BeamOpAllocator *allocator);
void beamopallocator_dtor(BeamOpAllocator *allocator);
ERTS_GLB_INLINE
BeamOp *beamopallocator_new_op(BeamOpAllocator *allocator);
ERTS_GLB_INLINE
void beamopallocator_free_op(BeamOpAllocator *allocator, BeamOp *op);
void beamopallocator_expand__(BeamOpAllocator *allocator);
typedef struct BeamCodeReader__ BeamCodeReader;
BeamCodeReader *beamfile_get_code(BeamFile *beam, BeamOpAllocator *op_alloc);
int beamcodereader_next(BeamCodeReader *code_reader, BeamOp **op);
void beamcodereader_close(BeamCodeReader *code_reader);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
#ifdef DEBUG
# define GARBAGE__ 0xCC
# define DEBUG_INIT_BEAMOP(Dst) sys_memset(Dst, GARBAGE__, sizeof(BeamOp))
#else
# define DEBUG_INIT_BEAMOP(Dst)
#endif
ERTS_GLB_INLINE
BeamOp *beamopallocator_new_op(BeamOpAllocator *allocator) {
    BeamOp *res;
    if (allocator->free == NULL) {
        beamopallocator_expand__(allocator);
    }
    res = allocator->free;
    allocator->free = res->next;
    DEBUG_INIT_BEAMOP(res);
    res->a = res->def_args;
    return res;
}
ERTS_GLB_INLINE
void beamopallocator_free_op(BeamOpAllocator *allocator, BeamOp *op) {
    if (op->a != op->def_args) {
        erts_free(ERTS_ALC_T_LOADER_TMP, op->a);
    }
    op->next = allocator->free;
    allocator->free = op;
}
#endif
#endif
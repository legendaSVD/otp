#ifndef PCRE2_CODE_UNIT_WIDTH
#error PCRE2_CODE_UNIT_WIDTH must be defined
#endif
#undef ACROSSCHAR
#undef BACKCHAR
#undef BYTES2CU
#undef CHMAX_255
#undef CU2BYTES
#undef FORWARDCHAR
#undef FORWARDCHARTEST
#undef GET
#undef GET2
#undef GETCHAR
#undef GETCHARINC
#undef GETCHARINCTEST
#undef GETCHARLEN
#undef GETCHARLENTEST
#undef GETCHARTEST
#undef GET_EXTRALEN
#undef HAS_EXTRALEN
#undef IMM2_SIZE
#undef MAX_255
#undef MAX_MARK
#undef MAX_PATTERN_SIZE
#undef MAX_UTF_SINGLE_CU
#undef NOT_FIRSTCU
#undef PUT
#undef PUT2
#undef PUT2INC
#undef PUTCHAR
#undef PUTINC
#undef TABLE_GET
#if PCRE2_CODE_UNIT_WIDTH != 0
#ifndef CONFIGURED_LINK_SIZE
#if LINK_SIZE == 2
#define CONFIGURED_LINK_SIZE 2
#elif LINK_SIZE == 3
#define CONFIGURED_LINK_SIZE 3
#elif LINK_SIZE == 4
#define CONFIGURED_LINK_SIZE 4
#else
#error LINK_SIZE must be 2, 3, or 4
#endif
#endif
#if PCRE2_CODE_UNIT_WIDTH == 8
#if CONFIGURED_LINK_SIZE == 2
#define PUT(a,n,d)   \
  (a[n] = (PCRE2_UCHAR)((d) >> 8)), \
  (a[(n)+1] = (PCRE2_UCHAR)((d) & 255))
#define GET(a,n) \
  (unsigned int)(((a)[n] << 8) | (a)[(n)+1])
#define MAX_PATTERN_SIZE (1 << 16)
#elif CONFIGURED_LINK_SIZE == 3
#define PUT(a,n,d)       \
  (a[n] = (PCRE2_UCHAR)((d) >> 16)),    \
  (a[(n)+1] = (PCRE2_UCHAR)((d) >> 8)), \
  (a[(n)+2] = (PCRE2_UCHAR)((d) & 255))
#define GET(a,n) \
  (unsigned int)(((a)[n] << 16) | ((a)[(n)+1] << 8) | (a)[(n)+2])
#define MAX_PATTERN_SIZE (1 << 24)
#elif CONFIGURED_LINK_SIZE == 4
#define PUT(a,n,d)        \
  (a[n] = (PCRE2_UCHAR)((d) >> 24)),     \
  (a[(n)+1] = (PCRE2_UCHAR)((d) >> 16)), \
  (a[(n)+2] = (PCRE2_UCHAR)((d) >> 8)),  \
  (a[(n)+3] = (PCRE2_UCHAR)((d) & 255))
#define GET(a,n) \
  (unsigned int)(((a)[n] << 24) | ((a)[(n)+1] << 16) | ((a)[(n)+2] << 8) | (a)[(n)+3])
#define MAX_PATTERN_SIZE (1 << 30)
#endif
#elif PCRE2_CODE_UNIT_WIDTH == 16
#if CONFIGURED_LINK_SIZE == 2
#undef LINK_SIZE
#define LINK_SIZE 1
#define PUT(a,n,d)   \
  (a[n] = (PCRE2_UCHAR)(d))
#define GET(a,n) \
  (a[n])
#define MAX_PATTERN_SIZE (1 << 16)
#elif CONFIGURED_LINK_SIZE == 3 || CONFIGURED_LINK_SIZE == 4
#undef LINK_SIZE
#define LINK_SIZE 2
#define PUT(a,n,d)   \
  (a[n] = (PCRE2_UCHAR)((d) >> 16)), \
  (a[(n)+1] = (PCRE2_UCHAR)((d) & 65535))
#define GET(a,n) \
  (unsigned int)(((a)[n] << 16) | (a)[(n)+1])
#define MAX_PATTERN_SIZE (1 << 30)
#endif
#elif PCRE2_CODE_UNIT_WIDTH == 32
#undef LINK_SIZE
#define LINK_SIZE 1
#define PUT(a,n,d)   \
  (a[n] = (d))
#define GET(a,n) \
  (a[n])
#define MAX_PATTERN_SIZE (1 << 30)
#else
#error Unsupported compiling mode
#endif
#if PCRE2_CODE_UNIT_WIDTH == 8
#define IMM2_SIZE 2
#define GET2(a,n) (unsigned int)(((a)[n] << 8) | (a)[(n)+1])
#define PUT2(a,n,d) a[n] = (d) >> 8, a[(n)+1] = (d) & 255
#elif PCRE2_CODE_UNIT_WIDTH == 16 || PCRE2_CODE_UNIT_WIDTH == 32
#define IMM2_SIZE 1
#define GET2(a,n) a[n]
#define PUT2(a,n,d) a[n] = d
#endif
#if PCRE2_CODE_UNIT_WIDTH == 8
#define MAX_255(c) TRUE
#define MAX_MARK ((1u << 8) - 1)
#define TABLE_GET(c, table, default) ((table)[c])
#ifdef SUPPORT_UNICODE
#define SUPPORT_WIDE_CHARS
#define CHMAX_255(c) ((c) <= 255u)
#else
#define CHMAX_255(c) TRUE
#endif
#elif PCRE2_CODE_UNIT_WIDTH == 16 || PCRE2_CODE_UNIT_WIDTH == 32
#define CHMAX_255(c) ((c) <= 255u)
#define MAX_255(c) ((c) <= 255u)
#define MAX_MARK ((1u << 16) - 1)
#define SUPPORT_WIDE_CHARS
#define TABLE_GET(c, table, default) (MAX_255(c)? ((table)[c]):(default))
#endif
#define UCHAR21(eptr)        (*(eptr))
#define UCHAR21TEST(eptr)    (*(eptr))
#define UCHAR21INC(eptr)     (*(eptr)++)
#define UCHAR21INCTEST(eptr) (*(eptr)++)
#ifndef SUPPORT_UNICODE
#define GETCHAR(c, eptr) c = *eptr;
#define GETCHARTEST(c, eptr) c = *eptr;
#define GETCHARINC(c, eptr) c = *eptr++;
#define GETCHARINCTEST(c, eptr) c = *eptr++;
#define GETCHARLEN(c, eptr, len) c = *eptr;
#define PUTCHAR(c, p) (*p = c, 1)
#else
#if PCRE2_CODE_UNIT_WIDTH == 8
#define MAYBE_UTF_MULTI
#define MAX_UTF_SINGLE_CU 127
#define HAS_EXTRALEN(c) HASUTF8EXTRALEN(c)
#define GET_EXTRALEN(c) (PRIV(utf8_table4)[(c) & 0x3fu])
#define NOT_FIRSTCU(c) (((c) & 0xc0u) == 0x80u)
#define GETCHAR(c, eptr) \
  c = *eptr; \
  if (c >= 0xc0u) GETUTF8(c, eptr);
#define GETCHARTEST(c, eptr) \
  c = *eptr; \
  if (utf && c >= 0xc0u) GETUTF8(c, eptr);
#define GETCHARINC(c, eptr) \
  c = *eptr++; \
  if (c >= 0xc0u) GETUTF8INC(c, eptr);
#define GETCHARINCTEST(c, eptr) \
  c = *eptr++; \
  if (utf && c >= 0xc0u) GETUTF8INC(c, eptr);
#define GETCHARLEN(c, eptr, len) \
  c = *eptr; \
  if (c >= 0xc0u) GETUTF8LEN(c, eptr, len);
#define GETCHARLENTEST(c, eptr, len) \
  c = *eptr; \
  if (utf && c >= 0xc0u) GETUTF8LEN(c, eptr, len);
#define BACKCHAR(eptr) while((*eptr & 0xc0u) == 0x80u) eptr--
#define FORWARDCHAR(eptr) while((*eptr & 0xc0u) == 0x80u) eptr++
#define FORWARDCHARTEST(eptr,end) while(eptr < end && (*eptr & 0xc0u) == 0x80u) eptr++
#define ACROSSCHAR(condition, eptr, action) \
  while((condition) && ((*eptr) & 0xc0u) == 0x80u) action
#define PUTCHAR(c, p) ((utf && c > MAX_UTF_SINGLE_CU)? \
  PRIV(ord2utf)(c,p) : (*p = c, 1))
#elif PCRE2_CODE_UNIT_WIDTH == 16
#define MAYBE_UTF_MULTI
#define MAX_UTF_SINGLE_CU 65535
#define HAS_EXTRALEN(c) (((c) & 0xfc00u) == 0xd800u)
#define GET_EXTRALEN(c) 1
#define NOT_FIRSTCU(c) (((c) & 0xfc00u) == 0xdc00u)
#define GETUTF16(c, eptr) \
   { c = (((c & 0x3ffu) << 10) | (eptr[1] & 0x3ffu)) + 0x10000u; }
#define GETCHAR(c, eptr) \
  c = *eptr; \
  if ((c & 0xfc00u) == 0xd800u) GETUTF16(c, eptr);
#define GETCHARTEST(c, eptr) \
  c = *eptr; \
  if (utf && (c & 0xfc00u) == 0xd800u) GETUTF16(c, eptr);
#define GETUTF16INC(c, eptr) \
   { c = (((c & 0x3ffu) << 10) | (*eptr++ & 0x3ffu)) + 0x10000u; }
#define GETCHARINC(c, eptr) \
  c = *eptr++; \
  if ((c & 0xfc00u) == 0xd800u) GETUTF16INC(c, eptr);
#define GETCHARINCTEST(c, eptr) \
  c = *eptr++; \
  if (utf && (c & 0xfc00u) == 0xd800u) GETUTF16INC(c, eptr);
#define GETUTF16LEN(c, eptr, len) \
   { c = (((c & 0x3ffu) << 10) | (eptr[1] & 0x3ffu)) + 0x10000u; len++; }
#define GETCHARLEN(c, eptr, len) \
  c = *eptr; \
  if ((c & 0xfc00u) == 0xd800u) GETUTF16LEN(c, eptr, len);
#define GETCHARLENTEST(c, eptr, len) \
  c = *eptr; \
  if (utf && (c & 0xfc00u) == 0xd800u) GETUTF16LEN(c, eptr, len);
#define BACKCHAR(eptr) if ((*eptr & 0xfc00u) == 0xdc00u) eptr--
#define FORWARDCHAR(eptr) if ((*eptr & 0xfc00u) == 0xdc00u) eptr++
#define FORWARDCHARTEST(eptr,end) if (eptr < end && (*eptr & 0xfc00u) == 0xdc00u) eptr++
#define ACROSSCHAR(condition, eptr, action) \
  if ((condition) && ((*eptr) & 0xfc00u) == 0xdc00u) action
#define PUTCHAR(c, p) ((utf && c > MAX_UTF_SINGLE_CU)? \
  PRIV(ord2utf)(c,p) : (*p = c, 1))
#elif PCRE2_CODE_UNIT_WIDTH == 32
#define MAX_UTF_SINGLE_CU (0x10ffffu)
#define HAS_EXTRALEN(c) (0)
#define GET_EXTRALEN(c) (0)
#define NOT_FIRSTCU(c) (0)
#define GETCHAR(c, eptr) \
  c = *(eptr);
#define GETCHARTEST(c, eptr) \
  c = *(eptr);
#define GETCHARINC(c, eptr) \
  c = *((eptr)++);
#define GETCHARINCTEST(c, eptr) \
  c = *((eptr)++);
#define GETCHARLEN(c, eptr, len) \
  GETCHAR(c, eptr)
#define GETCHARLENTEST(c, eptr, len) \
  GETCHARTEST(c, eptr)
#define BACKCHAR(eptr) do { } while (0)
#define FORWARDCHAR(eptr) do { } while (0)
#define FORWARDCHARTEST(eptr,end) do { } while (0)
#define ACROSSCHAR(condition, eptr, action) do { } while (0)
#define PUTCHAR(c, p) (*p = c, 1)
#endif
#endif
#define CU2BYTES(x)     ((x)*((PCRE2_CODE_UNIT_WIDTH/8)))
#define BYTES2CU(x)     ((x)/((PCRE2_CODE_UNIT_WIDTH/8)))
#define PUTINC(a,n,d)   PUT(a,n,d), a += LINK_SIZE
#define PUT2INC(a,n,d)  PUT2(a,n,d), a += IMM2_SIZE
#endif
#if PCRE2_CODE_UNIT_WIDTH == 8 && !defined PCRE2_INTMODEDEP_IDEMPOTENT_GUARD_8
#define PCRE2_INTMODEDEP_IDEMPOTENT_GUARD_8
#define PCRE2_INTMODEDEP_CAN_DEFINE
#endif
#if PCRE2_CODE_UNIT_WIDTH == 16 && !defined PCRE2_INTMODEDEP_IDEMPOTENT_GUARD_16
#define PCRE2_INTMODEDEP_IDEMPOTENT_GUARD_16
#define PCRE2_INTMODEDEP_CAN_DEFINE
#endif
#if PCRE2_CODE_UNIT_WIDTH == 32 && !defined PCRE2_INTMODEDEP_IDEMPOTENT_GUARD_32
#define PCRE2_INTMODEDEP_IDEMPOTENT_GUARD_32
#define PCRE2_INTMODEDEP_CAN_DEFINE
#endif
#ifdef PCRE2_INTMODEDEP_CAN_DEFINE
#undef PCRE2_INTMODEDEP_CAN_DEFINE
typedef struct pcre2_real_general_context {
  pcre2_memctl memctl;
} pcre2_real_general_context;
typedef struct pcre2_real_compile_context {
  pcre2_memctl memctl;
  int (*stack_guard)(uint32_t, void *);
  void *stack_guard_data;
  const uint8_t *tables;
  PCRE2_SIZE max_pattern_length;
  PCRE2_SIZE max_pattern_compiled_length;
  uint16_t bsr_convention;
  uint16_t newline_convention;
  uint32_t parens_nest_limit;
  uint32_t extra_options;
  uint32_t max_varlookbehind;
  uint32_t optimization_flags;
} pcre2_real_compile_context;
typedef struct pcre2_real_match_context {
  pcre2_memctl memctl;
#ifdef SUPPORT_JIT
  pcre2_jit_callback jit_callback;
  void *jit_callback_data;
#endif
  int        (*callout)(pcre2_callout_block *, void *);
  void        *callout_data;
  int        (*substitute_callout)(pcre2_substitute_callout_block *, void *);
  void        *substitute_callout_data;
  PCRE2_SIZE (*substitute_case_callout)(PCRE2_SPTR, PCRE2_SIZE, PCRE2_UCHAR *,
                                        PCRE2_SIZE, int, void *);
  void        *substitute_case_callout_data;
  PCRE2_SIZE offset_limit;
  uint32_t heap_limit;
  uint32_t match_limit;
  uint32_t depth_limit;
} pcre2_real_match_context;
typedef struct pcre2_real_convert_context {
  pcre2_memctl memctl;
  uint32_t glob_separator;
  uint32_t glob_escape;
} pcre2_real_convert_context;
#undef  CODE_BLOCKSIZE_TYPE
#define CODE_BLOCKSIZE_TYPE PCRE2_SIZE
#undef  LOOKBEHIND_MAX
#define LOOKBEHIND_MAX ((int)UINT16_MAX)
typedef struct pcre2_real_code {
  pcre2_memctl memctl;
  const uint8_t *tables;
  void    *executable_jit;
  uint8_t  start_bitmap[32];
  CODE_BLOCKSIZE_TYPE blocksize;
  CODE_BLOCKSIZE_TYPE code_start;
  uint32_t magic_number;
  uint32_t compile_options;
  uint32_t overall_options;
  uint32_t extra_options;
  uint32_t flags;
  uint32_t limit_heap;
  uint32_t limit_match;
  uint32_t limit_depth;
  uint32_t first_codeunit;
  uint32_t last_codeunit;
  uint16_t bsr_convention;
  uint16_t newline_convention;
  uint16_t max_lookbehind;
  uint16_t minlength;
  uint16_t top_bracket;
  uint16_t top_backref;
  uint16_t name_entry_size;
  uint16_t name_count;
  uint32_t optimization_flags;
} pcre2_real_code;
struct heapframe;
typedef struct pcre2_real_match_data {
  pcre2_memctl     memctl;
  const pcre2_real_code *code;
  PCRE2_SPTR       subject;
  PCRE2_SPTR       mark;
  struct heapframe *heapframes;
  PCRE2_SIZE       heapframes_size;
  PCRE2_SIZE       subject_length;
  PCRE2_SIZE       start_offset;
  PCRE2_SIZE       leftchar;
  PCRE2_SIZE       rightchar;
  PCRE2_SIZE       startchar;
  uint8_t          matchedby;
  uint8_t          flags;
  uint16_t         oveccount;
  uint32_t         options;
  int              rc;
#if defined(ERLANG_INTEGRATION)
  int32_t loops_left;
  void **restart_data;
  int restart_flags;
#endif
  PCRE2_SIZE       ovector[131072];
} pcre2_real_match_data;
#ifndef PCRE2_PCRE2TEST
typedef struct recurse_check {
  struct recurse_check *prev;
  PCRE2_SPTR group;
} recurse_check;
typedef struct parsed_recurse_check {
  struct parsed_recurse_check *prev;
  uint32_t *groupptr;
} parsed_recurse_check;
typedef struct recurse_cache {
  PCRE2_SPTR group;
  int groupnumber;
} recurse_cache;
typedef struct branch_chain {
  struct branch_chain *outer;
  PCRE2_UCHAR *current_branch;
} branch_chain;
typedef struct named_group {
  PCRE2_SPTR   name;
  uint32_t     number;
  uint16_t     length;
  uint16_t     hash_dup;
} named_group;
typedef struct compile_data {
  struct compile_data *next;
#ifdef PCRE2_DEBUG
  uint8_t type;
#endif
} compile_data;
typedef struct class_ranges {
  compile_data header;
  size_t char_lists_size;
  size_t char_lists_start;
  uint16_t range_list_size;
  uint16_t char_lists_types;
} class_ranges;
typedef struct recurse_arguments {
  compile_data header;
  size_t size;
  size_t skip_size;
} recurse_arguments;
typedef union class_bits_storage {
  uint8_t classbits[32];
  uint32_t classwords[8];
} class_bits_storage;
typedef struct compile_block {
  pcre2_real_compile_context *cx;
  const uint8_t *lcc;
  const uint8_t *fcc;
  const uint8_t *cbits;
  const uint8_t *ctypes;
  PCRE2_UCHAR *start_workspace;
  PCRE2_UCHAR *start_code;
  PCRE2_SPTR start_pattern;
  PCRE2_SPTR end_pattern;
  PCRE2_UCHAR *name_table;
  PCRE2_SIZE workspace_size;
  PCRE2_SIZE small_ref_offset[10];
  PCRE2_SIZE erroroffset;
  class_bits_storage classbits;
  uint16_t names_found;
  uint16_t name_entry_size;
  uint16_t parens_depth;
  uint16_t assert_depth;
  named_group *named_groups;
  uint32_t named_group_list_size;
  uint32_t external_options;
  uint32_t external_flags;
  uint32_t bracount;
  uint32_t lastcapture;
  uint32_t *parsed_pattern;
  uint32_t *parsed_pattern_end;
  uint32_t *groupinfo;
  uint32_t top_backref;
  uint32_t backref_map;
  uint32_t nltype;
  uint32_t nllen;
  PCRE2_UCHAR nl[4];
  uint8_t class_op_used[ECLASS_NEST_LIMIT];
  uint32_t req_varyopt;
  uint32_t max_varlookbehind;
  int  max_lookbehind;
  BOOL had_accept;
  BOOL had_pruneorskip;
  BOOL had_recurse;
  BOOL dupnames;
  compile_data *first_data;
  compile_data *last_data;
#ifdef SUPPORT_WIDE_CHARS
  size_t char_lists_size;
#endif
} compile_block;
typedef struct pcre2_real_jit_stack {
  pcre2_memctl memctl;
  void* stack;
} pcre2_real_jit_stack;
typedef struct dfa_recursion_info {
  struct dfa_recursion_info *prevrec;
  PCRE2_SPTR subject_position;
  PCRE2_SPTR last_used_ptr;
  uint32_t group_num;
} dfa_recursion_info;
typedef struct heapframe {
  PCRE2_SPTR ecode;
  PCRE2_SPTR temp_sptr[2];
  PCRE2_SIZE length;
  PCRE2_SIZE back_frame;
  PCRE2_SIZE temp_size;
  uint32_t rdepth;
  uint32_t group_frame_type;
  uint32_t temp_32[4];
  #ifdef ERLANG_INTEGRATION
  uint32_t return_id;
  #else
  uint8_t return_id;
  #endif
  uint8_t op;
#if PCRE2_CODE_UNIT_WIDTH == 8
  PCRE2_UCHAR occu[6];
#elif PCRE2_CODE_UNIT_WIDTH == 16
  PCRE2_UCHAR occu[2];
  uint8_t unused[2];
#else
  uint8_t unused[2];
  PCRE2_UCHAR occu[1];
#endif
  PCRE2_SPTR eptr;
  PCRE2_SPTR start_match;
  PCRE2_SPTR mark;
  PCRE2_SPTR recurse_last_used;
  uint32_t current_recurse;
  uint32_t capture_last;
  PCRE2_SIZE last_group_offset;
  PCRE2_SIZE offset_top;
  PCRE2_SIZE ovector[131072];
} heapframe;
typedef struct match_local_variable_store{
  PCRE2_SPTR start_ecode;
  uint16_t top_bracket;
  heapframe *frames_top;
  heapframe *assert_accept_frame;
  PCRE2_SIZE frame_copy_size;
  PCRE2_SPTR branch_end;
  PCRE2_SPTR branch_start;
  PCRE2_SPTR bracode;
  PCRE2_SIZE offset;
  PCRE2_SIZE length;
  int rrc;
  #ifdef SUPPORT_UNICODE
  int proptype;
  BOOL utf;
  BOOL ucp;
  BOOL notmatch;
  #endif
  uint32_t i;
  uint32_t fc;
  uint32_t number;
  uint32_t reptype;
  uint32_t group_frame_type;
  BOOL samelengths;
  BOOL condition;
  BOOL cur_is_word;
  BOOL prev_is_word;
  int rgb;
} match_local_variable_store;
STATIC_ASSERT((sizeof(heapframe) % sizeof(PCRE2_SIZE)) == 0, heapframe_size);
typedef struct heapframe_align {
  char unalign;
  heapframe frame;
} heapframe_align;
#define HEAPFRAME_ALIGNMENT offsetof(heapframe_align, frame)
typedef struct match_block {
#if defined(ERLANG_INTEGRATION)
  void *state_save;
#endif
  pcre2_memctl memctl;
  uint32_t heap_limit;
  uint32_t match_limit;
  uint32_t match_limit_depth;
  uint32_t match_call_count;
  BOOL hitend;
  BOOL hasthen;
  BOOL hasbsk;
  BOOL allowemptypartial;
  BOOL allowlookaroundbsk;
  const uint8_t *lcc;
  const uint8_t *fcc;
  const uint8_t *ctypes;
  PCRE2_SIZE start_offset;
  PCRE2_SIZE end_offset_top;
  uint16_t partial;
  uint16_t bsr_convention;
  uint16_t name_count;
  uint16_t name_entry_size;
  PCRE2_SPTR name_table;
  PCRE2_SPTR start_code;
  PCRE2_SPTR start_subject;
  PCRE2_SPTR check_subject;
  PCRE2_SPTR end_subject;
  PCRE2_SPTR true_end_subject;
  PCRE2_SPTR end_match_ptr;
  PCRE2_SPTR start_used_ptr;
  PCRE2_SPTR last_used_ptr;
  PCRE2_SPTR mark;
  PCRE2_SPTR nomatch_mark;
  PCRE2_SPTR verb_ecode_ptr;
  PCRE2_SPTR verb_skip_ptr;
  uint32_t verb_current_recurse;
  uint32_t moptions;
  uint32_t poptions;
  uint32_t skip_arg_count;
  uint32_t ignore_skip_arg;
  uint32_t nltype;
  uint32_t nllen;
  PCRE2_UCHAR nl[4];
  pcre2_callout_block *cb;
  void  *callout_data;
  int (*callout)(pcre2_callout_block *,void *);
} match_block;
typedef struct dfa_match_block {
  pcre2_memctl memctl;
  PCRE2_SPTR start_code;
  PCRE2_SPTR start_subject ;
  PCRE2_SPTR end_subject;
  PCRE2_SPTR start_used_ptr;
  PCRE2_SPTR last_used_ptr;
  const uint8_t *tables;
  PCRE2_SIZE start_offset;
  uint32_t heap_limit;
  PCRE2_SIZE heap_used;
  uint32_t match_limit;
  uint32_t match_limit_depth;
  uint32_t match_call_count;
  uint32_t moptions;
  uint32_t poptions;
  uint32_t nltype;
  uint32_t nllen;
  BOOL allowemptypartial;
  PCRE2_UCHAR nl[4];
  uint16_t bsr_convention;
  pcre2_callout_block *cb;
  void *callout_data;
  int (*callout)(pcre2_callout_block *,void *);
  dfa_recursion_info *recursive;
} dfa_match_block;
#endif
#endif
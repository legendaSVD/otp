#include "pcre2_compile.h"
#define NLBLOCK cb
#define PSSTART start_pattern
#define PSEND   end_pattern
#if 0
#ifdef EBCDIC
#define PRINTABLE(c) ((c) >= 64 && (c) < 255)
#else
#define PRINTABLE(c) ((c) >= 32 && (c) < 127)
#endif
#define CHAR_OUTPUT(c)      (c)
#define CHAR_OUTPUT_HEX(c)  (c)
#define CHAR_INPUT(c)       (c)
#define CHAR_INPUT_HEX(c)   (c)
#include "pcre2_printint_inc.h"
#undef PRINTABLE
#undef CHAR_OUTPUT
#undef CHAR_OUTPUT_HEX
#undef CHAR_INPUT
#define DEBUG_CALL_PRINTINT
#endif
#if PCRE2_CODE_UNIT_WIDTH == 8
#define STRING_UTFn_RIGHTPAR     STRING_UTF8_RIGHTPAR, 5
#define XDIGIT(c)                xdigitab[c]
#else
#define XDIGIT(c)                (MAX_255(c)? xdigitab[c] : 0xff)
#if PCRE2_CODE_UNIT_WIDTH == 16
#define STRING_UTFn_RIGHTPAR     STRING_UTF16_RIGHTPAR, 6
#else
#define STRING_UTFn_RIGHTPAR     STRING_UTF32_RIGHTPAR, 6
#endif
#endif
static int
  compile_regex(uint32_t, uint32_t, PCRE2_UCHAR **, uint32_t **, int *,
    uint32_t, uint32_t *, uint32_t *, uint32_t *, uint32_t *, branch_chain *,
    open_capitem *, compile_block *, PCRE2_SIZE *);
static int
  get_branchlength(uint32_t **, int *, int *, int *, parsed_recurse_check *,
    compile_block *);
static BOOL
  set_lookbehind_lengths(uint32_t **, int *, int *, parsed_recurse_check *,
    compile_block *);
static int
  check_lookbehinds(uint32_t *, uint32_t **, parsed_recurse_check *,
    compile_block *, int *);
#define MAX_GROUP_NUMBER   65535u
#define MAX_REPEAT_COUNT   65535u
#define REPEAT_UNLIMITED   (MAX_REPEAT_COUNT+1)
#define COMPILE_WORK_SIZE (3000*LINK_SIZE)
#define C16_WORK_SIZE \
  ((COMPILE_WORK_SIZE * sizeof(PCRE2_UCHAR))/sizeof(uint16_t))
#define GROUPINFO_DEFAULT_SIZE 256
#define WORK_SIZE_SAFETY_MARGIN (100)
#define NAMED_GROUP_LIST_SIZE  20
#define PARSED_PATTERN_DEFAULT_SIZE 1024
#define OFLOW_MAX (INT_MAX - 20)
static unsigned char meta_extra_lengths[] = {
  0,
  0,
  0,
  0,
  1+SIZEOFFSET,
  1,
  3,
  3+SIZEOFFSET,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  SIZEOFFSET,
  1+SIZEOFFSET,
  1+SIZEOFFSET,
  1+SIZEOFFSET,
  1+SIZEOFFSET,
  3,
  SIZEOFFSET,
  0,
  1,
  1,
  0,
  0,
  0,
  0,
  0,
  2,
  1,
  1,
  0,
  0,
  SIZEOFFSET,
  1+SIZEOFFSET,
  0,
  0,
  0,
  SIZEOFFSET,
  SIZEOFFSET,
  0,
  SIZEOFFSET,
  1,
  0,
  0,
  0,
  1,
  0,
  1,
  0,
  1,
  0,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  2,
  2,
  2,
  0,
  0,
  0,
  0,
  0
};
enum { PSKIP_ALT, PSKIP_CLASS, PSKIP_KET };
#define REQ_UNSET     0xffffffffu
#define REQ_NONE      0xfffffffeu
#define REQ_CASELESS  0x00000001u
#define REQ_VARY      0x00000002u
#define GI_SET_FIXED_LENGTH    0x80000000u
#define GI_NOT_FIXED_LENGTH    0x40000000u
#define GI_FIXED_LENGTH_MASK   0x0000ffffu
#define IS_DIGIT(x) ((x) >= CHAR_0 && (x) <= CHAR_9)
#ifndef EBCDIC
static const uint8_t xdigitab[] =
  {
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
  0x08,0x09,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
#else
static const uint8_t xdigitab[] =
  {
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
  0x08,0x09,0xff,0xff,0xff,0xff,0xff,0xff};
#endif
#ifndef EBCDIC
#define ESCAPES_FIRST       CHAR_0
#define ESCAPES_LAST        CHAR_z
#define UPPER_CASE(c)       (c-32)
static const short int escapes[] = {
     0,                        0,
     0,                        0,
     0,                        0,
     0,                        0,
     0,                        0,
     ESCAPES_FIRST+0x0a,       ESCAPES_FIRST+0x0b,
     ESCAPES_FIRST+0x0c,       ESCAPES_FIRST+0x0d,
     ESCAPES_FIRST+0x0e,       ESCAPES_FIRST+0x0f,
     ESCAPES_FIRST+0x10,       -ESC_A,
     -ESC_B,                   -ESC_C,
     -ESC_D,                   -ESC_E,
     0,                        -ESC_G,
     -ESC_H,                   0,
     0,                        -ESC_K,
     0,                        0,
     -ESC_N,                   0,
     -ESC_P,                   -ESC_Q,
     -ESC_R,                   -ESC_S,
     0,                        0,
     -ESC_V,                   -ESC_W,
     -ESC_X,                   0,
     -ESC_Z,                   ESCAPES_FIRST+0x2b,
     ESCAPES_FIRST+0x2c,       ESCAPES_FIRST+0x2d,
     ESCAPES_FIRST+0x2e,       ESCAPES_FIRST+0x2f,
     ESCAPES_FIRST+0x30,       CHAR_BEL,
     -ESC_b,                   0,
     -ESC_d,                   CHAR_ESC,
     CHAR_FF,                  0,
     -ESC_h,                   0,
     0,                        -ESC_k,
     0,                        0,
     CHAR_LF,                  0,
     -ESC_p,                   0,
     CHAR_CR,                  -ESC_s,
     CHAR_HT,                  0,
     -ESC_v,                   -ESC_w,
     0,                        0,
     -ESC_z
};
#else
#define ESCAPES_FIRST       CHAR_a
#define ESCAPES_LAST        CHAR_9
#define UPPER_CASE(c)       (c+64)
static const short int escapes[] = {
     CHAR_BEL,              -ESC_b,
     0,                     -ESC_d,
     CHAR_ESC,              CHAR_FF,
     0,                     -ESC_h,
     0,                     ESCAPES_FIRST+0x09,
     ESCAPES_FIRST+0x0a,    ESCAPES_FIRST+0x0b,
     ESCAPES_FIRST+0x0c,    ESCAPES_FIRST+0x0d,
     ESCAPES_FIRST+0x0e,    ESCAPES_FIRST+0x0f,
     0,                     -ESC_k,
     0,                     0,
     CHAR_LF,               0,
     -ESC_p,                0,
     CHAR_CR,               ESCAPES_FIRST+0x19,
     ESCAPES_FIRST+0x1a,    ESCAPES_FIRST+0x1b,
     ESCAPES_FIRST+0x1c,    ESCAPES_FIRST+0x1d,
     ESCAPES_FIRST+0x1e,    ESCAPES_FIRST+0x1f,
     ESCAPES_FIRST+0x20,    -ESC_s,
     CHAR_HT,               0,
     -ESC_v,                -ESC_w,
     0,                     0,
     -ESC_z,                ESCAPES_FIRST+0x29,
     ESCAPES_FIRST+0x2a,    ESCAPES_FIRST+0x2b,
     ESCAPES_FIRST+0x2c,    ESCAPES_FIRST+0x2d,
     ESCAPES_FIRST+0x2e,    ESCAPES_FIRST+0x2f,
     ESCAPES_FIRST+0x30,    ESCAPES_FIRST+0x31,
     ESCAPES_FIRST+0x32,    ESCAPES_FIRST+0x33,
     ESCAPES_FIRST+0x34,    ESCAPES_FIRST+0x35,
     ESCAPES_FIRST+0x36,    ESCAPES_FIRST+0x37,
     ESCAPES_FIRST+0x38,    ESCAPES_FIRST+0x39,
     ESCAPES_FIRST+0x3a,    ESCAPES_FIRST+0x3b,
     ESCAPES_FIRST+0x3c,    ESCAPES_FIRST+0x3d,
     ESCAPES_FIRST+0x3e,    ESCAPES_FIRST+0x3f,
     -ESC_A,                -ESC_B,
     -ESC_C,                -ESC_D,
     -ESC_E,                0,
     -ESC_G,                -ESC_H,
     0,                     ESCAPES_FIRST+0x49,
     ESCAPES_FIRST+0x4a,    ESCAPES_FIRST+0x4b,
     ESCAPES_FIRST+0x4c,    ESCAPES_FIRST+0x4d,
     ESCAPES_FIRST+0x4e,    ESCAPES_FIRST+0x4f,
     0,                     -ESC_K,
     0,                     0,
     -ESC_N,                0,
     -ESC_P,                -ESC_Q,
     -ESC_R,                ESCAPES_FIRST+0x59,
     ESCAPES_FIRST+0x5a,    ESCAPES_FIRST+0x5b,
     ESCAPES_FIRST+0x5c,    ESCAPES_FIRST+0x5d,
     ESCAPES_FIRST+0x5e,    ESCAPES_FIRST+0x5f,
     ESCAPES_FIRST+0x60,    -ESC_S,
     0,                     0,
     -ESC_V,                -ESC_W,
     -ESC_X,                0,
     -ESC_Z,                ESCAPES_FIRST+0x69,
     ESCAPES_FIRST+0x6a,    ESCAPES_FIRST+0x6b,
     ESCAPES_FIRST+0x6c,    ESCAPES_FIRST+0x6d,
     ESCAPES_FIRST+0x6e,    0,
     0,                     0,
     0,                     0,
     0,                     0,
     0,                     0,
     0,
};
static unsigned char ebcdic_escape_c[] = {
  CHAR_COMMERCIAL_AT, CHAR_A, CHAR_B, CHAR_C, CHAR_D, CHAR_E, CHAR_F, CHAR_G,
  CHAR_H, CHAR_I, CHAR_J, CHAR_K, CHAR_L, CHAR_M, CHAR_N, CHAR_O, CHAR_P,
  CHAR_Q, CHAR_R, CHAR_S, CHAR_T, CHAR_U, CHAR_V, CHAR_W, CHAR_X, CHAR_Y,
  CHAR_Z, CHAR_LEFT_SQUARE_BRACKET, CHAR_BACKSLASH, CHAR_RIGHT_SQUARE_BRACKET,
  CHAR_CIRCUMFLEX_ACCENT, CHAR_UNDERSCORE
};
#endif
typedef struct verbitem {
  unsigned int len;
  uint32_t meta;
  int has_arg;
} verbitem;
static const char verbnames[] =
  "\0"
  STRING_MARK0
  STRING_ACCEPT0
  STRING_F0
  STRING_FAIL0
  STRING_COMMIT0
  STRING_PRUNE0
  STRING_SKIP0
  STRING_THEN;
static const verbitem verbs[] = {
  { 0, META_MARK,   +1 },
  { 4, META_MARK,   +1 },
  { 6, META_ACCEPT, -1 },
  { 1, META_FAIL,   -1 },
  { 4, META_FAIL,   -1 },
  { 6, META_COMMIT,  0 },
  { 5, META_PRUNE,   0 },
  { 4, META_SKIP,    0 },
  { 4, META_THEN,    0 }
};
static const int verbcount = sizeof(verbs)/sizeof(verbitem);
static const uint32_t verbops[] = {
  OP_MARK, OP_ACCEPT, OP_FAIL, OP_COMMIT, OP_COMMIT_ARG, OP_PRUNE,
  OP_PRUNE_ARG, OP_SKIP, OP_SKIP_ARG, OP_THEN, OP_THEN_ARG };
typedef struct alasitem {
  unsigned int len;
  uint32_t meta;
} alasitem;
static const char alasnames[] =
  STRING_pla0
  STRING_plb0
  STRING_napla0
  STRING_naplb0
  STRING_nla0
  STRING_nlb0
  STRING_positive_lookahead0
  STRING_positive_lookbehind0
  STRING_non_atomic_positive_lookahead0
  STRING_non_atomic_positive_lookbehind0
  STRING_negative_lookahead0
  STRING_negative_lookbehind0
  STRING_scs0
  STRING_scan_substring0
  STRING_atomic0
  STRING_sr0
  STRING_asr0
  STRING_script_run0
  STRING_atomic_script_run;
static const alasitem alasmeta[] = {
  {  3, META_LOOKAHEAD         },
  {  3, META_LOOKBEHIND        },
  {  5, META_LOOKAHEAD_NA      },
  {  5, META_LOOKBEHIND_NA     },
  {  3, META_LOOKAHEADNOT      },
  {  3, META_LOOKBEHINDNOT     },
  { 18, META_LOOKAHEAD         },
  { 19, META_LOOKBEHIND        },
  { 29, META_LOOKAHEAD_NA      },
  { 30, META_LOOKBEHIND_NA     },
  { 18, META_LOOKAHEADNOT      },
  { 19, META_LOOKBEHINDNOT     },
  {  3, META_SCS               },
  { 14, META_SCS               },
  {  6, META_ATOMIC            },
  {  2, META_SCRIPT_RUN        },
  {  3, META_ATOMIC_SCRIPT_RUN },
  { 10, META_SCRIPT_RUN        },
  { 17, META_ATOMIC_SCRIPT_RUN }
};
static const int alascount = sizeof(alasmeta)/sizeof(alasitem);
static uint32_t chartypeoffset[] = {
  OP_STAR - OP_STAR,    OP_STARI - OP_STAR,
  OP_NOTSTAR - OP_STAR, OP_NOTSTARI - OP_STAR };
static const char posix_names[] =
  STRING_alpha0 STRING_lower0 STRING_upper0 STRING_alnum0
  STRING_ascii0 STRING_blank0 STRING_cntrl0 STRING_digit0
  STRING_graph0 STRING_print0 STRING_punct0 STRING_space0
  STRING_word0  STRING_xdigit;
static const uint8_t posix_name_lengths[] = {
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 6, 0 };
const int PRIV(posix_class_maps)[] = {
  cbit_word,   cbit_digit, -2,
  cbit_lower,  -1,          0,
  cbit_upper,  -1,          0,
  cbit_word,   -1,          2,
  cbit_print,  cbit_cntrl,  0,
  cbit_space,  -1,          1,
  cbit_cntrl,  -1,          0,
  cbit_digit,  -1,          0,
  cbit_graph,  -1,          0,
  cbit_print,  -1,          0,
  cbit_punct,  -1,          0,
  cbit_space,  -1,          0,
  cbit_word,   -1,          0,
  cbit_xdigit, -1,          0
};
#ifdef SUPPORT_UNICODE
static int posix_substitutes[] = {
  PT_GC, ucp_L,
  PT_PC, ucp_Ll,
  PT_PC, ucp_Lu,
  PT_ALNUM, 0,
  -1, 0,
  -1, 1,
  PT_PC, ucp_Cc,
  PT_PC, ucp_Nd,
  PT_PXGRAPH, 0,
  PT_PXPRINT, 0,
  PT_PXPUNCT, 0,
  PT_PXSPACE, 0,
  PT_WORD, 0,
  PT_PXXDIGIT, 0
};
#endif
#define PUBLIC_LITERAL_COMPILE_OPTIONS \
  (PCRE2_ANCHORED|PCRE2_AUTO_CALLOUT|PCRE2_CASELESS|PCRE2_ENDANCHORED| \
   PCRE2_FIRSTLINE|PCRE2_LITERAL|PCRE2_MATCH_INVALID_UTF| \
   PCRE2_NO_START_OPTIMIZE|PCRE2_NO_UTF_CHECK|PCRE2_USE_OFFSET_LIMIT|PCRE2_UTF)
#define PUBLIC_COMPILE_OPTIONS \
  (PUBLIC_LITERAL_COMPILE_OPTIONS| \
   PCRE2_ALLOW_EMPTY_CLASS|PCRE2_ALT_BSUX|PCRE2_ALT_CIRCUMFLEX| \
   PCRE2_ALT_VERBNAMES|PCRE2_DOLLAR_ENDONLY|PCRE2_DOTALL|PCRE2_DUPNAMES| \
   PCRE2_EXTENDED|PCRE2_EXTENDED_MORE|PCRE2_MATCH_UNSET_BACKREF| \
   PCRE2_MULTILINE|PCRE2_NEVER_BACKSLASH_C|PCRE2_NEVER_UCP| \
   PCRE2_NEVER_UTF|PCRE2_NO_AUTO_CAPTURE|PCRE2_NO_AUTO_POSSESS| \
   PCRE2_NO_DOTSTAR_ANCHOR|PCRE2_UCP|PCRE2_UNGREEDY|PCRE2_ALT_EXTENDED_CLASS)
#define PUBLIC_LITERAL_COMPILE_EXTRA_OPTIONS \
   (PCRE2_EXTRA_MATCH_LINE|PCRE2_EXTRA_MATCH_WORD| \
    PCRE2_EXTRA_CASELESS_RESTRICT|PCRE2_EXTRA_TURKISH_CASING)
#define PUBLIC_COMPILE_EXTRA_OPTIONS \
   (PUBLIC_LITERAL_COMPILE_EXTRA_OPTIONS| \
    PCRE2_EXTRA_ALLOW_SURROGATE_ESCAPES|PCRE2_EXTRA_BAD_ESCAPE_IS_LITERAL| \
    PCRE2_EXTRA_ESCAPED_CR_IS_LF|PCRE2_EXTRA_ALT_BSUX| \
    PCRE2_EXTRA_ALLOW_LOOKAROUND_BSK|PCRE2_EXTRA_ASCII_BSD| \
    PCRE2_EXTRA_ASCII_BSS|PCRE2_EXTRA_ASCII_BSW|PCRE2_EXTRA_ASCII_POSIX| \
    PCRE2_EXTRA_ASCII_DIGIT|PCRE2_EXTRA_PYTHON_OCTAL|PCRE2_EXTRA_NO_BS0| \
    PCRE2_EXTRA_NEVER_CALLOUT|\
    PCRE2_EXTRA_LOOP_LIMIT)
enum { PSO_OPT,
       PSO_XOPT,
       PSO_FLG,
       PSO_NL,
       PSO_BSR,
       PSO_LIMH,
       PSO_LIMM,
       PSO_LIMD,
       PSO_OPTMZ
     };
typedef struct pso {
  const char *name;
  uint16_t length;
  uint16_t type;
  uint32_t value;
} pso;
static const pso pso_list[] = {
  { STRING_UTFn_RIGHTPAR,                  PSO_OPT, PCRE2_UTF },
  { STRING_UTF_RIGHTPAR,                4, PSO_OPT, PCRE2_UTF },
  { STRING_UCP_RIGHTPAR,                4, PSO_OPT, PCRE2_UCP },
  { STRING_NOTEMPTY_RIGHTPAR,           9, PSO_FLG, PCRE2_NOTEMPTY_SET },
  { STRING_NOTEMPTY_ATSTART_RIGHTPAR,  17, PSO_FLG, PCRE2_NE_ATST_SET },
  { STRING_NO_AUTO_POSSESS_RIGHTPAR,   16, PSO_OPTMZ, PCRE2_OPTIM_AUTO_POSSESS },
  { STRING_NO_DOTSTAR_ANCHOR_RIGHTPAR, 18, PSO_OPTMZ, PCRE2_OPTIM_DOTSTAR_ANCHOR },
  { STRING_NO_JIT_RIGHTPAR,             7, PSO_FLG, PCRE2_NOJIT },
  { STRING_NO_START_OPT_RIGHTPAR,      13, PSO_OPTMZ, PCRE2_OPTIM_START_OPTIMIZE },
  { STRING_CASELESS_RESTRICT_RIGHTPAR, 18, PSO_XOPT, PCRE2_EXTRA_CASELESS_RESTRICT },
  { STRING_TURKISH_CASING_RIGHTPAR,    15, PSO_XOPT, PCRE2_EXTRA_TURKISH_CASING },
  { STRING_LIMIT_HEAP_EQ,              11, PSO_LIMH, 0 },
  { STRING_LIMIT_MATCH_EQ,             12, PSO_LIMM, 0 },
  { STRING_LIMIT_DEPTH_EQ,             12, PSO_LIMD, 0 },
  { STRING_LIMIT_RECURSION_EQ,         16, PSO_LIMD, 0 },
  { STRING_CR_RIGHTPAR,                 3, PSO_NL,  PCRE2_NEWLINE_CR },
  { STRING_LF_RIGHTPAR,                 3, PSO_NL,  PCRE2_NEWLINE_LF },
  { STRING_CRLF_RIGHTPAR,               5, PSO_NL,  PCRE2_NEWLINE_CRLF },
  { STRING_ANY_RIGHTPAR,                4, PSO_NL,  PCRE2_NEWLINE_ANY },
  { STRING_NUL_RIGHTPAR,                4, PSO_NL,  PCRE2_NEWLINE_NUL },
  { STRING_ANYCRLF_RIGHTPAR,            8, PSO_NL,  PCRE2_NEWLINE_ANYCRLF },
  { STRING_BSR_ANYCRLF_RIGHTPAR,       12, PSO_BSR, PCRE2_BSR_ANYCRLF },
  { STRING_BSR_UNICODE_RIGHTPAR,       12, PSO_BSR, PCRE2_BSR_UNICODE }
};
static const uint8_t opcode_possessify[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0,
  OP_POSSTAR, 0,
  OP_POSPLUS, 0,
  OP_POSQUERY, 0,
  OP_POSUPTO, 0,
  0,
  0, 0, 0, 0,
  OP_POSSTARI, 0,
  OP_POSPLUSI, 0,
  OP_POSQUERYI, 0,
  OP_POSUPTOI, 0,
  0,
  0, 0, 0, 0,
  OP_NOTPOSSTAR, 0,
  OP_NOTPOSPLUS, 0,
  OP_NOTPOSQUERY, 0,
  OP_NOTPOSUPTO, 0,
  0,
  0, 0, 0, 0,
  OP_NOTPOSSTARI, 0,
  OP_NOTPOSPLUSI, 0,
  OP_NOTPOSQUERYI, 0,
  OP_NOTPOSUPTOI, 0,
  0,
  0, 0, 0, 0,
  OP_TYPEPOSSTAR, 0,
  OP_TYPEPOSPLUS, 0,
  OP_TYPEPOSQUERY, 0,
  OP_TYPEPOSUPTO, 0,
  0,
  0, 0, 0, 0,
  OP_CRPOSSTAR, 0,
  OP_CRPOSPLUS, 0,
  OP_CRPOSQUERY, 0,
  OP_CRPOSRANGE, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0,
  0, 0,
  0, 0,
};
STATIC_ASSERT(sizeof(opcode_possessify) == OP_CALLOUT+1, opcode_possessify);
#ifdef DEBUG_SHOW_PARSED
static void show_parsed(compile_block *cb)
{
uint32_t *pptr = cb->parsed_pattern;
for (;;)
  {
  int max, min;
  PCRE2_SIZE offset;
  uint32_t i;
  uint32_t length;
  uint32_t meta_arg = META_DATA(*pptr);
  fprintf(stderr, "+++ %02d %.8x ", (int)(pptr - cb->parsed_pattern), *pptr);
  if (*pptr < META_END)
    {
    if (*pptr > 32 && *pptr < 128) fprintf(stderr, "%c", *pptr);
    pptr++;
    }
  else switch (META_CODE(*pptr++))
    {
    default:
    fprintf(stderr, "**** OOPS - unknown META value - giving up ****\n");
    return;
    case META_END:
    fprintf(stderr, "META_END\n");
    return;
    case META_CAPTURE:
    fprintf(stderr, "META_CAPTURE %d", meta_arg);
    break;
    case META_RECURSE:
    GETOFFSET(offset, pptr);
    fprintf(stderr, "META_RECURSE %d %zd", meta_arg, offset);
    break;
    case META_BACKREF:
    if (meta_arg < 10)
      offset = cb->small_ref_offset[meta_arg];
    else
      GETOFFSET(offset, pptr);
    fprintf(stderr, "META_BACKREF %d %zd", meta_arg, offset);
    break;
    case META_ESCAPE:
    if (meta_arg == ESC_P || meta_arg == ESC_p)
      {
      uint32_t ptype = *pptr >> 16;
      uint32_t pvalue = *pptr++ & 0xffff;
      fprintf(stderr, "META \\%c %d %d", (meta_arg == ESC_P)? CHAR_P:CHAR_p,
        ptype, pvalue);
      }
    else
      {
      uint32_t cc;
      if (meta_arg == ESC_g) cc = CHAR_g;
      else for (cc = ESCAPES_FIRST; cc <= ESCAPES_LAST; cc++)
        {
        if (meta_arg == (uint32_t)(-escapes[cc - ESCAPES_FIRST])) break;
        }
      if (cc > ESCAPES_LAST) cc = CHAR_QUESTION_MARK;
      fprintf(stderr, "META \\%c", cc);
      }
    break;
    case META_MINMAX:
    min = *pptr++;
    max = *pptr++;
    if (max != REPEAT_UNLIMITED)
      fprintf(stderr, "META {%d,%d}", min, max);
    else
      fprintf(stderr, "META {%d,}", min);
    break;
    case META_MINMAX_QUERY:
    min = *pptr++;
    max = *pptr++;
    if (max != REPEAT_UNLIMITED)
      fprintf(stderr, "META {%d,%d}?", min, max);
    else
      fprintf(stderr, "META {%d,}?", min);
    break;
    case META_MINMAX_PLUS:
    min = *pptr++;
    max = *pptr++;
    if (max != REPEAT_UNLIMITED)
      fprintf(stderr, "META {%d,%d}+", min, max);
    else
      fprintf(stderr, "META {%d,}+", min);
    break;
    case META_BIGVALUE: fprintf(stderr, "META_BIGVALUE %.8x", *pptr++); break;
    case META_CIRCUMFLEX: fprintf(stderr, "META_CIRCUMFLEX"); break;
    case META_COND_ASSERT: fprintf(stderr, "META_COND_ASSERT"); break;
    case META_DOLLAR: fprintf(stderr, "META_DOLLAR"); break;
    case META_DOT: fprintf(stderr, "META_DOT"); break;
    case META_ASTERISK: fprintf(stderr, "META *"); break;
    case META_ASTERISK_QUERY: fprintf(stderr, "META *?"); break;
    case META_ASTERISK_PLUS: fprintf(stderr, "META *+"); break;
    case META_PLUS: fprintf(stderr, "META +"); break;
    case META_PLUS_QUERY: fprintf(stderr, "META +?"); break;
    case META_PLUS_PLUS: fprintf(stderr, "META ++"); break;
    case META_QUERY: fprintf(stderr, "META ?"); break;
    case META_QUERY_QUERY: fprintf(stderr, "META ??"); break;
    case META_QUERY_PLUS: fprintf(stderr, "META ?+"); break;
    case META_ATOMIC: fprintf(stderr, "META (?>"); break;
    case META_NOCAPTURE: fprintf(stderr, "META (?:"); break;
    case META_LOOKAHEAD: fprintf(stderr, "META (?="); break;
    case META_LOOKAHEADNOT: fprintf(stderr, "META (?!"); break;
    case META_LOOKAHEAD_NA: fprintf(stderr, "META (*napla:"); break;
    case META_SCRIPT_RUN: fprintf(stderr, "META (*sr:"); break;
    case META_KET: fprintf(stderr, "META )"); break;
    case META_ALT: fprintf(stderr, "META | %d", meta_arg); break;
    case META_CLASS: fprintf(stderr, "META ["); break;
    case META_CLASS_NOT: fprintf(stderr, "META [^"); break;
    case META_CLASS_END: fprintf(stderr, "META ]"); break;
    case META_CLASS_EMPTY: fprintf(stderr, "META []"); break;
    case META_CLASS_EMPTY_NOT: fprintf(stderr, "META [^]"); break;
    case META_RANGE_LITERAL: fprintf(stderr, "META - (literal)"); break;
    case META_RANGE_ESCAPED: fprintf(stderr, "META - (escaped)"); break;
    case META_POSIX: fprintf(stderr, "META_POSIX %d", *pptr++); break;
    case META_POSIX_NEG: fprintf(stderr, "META_POSIX_NEG %d", *pptr++); break;
    case META_ACCEPT: fprintf(stderr, "META (*ACCEPT)"); break;
    case META_FAIL: fprintf(stderr, "META (*FAIL)"); break;
    case META_COMMIT: fprintf(stderr, "META (*COMMIT)"); break;
    case META_PRUNE: fprintf(stderr, "META (*PRUNE)"); break;
    case META_SKIP: fprintf(stderr, "META (*SKIP)"); break;
    case META_THEN: fprintf(stderr, "META (*THEN)"); break;
    case META_OPTIONS:
    fprintf(stderr, "META_OPTIONS 0x%08x 0x%08x", pptr[0], pptr[1]);
    pptr += 2;
    break;
    case META_LOOKBEHIND:
    fprintf(stderr, "META (?<= %d %d", meta_arg, *pptr);
    pptr += 2;
    break;
    case META_LOOKBEHIND_NA:
    fprintf(stderr, "META (*naplb: %d %d", meta_arg, *pptr);
    pptr += 2;
    break;
    case META_LOOKBEHINDNOT:
    fprintf(stderr, "META (?<! %d %d", meta_arg, *pptr);
    pptr += 2;
    break;
    case META_CALLOUT_NUMBER:
    fprintf(stderr, "META (?C%d) next=%d/%d", pptr[2], pptr[0],
       pptr[1]);
    pptr += 3;
    break;
    case META_CALLOUT_STRING:
      {
      uint32_t patoffset = *pptr++;
      uint32_t patlength = *pptr++;
      fprintf(stderr, "META (?Cstring) length=%d offset=", *pptr++);
      GETOFFSET(offset, pptr);
      fprintf(stderr, "%zd next=%d/%d", offset, patoffset, patlength);
      }
    break;
    case META_RECURSE_BYNAME:
    fprintf(stderr, "META (?(&name) length=%d offset=", *pptr++);
    GETOFFSET(offset, pptr);
    fprintf(stderr, "%zd", offset);
    break;
    case META_BACKREF_BYNAME:
    fprintf(stderr, "META_BACKREF_BYNAME length=%d offset=", *pptr++);
    GETOFFSET(offset, pptr);
    fprintf(stderr, "%zd", offset);
    break;
    case META_COND_NUMBER:
    fprintf(stderr, "META_COND_NUMBER %d offset=", pptr[SIZEOFFSET]);
    GETOFFSET(offset, pptr);
    fprintf(stderr, "%zd", offset);
    pptr++;
    break;
    case META_COND_DEFINE:
    fprintf(stderr, "META (?(DEFINE) offset=");
    GETOFFSET(offset, pptr);
    fprintf(stderr, "%zd", offset);
    break;
    case META_COND_VERSION:
    fprintf(stderr, "META (?(VERSION%s", (*pptr++ == 0)? "=" : ">=");
    fprintf(stderr, "%d.", *pptr++);
    fprintf(stderr, "%d)", *pptr++);
    break;
    case META_COND_NAME:
    fprintf(stderr, "META (?(<name>) length=%d offset=", *pptr++);
    GETOFFSET(offset, pptr);
    fprintf(stderr, "%zd", offset);
    break;
    case META_COND_RNAME:
    fprintf(stderr, "META (?(R&name) length=%d offset=", *pptr++);
    GETOFFSET(offset, pptr);
    fprintf(stderr, "%zd", offset);
    break;
    case META_COND_RNUMBER:
    fprintf(stderr, "META (?(Rnumber) length=%d offset=", *pptr++);
    GETOFFSET(offset, pptr);
    fprintf(stderr, "%zd", offset);
    break;
    case META_OFFSET:
    fprintf(stderr, "META_OFFSET offset=");
    GETOFFSET(offset, pptr);
    fprintf(stderr, "%zd", offset);
    break;
    case META_SCS:
    fprintf(stderr, "META (*scan_substring:");
    break;
    case META_CAPTURE_NAME:
    fprintf(stderr, "META_CAPTURE_NAME length=%d relative_offset=%d", *pptr++, (int)meta_arg);
    break;
    case META_CAPTURE_NUMBER:
    fprintf(stderr, "META_CAPTURE_NUMBER %d relative_offset=%d", *pptr++, (int)meta_arg);
    break;
    case META_MARK:
    fprintf(stderr, "META (*MARK:");
    goto SHOWARG;
    case META_COMMIT_ARG:
    fprintf(stderr, "META (*COMMIT:");
    goto SHOWARG;
    case META_PRUNE_ARG:
    fprintf(stderr, "META (*PRUNE:");
    goto SHOWARG;
    case META_SKIP_ARG:
    fprintf(stderr, "META (*SKIP:");
    goto SHOWARG;
    case META_THEN_ARG:
    fprintf(stderr, "META (*THEN:");
    SHOWARG:
    length = *pptr++;
    for (i = 0; i < length; i++)
      {
      uint32_t cc = *pptr++;
      if (cc > 32 && cc < 128) fprintf(stderr, "%c", cc);
        else fprintf(stderr, "\\x{%x}", cc);
      }
    fprintf(stderr, ") length=%u", length);
    break;
    case META_ECLASS_AND: fprintf(stderr, "META_ECLASS_AND"); break;
    case META_ECLASS_OR: fprintf(stderr, "META_ECLASS_OR"); break;
    case META_ECLASS_SUB: fprintf(stderr, "META_ECLASS_SUB"); break;
    case META_ECLASS_XOR: fprintf(stderr, "META_ECLASS_XOR"); break;
    case META_ECLASS_NOT: fprintf(stderr, "META_ECLASS_NOT"); break;
    }
  fprintf(stderr, "\n");
  }
return;
}
#endif
PCRE2_EXP_DEFN pcre2_code * PCRE2_CALL_CONVENTION
pcre2_code_copy(const pcre2_code *code)
{
PCRE2_SIZE *ref_count;
pcre2_code *newcode;
if (code == NULL) return NULL;
newcode = code->memctl.malloc(code->blocksize, code->memctl.memory_data);
if (newcode == NULL) return NULL;
memcpy(newcode, code, code->blocksize);
newcode->executable_jit = NULL;
if ((code->flags & PCRE2_DEREF_TABLES) != 0)
  {
  ref_count = (PCRE2_SIZE *)(code->tables + TABLES_LENGTH);
  (*ref_count)++;
  }
return newcode;
}
PCRE2_EXP_DEFN pcre2_code * PCRE2_CALL_CONVENTION
pcre2_code_copy_with_tables(const pcre2_code *code)
{
PCRE2_SIZE* ref_count;
pcre2_code *newcode;
uint8_t *newtables;
if (code == NULL) return NULL;
newcode = code->memctl.malloc(code->blocksize, code->memctl.memory_data);
if (newcode == NULL) return NULL;
memcpy(newcode, code, code->blocksize);
newcode->executable_jit = NULL;
newtables = code->memctl.malloc(TABLES_LENGTH + sizeof(PCRE2_SIZE),
  code->memctl.memory_data);
if (newtables == NULL)
  {
  code->memctl.free((void *)newcode, code->memctl.memory_data);
  return NULL;
  }
memcpy(newtables, code->tables, TABLES_LENGTH);
ref_count = (PCRE2_SIZE *)(newtables + TABLES_LENGTH);
*ref_count = 1;
newcode->tables = newtables;
newcode->flags |= PCRE2_DEREF_TABLES;
return newcode;
}
PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_code_free(pcre2_code *code)
{
PCRE2_SIZE* ref_count;
if (code != NULL)
  {
#ifdef SUPPORT_JIT
  if (code->executable_jit != NULL)
    PRIV(jit_free)(code->executable_jit, &code->memctl);
#endif
  if ((code->flags & PCRE2_DEREF_TABLES) != 0)
    {
    ref_count = (PCRE2_SIZE *)(code->tables + TABLES_LENGTH);
    if (*ref_count > 0)
      {
      (*ref_count)--;
      if (*ref_count == 0)
        code->memctl.free((void *)code->tables, code->memctl.memory_data);
      }
    }
  code->memctl.free(code, code->memctl.memory_data);
  }
}
static BOOL
read_number(PCRE2_SPTR *ptrptr, PCRE2_SPTR ptrend, int32_t allow_sign,
  uint32_t max_value, uint32_t max_error, int *intptr, int *errorcodeptr)
{
int sign = 0;
uint32_t n = 0;
PCRE2_SPTR ptr = *ptrptr;
BOOL yield = FALSE;
PCRE2_ASSERT(max_value <= INT_MAX/10 - 1);
*errorcodeptr = 0;
if (allow_sign >= 0 && ptr < ptrend)
  {
  if (*ptr == CHAR_PLUS)
    {
    sign = +1;
    max_value -= allow_sign;
    ptr++;
    }
  else if (*ptr == CHAR_MINUS)
    {
    sign = -1;
    ptr++;
    }
  }
if (ptr >= ptrend || !IS_DIGIT(*ptr)) return FALSE;
while (ptr < ptrend && IS_DIGIT(*ptr))
  {
  n = n * 10 + (*ptr++ - CHAR_0);
  if (n > max_value)
    {
    *errorcodeptr = max_error;
    while (ptr < ptrend && IS_DIGIT(*ptr)) ptr++;
    goto EXIT;
    }
  }
if (allow_sign >= 0 && sign != 0)
  {
  if (n == 0)
    {
    *errorcodeptr = ERR26;
    goto EXIT;
    }
  if (sign > 0) n += allow_sign;
  else if (n > (uint32_t)allow_sign)
    {
    *errorcodeptr = ERR15;
    goto EXIT;
    }
  else n = allow_sign + 1 - n;
  }
yield = TRUE;
EXIT:
*intptr = n;
*ptrptr = ptr;
return yield;
}
static BOOL
read_repeat_counts(PCRE2_SPTR *ptrptr, PCRE2_SPTR ptrend, uint32_t *minp,
  uint32_t *maxp, int *errorcodeptr)
{
PCRE2_SPTR p = *ptrptr;
PCRE2_SPTR pp;
BOOL yield = FALSE;
BOOL had_minimum = FALSE;
int32_t min = 0;
int32_t max = REPEAT_UNLIMITED;
*errorcodeptr = 0;
while (p < ptrend && (*p == CHAR_SPACE || *p == CHAR_HT)) p++;
pp = p;
if (pp < ptrend && IS_DIGIT(*pp))
  {
  had_minimum = TRUE;
  while (++pp < ptrend && IS_DIGIT(*pp)) {}
  }
while (pp < ptrend && (*pp == CHAR_SPACE || *pp == CHAR_HT)) pp++;
if (pp >= ptrend) return FALSE;
if (*pp == CHAR_RIGHT_CURLY_BRACKET)
  {
  if (!had_minimum) return FALSE;
  }
else
  {
  if (*pp++ != CHAR_COMMA) return FALSE;
  while (pp < ptrend && (*pp == CHAR_SPACE || *pp == CHAR_HT)) pp++;
  if (pp >= ptrend) return FALSE;
  if (IS_DIGIT(*pp))
    {
    while (++pp < ptrend && IS_DIGIT(*pp)) {}
    }
  else if (!had_minimum) return FALSE;
  while (pp < ptrend && (*pp == CHAR_SPACE || *pp == CHAR_HT)) pp++;
  if (pp >= ptrend || *pp != CHAR_RIGHT_CURLY_BRACKET) return FALSE;
  }
if (!read_number(&p, ptrend, -1, MAX_REPEAT_COUNT, ERR5, &min, errorcodeptr))
  {
  if (*errorcodeptr != 0) goto EXIT;
  p++;
  while (p < ptrend && (*p == CHAR_SPACE || *p == CHAR_HT)) p++;
  if (!read_number(&p, ptrend, -1, MAX_REPEAT_COUNT, ERR5, &max, errorcodeptr))
    {
    if (*errorcodeptr != 0) goto EXIT;
    }
  }
else
  {
  while (p < ptrend && (*p == CHAR_SPACE || *p == CHAR_HT)) p++;
  if (*p == CHAR_RIGHT_CURLY_BRACKET)
    {
    max = min;
    }
  else
    {
    p++;
    while (p < ptrend && (*p == CHAR_SPACE || *p == CHAR_HT)) p++;
    if (!read_number(&p, ptrend, -1, MAX_REPEAT_COUNT, ERR5, &max, errorcodeptr))
      {
      if (*errorcodeptr != 0) goto EXIT;
      }
    if (max < min)
      {
      *errorcodeptr = ERR4;
      goto EXIT;
      }
    }
  }
while (p < ptrend && (*p == CHAR_SPACE || *p == CHAR_HT)) p++;
p++;
yield = TRUE;
if (minp != NULL) *minp = (uint32_t)min;
if (maxp != NULL) *maxp = (uint32_t)max;
EXIT:
*ptrptr = p;
return yield;
}
int
PRIV(check_escape)(PCRE2_SPTR *ptrptr, PCRE2_SPTR ptrend, uint32_t *chptr,
  int *errorcodeptr, uint32_t options, uint32_t xoptions, uint32_t bracount,
  BOOL isclass, compile_block *cb)
{
BOOL utf = (options & PCRE2_UTF) != 0;
BOOL alt_bsux =
  ((options & PCRE2_ALT_BSUX) | (xoptions & PCRE2_EXTRA_ALT_BSUX)) != 0;
PCRE2_SPTR ptr = *ptrptr;
uint32_t c, cc;
int escape = 0;
int i;
if (ptr >= ptrend)
  {
  *errorcodeptr = ERR1;
  return 0;
  }
GETCHARINCTEST(c, ptr);
*errorcodeptr = 0;
if (c < ESCAPES_FIRST || c > ESCAPES_LAST) {}
else if ((i = escapes[c - ESCAPES_FIRST]) != 0)
  {
  if (i > 0)
    {
    c = (uint32_t)i;
    if (c == CHAR_CR && (xoptions & PCRE2_EXTRA_ESCAPED_CR_IS_LF) != 0)
      c = CHAR_LF;
    }
  else
    {
    escape = -i;
    if (cb != NULL && (escape == ESC_P || escape == ESC_p || escape == ESC_X))
      cb->external_flags |= PCRE2_HASBKPORX;
    if (escape == ESC_N && ptr < ptrend && *ptr == CHAR_LEFT_CURLY_BRACKET)
      {
      PCRE2_SPTR p = ptr + 1;
      while (p < ptrend && (*p == CHAR_SPACE || *p == CHAR_HT)) p++;
      if (ptrend - p > 1 && *p == CHAR_U && p[1] == CHAR_PLUS)
        {
#ifndef EBCDIC
        if (utf)
          {
          ptr = p + 2;
          escape = 0;
          goto COME_FROM_NU;
          }
#endif
        ptr = p + 2;
        while (ptr < ptrend && XDIGIT(*ptr) != 0xff) ptr++;
        while (ptr < ptrend && (*ptr == CHAR_SPACE || *ptr == CHAR_HT)) ptr++;
        if (ptr < ptrend && *ptr == CHAR_RIGHT_CURLY_BRACKET) ptr++;
        *errorcodeptr = ERR93;
        }
      else if (isclass || cb == NULL)
        {
        ptr++;
        *errorcodeptr = ERR37;
        }
      else
        {
        if (!read_repeat_counts(&p, ptrend, NULL, NULL, errorcodeptr) &&
             *errorcodeptr == 0)
          {
          ptr++;
          *errorcodeptr = ERR37;
          }
        }
      }
    }
  }
else
  {
  int s;
  PCRE2_SPTR oldptr;
  BOOL overflow;
  if (cb == NULL)
    {
    if (!(c >= CHAR_0 && c <= CHAR_9) && c != CHAR_c && c != CHAR_o &&
        c != CHAR_x && c != CHAR_g)
      {
      *errorcodeptr = ERR3;
      goto EXIT;
      }
    alt_bsux = FALSE;
    }
  switch (c)
    {
    case CHAR_F:
    case CHAR_l:
    case CHAR_L:
    *errorcodeptr = ERR37;
    break;
    case CHAR_u:
    if (!alt_bsux)
      *errorcodeptr = ERR37;
    else
      {
      uint32_t xc;
      if (ptr >= ptrend) break;
      if (*ptr == CHAR_LEFT_CURLY_BRACKET &&
          (xoptions & PCRE2_EXTRA_ALT_BSUX) != 0)
        {
        PCRE2_SPTR hptr = ptr + 1;
        cc = 0;
        while (hptr < ptrend && (xc = XDIGIT(*hptr)) != 0xff)
          {
          if ((cc & 0xf0000000) != 0)
            {
            *errorcodeptr = ERR77;
            ptr = hptr;
            break;
            }
          cc = (cc << 4) | xc;
          hptr++;
          }
        if (hptr == ptr + 1 ||
            hptr >= ptrend ||
            *hptr != CHAR_RIGHT_CURLY_BRACKET)
          {
          if (isclass) break;
          escape = ESC_ub;
          ptr++;
          break;
          }
        c = cc;
        ptr = hptr + 1;
        }
      else
        {
        if (ptrend - ptr < 4) break;
        if ((cc = XDIGIT(ptr[0])) == 0xff) break;
        if ((xc = XDIGIT(ptr[1])) == 0xff) break;
        cc = (cc << 4) | xc;
        if ((xc = XDIGIT(ptr[2])) == 0xff) break;
        cc = (cc << 4) | xc;
        if ((xc = XDIGIT(ptr[3])) == 0xff) break;
        c = (cc << 4) | xc;
        ptr += 4;
        }
      if (utf)
        {
        if (c > 0x10ffffU) *errorcodeptr = ERR77;
        else
          if (c >= 0xd800 && c <= 0xdfff &&
              (xoptions & PCRE2_EXTRA_ALLOW_SURROGATE_ESCAPES) == 0)
                *errorcodeptr = ERR73;
        }
      else if (c > MAX_NON_UTF_CHAR) *errorcodeptr = ERR77;
      }
    break;
    case CHAR_U:
    if (!alt_bsux) *errorcodeptr = ERR37;
    break;
    case CHAR_g:
    if (isclass) break;
    if (ptr >= ptrend)
      {
      *errorcodeptr = ERR57;
      break;
      }
    if (cb == NULL)
      {
      PCRE2_SPTR p;
      if (*ptr != CHAR_LESS_THAN_SIGN)
        {
        *errorcodeptr = ERR57;
        break;
        }
      p = ptr + 1;
      if (!read_number(&p, ptrend, -1, MAX_GROUP_NUMBER, ERR61, &s,
          errorcodeptr))
        {
        if (*errorcodeptr == 0) escape = ESC_g;
        break;
        }
      if (p >= ptrend || *p != CHAR_GREATER_THAN_SIGN)
        {
        ptr = p;
        *errorcodeptr = ERR119;
        break;
        }
      ptr = p + 1;
      escape = -(s+1);
      break;
      }
    if (*ptr == CHAR_LESS_THAN_SIGN || *ptr == CHAR_APOSTROPHE)
      {
      escape = ESC_g;
      break;
      }
    if (*ptr == CHAR_LEFT_CURLY_BRACKET)
      {
      PCRE2_SPTR p = ptr + 1;
      while (p < ptrend && (*p == CHAR_SPACE || *p == CHAR_HT)) p++;
      if (!read_number(&p, ptrend, bracount, MAX_GROUP_NUMBER, ERR61, &s,
          errorcodeptr))
        {
        if (*errorcodeptr == 0) escape = ESC_k;
        break;
        }
      while (p < ptrend && (*p == CHAR_SPACE || *p == CHAR_HT)) p++;
      if (p >= ptrend || *p != CHAR_RIGHT_CURLY_BRACKET)
        {
        ptr = p;
        *errorcodeptr = ERR119;
        break;
        }
      ptr = p + 1;
      }
    else
      {
      if (!read_number(&ptr, ptrend, bracount, MAX_GROUP_NUMBER, ERR61, &s,
          errorcodeptr))
        {
        if (*errorcodeptr == 0) *errorcodeptr = ERR57;
        break;
        }
      }
    if (s <= 0)
      {
      *errorcodeptr = ERR15;
      break;
      }
    escape = -(s+1);
    break;
    case CHAR_1: case CHAR_2: case CHAR_3: case CHAR_4: case CHAR_5:
    case CHAR_6: case CHAR_7: case CHAR_8: case CHAR_9:
    if (isclass)
      {
      }
    else if ((xoptions & PCRE2_EXTRA_PYTHON_OCTAL) != 0)
      {
      if (ptr[-1] <= CHAR_7 && ptr + 1 < ptrend && ptr[0] >= CHAR_0 &&
          ptr[0] <= CHAR_7 && ptr[1] >= CHAR_0 && ptr[1] <= CHAR_7)
        {
        }
      else
        {
        ptr--;
        if (!read_number(&ptr, ptrend, -1, MAX_GROUP_NUMBER, 0, &s, errorcodeptr))
          {
          *errorcodeptr = ERR61;
          break;
          }
        escape = -(s+1);
        break;
        }
      }
    else
      {
      oldptr = ptr;
      ptr--;
      if (!read_number(&ptr, ptrend, -1, MAX_GROUP_NUMBER, 0, &s, errorcodeptr))
        s = INT_MAX;
      if (s < 10 || c >= CHAR_8 || (unsigned)s <= bracount)
        {
        if ((unsigned)s > MAX_GROUP_NUMBER)
          {
          PCRE2_ASSERT(s == INT_MAX);
          *errorcodeptr = ERR61;
          }
        else escape = -(s+1);
        break;
        }
      ptr = oldptr;
      }
    if (c >= CHAR_8) break;
    PCRE2_FALLTHROUGH
    case CHAR_0:
    c -= CHAR_0;
    while(i++ < 2 && ptr < ptrend && *ptr >= CHAR_0 && *ptr <= CHAR_7)
        c = c * 8 + *ptr++ - CHAR_0;
    if (c > 0xff)
      {
      if ((xoptions & PCRE2_EXTRA_PYTHON_OCTAL) != 0) *errorcodeptr = ERR102;
#if PCRE2_CODE_UNIT_WIDTH == 8
      else if (!utf) *errorcodeptr = ERR51;
#endif
      }
    if ((xoptions & PCRE2_EXTRA_NO_BS0) != 0 && c == 0 && i == 1)
        *errorcodeptr = ERR98;
    break;
    case CHAR_o:
    if (ptr >= ptrend || *ptr != CHAR_LEFT_CURLY_BRACKET)
      {
      *errorcodeptr = ERR55;
      break;
      }
    ptr++;
    while (ptr < ptrend && (*ptr == CHAR_SPACE || *ptr == CHAR_HT)) ptr++;
    if (ptr >= ptrend || *ptr == CHAR_RIGHT_CURLY_BRACKET)
      {
      *errorcodeptr = ERR78;
      break;
      }
    c = 0;
    overflow = FALSE;
    while (ptr < ptrend && *ptr >= CHAR_0 && *ptr <= CHAR_7)
      {
      cc = *ptr++;
      if (c == 0 && cc == CHAR_0) continue;
#if PCRE2_CODE_UNIT_WIDTH == 32
      if (c >= 0x20000000u) { overflow = TRUE; break; }
#endif
      c = (c << 3) + (cc - CHAR_0);
#if PCRE2_CODE_UNIT_WIDTH == 8
      if (c > (utf ? 0x10ffffU : 0xffU)) { overflow = TRUE; break; }
#elif PCRE2_CODE_UNIT_WIDTH == 16
      if (c > (utf ? 0x10ffffU : 0xffffU)) { overflow = TRUE; break; }
#elif PCRE2_CODE_UNIT_WIDTH == 32
      if (utf && c > 0x10ffffU) { overflow = TRUE; break; }
#endif
      }
    while (ptr < ptrend && (*ptr == CHAR_SPACE || *ptr == CHAR_HT)) ptr++;
    if (overflow)
      {
      while (ptr < ptrend && *ptr >= CHAR_0 && *ptr <= CHAR_7) ptr++;
      *errorcodeptr = ERR34;
      }
    else if (utf && c >= 0xd800 && c <= 0xdfff &&
             (xoptions & PCRE2_EXTRA_ALLOW_SURROGATE_ESCAPES) == 0)
      {
      *errorcodeptr = ERR73;
      }
    else if (ptr < ptrend && *ptr == CHAR_RIGHT_CURLY_BRACKET)
      {
      ptr++;
      }
    else
      {
      *errorcodeptr = ERR64;
      goto ESCAPE_FAILED_FORWARD;
      }
    break;
    case CHAR_x:
    if (alt_bsux)
      {
      uint32_t xc;
      if (ptrend - ptr < 2) break;
      if ((cc = XDIGIT(ptr[0])) == 0xff) break;
      if ((xc = XDIGIT(ptr[1])) == 0xff) break;
      c = (cc << 4) | xc;
      ptr += 2;
      }
    else
      {
      if (ptr < ptrend && *ptr == CHAR_LEFT_CURLY_BRACKET)
        {
        ptr++;
        while (ptr < ptrend && (*ptr == CHAR_SPACE || *ptr == CHAR_HT)) ptr++;
#ifndef EBCDIC
        COME_FROM_NU:
#endif
        if (ptr >= ptrend || *ptr == CHAR_RIGHT_CURLY_BRACKET)
          {
          *errorcodeptr = ERR78;
          break;
          }
        c = 0;
        overflow = FALSE;
        while (ptr < ptrend && (cc = XDIGIT(*ptr)) != 0xff)
          {
          ptr++;
          if (c == 0 && cc == 0) continue;
#if PCRE2_CODE_UNIT_WIDTH == 32
          if (c >= 0x10000000l) { overflow = TRUE; break; }
#endif
          c = (c << 4) | cc;
          if ((utf && c > 0x10ffffU) || (!utf && c > MAX_NON_UTF_CHAR))
            {
            overflow = TRUE;
            break;
            }
          }
        while (ptr < ptrend && (*ptr == CHAR_SPACE || *ptr == CHAR_HT)) ptr++;
        if (overflow)
          {
          while (ptr < ptrend && XDIGIT(*ptr) != 0xff) ptr++;
          *errorcodeptr = ERR34;
          }
        else if (utf && c >= 0xd800 && c <= 0xdfff &&
                 (xoptions & PCRE2_EXTRA_ALLOW_SURROGATE_ESCAPES) == 0)
          {
          *errorcodeptr = ERR73;
          }
        else if (ptr < ptrend && *ptr == CHAR_RIGHT_CURLY_BRACKET)
          {
          ptr++;
          }
        else
          {
          *errorcodeptr = ERR67;
          goto ESCAPE_FAILED_FORWARD;
          }
        }
      else
        {
        if (ptr >= ptrend || (cc = XDIGIT(*ptr)) == 0xff)
          {
          *errorcodeptr = ERR78;
          break;
          }
        ptr++;
        c = cc;
        if (ptr >= ptrend || (cc = XDIGIT(*ptr)) == 0xff) break;
        ptr++;
        c = (c << 4) | cc;
        }
      }
    break;
    case CHAR_c:
    if (ptr >= ptrend)
      {
      *errorcodeptr = ERR2;
      break;
      }
    c = *ptr;
    if (c >= CHAR_a && c <= CHAR_z) c = UPPER_CASE(c);
#ifndef EBCDIC
    if (c < 32 || c > 126)
      {
      *errorcodeptr = ERR68;
      goto ESCAPE_FAILED_FORWARD;
      }
    c ^= 0x40;
#else
    if (c == CHAR_QUESTION_MARK)
      c = (CHAR_BACKSLASH == 188 && CHAR_GRAVE_ACCENT == 74)? 0x5f : 0xff;
    else
      {
      for (i = 0; i < 32; i++)
        {
        if (c == ebcdic_escape_c[i]) break;
        }
      if (i < 32)
        c = i;
      else
        {
        *errorcodeptr = ERR68;
        goto ESCAPE_FAILED_FORWARD;
        }
      }
#endif
    ptr++;
    break;
    default:
    *errorcodeptr = ERR3;
    break;
    }
  }
EXIT:
*ptrptr = ptr;
*chptr = c;
return escape;
ESCAPE_FAILED_FORWARD:
ptr++;
#ifdef SUPPORT_UNICODE
if (utf) FORWARDCHARTEST(ptr, ptrend);
#endif
goto EXIT;
}
#ifdef SUPPORT_UNICODE
static BOOL
get_ucp(PCRE2_SPTR *ptrptr, BOOL utf, BOOL *negptr, uint16_t *ptypeptr,
  uint16_t *pdataptr, int *errorcodeptr, compile_block *cb)
{
uint32_t c;
ptrdiff_t i;
PCRE2_SIZE bot, top;
PCRE2_SPTR ptr = *ptrptr;
PCRE2_UCHAR name[50];
PCRE2_UCHAR *vptr = NULL;
uint16_t ptscript = PT_NOTSCRIPT;
#ifndef MAYBE_UTF_MULTI
(void)utf;
#endif
if (ptr >= cb->end_pattern) goto ERROR_RETURN;
GETCHARINCTEST(c, ptr);
*negptr = FALSE;
if (c == CHAR_LEFT_CURLY_BRACKET)
  {
  if (ptr >= cb->end_pattern) goto ERROR_RETURN;
  for (i = 0; i < (int)(sizeof(name) / sizeof(PCRE2_UCHAR)) - 1; i++)
    {
    REDO:
    if (ptr >= cb->end_pattern) goto ERROR_RETURN;
    GETCHARINCTEST(c, ptr);
    if (c == CHAR_UNDERSCORE || c == CHAR_MINUS || c == CHAR_SPACE ||
        (c >= CHAR_HT && c <= CHAR_CR))
      {
      goto REDO;
      }
    if (i == 0 && !*negptr && c == CHAR_CIRCUMFLEX_ACCENT)
      {
      *negptr = TRUE;
      goto REDO;
      }
    if (c == CHAR_RIGHT_CURLY_BRACKET) break;
    if (c < CHAR_AMPERSAND || c > CHAR_z) goto ERROR_RETURN;
    if (c >= CHAR_A && c <= CHAR_Z) c |= 0x20;
    else if ((c == CHAR_COLON || c == CHAR_EQUALS_SIGN) && vptr == NULL)
      vptr = name + i;
    name[i] = c;
    }
  if (c != CHAR_RIGHT_CURLY_BRACKET) goto ERROR_RETURN;
  name[i] = 0;
  }
else if (c >= CHAR_A && c <= CHAR_Z)
  {
  name[0] = c | 0x20;
  name[1] = 0;
  }
else if (c >= CHAR_a && c <= CHAR_z)
  {
  name[0] = c;
  name[1] = 0;
  }
else goto ERROR_RETURN;
*ptrptr = ptr;
if (vptr != NULL)
  {
  int offset = 0;
  PCRE2_UCHAR sname[8];
  *vptr = 0;
  if (PRIV(strcmp_c8)(name, STRING_bidiclass) == 0 ||
      PRIV(strcmp_c8)(name, STRING_bc) == 0)
    {
    offset = 4;
    sname[0] = CHAR_b;
    sname[1] = CHAR_i;
    sname[2] = CHAR_d;
    sname[3] = CHAR_i;
    }
  else if (PRIV(strcmp_c8)(name, STRING_script) == 0 ||
           PRIV(strcmp_c8)(name, STRING_sc) == 0)
    ptscript = PT_SC;
  else if (PRIV(strcmp_c8)(name, STRING_scriptextensions) == 0 ||
           PRIV(strcmp_c8)(name, STRING_scx) == 0)
    ptscript = PT_SCX;
  else
    {
    *errorcodeptr = ERR47;
    return FALSE;
    }
  memmove(name + offset, vptr + 1, (name + i - vptr)*sizeof(PCRE2_UCHAR));
  if (offset != 0) memmove(name, sname, offset*sizeof(PCRE2_UCHAR));
  }
bot = 0;
top = PRIV(utt_size);
while (bot < top)
  {
  int r;
  i = (bot + top) >> 1;
  r = PRIV(strcmp_c8)(name, PRIV(utt_names) + PRIV(utt)[i].name_offset);
  if (r == 0)
    {
    *pdataptr = PRIV(utt)[i].value;
    if (vptr == NULL || ptscript == PT_NOTSCRIPT)
      {
      *ptypeptr = PRIV(utt)[i].type;
      return TRUE;
      }
    switch (PRIV(utt)[i].type)
      {
      case PT_SC:
      *ptypeptr = PT_SC;
      return TRUE;
      case PT_SCX:
      *ptypeptr = ptscript;
      return TRUE;
      }
    break;
    }
  if (r > 0) bot = i + 1; else top = i;
  }
*errorcodeptr = ERR47;
return FALSE;
ERROR_RETURN:
*errorcodeptr = ERR46;
*ptrptr = ptr;
return FALSE;
}
#endif
static BOOL
check_posix_syntax(PCRE2_SPTR ptr, PCRE2_SPTR ptrend, PCRE2_SPTR *endptr)
{
PCRE2_UCHAR terminator;
terminator = *ptr++;
for (; ptrend - ptr >= 2; ptr++)
  {
  if (*ptr == CHAR_BACKSLASH &&
      (ptr[1] == CHAR_RIGHT_SQUARE_BRACKET || ptr[1] == CHAR_BACKSLASH))
    ptr++;
  else if ((*ptr == CHAR_LEFT_SQUARE_BRACKET && ptr[1] == terminator) ||
            *ptr == CHAR_RIGHT_SQUARE_BRACKET) return FALSE;
  else if (*ptr == terminator && ptr[1] == CHAR_RIGHT_SQUARE_BRACKET)
    {
    *endptr = ptr;
    return TRUE;
    }
  }
return FALSE;
}
static int
check_posix_name(PCRE2_SPTR ptr, int len)
{
const char *pn = posix_names;
int yield = 0;
while (posix_name_lengths[yield] != 0)
  {
  if (len == posix_name_lengths[yield] &&
    PRIV(strncmp_c8)(ptr, pn, (unsigned int)len) == 0) return yield;
  pn += posix_name_lengths[yield] + 1;
  yield++;
  }
return -1;
}
static BOOL
read_name(PCRE2_SPTR *ptrptr, PCRE2_SPTR ptrend, BOOL utf, uint32_t terminator,
  PCRE2_SIZE *offsetptr, PCRE2_SPTR *nameptr, uint32_t *namelenptr,
  int *errorcodeptr, compile_block *cb)
{
PCRE2_SPTR ptr = *ptrptr;
BOOL is_group = (*ptr++ != CHAR_ASTERISK);
BOOL is_braced = terminator == CHAR_RIGHT_CURLY_BRACKET;
if (is_braced)
  while (ptr < ptrend && (*ptr == CHAR_SPACE || *ptr == CHAR_HT)) ptr++;
if (ptr >= ptrend)
  {
  *errorcodeptr = is_group? ERR62:
                            ERR60;
  goto FAILED;
  }
*nameptr = ptr;
*offsetptr = (PCRE2_SIZE)(ptr - cb->start_pattern);
#ifdef SUPPORT_UNICODE
if (utf && is_group)
  {
  uint32_t c, type;
  PCRE2_SPTR p = ptr;
  GETCHARINC(c, p);
  type = UCD_CHARTYPE(c);
  if (type == ucp_Nd)
    {
    ptr = p;
    *errorcodeptr = ERR44;
    goto FAILED;
    }
  for(;;)
    {
    if (type != ucp_Nd && PRIV(ucp_gentype)[type] != ucp_L &&
        c != CHAR_UNDERSCORE) break;
    ptr = p;
    if (p >= ptrend) break;
    GETCHARINC(c, p);
    type = UCD_CHARTYPE(c);
    }
  }
else
#else
(void)utf;
#endif
  {
  if (is_group && IS_DIGIT(*ptr))
    {
    ++ptr;
    *errorcodeptr = ERR44;
    goto FAILED;
    }
  while (ptr < ptrend && MAX_255(*ptr) && (cb->ctypes[*ptr] & ctype_word) != 0)
    {
    ptr++;
    }
  }
if (ptr - *nameptr > MAX_NAME_SIZE)
  {
  *errorcodeptr = ERR48;
  goto FAILED;
  }
*namelenptr = (uint32_t)(ptr - *nameptr);
if (is_group)
  {
  if (ptr == *nameptr)
    {
    *errorcodeptr = ERR62;
    goto FAILED;
    }
  if (is_braced)
    while (ptr < ptrend && (*ptr == CHAR_SPACE || *ptr == CHAR_HT)) ptr++;
  if (terminator != 0)
    {
    if (ptr >= ptrend || *ptr != (PCRE2_UCHAR)terminator)
      {
      *errorcodeptr = ERR42;
      goto FAILED;
      }
    ptr++;
    }
  }
*ptrptr = ptr;
return TRUE;
FAILED:
*ptrptr = ptr;
return FALSE;
}
static uint32_t *
parse_capture_list(PCRE2_SPTR *ptrptr, PCRE2_SPTR ptrend,
  BOOL utf, uint32_t *parsed_pattern, PCRE2_SIZE offset,
  int *errorcodeptr, compile_block *cb)
{
PCRE2_SIZE next_offset;
PCRE2_SPTR ptr = *ptrptr;
PCRE2_SPTR name;
PCRE2_UCHAR terminator;
uint32_t meta, namelen;
int i;
if (ptr >= ptrend || *ptr != CHAR_LEFT_PARENTHESIS)
  {
  *errorcodeptr = ERR118;
  goto FAILED;
  }
for (;;)
  {
  ptr++;
  next_offset = (PCRE2_SIZE)(ptr - cb->start_pattern);
  if (ptr >= ptrend)
    {
    *errorcodeptr = ERR117;
    goto FAILED;
    }
  if (read_number(&ptr, ptrend, cb->bracount, MAX_GROUP_NUMBER, ERR61,
      &i, errorcodeptr))
    {
    PCRE2_ASSERT(i >= 0);
    if (i <= 0)
      {
      *errorcodeptr = ERR15;
      goto FAILED;
      }
    meta = META_CAPTURE_NUMBER;
    namelen = (uint32_t)i;
    }
  else if (*errorcodeptr != 0) goto FAILED;
  else
    {
    if (*ptr == CHAR_LESS_THAN_SIGN)
      terminator = CHAR_GREATER_THAN_SIGN;
    else if (*ptr == CHAR_APOSTROPHE)
      terminator = CHAR_APOSTROPHE;
    else
      {
      *errorcodeptr = ERR117;
      goto FAILED;
      }
    if (!read_name(&ptr, ptrend, utf, terminator, &next_offset,
        &name, &namelen, errorcodeptr, cb)) goto FAILED;
    meta = META_CAPTURE_NAME;
    }
  PCRE2_ASSERT(next_offset > 0);
  if (offset == 0 || (next_offset - offset) >= 0x10000)
    {
    *parsed_pattern++ = META_OFFSET;
    PUTOFFSET(next_offset, parsed_pattern);
    offset = next_offset;
    }
  *parsed_pattern++ = meta | (uint32_t)(next_offset - offset);
  *parsed_pattern++ = namelen;
  offset = next_offset;
  if (ptr >= ptrend) goto UNCLOSED_PARENTHESIS;
  if (*ptr == CHAR_RIGHT_PARENTHESIS) break;
  if (*ptr != CHAR_COMMA)
    {
    *errorcodeptr = ERR24;
    goto FAILED;
    }
  }
*ptrptr = ptr + 1;
return parsed_pattern;
UNCLOSED_PARENTHESIS:
*errorcodeptr = ERR14;
FAILED:
*ptrptr = ptr;
return NULL;
}
static uint32_t *
manage_callouts(PCRE2_SPTR ptr, uint32_t **pcalloutptr, BOOL auto_callout,
  uint32_t *parsed_pattern, compile_block *cb)
{
uint32_t *previous_callout = *pcalloutptr;
if (previous_callout != NULL) previous_callout[2] = (uint32_t)(ptr -
  cb->start_pattern - (PCRE2_SIZE)previous_callout[1]);
if (!auto_callout) previous_callout = NULL; else
  {
  if (previous_callout == NULL ||
      previous_callout != parsed_pattern - 4 ||
      previous_callout[3] != 255)
    {
    previous_callout = parsed_pattern;
    parsed_pattern += 4;
    previous_callout[0] = META_CALLOUT_NUMBER;
    previous_callout[2] = 0;
    previous_callout[3] = 255;
    }
  previous_callout[1] = (uint32_t)(ptr - cb->start_pattern);
  }
*pcalloutptr = previous_callout;
return parsed_pattern;
}
static uint32_t *
handle_escdsw(int escape, uint32_t *parsed_pattern, uint32_t options,
  uint32_t xoptions)
{
uint32_t ascii_option = 0;
uint32_t prop = ESC_p;
switch(escape)
  {
  case ESC_D:
  prop = ESC_P;
  PCRE2_FALLTHROUGH
  case ESC_d:
  ascii_option = PCRE2_EXTRA_ASCII_BSD;
  break;
  case ESC_S:
  prop = ESC_P;
  PCRE2_FALLTHROUGH
  case ESC_s:
  ascii_option = PCRE2_EXTRA_ASCII_BSS;
  break;
  case ESC_W:
  prop = ESC_P;
  PCRE2_FALLTHROUGH
  case ESC_w:
  ascii_option = PCRE2_EXTRA_ASCII_BSW;
  break;
  }
if ((options & PCRE2_UCP) == 0 || (xoptions & ascii_option) != 0)
  {
  *parsed_pattern++ = META_ESCAPE + escape;
  }
else
  {
  *parsed_pattern++ = META_ESCAPE + prop;
  switch(escape)
    {
    case ESC_d:
    case ESC_D:
    *parsed_pattern++ = (PT_PC << 16) | ucp_Nd;
    break;
    case ESC_s:
    case ESC_S:
    *parsed_pattern++ = PT_SPACE << 16;
    break;
    case ESC_w:
    case ESC_W:
    *parsed_pattern++ = PT_WORD << 16;
    break;
    }
  }
return parsed_pattern;
}
static ptrdiff_t
max_parsed_pattern(PCRE2_SPTR ptr, PCRE2_SPTR ptrend, BOOL utf,
  uint32_t options)
{
PCRE2_SIZE big32count = 0;
ptrdiff_t parsed_size_needed;
#if PCRE2_CODE_UNIT_WIDTH == 32
if (!utf)
  {
  PCRE2_SPTR p;
  for (p = ptr; p < ptrend; p++) if (*p >= META_END) big32count++;
  }
#else
(void)utf;
#endif
parsed_size_needed = (ptrend - ptr) + big32count;
if ((options & PCRE2_AUTO_CALLOUT) != 0)
  parsed_size_needed += (ptrend - ptr) * 4;
return parsed_size_needed;
}
typedef struct nest_save {
  uint16_t  nest_depth;
  uint16_t  reset_group;
  uint16_t  max_group;
  uint16_t  flags;
  uint32_t  options;
  uint32_t  xoptions;
} nest_save;
#define NSF_RESET          0x0001u
#define NSF_CONDASSERT     0x0002u
#define NSF_ATOMICSR       0x0004u
#define PARSE_TRACKED_OPTIONS (PCRE2_CASELESS|PCRE2_DOTALL|PCRE2_DUPNAMES| \
  PCRE2_EXTENDED|PCRE2_EXTENDED_MORE|PCRE2_MULTILINE|PCRE2_NO_AUTO_CAPTURE| \
  PCRE2_UNGREEDY)
#define PARSE_TRACKED_EXTRA_OPTIONS (PCRE2_EXTRA_CASELESS_RESTRICT| \
  PCRE2_EXTRA_ASCII_BSD|PCRE2_EXTRA_ASCII_BSS|PCRE2_EXTRA_ASCII_BSW| \
  PCRE2_EXTRA_ASCII_DIGIT|PCRE2_EXTRA_ASCII_POSIX)
enum {
  RANGE_NO,
  RANGE_STARTED,
  RANGE_FORBID_NO,
  RANGE_FORBID_STARTED,
  RANGE_OK_ESCAPED,
  RANGE_OK_LITERAL
};
enum {
  CLASS_OP_EMPTY,
  CLASS_OP_OPERAND,
  CLASS_OP_OPERATOR
};
enum {
  CLASS_MODE_NORMAL,
  CLASS_MODE_ALT_EXT,
  CLASS_MODE_PERL_EXT,
  CLASS_MODE_PERL_EXT_LEAF
};
#if PCRE2_CODE_UNIT_WIDTH == 32
#define PARSED_LITERAL(c, p) \
  { \
  if (c >= META_END) *p++ = META_BIGVALUE; \
  *p++ = c; \
  okquantifier = TRUE; \
  }
#else
#define PARSED_LITERAL(c, p) *p++ = c; okquantifier = TRUE;
#endif
static int parse_regex(PCRE2_SPTR ptr, uint32_t options, uint32_t xoptions,
  BOOL *has_lookbehind, compile_block *cb)
{
uint32_t c;
uint32_t delimiter;
uint32_t namelen;
uint32_t class_range_state;
uint32_t class_op_state;
uint32_t class_mode_state;
uint32_t *class_start;
uint32_t *verblengthptr = NULL;
uint32_t *verbstartptr = NULL;
uint32_t *previous_callout = NULL;
uint32_t *parsed_pattern = cb->parsed_pattern;
uint32_t *parsed_pattern_end = cb->parsed_pattern_end;
uint32_t *this_parsed_item = NULL;
uint32_t *prev_parsed_item = NULL;
uint32_t meta_quantifier = 0;
uint32_t add_after_mark = 0;
uint16_t nest_depth = 0;
int16_t class_depth_m1 = -1;
int16_t class_maxdepth_m1 = -1;
uint16_t hash;
int after_manual_callout = 0;
int expect_cond_assert = 0;
int errorcode = 0;
int escape;
int i;
BOOL inescq = FALSE;
BOOL inverbname = FALSE;
BOOL utf = (options & PCRE2_UTF) != 0;
BOOL auto_callout = (options & PCRE2_AUTO_CALLOUT) != 0;
BOOL is_dupname;
BOOL negate_class;
BOOL okquantifier = FALSE;
PCRE2_SPTR thisptr;
PCRE2_SPTR name;
PCRE2_SPTR ptrend = cb->end_pattern;
PCRE2_SPTR verbnamestart = NULL;
PCRE2_SPTR class_range_forbid_ptr = NULL;
named_group *ng;
nest_save *top_nest, *end_nests;
#ifdef PCRE2_DEBUG
uint32_t *parsed_pattern_check;
ptrdiff_t parsed_pattern_extra = 0;
ptrdiff_t parsed_pattern_extra_check = 0;
PCRE2_SPTR ptr_check;
#endif
PCRE2_ASSERT(parsed_pattern != NULL);
if ((xoptions & PCRE2_EXTRA_MATCH_LINE) != 0)
  {
  *parsed_pattern++ = META_CIRCUMFLEX;
  *parsed_pattern++ = META_NOCAPTURE;
  }
else if ((xoptions & PCRE2_EXTRA_MATCH_WORD) != 0)
  {
  *parsed_pattern++ = META_ESCAPE + ESC_b;
  *parsed_pattern++ = META_NOCAPTURE;
  }
#ifdef PCRE2_DEBUG
parsed_pattern_check = parsed_pattern;
ptr_check = ptr;
#endif
if ((options & PCRE2_LITERAL) != 0)
  {
  while (ptr < ptrend)
    {
    if (parsed_pattern >= parsed_pattern_end)
      {
      PCRE2_DEBUG_UNREACHABLE();
      errorcode = ERR63;
      goto FAILED;
      }
    thisptr = ptr;
    GETCHARINCTEST(c, ptr);
    if (auto_callout)
      parsed_pattern = manage_callouts(thisptr, &previous_callout,
        auto_callout, parsed_pattern, cb);
    PARSED_LITERAL(c, parsed_pattern);
    }
  goto PARSED_END;
  }
top_nest = NULL;
end_nests = (nest_save *)(cb->start_workspace + cb->workspace_size);
end_nests = (nest_save *)((char *)end_nests -
  ((cb->workspace_size * sizeof(PCRE2_UCHAR)) % sizeof(nest_save)));
if ((options & PCRE2_EXTENDED_MORE) != 0) options |= PCRE2_EXTENDED;
while (ptr < ptrend)
  {
  int prev_expect_cond_assert;
  uint32_t min_repeat = 0, max_repeat = 0;
  uint32_t set, unset, *optset;
  uint32_t xset, xunset, *xoptset;
  uint32_t terminator;
  uint32_t prev_meta_quantifier;
  BOOL prev_okquantifier;
  PCRE2_SPTR tempptr;
  PCRE2_SIZE offset;
  if (nest_depth > cb->cx->parens_nest_limit)
    {
    errorcode = ERR19;
    goto FAILED;
    }
#ifdef PCRE2_DEBUG
  PCRE2_ASSERT((parsed_pattern - parsed_pattern_check) +
               (parsed_pattern_extra - parsed_pattern_extra_check) <=
                 max_parsed_pattern(ptr_check, ptr, utf, options));
  parsed_pattern_check = parsed_pattern;
  parsed_pattern_extra_check = parsed_pattern_extra;
  ptr_check = ptr;
#endif
  if (parsed_pattern >= parsed_pattern_end)
    {
    PCRE2_DEBUG_UNREACHABLE();
    errorcode = ERR63;
    goto FAILED;
    }
  if (this_parsed_item != parsed_pattern)
    {
    prev_parsed_item = this_parsed_item;
    this_parsed_item = parsed_pattern;
    }
  thisptr = ptr;
  GETCHARINCTEST(c, ptr);
  if (inescq)
    {
    if (c == CHAR_BACKSLASH && ptr < ptrend && *ptr == CHAR_E)
      {
      inescq = FALSE;
      ptr++;
      }
    else
      {
      if (inverbname)
        {
#if PCRE2_CODE_UNIT_WIDTH == 32
        if (c >= META_END) *parsed_pattern++ = META_BIGVALUE;
#endif
        *parsed_pattern++ = c;
        }
      else
        {
        if (after_manual_callout-- <= 0)
          parsed_pattern = manage_callouts(thisptr, &previous_callout,
            auto_callout, parsed_pattern, cb);
        PARSED_LITERAL(c, parsed_pattern);
        }
      meta_quantifier = 0;
      }
    continue;
    }
  if (inverbname &&
       (
        ((options & (PCRE2_EXTENDED | PCRE2_ALT_VERBNAMES)) !=
                    (PCRE2_EXTENDED | PCRE2_ALT_VERBNAMES)) ||
#ifdef SUPPORT_UNICODE
        (c > 255 && (c|1) != 0x200f && (c|1) != 0x2029) ||
#endif
        (c < 256 && c != CHAR_NUMBER_SIGN && (cb->ctypes[c] & ctype_space) == 0
#ifdef SUPPORT_UNICODE
          && c != CHAR_NEL
#endif
       )))
    {
    PCRE2_SIZE verbnamelength;
    switch(c)
      {
      default:
#if PCRE2_CODE_UNIT_WIDTH == 32
      if (c >= META_END) *parsed_pattern++ = META_BIGVALUE;
#endif
      *parsed_pattern++ = c;
      break;
      case CHAR_RIGHT_PARENTHESIS:
      inverbname = FALSE;
      verbnamelength = (PCRE2_SIZE)(parsed_pattern - verblengthptr - 1);
      if (ptr - verbnamestart - 1 > (int)MAX_MARK)
        {
        ptr--;
        errorcode = ERR76;
        goto FAILED;
        }
      *verblengthptr = (uint32_t)verbnamelength;
      if (add_after_mark != 0)
        {
        *parsed_pattern++ = add_after_mark;
        add_after_mark = 0;
        }
      break;
      case CHAR_BACKSLASH:
      if ((options & PCRE2_ALT_VERBNAMES) != 0)
        {
        escape = PRIV(check_escape)(&ptr, ptrend, &c, &errorcode, options,
          xoptions, cb->bracount, FALSE, cb);
        if (errorcode != 0) goto FAILED;
        }
      else escape = 0;
      switch(escape)
        {
        case 0:
#if PCRE2_CODE_UNIT_WIDTH == 32
        if (c >= META_END) *parsed_pattern++ = META_BIGVALUE;
#endif
        *parsed_pattern++ = c;
        break;
        case ESC_ub:
        *parsed_pattern++ = CHAR_u;
        PARSED_LITERAL(CHAR_LEFT_CURLY_BRACKET, parsed_pattern);
        break;
        case ESC_Q:
        inescq = TRUE;
        break;
        case ESC_E:
        break;
        default:
        errorcode = ERR40;
        goto FAILED;
        }
      }
    continue;
    }
  if (c == CHAR_BACKSLASH && ptr < ptrend)
    {
    if (*ptr == CHAR_Q || *ptr == CHAR_E)
      {
      if (expect_cond_assert > 0 && *ptr == CHAR_Q &&
          !(ptrend - ptr >= 3 && ptr[1] == CHAR_BACKSLASH && ptr[2] == CHAR_E))
        {
        ptr--;
        errorcode = ERR28;
        goto FAILED;
        }
      inescq = *ptr == CHAR_Q;
      ptr++;
      continue;
      }
    }
  if ((options & PCRE2_EXTENDED) != 0)
    {
    if (c < 256 && (cb->ctypes[c] & ctype_space) != 0) continue;
#ifdef SUPPORT_UNICODE
    if (c == CHAR_NEL || (c|1) == 0x200f || (c|1) == 0x2029) continue;
#endif
    if (c == CHAR_NUMBER_SIGN)
      {
      while (ptr < ptrend)
        {
        if (IS_NEWLINE(ptr))
          {
          ptr += cb->nllen;
          break;
          }
        ptr++;
#ifdef SUPPORT_UNICODE
        if (utf) FORWARDCHARTEST(ptr, ptrend);
#endif
        }
      continue;
      }
    }
  if (c == CHAR_LEFT_PARENTHESIS && ptrend - ptr >= 2 &&
      ptr[0] == CHAR_QUESTION_MARK && ptr[1] == CHAR_NUMBER_SIGN)
    {
    while (++ptr < ptrend && *ptr != CHAR_RIGHT_PARENTHESIS);
    if (ptr >= ptrend)
      {
      errorcode = ERR18;
      goto FAILED;
      }
    ptr++;
    continue;
    }
  if (c != CHAR_ASTERISK && c != CHAR_PLUS && c != CHAR_QUESTION_MARK &&
       (c != CHAR_LEFT_CURLY_BRACKET ||
         (tempptr = ptr,
         !read_repeat_counts(&tempptr, ptrend, NULL, NULL, &errorcode))))
    {
    if (after_manual_callout-- <= 0)
      {
      parsed_pattern = manage_callouts(thisptr, &previous_callout, auto_callout,
        parsed_pattern, cb);
      this_parsed_item = parsed_pattern;
      }
    }
  if (expect_cond_assert > 0)
    {
    BOOL ok = c == CHAR_LEFT_PARENTHESIS && ptrend - ptr >= 3 &&
              (ptr[0] == CHAR_QUESTION_MARK || ptr[0] == CHAR_ASTERISK);
    if (ok)
      {
      if (ptr[0] == CHAR_ASTERISK)
        {
        ok = MAX_255(ptr[1]) && (cb->ctypes[ptr[1]] & ctype_lcletter) != 0;
        }
      else switch(ptr[1])
        {
        case CHAR_C:
        ok = expect_cond_assert == 2;
        break;
        case CHAR_EQUALS_SIGN:
        case CHAR_EXCLAMATION_MARK:
        break;
        case CHAR_LESS_THAN_SIGN:
        ok = ptr[2] == CHAR_EQUALS_SIGN || ptr[2] == CHAR_EXCLAMATION_MARK;
        break;
        default:
        ok = FALSE;
        }
      }
    if (!ok)
      {
      errorcode = ERR28;
      if (expect_cond_assert == 2) goto FAILED;
      goto FAILED_BACK;
      }
    }
  prev_expect_cond_assert = expect_cond_assert;
  expect_cond_assert = 0;
  prev_okquantifier = okquantifier;
  prev_meta_quantifier = meta_quantifier;
  okquantifier = FALSE;
  meta_quantifier = 0;
  if (prev_meta_quantifier != 0 && (c == CHAR_QUESTION_MARK || c == CHAR_PLUS))
    {
    parsed_pattern[(prev_meta_quantifier == META_MINMAX)? -3 : -1] =
      prev_meta_quantifier + ((c == CHAR_QUESTION_MARK)?
        0x00020000u : 0x00010000u);
    continue;
    }
  switch(c)
    {
    default:
    PARSED_LITERAL(c, parsed_pattern);
    break;
    case CHAR_BACKSLASH:
    tempptr = ptr;
    escape = PRIV(check_escape)(&ptr, ptrend, &c, &errorcode, options,
      xoptions, cb->bracount, FALSE, cb);
    if (errorcode != 0)
      {
      ESCAPE_FAILED:
      if ((xoptions & PCRE2_EXTRA_BAD_ESCAPE_IS_LITERAL) == 0)
        goto FAILED;
      ptr = tempptr;
      if (ptr >= ptrend) c = CHAR_BACKSLASH; else
        {
        GETCHARINCTEST(c, ptr);
        }
      escape = 0;
      }
    if (escape == 0)
      {
      PARSED_LITERAL(c, parsed_pattern);
      }
    else if (escape < 0)
      {
      offset = (PCRE2_SIZE)(ptr - cb->start_pattern);
      escape = -escape - 1;
      *parsed_pattern++ = META_BACKREF | (uint32_t)escape;
      if (escape < 10)
        {
        if (cb->small_ref_offset[escape] == PCRE2_UNSET)
          cb->small_ref_offset[escape] = offset;
        }
      else
        {
        PUTOFFSET(offset, parsed_pattern);
        }
      okquantifier = TRUE;
      }
    else switch (escape)
      {
      case ESC_C:
#ifdef NEVER_BACKSLASH_C
      errorcode = ERR85;
      goto ESCAPE_FAILED;
#else
      if ((options & PCRE2_NEVER_BACKSLASH_C) != 0)
        {
        errorcode = ERR83;
        goto ESCAPE_FAILED;
        }
#endif
      okquantifier = TRUE;
      *parsed_pattern++ = META_ESCAPE + escape;
      break;
      case ESC_ub:
      *parsed_pattern++ = CHAR_u;
      PARSED_LITERAL(CHAR_LEFT_CURLY_BRACKET, parsed_pattern);
      break;
      case ESC_X:
#ifndef SUPPORT_UNICODE
      errorcode = ERR45;
      goto ESCAPE_FAILED;
#endif
      case ESC_H:
      case ESC_h:
      case ESC_N:
      case ESC_R:
      case ESC_V:
      case ESC_v:
      okquantifier = TRUE;
      *parsed_pattern++ = META_ESCAPE + escape;
      break;
      default:
      *parsed_pattern++ = META_ESCAPE + escape;
      break;
      case ESC_d:
      case ESC_D:
      case ESC_s:
      case ESC_S:
      case ESC_w:
      case ESC_W:
      okquantifier = TRUE;
      parsed_pattern = handle_escdsw(escape, parsed_pattern, options,
        xoptions);
      break;
      case ESC_P:
      case ESC_p:
#ifdef SUPPORT_UNICODE
        {
        BOOL negated;
        uint16_t ptype = 0, pdata = 0;
        if (!get_ucp(&ptr, utf, &negated, &ptype, &pdata, &errorcode, cb))
          goto ESCAPE_FAILED;
        if (negated) escape = (escape == ESC_P)? ESC_p : ESC_P;
        *parsed_pattern++ = META_ESCAPE + escape;
        *parsed_pattern++ = (ptype << 16) | pdata;
        okquantifier = TRUE;
        }
#else
      errorcode = ERR45;
      goto ESCAPE_FAILED;
#endif
      break;
      case ESC_g:
      case ESC_k:
      if (ptr >= ptrend || (*ptr != CHAR_LEFT_CURLY_BRACKET &&
          *ptr != CHAR_LESS_THAN_SIGN && *ptr != CHAR_APOSTROPHE))
        {
        errorcode = (escape == ESC_g)? ERR57 : ERR69;
        goto ESCAPE_FAILED;
        }
      terminator = (*ptr == CHAR_LESS_THAN_SIGN)?
        CHAR_GREATER_THAN_SIGN : (*ptr == CHAR_APOSTROPHE)?
        CHAR_APOSTROPHE : CHAR_RIGHT_CURLY_BRACKET;
      if (escape == ESC_g && terminator != CHAR_RIGHT_CURLY_BRACKET)
        {
        PCRE2_SPTR p = ptr + 1;
        if (read_number(&p, ptrend, cb->bracount, MAX_GROUP_NUMBER, ERR61, &i,
            &errorcode))
          {
          if (p >= ptrend || *p != terminator)
            {
            ptr = p;
            errorcode = ERR119;
            goto ESCAPE_FAILED;
            }
          ptr = p + 1;
          goto SET_RECURSION;
          }
        if (errorcode != 0) goto ESCAPE_FAILED;
        }
      if (!read_name(&ptr, ptrend, utf, terminator, &offset, &name, &namelen,
          &errorcode, cb)) goto ESCAPE_FAILED;
      *parsed_pattern++ =
        (escape == ESC_k || terminator == CHAR_RIGHT_CURLY_BRACKET)?
          META_BACKREF_BYNAME : META_RECURSE_BYNAME;
      *parsed_pattern++ = namelen;
      PUTOFFSET(offset, parsed_pattern);
      okquantifier = TRUE;
      break;
      }
    break;
    case CHAR_CIRCUMFLEX_ACCENT:
    *parsed_pattern++ = META_CIRCUMFLEX;
    break;
    case CHAR_DOLLAR_SIGN:
    *parsed_pattern++ = META_DOLLAR;
    break;
    case CHAR_DOT:
    *parsed_pattern++ = META_DOT;
    okquantifier = TRUE;
    break;
    case CHAR_ASTERISK:
    meta_quantifier = META_ASTERISK;
    goto CHECK_QUANTIFIER;
    case CHAR_PLUS:
    meta_quantifier = META_PLUS;
    goto CHECK_QUANTIFIER;
    case CHAR_QUESTION_MARK:
    meta_quantifier = META_QUERY;
    goto CHECK_QUANTIFIER;
    case CHAR_LEFT_CURLY_BRACKET:
    if (!read_repeat_counts(&ptr, ptrend, &min_repeat, &max_repeat,
        &errorcode))
      {
      if (errorcode != 0) goto FAILED;
      PARSED_LITERAL(c, parsed_pattern);
      break;
      }
    meta_quantifier = META_MINMAX;
    CHECK_QUANTIFIER:
    if (!prev_okquantifier)
      {
      errorcode = ERR9;
      goto FAILED;
      }
    if (*prev_parsed_item == META_ACCEPT)
      {
      uint32_t *p;
      for (p = parsed_pattern - 1; p >= verbstartptr; p--) p[1] = p[0];
      *verbstartptr = META_NOCAPTURE;
      parsed_pattern[1] = META_KET;
      parsed_pattern += 2;
#ifdef PCRE2_DEBUG
      PCRE2_ASSERT(parsed_pattern_extra >= 2);
      parsed_pattern_extra -= 2;
#endif
      }
    *parsed_pattern++ = meta_quantifier;
    if (c == CHAR_LEFT_CURLY_BRACKET)
      {
      *parsed_pattern++ = min_repeat;
      *parsed_pattern++ = max_repeat;
      }
    break;
    case CHAR_LEFT_SQUARE_BRACKET:
    if (ptrend - ptr >= 6 &&
         (PRIV(strncmp_c8)(ptr, STRING_WEIRD_STARTWORD, 6) == 0 ||
          PRIV(strncmp_c8)(ptr, STRING_WEIRD_ENDWORD, 6) == 0))
      {
      *parsed_pattern++ = META_ESCAPE + ESC_b;
      if (ptr[2] == CHAR_LESS_THAN_SIGN)
        {
        *parsed_pattern++ = META_LOOKAHEAD;
        }
      else
        {
        *parsed_pattern++ = META_LOOKBEHIND;
        *has_lookbehind = TRUE;
        PUTOFFSET((PCRE2_SIZE)0, parsed_pattern);
        }
      if ((options & PCRE2_UCP) == 0)
        *parsed_pattern++ = META_ESCAPE + ESC_w;
      else
        {
        *parsed_pattern++ = META_ESCAPE + ESC_p;
        *parsed_pattern++ = PT_WORD << 16;
        }
      *parsed_pattern++ = META_KET;
      ptr += 6;
      okquantifier = TRUE;
      break;
      }
    if (ptr < ptrend && (*ptr == CHAR_COLON || *ptr == CHAR_DOT ||
         *ptr == CHAR_EQUALS_SIGN) &&
        check_posix_syntax(ptr, ptrend, &tempptr))
      {
      errorcode = (*ptr-- == CHAR_COLON)? ERR12 : ERR13;
      ptr = tempptr + 2;
      goto FAILED;
      }
    class_mode_state = ((options & PCRE2_ALT_EXTENDED_CLASS) != 0)?
        CLASS_MODE_ALT_EXT : CLASS_MODE_NORMAL;
    FROM_PERL_EXTENDED_CLASS:
    okquantifier = TRUE;
    class_depth_m1 = -1;
    class_maxdepth_m1 = -1;
    class_range_state = RANGE_NO;
    class_op_state = CLASS_OP_EMPTY;
    class_start = NULL;
    for (;;)
      {
      BOOL char_is_literal = TRUE;
      if (inescq)
        {
        if (c == CHAR_BACKSLASH && ptr < ptrend && *ptr == CHAR_E)
          {
          inescq = FALSE;
          ptr++;
          goto CLASS_CONTINUE;
          }
        if (class_mode_state == CLASS_MODE_PERL_EXT)
          {
          errorcode = ERR116;
          goto FAILED;
          }
        goto CLASS_LITERAL;
        }
      if ((c == CHAR_SPACE || c == CHAR_HT) &&
          ((options & PCRE2_EXTENDED_MORE) != 0 ||
           class_mode_state >= CLASS_MODE_PERL_EXT))
        goto CLASS_CONTINUE;
      if (class_depth_m1 >= 0 &&
          c == CHAR_LEFT_SQUARE_BRACKET &&
          ptrend - ptr >= 3 &&
          (*ptr == CHAR_COLON || *ptr == CHAR_DOT ||
           *ptr == CHAR_EQUALS_SIGN) &&
          check_posix_syntax(ptr, ptrend, &tempptr))
        {
        BOOL posix_negate = FALSE;
        int posix_class;
        if (class_range_state == RANGE_STARTED)
          {
          ptr = tempptr + 2;
          errorcode = ERR50;
          goto FAILED;
          }
        if (class_range_state == RANGE_FORBID_STARTED)
          {
          ptr = class_range_forbid_ptr;
          errorcode = ERR50;
          goto FAILED;
          }
        if (class_op_state == CLASS_OP_OPERAND &&
            class_mode_state == CLASS_MODE_PERL_EXT)
          {
          ptr = tempptr + 2;
          errorcode = ERR113;
          goto FAILED;
          }
        if (*ptr != CHAR_COLON)
          {
          ptr = tempptr + 2;
          errorcode = ERR13;
          goto FAILED;
          }
        if (*(++ptr) == CHAR_CIRCUMFLEX_ACCENT)
          {
          posix_negate = TRUE;
          ptr++;
          }
        posix_class = check_posix_name(ptr, (int)(tempptr - ptr));
        ptr = tempptr + 2;
        if (posix_class < 0)
          {
          errorcode = ERR30;
          goto FAILED;
          }
        class_range_state = RANGE_FORBID_NO;
        class_op_state = CLASS_OP_OPERAND;
#ifdef SUPPORT_UNICODE
        if ((options & PCRE2_UCP) != 0 &&
            (xoptions & PCRE2_EXTRA_ASCII_POSIX) == 0 &&
            !((xoptions & PCRE2_EXTRA_ASCII_DIGIT) != 0 &&
              (posix_class == PC_DIGIT || posix_class == PC_XDIGIT)))
          {
          int ptype = posix_substitutes[2*posix_class];
          int pvalue = posix_substitutes[2*posix_class + 1];
          if (ptype >= 0)
            {
            *parsed_pattern++ = META_ESCAPE + (posix_negate? ESC_P : ESC_p);
            *parsed_pattern++ = (ptype << 16) | pvalue;
            goto CLASS_CONTINUE;
            }
          if (pvalue != 0)
            {
            *parsed_pattern++ = META_ESCAPE + (posix_negate? ESC_H : ESC_h);
            goto CLASS_CONTINUE;
            }
          }
#endif
        *parsed_pattern++ = posix_negate? META_POSIX_NEG : META_POSIX;
        *parsed_pattern++ = posix_class;
        }
      else if ((c == CHAR_LEFT_SQUARE_BRACKET &&
                (class_depth_m1 < 0 || class_mode_state == CLASS_MODE_ALT_EXT ||
                 class_mode_state == CLASS_MODE_PERL_EXT)) ||
               (c == CHAR_LEFT_PARENTHESIS &&
                class_mode_state == CLASS_MODE_PERL_EXT))
        {
        uint32_t start_c = c;
        uint32_t new_class_mode_state;
        if (start_c == CHAR_LEFT_SQUARE_BRACKET &&
            class_mode_state == CLASS_MODE_PERL_EXT && class_depth_m1 >= 0)
          new_class_mode_state = CLASS_MODE_PERL_EXT_LEAF;
        else
          new_class_mode_state = class_mode_state;
        if (class_range_state == RANGE_STARTED)
          parsed_pattern[-1] = CHAR_MINUS;
        if (class_op_state == CLASS_OP_OPERAND &&
            class_mode_state == CLASS_MODE_PERL_EXT)
          {
          errorcode = ERR113;
          goto FAILED;
          }
        if (class_depth_m1 >= ECLASS_NEST_LIMIT - 1)
          {
          ptr--;
          errorcode = ERR107;
          goto FAILED;
          }
        negate_class = FALSE;
        for (;;)
          {
          if (ptr >= ptrend)
            {
            if (start_c == CHAR_LEFT_PARENTHESIS)
              errorcode = ERR14;
            else
              errorcode = ERR6;
            goto FAILED;
            }
          GETCHARINCTEST(c, ptr);
          if (new_class_mode_state == CLASS_MODE_PERL_EXT) break;
          else if (c == CHAR_BACKSLASH)
            {
            if (ptr < ptrend && *ptr == CHAR_E) ptr++;
            else if (ptrend - ptr >= 3 &&
                PRIV(strncmp_c8)(ptr, STR_Q STR_BACKSLASH STR_E, 3) == 0)
              ptr += 3;
            else
              break;
            }
          else if ((c == CHAR_SPACE || c == CHAR_HT) &&
                   ((options & PCRE2_EXTENDED_MORE) != 0 ||
                    new_class_mode_state >= CLASS_MODE_PERL_EXT))
            continue;
          else if (!negate_class && c == CHAR_CIRCUMFLEX_ACCENT)
            negate_class = TRUE;
          else break;
          }
        if (c == CHAR_RIGHT_SQUARE_BRACKET &&
            (cb->external_options & PCRE2_ALLOW_EMPTY_CLASS) != 0 &&
            new_class_mode_state < CLASS_MODE_PERL_EXT)
          {
          PCRE2_ASSERT(start_c == CHAR_LEFT_SQUARE_BRACKET);
          if (class_start != NULL)
            {
            PCRE2_ASSERT(class_depth_m1 >= 0);
            *class_start |= CLASS_IS_ECLASS;
            class_start = NULL;
            }
          *parsed_pattern++ = negate_class? META_CLASS_EMPTY_NOT : META_CLASS_EMPTY;
          if (class_depth_m1 < 0) break;
          class_range_state = RANGE_NO;
          class_op_state = CLASS_OP_OPERAND;
          goto CLASS_CONTINUE;
          }
        if (class_start != NULL)
          {
          PCRE2_ASSERT(class_depth_m1 >= 0);
          *class_start |= CLASS_IS_ECLASS;
          class_start = NULL;
          }
        class_start = parsed_pattern;
        *parsed_pattern++ = negate_class? META_CLASS_NOT : META_CLASS;
        class_range_state = RANGE_NO;
        class_op_state = CLASS_OP_EMPTY;
        class_mode_state = new_class_mode_state;
        ++class_depth_m1;
        if (class_maxdepth_m1 < class_depth_m1)
          class_maxdepth_m1 = class_depth_m1;
        cb->class_op_used[class_depth_m1] = 0;
        if (c == CHAR_RIGHT_SQUARE_BRACKET &&
            new_class_mode_state != CLASS_MODE_PERL_EXT)
          {
          class_range_state = RANGE_OK_LITERAL;
          class_op_state = CLASS_OP_OPERAND;
          PARSED_LITERAL(c, parsed_pattern);
          goto CLASS_CONTINUE;
          }
        continue;
        }
      else if (c == CHAR_RIGHT_SQUARE_BRACKET ||
               (c == CHAR_RIGHT_PARENTHESIS && class_mode_state == CLASS_MODE_PERL_EXT))
        {
        if (class_mode_state == CLASS_MODE_PERL_EXT)
          {
          if (c == CHAR_RIGHT_SQUARE_BRACKET && class_depth_m1 != 0)
            {
            errorcode = ERR14;
            ptr--;
            goto FAILED;
            }
          if (c == CHAR_RIGHT_PARENTHESIS && class_depth_m1 < 1)
            {
            errorcode = ERR22;
            goto FAILED;
            }
          }
        if (class_op_state == CLASS_OP_OPERATOR)
          {
          errorcode = ERR110;
          goto FAILED;
          }
        if (class_mode_state == CLASS_MODE_PERL_EXT &&
            class_op_state == CLASS_OP_EMPTY)
          {
          errorcode = ERR114;
          goto FAILED;
          }
        if (class_range_state == RANGE_STARTED)
          parsed_pattern[-1] = CHAR_MINUS;
        *parsed_pattern++ = META_CLASS_END;
        if (--class_depth_m1 < 0)
          {
          PCRE2_ASSERT(class_mode_state != CLASS_MODE_PERL_EXT_LEAF);
          if (class_mode_state == CLASS_MODE_PERL_EXT)
            {
            if (ptr >= ptrend || *ptr != CHAR_RIGHT_PARENTHESIS)
              {
              errorcode = ERR115;
              goto FAILED;
              }
            ptr++;
            }
          break;
          }
        class_range_state = RANGE_NO;
        class_op_state = CLASS_OP_OPERAND;
        if (class_mode_state == CLASS_MODE_PERL_EXT_LEAF)
          class_mode_state = CLASS_MODE_PERL_EXT;
        class_start = NULL;
        }
      else if (class_mode_state == CLASS_MODE_PERL_EXT &&
               (c == CHAR_PLUS || c == CHAR_VERTICAL_LINE || c == CHAR_MINUS ||
                c == CHAR_AMPERSAND || c == CHAR_CIRCUMFLEX_ACCENT))
        {
        if (class_op_state != CLASS_OP_OPERAND)
          {
          errorcode = ERR109;
          goto FAILED;
          }
        if (class_start != NULL)
          {
          PCRE2_ASSERT(class_depth_m1 >= 0);
          *class_start |= CLASS_IS_ECLASS;
          class_start = NULL;
          }
        PCRE2_ASSERT(class_range_state != RANGE_STARTED &&
                     class_range_state != RANGE_FORBID_STARTED);
        *parsed_pattern++ = c == CHAR_PLUS? META_ECLASS_OR :
                            c == CHAR_VERTICAL_LINE? META_ECLASS_OR :
                            c == CHAR_MINUS? META_ECLASS_SUB :
                            c == CHAR_AMPERSAND? META_ECLASS_AND :
                            META_ECLASS_XOR;
        class_range_state = RANGE_NO;
        class_op_state = CLASS_OP_OPERATOR;
        }
      else if (class_mode_state == CLASS_MODE_PERL_EXT &&
               c == CHAR_EXCLAMATION_MARK)
        {
        if (class_op_state == CLASS_OP_OPERAND)
          {
          errorcode = ERR113;
          goto FAILED;
          }
        if (class_start != NULL)
          {
          PCRE2_ASSERT(class_depth_m1 >= 0);
          *class_start |= CLASS_IS_ECLASS;
          class_start = NULL;
          }
        PCRE2_ASSERT(class_range_state != RANGE_STARTED &&
                     class_range_state != RANGE_FORBID_STARTED);
        *parsed_pattern++ = META_ECLASS_NOT;
        class_range_state = RANGE_NO;
        class_op_state = CLASS_OP_OPERATOR;
        }
      else if (class_mode_state == CLASS_MODE_ALT_EXT &&
               (c == CHAR_VERTICAL_LINE || c == CHAR_MINUS ||
                c == CHAR_AMPERSAND || c == CHAR_TILDE) &&
               ptr < ptrend && *ptr == c)
        {
        ++ptr;
        if (ptr < ptrend && *ptr == c)
          {
          while (ptr < ptrend && *ptr == c) ++ptr;
          errorcode = ERR108;
          goto FAILED;
          }
        if (class_op_state != CLASS_OP_OPERAND)
          {
          errorcode = ERR109;
          goto FAILED;
          }
        if (cb->class_op_used[class_depth_m1] != 0 &&
            cb->class_op_used[class_depth_m1] != (uint8_t)c)
          {
          errorcode = ERR111;
          goto FAILED;
          }
        if (class_start != NULL)
          {
          PCRE2_ASSERT(class_depth_m1 >= 0);
          *class_start |= CLASS_IS_ECLASS;
          class_start = NULL;
          }
        if (class_range_state == RANGE_STARTED)
          parsed_pattern[-1] = CHAR_MINUS;
        *parsed_pattern++ = c == CHAR_VERTICAL_LINE? META_ECLASS_OR :
                            c == CHAR_MINUS? META_ECLASS_SUB :
                            c == CHAR_AMPERSAND? META_ECLASS_AND :
                            META_ECLASS_XOR;
        class_range_state = RANGE_NO;
        class_op_state = CLASS_OP_OPERATOR;
        cb->class_op_used[class_depth_m1] = (uint8_t)c;
        }
      else if (c == CHAR_BACKSLASH)
        {
        tempptr = ptr;
        escape = PRIV(check_escape)(&ptr, ptrend, &c, &errorcode, options,
          xoptions, cb->bracount, TRUE, cb);
        if (errorcode != 0)
          {
          if ((xoptions & PCRE2_EXTRA_BAD_ESCAPE_IS_LITERAL) == 0 ||
              class_mode_state >= CLASS_MODE_PERL_EXT)
            goto FAILED;
          ptr = tempptr;
          if (ptr >= ptrend) c = CHAR_BACKSLASH; else
            {
            GETCHARINCTEST(c, ptr);
            }
          escape = 0;
          }
        switch(escape)
          {
          case 0:
          char_is_literal = FALSE;
          goto CLASS_LITERAL;
          case ESC_b:
          c = CHAR_BS;
          char_is_literal = FALSE;
          goto CLASS_LITERAL;
          case ESC_k:
          c = CHAR_k;
          char_is_literal = FALSE;
          goto CLASS_LITERAL;
          case ESC_Q:
          inescq = TRUE;
          goto CLASS_CONTINUE;
          case ESC_E:
          goto CLASS_CONTINUE;
          case ESC_B:
          case ESC_R:
          case ESC_X:
          errorcode = ERR7;
          goto FAILED;
          case ESC_N:
          errorcode = ERR71;
          goto FAILED;
          case ESC_H:
          case ESC_h:
          case ESC_V:
          case ESC_v:
          *parsed_pattern++ = META_ESCAPE + escape;
          break;
          case ESC_d:
          case ESC_D:
          case ESC_s:
          case ESC_S:
          case ESC_w:
          case ESC_W:
          parsed_pattern = handle_escdsw(escape, parsed_pattern, options,
            xoptions);
          break;
          case ESC_P:
          case ESC_p:
#ifdef SUPPORT_UNICODE
            {
            BOOL negated;
            uint16_t ptype = 0, pdata = 0;
            if (!get_ucp(&ptr, utf, &negated, &ptype, &pdata, &errorcode, cb))
              goto FAILED;
            if ((options & PCRE2_CASELESS) != 0 && ptype == PT_PC &&
                (pdata == ucp_Lu || pdata == ucp_Ll || pdata == ucp_Lt))
              {
              ptype = PT_LAMP;
              pdata = 0;
              }
            if (negated) escape = (escape == ESC_P)? ESC_p : ESC_P;
            *parsed_pattern++ = META_ESCAPE + escape;
            *parsed_pattern++ = (ptype << 16) | pdata;
            }
#else
          errorcode = ERR45;
          goto FAILED;
#endif
          break;
          default:
          PCRE2_DEBUG_UNREACHABLE();
          PCRE2_FALLTHROUGH
          case ESC_A:
          case ESC_Z:
          case ESC_z:
          case ESC_G:
          case ESC_K:
          case ESC_C:
          errorcode = ERR7;
          goto FAILED;
          }
        if (class_range_state == RANGE_STARTED)
          {
          errorcode = ERR50;
          goto FAILED;
          }
        if (class_range_state == RANGE_FORBID_STARTED)
          {
          ptr = class_range_forbid_ptr;
          errorcode = ERR50;
          goto FAILED;
          }
        if (class_op_state == CLASS_OP_OPERAND &&
            class_mode_state == CLASS_MODE_PERL_EXT)
          {
          errorcode = ERR113;
          goto FAILED;
          }
        class_range_state = RANGE_FORBID_NO;
        class_op_state = CLASS_OP_OPERAND;
        }
      else if (class_mode_state == CLASS_MODE_PERL_EXT)
        {
        errorcode = ERR116;
        goto FAILED;
        }
      else if (c == CHAR_MINUS && class_range_state >= RANGE_OK_ESCAPED)
        {
        *parsed_pattern++ = (class_range_state == RANGE_OK_LITERAL)?
          META_RANGE_LITERAL : META_RANGE_ESCAPED;
        class_range_state = RANGE_STARTED;
        }
      else if (c == CHAR_MINUS && class_range_state == RANGE_FORBID_NO)
        {
        *parsed_pattern++ = CHAR_MINUS;
        class_range_state = RANGE_FORBID_STARTED;
        class_range_forbid_ptr = ptr;
        }
      else
        {
        CLASS_LITERAL:
        if (class_op_state == CLASS_OP_OPERAND &&
            class_mode_state == CLASS_MODE_PERL_EXT)
          {
          errorcode = ERR113;
          goto FAILED;
          }
        if (class_range_state == RANGE_STARTED)
          {
          if (c == parsed_pattern[-2])
            parsed_pattern--;
          else if (parsed_pattern[-2] > c)
            {
            errorcode = ERR8;
            goto FAILED;
            }
          else
            {
            if (!char_is_literal && parsed_pattern[-1] == META_RANGE_LITERAL)
              parsed_pattern[-1] = META_RANGE_ESCAPED;
            PARSED_LITERAL(c, parsed_pattern);
            }
          class_range_state = RANGE_NO;
          class_op_state = CLASS_OP_OPERAND;
          }
        else if (class_range_state == RANGE_FORBID_STARTED)
          {
          ptr = class_range_forbid_ptr;
          errorcode = ERR50;
          goto FAILED;
          }
        else
          {
          class_range_state = char_is_literal?
            RANGE_OK_LITERAL : RANGE_OK_ESCAPED;
          class_op_state = CLASS_OP_OPERAND;
          PARSED_LITERAL(c, parsed_pattern);
          }
        }
      CLASS_CONTINUE:
      if (ptr >= ptrend)
        {
        if (class_mode_state == CLASS_MODE_PERL_EXT && class_depth_m1 > 0)
          errorcode = ERR14;
        if (class_mode_state == CLASS_MODE_ALT_EXT &&
            class_depth_m1 == 0 && class_maxdepth_m1 == 1)
          errorcode = ERR112;
        else
          errorcode = ERR6;
        goto FAILED;
        }
      GETCHARINCTEST(c, ptr);
      }
    break;
    case CHAR_LEFT_PARENTHESIS:
    if (ptr >= ptrend) goto UNCLOSED_PARENTHESIS;
    if (*ptr != CHAR_QUESTION_MARK)
      {
      const char *vn;
      if (*ptr != CHAR_ASTERISK)
        {
        nest_depth++;
        if ((options & PCRE2_NO_AUTO_CAPTURE) == 0)
          {
          if (cb->bracount >= MAX_GROUP_NUMBER)
            {
            errorcode = ERR97;
            goto FAILED;
            }
          cb->bracount++;
          *parsed_pattern++ = META_CAPTURE | cb->bracount;
          }
        else *parsed_pattern++ = META_NOCAPTURE;
        }
      else if (ptrend - ptr <= 1 || (c = ptr[1]) == CHAR_RIGHT_PARENTHESIS)
        break;
      else if (CHMAX_255(c) && (cb->ctypes[c] & ctype_lcletter) != 0)
        {
        uint32_t meta;
        vn = alasnames;
        if (!read_name(&ptr, ptrend, utf, 0, &offset, &name, &namelen,
          &errorcode, cb)) goto FAILED;
        if (ptr >= ptrend) goto UNCLOSED_PARENTHESIS;
        if (*ptr != CHAR_COLON)
          {
          errorcode = ERR95;
          goto FAILED_FORWARD;
          }
        for (i = 0; i < alascount; i++)
          {
          if (namelen == alasmeta[i].len &&
              PRIV(strncmp_c8)(name, vn, namelen) == 0)
            break;
          vn += alasmeta[i].len + 1;
          }
        if (i >= alascount)
          {
          errorcode = ERR95;
          goto FAILED;
          }
        meta = alasmeta[i].meta;
        if (prev_expect_cond_assert > 0 &&
            (meta < META_LOOKAHEAD || meta > META_LOOKBEHINDNOT))
          {
          errorcode = ERR28;
          goto FAILED;
          }
        switch(meta)
          {
          default:
          PCRE2_DEBUG_UNREACHABLE();
          errorcode = ERR89;
          goto FAILED;
          case META_ATOMIC:
          goto ATOMIC_GROUP;
          case META_LOOKAHEAD:
          goto POSITIVE_LOOK_AHEAD;
          case META_LOOKAHEAD_NA:
          goto POSITIVE_NONATOMIC_LOOK_AHEAD;
          case META_LOOKAHEADNOT:
          goto NEGATIVE_LOOK_AHEAD;
          case META_SCS:
          ptr++;
          *parsed_pattern++ = META_SCS;
          parsed_pattern = parse_capture_list(&ptr, ptrend, utf, parsed_pattern,
                                              0, &errorcode, cb);
          if (parsed_pattern == NULL) goto FAILED;
          goto POST_ASSERTION;
          case META_LOOKBEHIND:
          case META_LOOKBEHINDNOT:
          case META_LOOKBEHIND_NA:
          *parsed_pattern++ = meta;
          ptr--;
          goto POST_LOOKBEHIND;
          case META_SCRIPT_RUN:
          case META_ATOMIC_SCRIPT_RUN:
#ifdef SUPPORT_UNICODE
          *parsed_pattern++ = META_SCRIPT_RUN;
          nest_depth++;
          ptr++;
          if (meta == META_ATOMIC_SCRIPT_RUN)
            {
            *parsed_pattern++ = META_ATOMIC;
            if (top_nest == NULL) top_nest = (nest_save *)(cb->start_workspace);
            else if (++top_nest >= end_nests)
              {
              errorcode = ERR84;
              goto FAILED;
              }
            top_nest->nest_depth = nest_depth;
            top_nest->flags = NSF_ATOMICSR;
            top_nest->options = options & PARSE_TRACKED_OPTIONS;
            top_nest->xoptions = xoptions & PARSE_TRACKED_EXTRA_OPTIONS;
#ifdef PCRE2_DEBUG
            parsed_pattern_extra++;
#endif
            }
          break;
#else
          errorcode = ERR96;
          goto FAILED;
#endif
          }
        }
      else
        {
        vn = verbnames;
        if (!read_name(&ptr, ptrend, utf, 0, &offset, &name, &namelen,
          &errorcode, cb)) goto FAILED;
        if (ptr >= ptrend || (*ptr != CHAR_COLON &&
                              *ptr != CHAR_RIGHT_PARENTHESIS))
          {
          errorcode = ERR60;
          goto FAILED;
          }
        for (i = 0; i < verbcount; i++)
          {
          if (namelen == verbs[i].len &&
              PRIV(strncmp_c8)(name, vn, namelen) == 0)
            break;
          vn += verbs[i].len + 1;
          }
        if (i >= verbcount)
          {
          errorcode = ERR60;
          goto FAILED;
          }
        if (*ptr == CHAR_COLON && ptr + 1 < ptrend &&
             ptr[1] == CHAR_RIGHT_PARENTHESIS)
          ptr++;
        if (verbs[i].has_arg > 0 && *ptr != CHAR_COLON)
          {
          errorcode = ERR66;
          goto FAILED;
          }
        verbstartptr = parsed_pattern;
        okquantifier = (verbs[i].meta == META_ACCEPT);
#ifdef PCRE2_DEBUG
        if (okquantifier) parsed_pattern_extra += 2;
#endif
        if (*ptr++ == CHAR_COLON)
          {
          if (verbs[i].has_arg < 0)
            {
            add_after_mark = verbs[i].meta;
            *parsed_pattern++ = META_MARK;
            }
          else
            {
            *parsed_pattern++ = verbs[i].meta +
              ((verbs[i].meta != META_MARK)? 0x00010000u:0);
            }
          verblengthptr = parsed_pattern++;
          verbnamestart = ptr;
          inverbname = TRUE;
          }
        else
          {
          *parsed_pattern++ = verbs[i].meta;
          }
        }
      break;
      }
    if (++ptr >= ptrend) goto UNCLOSED_PARENTHESIS;
    switch(*ptr)
      {
      default:
      if (*ptr == CHAR_MINUS && ptrend - ptr > 1 && IS_DIGIT(ptr[1]))
        goto RECURSION_BYNUMBER;
      nest_depth++;
      if (top_nest == NULL) top_nest = (nest_save *)(cb->start_workspace);
      else if (++top_nest >= end_nests)
        {
        errorcode = ERR84;
        goto FAILED;
        }
      top_nest->nest_depth = nest_depth;
      top_nest->flags = 0;
      top_nest->options = options & PARSE_TRACKED_OPTIONS;
      top_nest->xoptions = xoptions & PARSE_TRACKED_EXTRA_OPTIONS;
      if (*ptr == CHAR_VERTICAL_LINE)
        {
        top_nest->reset_group = (uint16_t)cb->bracount;
        top_nest->max_group = (uint16_t)cb->bracount;
        top_nest->flags |= NSF_RESET;
        cb->external_flags |= PCRE2_DUPCAPUSED;
        *parsed_pattern++ = META_NOCAPTURE;
        ptr++;
        }
      else
        {
        BOOL hyphenok = TRUE;
        uint32_t oldoptions = options;
        uint32_t oldxoptions = xoptions;
        top_nest->reset_group = 0;
        top_nest->max_group = 0;
        set = unset = 0;
        optset = &set;
        xset = xunset = 0;
        xoptset = &xset;
        if (ptr < ptrend && *ptr == CHAR_CIRCUMFLEX_ACCENT)
          {
          options &= ~(PCRE2_CASELESS|PCRE2_MULTILINE|PCRE2_NO_AUTO_CAPTURE|
                       PCRE2_DOTALL|PCRE2_EXTENDED|PCRE2_EXTENDED_MORE);
          xoptions &= ~(PCRE2_EXTRA_CASELESS_RESTRICT);
          hyphenok = FALSE;
          ptr++;
          }
        while (ptr < ptrend && *ptr != CHAR_RIGHT_PARENTHESIS &&
                               *ptr != CHAR_COLON)
          {
          switch (*ptr++)
            {
            case CHAR_MINUS:
            if (!hyphenok)
              {
              errorcode = ERR94;
              goto FAILED;
              }
            optset = &unset;
            xoptset = &xunset;
            hyphenok = FALSE;
            break;
            case CHAR_a:
            if (ptr < ptrend)
              {
              if (*ptr == CHAR_D)
                {
                *xoptset |= PCRE2_EXTRA_ASCII_BSD;
                ptr++;
                break;
                }
              if (*ptr == CHAR_P)
                {
                *xoptset |= (PCRE2_EXTRA_ASCII_POSIX|PCRE2_EXTRA_ASCII_DIGIT);
                ptr++;
                break;
                }
              if (*ptr == CHAR_S)
                {
                *xoptset |= PCRE2_EXTRA_ASCII_BSS;
                ptr++;
                break;
                }
              if (*ptr == CHAR_T)
                {
                *xoptset |= PCRE2_EXTRA_ASCII_DIGIT;
                ptr++;
                break;
                }
              if (*ptr == CHAR_W)
                {
                *xoptset |= PCRE2_EXTRA_ASCII_BSW;
                ptr++;
                break;
                }
              }
            *xoptset |= PCRE2_EXTRA_ASCII_BSD|PCRE2_EXTRA_ASCII_BSS|
                        PCRE2_EXTRA_ASCII_BSW|
                        PCRE2_EXTRA_ASCII_DIGIT|PCRE2_EXTRA_ASCII_POSIX;
            break;
            case CHAR_J:
            *optset |= PCRE2_DUPNAMES;
            cb->external_flags |= PCRE2_JCHANGED;
            break;
            case CHAR_i: *optset |= PCRE2_CASELESS; break;
            case CHAR_m: *optset |= PCRE2_MULTILINE; break;
            case CHAR_n: *optset |= PCRE2_NO_AUTO_CAPTURE; break;
            case CHAR_r: *xoptset|= PCRE2_EXTRA_CASELESS_RESTRICT; break;
            case CHAR_s: *optset |= PCRE2_DOTALL; break;
            case CHAR_U: *optset |= PCRE2_UNGREEDY; break;
            case CHAR_x:
            *optset |= PCRE2_EXTENDED;
            if (ptr < ptrend && *ptr == CHAR_x)
              {
              *optset |= PCRE2_EXTENDED_MORE;
              ptr++;
              }
            break;
            default:
            errorcode = ERR11;
            goto FAILED;
            }
          }
        if ((set & (PCRE2_EXTENDED|PCRE2_EXTENDED_MORE)) == PCRE2_EXTENDED ||
            (unset & PCRE2_EXTENDED) != 0)
          unset |= PCRE2_EXTENDED_MORE;
        options = (options | set) & (~unset);
        xoptions = (xoptions | xset) & (~xunset);
        if (ptr >= ptrend) goto UNCLOSED_PARENTHESIS;
        if (*ptr++ == CHAR_RIGHT_PARENTHESIS)
          {
          nest_depth--;
          if (top_nest > (nest_save *)(cb->start_workspace) &&
              (top_nest-1)->nest_depth == nest_depth) top_nest--;
          else top_nest->nest_depth = nest_depth;
          }
        else *parsed_pattern++ = META_NOCAPTURE;
        if (options != oldoptions || xoptions != oldxoptions)
          {
          *parsed_pattern++ = META_OPTIONS;
          *parsed_pattern++ = options;
          *parsed_pattern++ = xoptions;
          }
        }
      break;
      case CHAR_P:
      if (++ptr >= ptrend) goto UNCLOSED_PARENTHESIS;
      if (*ptr == CHAR_LESS_THAN_SIGN)
        {
        terminator = CHAR_GREATER_THAN_SIGN;
        goto DEFINE_NAME;
        }
      if (*ptr == CHAR_GREATER_THAN_SIGN) goto RECURSE_BY_NAME;
      if (*ptr != CHAR_EQUALS_SIGN)
        {
        errorcode = ERR41;
        goto FAILED_FORWARD;
        }
      if (!read_name(&ptr, ptrend, utf, CHAR_RIGHT_PARENTHESIS, &offset, &name,
          &namelen, &errorcode, cb)) goto FAILED;
      *parsed_pattern++ = META_BACKREF_BYNAME;
      *parsed_pattern++ = namelen;
      PUTOFFSET(offset, parsed_pattern);
      okquantifier = TRUE;
      break;
      case CHAR_R:
      i = 0;
      ptr++;
      if (ptr >= ptrend || (*ptr != CHAR_RIGHT_PARENTHESIS && *ptr != CHAR_LEFT_PARENTHESIS))
        {
        errorcode = ERR58;
        goto FAILED;
        }
      terminator = CHAR_NUL;
      goto SET_RECURSION;
      case CHAR_PLUS:
      if (ptr + 1 >= ptrend)
        {
        ++ptr;
        goto UNCLOSED_PARENTHESIS;
        }
      if (!IS_DIGIT(ptr[1]))
        {
        errorcode = ERR29;
        ++ptr;
        goto FAILED_FORWARD;
        }
      PCRE2_FALLTHROUGH
      case CHAR_0: case CHAR_1: case CHAR_2: case CHAR_3: case CHAR_4:
      case CHAR_5: case CHAR_6: case CHAR_7: case CHAR_8: case CHAR_9:
      RECURSION_BYNUMBER:
      if (!read_number(&ptr, ptrend,
          (IS_DIGIT(*ptr))? -1:(int)(cb->bracount),
          MAX_GROUP_NUMBER, ERR61,
          &i, &errorcode)) goto FAILED;
      PCRE2_ASSERT(i >= 0);
      terminator = CHAR_NUL;
      SET_RECURSION:
      *parsed_pattern++ = META_RECURSE | (uint32_t)i;
      offset = (PCRE2_SIZE)(ptr - cb->start_pattern);
      goto READ_RECURSION_ARGUMENTS;
      case CHAR_AMPERSAND:
      RECURSE_BY_NAME:
      if (!read_name(&ptr, ptrend, utf, 0, &offset, &name,
          &namelen, &errorcode, cb)) goto FAILED;
      *parsed_pattern++ = META_RECURSE_BYNAME;
      *parsed_pattern++ = namelen;
      terminator = CHAR_NUL;
      READ_RECURSION_ARGUMENTS:
      PUTOFFSET(offset, parsed_pattern);
      okquantifier = TRUE;
      if (terminator != CHAR_NUL) break;
      if (ptr < ptrend && *ptr == CHAR_LEFT_PARENTHESIS)
        {
        parsed_pattern = parse_capture_list(&ptr, ptrend, utf, parsed_pattern,
                                            offset, &errorcode, cb);
        if (parsed_pattern == NULL) goto FAILED;
        }
      if (ptr >= ptrend || *ptr != CHAR_RIGHT_PARENTHESIS)
        goto UNCLOSED_PARENTHESIS;
      ptr++;
      break;
      case CHAR_C:
      if ((xoptions & PCRE2_EXTRA_NEVER_CALLOUT) != 0)
        {
        ptr++;
        errorcode = ERR103;
        goto FAILED;
        }
      if (++ptr >= ptrend) goto UNCLOSED_PARENTHESIS;
      expect_cond_assert = prev_expect_cond_assert - 1;
      if (previous_callout != NULL && (options & PCRE2_AUTO_CALLOUT) != 0 &&
          previous_callout == parsed_pattern - 4 &&
          parsed_pattern[-1] == 255)
        parsed_pattern = previous_callout;
      previous_callout = parsed_pattern;
      after_manual_callout = 1;
      if (*ptr != CHAR_RIGHT_PARENTHESIS && !IS_DIGIT(*ptr))
        {
        PCRE2_SIZE calloutlength;
        PCRE2_SPTR startptr = ptr;
        delimiter = 0;
        for (i = 0; PRIV(callout_start_delims)[i] != 0; i++)
          {
          if (*ptr == PRIV(callout_start_delims)[i])
            {
            delimiter = PRIV(callout_end_delims)[i];
            break;
            }
          }
        if (delimiter == 0)
          {
          errorcode = ERR82;
          goto FAILED_FORWARD;
          }
        *parsed_pattern = META_CALLOUT_STRING;
        parsed_pattern += 3;
        for (;;)
          {
          if (++ptr >= ptrend)
            {
            errorcode = ERR81;
            ptr = startptr;
            goto FAILED;
            }
          if (*ptr == delimiter && (++ptr >= ptrend || *ptr != delimiter))
            break;
          }
        calloutlength = (PCRE2_SIZE)(ptr - startptr);
        if (calloutlength > UINT32_MAX)
          {
          errorcode = ERR72;
          goto FAILED;
          }
        *parsed_pattern++ = (uint32_t)calloutlength;
        offset = (PCRE2_SIZE)(startptr - cb->start_pattern);
        PUTOFFSET(offset, parsed_pattern);
        }
      else
        {
        int n = 0;
        *parsed_pattern = META_CALLOUT_NUMBER;
        parsed_pattern += 3;
        while (ptr < ptrend && IS_DIGIT(*ptr))
          {
          n = n * 10 + (*ptr++ - CHAR_0);
          if (n > 255)
            {
            errorcode = ERR38;
            goto FAILED;
            }
          }
        *parsed_pattern++ = n;
        }
      if (ptr >= ptrend || *ptr != CHAR_RIGHT_PARENTHESIS)
        {
        errorcode = ERR39;
        goto FAILED;
        }
      ptr++;
      previous_callout[1] = (uint32_t)(ptr - cb->start_pattern);
      previous_callout[2] = 0;
      break;
      case CHAR_LEFT_PARENTHESIS:
      if (++ptr >= ptrend) goto UNCLOSED_PARENTHESIS;
      nest_depth++;
      if (*ptr == CHAR_QUESTION_MARK || *ptr == CHAR_ASTERISK)
        {
        *parsed_pattern++ = META_COND_ASSERT;
        ptr--;
        expect_cond_assert = 2;
        break;
        }
      if (read_number(&ptr, ptrend, cb->bracount, MAX_GROUP_NUMBER, ERR61, &i,
          &errorcode))
        {
        PCRE2_ASSERT(i >= 0);
        if (i <= 0)
          {
          errorcode = ERR15;
          goto FAILED;
          }
        *parsed_pattern++ = META_COND_NUMBER;
        offset = (PCRE2_SIZE)(ptr - cb->start_pattern - 2);
        PUTOFFSET(offset, parsed_pattern);
        *parsed_pattern++ = i;
        }
      else if (errorcode != 0) goto FAILED;
      else if (ptrend - ptr >= 10 &&
               PRIV(strncmp_c8)(ptr, STRING_VERSION, 7) == 0 &&
               ptr[7] != CHAR_RIGHT_PARENTHESIS)
        {
        uint32_t ge = 0;
        int major = 0;
        int minor = 0;
        ptr += 7;
        if (*ptr == CHAR_GREATER_THAN_SIGN)
          {
          ge = 1;
          ptr++;
          }
        if (*ptr != CHAR_EQUALS_SIGN || (ptr++, !IS_DIGIT(*ptr)))
          {
          errorcode = ERR79;
          if (!ge) goto FAILED_FORWARD;
          goto FAILED;
          }
        if (!read_number(&ptr, ptrend, -1, 1000, ERR79, &major, &errorcode))
          goto FAILED;
        if (ptr < ptrend && *ptr == CHAR_DOT)
          {
          if (++ptr >= ptrend || !IS_DIGIT(*ptr))
            {
            errorcode = ERR79;
            if (ptr < ptrend) goto FAILED_FORWARD;
            goto FAILED;
            }
          if (!read_number(&ptr, ptrend, -1, 1000, ERR79, &minor, &errorcode))
            goto FAILED;
          }
        if (ptr >= ptrend || *ptr != CHAR_RIGHT_PARENTHESIS)
          {
          errorcode = ERR79;
          if (ptr < ptrend) goto FAILED_FORWARD;
          goto FAILED;
          }
        *parsed_pattern++ = META_COND_VERSION;
        *parsed_pattern++ = ge;
        *parsed_pattern++ = major;
        *parsed_pattern++ = minor;
        }
      else
        {
        BOOL was_r_ampersand = FALSE;
        if (*ptr == CHAR_R && ptrend - ptr > 1 && ptr[1] == CHAR_AMPERSAND)
          {
          terminator = CHAR_RIGHT_PARENTHESIS;
          was_r_ampersand = TRUE;
          ptr++;
          }
        else if (*ptr == CHAR_LESS_THAN_SIGN)
          terminator = CHAR_GREATER_THAN_SIGN;
        else if (*ptr == CHAR_APOSTROPHE)
          terminator = CHAR_APOSTROPHE;
        else
          {
          terminator = CHAR_RIGHT_PARENTHESIS;
          ptr--;
          }
        if (!read_name(&ptr, ptrend, utf, terminator, &offset, &name, &namelen,
            &errorcode, cb)) goto FAILED;
        if (was_r_ampersand)
          {
          *parsed_pattern = META_COND_RNAME;
          ptr--;
          }
        else if (terminator == CHAR_RIGHT_PARENTHESIS)
          {
          if (namelen == 6 && PRIV(strncmp_c8)(name, STRING_DEFINE, 6) == 0)
            *parsed_pattern = META_COND_DEFINE;
          else
            {
            for (i = 1; i < (int)namelen; i++)
              if (!IS_DIGIT(name[i])) break;
            *parsed_pattern = (*name == CHAR_R && i >= (int)namelen)?
              META_COND_RNUMBER : META_COND_NAME;
            }
          ptr--;
          }
        else *parsed_pattern = META_COND_NAME;
        if (*parsed_pattern++ != META_COND_DEFINE) *parsed_pattern++ = namelen;
        PUTOFFSET(offset, parsed_pattern);
        }
      if (ptr >= ptrend || *ptr != CHAR_RIGHT_PARENTHESIS)
        {
        errorcode = ERR24;
        goto FAILED;
        }
      ptr++;
      break;
      case CHAR_GREATER_THAN_SIGN:
      ATOMIC_GROUP:
      *parsed_pattern++ = META_ATOMIC;
      nest_depth++;
      ptr++;
      break;
      case CHAR_EQUALS_SIGN:
      POSITIVE_LOOK_AHEAD:
      *parsed_pattern++ = META_LOOKAHEAD;
      ptr++;
      goto POST_ASSERTION;
      case CHAR_ASTERISK:
      POSITIVE_NONATOMIC_LOOK_AHEAD:
      *parsed_pattern++ = META_LOOKAHEAD_NA;
      ptr++;
      goto POST_ASSERTION;
      case CHAR_EXCLAMATION_MARK:
      NEGATIVE_LOOK_AHEAD:
      *parsed_pattern++ = META_LOOKAHEADNOT;
      ptr++;
      goto POST_ASSERTION;
      case CHAR_LESS_THAN_SIGN:
      if (ptrend - ptr <= 1 ||
         (ptr[1] != CHAR_EQUALS_SIGN &&
          ptr[1] != CHAR_EXCLAMATION_MARK &&
          ptr[1] != CHAR_ASTERISK))
        {
        terminator = CHAR_GREATER_THAN_SIGN;
        goto DEFINE_NAME;
        }
      *parsed_pattern++ = (ptr[1] == CHAR_EQUALS_SIGN)?
        META_LOOKBEHIND : (ptr[1] == CHAR_EXCLAMATION_MARK)?
        META_LOOKBEHINDNOT : META_LOOKBEHIND_NA;
      POST_LOOKBEHIND:
      *has_lookbehind = TRUE;
      offset = (PCRE2_SIZE)(ptr - cb->start_pattern - 2);
      PUTOFFSET(offset, parsed_pattern);
      ptr += 2;
      POST_ASSERTION:
      nest_depth++;
      if (prev_expect_cond_assert > 0)
        {
        if (top_nest == NULL) top_nest = (nest_save *)(cb->start_workspace);
        else if (++top_nest >= end_nests)
          {
          errorcode = ERR84;
          goto FAILED;
          }
        top_nest->nest_depth = nest_depth;
        top_nest->flags = NSF_CONDASSERT;
        top_nest->options = options & PARSE_TRACKED_OPTIONS;
        top_nest->xoptions = xoptions & PARSE_TRACKED_EXTRA_OPTIONS;
        }
      break;
      case CHAR_APOSTROPHE:
      terminator = CHAR_APOSTROPHE;
      DEFINE_NAME:
      if (!read_name(&ptr, ptrend, utf, terminator, &offset, &name, &namelen,
          &errorcode, cb)) goto FAILED;
      if (cb->bracount >= MAX_GROUP_NUMBER)
        {
        errorcode = ERR97;
        goto FAILED;
        }
      cb->bracount++;
      *parsed_pattern++ = META_CAPTURE | cb->bracount;
      nest_depth++;
      if (cb->names_found >= MAX_NAME_COUNT)
        {
        errorcode = ERR49;
        goto FAILED;
        }
      if (namelen + IMM2_SIZE + 1 > cb->name_entry_size)
        cb->name_entry_size = (uint16_t)(namelen + IMM2_SIZE + 1);
      is_dupname = FALSE;
      hash = PRIV(compile_get_hash_from_name)(name, namelen);
      ng = cb->named_groups;
      for (i = 0; i < cb->names_found; i++, ng++)
        {
        if (namelen == ng->length && hash == NAMED_GROUP_GET_HASH(ng) &&
            PRIV(strncmp)(name, ng->name, (PCRE2_SIZE)namelen) == 0)
          {
          if (ng->number == cb->bracount) break;
          if ((options & PCRE2_DUPNAMES) == 0)
            {
            errorcode = ERR43;
            goto FAILED;
            }
          ng->hash_dup |= NAMED_GROUP_IS_DUPNAME;
          is_dupname = TRUE;
          cb->dupnames = TRUE;
          name = ng->name;
          namelen = 0;
          for (; i < cb->names_found; i++, ng++)
            if (ng->name == name && ng->number == cb->bracount)
              break;
          break;
          }
        else if (ng->number == cb->bracount)
          {
          errorcode = ERR65;
          goto FAILED;
          }
        }
      if (i < cb->names_found) break;
      if (cb->names_found >= cb->named_group_list_size)
        {
        uint32_t newsize = cb->named_group_list_size * 2;
        named_group *newspace =
          cb->cx->memctl.malloc(newsize * sizeof(named_group),
          cb->cx->memctl.memory_data);
        if (newspace == NULL)
          {
          errorcode = ERR21;
          goto FAILED;
          }
        memcpy(newspace, cb->named_groups,
          cb->named_group_list_size * sizeof(named_group));
        if (cb->named_group_list_size > NAMED_GROUP_LIST_SIZE)
          cb->cx->memctl.free((void *)cb->named_groups,
          cb->cx->memctl.memory_data);
        cb->named_groups = newspace;
        cb->named_group_list_size = newsize;
        }
      if (is_dupname)
        hash |= NAMED_GROUP_IS_DUPNAME;
      cb->named_groups[cb->names_found].name = name;
      cb->named_groups[cb->names_found].length = (uint16_t)namelen;
      cb->named_groups[cb->names_found].number = cb->bracount;
      cb->named_groups[cb->names_found].hash_dup = hash;
      cb->names_found++;
      break;
      case CHAR_LEFT_SQUARE_BRACKET:
      class_mode_state = CLASS_MODE_PERL_EXT;
      c = *ptr++;
      goto FROM_PERL_EXTENDED_CLASS;
      }
    break;
    case CHAR_VERTICAL_LINE:
    if (top_nest != NULL && top_nest->nest_depth == nest_depth &&
        (top_nest->flags & NSF_RESET) != 0)
      {
      if (cb->bracount > top_nest->max_group)
        top_nest->max_group = (uint16_t)cb->bracount;
      cb->bracount = top_nest->reset_group;
      }
    *parsed_pattern++ = META_ALT;
    break;
    case CHAR_RIGHT_PARENTHESIS:
    okquantifier = TRUE;
    if (top_nest != NULL && top_nest->nest_depth == nest_depth)
      {
      options = (options & ~PARSE_TRACKED_OPTIONS) | top_nest->options;
      xoptions = (xoptions & ~PARSE_TRACKED_EXTRA_OPTIONS) | top_nest->xoptions;
      if ((top_nest->flags & NSF_RESET) != 0 &&
          top_nest->max_group > cb->bracount)
        cb->bracount = top_nest->max_group;
      if ((top_nest->flags & NSF_CONDASSERT) != 0)
        okquantifier = FALSE;
      if ((top_nest->flags & NSF_ATOMICSR) != 0)
        {
        *parsed_pattern++ = META_KET;
#ifdef PCRE2_DEBUG
        PCRE2_ASSERT(parsed_pattern_extra > 0);
        parsed_pattern_extra--;
#endif
        }
      if (top_nest == (nest_save *)(cb->start_workspace)) top_nest = NULL;
        else top_nest--;
      }
    if (nest_depth == 0)
      {
      errorcode = ERR22;
      goto FAILED;
      }
    nest_depth--;
    *parsed_pattern++ = META_KET;
    break;
    }
  }
if (inverbname && ptr >= ptrend)
  {
  errorcode = ERR60;
  goto FAILED;
  }
PARSED_END:
PCRE2_ASSERT((parsed_pattern - parsed_pattern_check) +
             (parsed_pattern_extra - parsed_pattern_extra_check) <=
               max_parsed_pattern(ptr_check, ptr, utf, options));
parsed_pattern = manage_callouts(ptr, &previous_callout, auto_callout,
  parsed_pattern, cb);
if ((xoptions & PCRE2_EXTRA_MATCH_LINE) != 0)
  {
  *parsed_pattern++ = META_KET;
  *parsed_pattern++ = META_DOLLAR;
  }
else if ((xoptions & PCRE2_EXTRA_MATCH_WORD) != 0)
  {
  *parsed_pattern++ = META_KET;
  *parsed_pattern++ = META_ESCAPE + ESC_b;
  }
if (parsed_pattern >= parsed_pattern_end)
  {
  PCRE2_DEBUG_UNREACHABLE();
  errorcode = ERR63;
  goto FAILED;
  }
*parsed_pattern = META_END;
if (nest_depth == 0) return 0;
UNCLOSED_PARENTHESIS:
errorcode = ERR14;
FAILED:
cb->erroroffset = (PCRE2_SIZE)(ptr - cb->start_pattern);
return errorcode;
FAILED_BACK:
ptr--;
#ifdef SUPPORT_UNICODE
if (utf) BACKCHAR(ptr);
#endif
goto FAILED;
FAILED_FORWARD:
ptr++;
#ifdef SUPPORT_UNICODE
if (utf) FORWARDCHARTEST(ptr, ptrend);
#endif
goto FAILED;
}
static const PCRE2_UCHAR*
first_significant_code(PCRE2_SPTR code, BOOL skipassert)
{
for (;;)
  {
  switch ((int)*code)
    {
    case OP_ASSERT_NOT:
    case OP_ASSERTBACK:
    case OP_ASSERTBACK_NOT:
    case OP_ASSERTBACK_NA:
    if (!skipassert) return code;
    do code += GET(code, 1); while (*code == OP_ALT);
    code += PRIV(OP_lengths)[*code];
    break;
    case OP_WORD_BOUNDARY:
    case OP_NOT_WORD_BOUNDARY:
    case OP_UCP_WORD_BOUNDARY:
    case OP_NOT_UCP_WORD_BOUNDARY:
    if (!skipassert) return code;
    PCRE2_FALLTHROUGH
    case OP_CALLOUT:
    case OP_CREF:
    case OP_DNCREF:
    case OP_RREF:
    case OP_DNRREF:
    case OP_FALSE:
    case OP_TRUE:
    code += PRIV(OP_lengths)[*code];
    break;
    case OP_CALLOUT_STR:
    code += GET(code, 1 + 2*LINK_SIZE);
    break;
    case OP_SKIPZERO:
    code += 2 + GET(code, 2) + LINK_SIZE;
    break;
    case OP_COND:
    case OP_SCOND:
    if (code[1+LINK_SIZE] != OP_FALSE ||
        code[GET(code, 1)] != OP_KET)
      return code;
    code += GET(code, 1) + 1 + LINK_SIZE;
    break;
    case OP_MARK:
    case OP_COMMIT_ARG:
    case OP_PRUNE_ARG:
    case OP_SKIP_ARG:
    case OP_THEN_ARG:
    code += code[1] + PRIV(OP_lengths)[*code];
    break;
    default:
    return code;
    }
  }
PCRE2_DEBUG_UNREACHABLE();
}
static int
compile_branch(uint32_t *optionsptr, uint32_t *xoptionsptr,
  PCRE2_UCHAR **codeptr, uint32_t **pptrptr, int *errorcodeptr,
  uint32_t *firstcuptr, uint32_t *firstcuflagsptr, uint32_t *reqcuptr,
  uint32_t *reqcuflagsptr, branch_chain *bcptr, open_capitem *open_caps,
  compile_block *cb, PCRE2_SIZE *lengthptr)
{
int bravalue = 0;
int okreturn = -1;
int group_return = 0;
uint32_t repeat_min = 0, repeat_max = 0;
uint32_t greedy_default, greedy_non_default;
uint32_t repeat_type, op_type;
uint32_t options = *optionsptr;
uint32_t xoptions = *xoptionsptr;
uint32_t firstcu, reqcu;
uint32_t zeroreqcu, zerofirstcu;
uint32_t *pptr = *pptrptr;
uint32_t meta, meta_arg;
uint32_t firstcuflags, reqcuflags;
uint32_t zeroreqcuflags, zerofirstcuflags;
uint32_t req_caseopt, reqvary, tempreqvary;
PCRE2_SIZE offset = 0;
PCRE2_SIZE length_prevgroup = 0;
PCRE2_UCHAR *code = *codeptr;
PCRE2_UCHAR *last_code = code;
PCRE2_UCHAR *orig_code = code;
PCRE2_UCHAR *tempcode;
PCRE2_UCHAR *previous = NULL;
PCRE2_UCHAR op_previous;
BOOL groupsetfirstcu = FALSE;
BOOL had_accept = FALSE;
BOOL matched_char = FALSE;
BOOL previous_matched_char = FALSE;
BOOL reset_caseful = FALSE;
#ifdef SUPPORT_UNICODE
BOOL utf = (options & PCRE2_UTF) != 0;
BOOL ucp = (options & PCRE2_UCP) != 0;
#else
BOOL utf = FALSE;
#endif
greedy_default = ((options & PCRE2_UNGREEDY) != 0);
greedy_non_default = greedy_default ^ 1;
firstcu = reqcu = zerofirstcu = zeroreqcu = 0;
firstcuflags = reqcuflags = zerofirstcuflags = zeroreqcuflags = REQ_UNSET;
req_caseopt = ((options & PCRE2_CASELESS) != 0)? REQ_CASELESS : 0;
for (;; pptr++)
  {
  BOOL possessive_quantifier;
  BOOL note_group_empty;
  uint32_t mclength;
  uint32_t skipunits;
  uint32_t subreqcu, subfirstcu;
  uint32_t groupnumber;
  uint32_t verbarglen, verbculen;
  uint32_t subreqcuflags, subfirstcuflags;
  open_capitem *oc;
  PCRE2_UCHAR mcbuffer[8];
  meta = META_CODE(*pptr);
  meta_arg = META_DATA(*pptr);
  if (lengthptr != NULL)
    {
    if (code >= cb->start_workspace + cb->workspace_size)
      {
      PCRE2_DEBUG_UNREACHABLE();
      *errorcodeptr = ERR52;
      cb->erroroffset = 0;
      return 0;
      }
    if (code > cb->start_workspace + cb->workspace_size -
        WORK_SIZE_SAFETY_MARGIN)
      {
      *errorcodeptr = ERR86;
      cb->erroroffset = 0;
      return 0;
      }
    if (code < last_code) code = last_code;
    if (meta < META_ASTERISK || meta > META_MINMAX_QUERY)
      {
      if (OFLOW_MAX - *lengthptr < (PCRE2_SIZE)(code - orig_code))
        {
        *errorcodeptr = ERR20;
        cb->erroroffset = 0;
        return 0;
        }
      *lengthptr += (PCRE2_SIZE)(code - orig_code);
      if (*lengthptr > MAX_PATTERN_SIZE)
        {
        *errorcodeptr = ERR20;
        cb->erroroffset = 0;
        return 0;
        }
      code = orig_code;
      }
    last_code = code;
    }
  if (meta < META_ASTERISK || meta > META_MINMAX_QUERY)
    {
    previous = code;
    if (matched_char && !had_accept) okreturn = 1;
    }
  previous_matched_char = matched_char;
  matched_char = FALSE;
  note_group_empty = FALSE;
  skipunits = 0;
  switch(meta)
    {
    case META_END:
    case META_ALT:
    case META_KET:
    *firstcuptr = firstcu;
    *firstcuflagsptr = firstcuflags;
    *reqcuptr = reqcu;
    *reqcuflagsptr = reqcuflags;
    *codeptr = code;
    *pptrptr = pptr;
    return okreturn;
    case META_CIRCUMFLEX:
    if ((options & PCRE2_MULTILINE) != 0)
      {
      if (firstcuflags == REQ_UNSET)
        zerofirstcuflags = firstcuflags = REQ_NONE;
      *code++ = OP_CIRCM;
      }
    else *code++ = OP_CIRC;
    break;
    case META_DOLLAR:
    *code++ = ((options & PCRE2_MULTILINE) != 0)? OP_DOLLM : OP_DOLL;
    break;
    case META_DOT:
    matched_char = TRUE;
    if (firstcuflags == REQ_UNSET) firstcuflags = REQ_NONE;
    zerofirstcu = firstcu;
    zerofirstcuflags = firstcuflags;
    zeroreqcu = reqcu;
    zeroreqcuflags = reqcuflags;
    *code++ = ((options & PCRE2_DOTALL) != 0)? OP_ALLANY: OP_ANY;
    break;
    case META_CLASS_EMPTY:
    case META_CLASS_EMPTY_NOT:
    matched_char = TRUE;
    if (meta == META_CLASS_EMPTY_NOT) *code++ = OP_ALLANY;
    else
      {
      *code++ = OP_CLASS;
      memset(code, 0, 32);
      code += 32 / sizeof(PCRE2_UCHAR);
      }
    if (firstcuflags == REQ_UNSET) firstcuflags = REQ_NONE;
    zerofirstcu = firstcu;
    zerofirstcuflags = firstcuflags;
    break;
    case META_CLASS_NOT:
    case META_CLASS:
    matched_char = TRUE;
    if ((*pptr & CLASS_IS_ECLASS) != 0)
      {
      if (!PRIV(compile_class_nested)(options, xoptions, &pptr, &code,
                                      errorcodeptr, cb, lengthptr))
        return 0;
      goto CLASS_END_PROCESSING;
      }
    if (pptr[1] < META_END && pptr[2] == META_CLASS_END)
      {
      uint32_t c = pptr[1];
      pptr += 2;
      if (meta == META_CLASS)
        {
        meta = c;
        goto NORMAL_CHAR_SET;
        }
      zeroreqcu = reqcu;
      zeroreqcuflags = reqcuflags;
      if (firstcuflags == REQ_UNSET) firstcuflags = REQ_NONE;
      zerofirstcu = firstcu;
      zerofirstcuflags = firstcuflags;
#ifdef SUPPORT_UNICODE
      if ((utf||ucp) && (options & PCRE2_CASELESS) != 0)
        {
        uint32_t caseset;
        if ((xoptions & (PCRE2_EXTRA_TURKISH_CASING|PCRE2_EXTRA_CASELESS_RESTRICT)) ==
              PCRE2_EXTRA_TURKISH_CASING &&
            UCD_ANY_I(c))
          {
          caseset = PRIV(ucd_turkish_dotted_i_caseset) + (UCD_DOTTED_I(c)? 0 : 3);
          }
        else if ((caseset = UCD_CASESET(c)) != 0 &&
                 (xoptions & PCRE2_EXTRA_CASELESS_RESTRICT) != 0 &&
                 PRIV(ucd_caseless_sets)[caseset] < 128)
          {
          caseset = 0;
          }
        if (caseset != 0)
          {
          *code++ = OP_NOTPROP;
          *code++ = PT_CLIST;
          *code++ = caseset;
          break;
          }
        }
#endif
      *code++ = ((options & PCRE2_CASELESS) != 0)? OP_NOTI: OP_NOT;
      code += PUTCHAR(c, code);
      break;
      }
    if (meta == META_CLASS && pptr[1] < META_END && pptr[2] < META_END &&
        pptr[3] == META_CLASS_END)
      {
      uint32_t c = pptr[1];
#ifdef SUPPORT_UNICODE
      if ((UCD_CASESET(c) == 0 ||
           ((xoptions & PCRE2_EXTRA_CASELESS_RESTRICT) != 0 &&
            c < 128 && pptr[2] < 128)) &&
          !((xoptions & (PCRE2_EXTRA_TURKISH_CASING|PCRE2_EXTRA_CASELESS_RESTRICT)) ==
              PCRE2_EXTRA_TURKISH_CASING &&
            UCD_ANY_I(c)))
#endif
        {
        uint32_t d;
#ifdef SUPPORT_UNICODE
        if ((utf || ucp) && c > 127) d = UCD_OTHERCASE(c); else
#endif
          {
#if PCRE2_CODE_UNIT_WIDTH != 8
          if (c > 255) d = c; else
#endif
          d = TABLE_GET(c, cb->fcc, c);
          }
        if (c != d && pptr[2] == d)
          {
          pptr += 3;
          meta = c;
          if ((options & PCRE2_CASELESS) == 0)
            {
            reset_caseful = TRUE;
            options |= PCRE2_CASELESS;
            req_caseopt = REQ_CASELESS;
            }
          goto CLASS_CASELESS_CHAR;
          }
        }
      }
    pptr = PRIV(compile_class_not_nested)(options, xoptions, pptr + 1,
                                          &code, meta == META_CLASS_NOT, NULL,
                                          errorcodeptr, cb, lengthptr);
    if (pptr == NULL) return 0;
    PCRE2_ASSERT(*pptr == META_CLASS_END);
    CLASS_END_PROCESSING:
    if (firstcuflags == REQ_UNSET) firstcuflags = REQ_NONE;
    zerofirstcu = firstcu;
    zerofirstcuflags = firstcuflags;
    zeroreqcu = reqcu;
    zeroreqcuflags = reqcuflags;
    break;
    case META_ACCEPT:
    cb->had_accept = had_accept = TRUE;
    for (oc = open_caps;
         oc != NULL && oc->assert_depth >= cb->assert_depth;
         oc = oc->next)
      {
      if (lengthptr != NULL)
        {
        *lengthptr += CU2BYTES(1) + IMM2_SIZE;
        }
      else
        {
        *code++ = OP_CLOSE;
        PUT2INC(code, 0, oc->number);
        }
      }
    *code++ = (cb->assert_depth > 0)? OP_ASSERT_ACCEPT : OP_ACCEPT;
    if (firstcuflags == REQ_UNSET) firstcuflags = REQ_NONE;
    break;
    case META_PRUNE:
    case META_SKIP:
    cb->had_pruneorskip = TRUE;
    PCRE2_FALLTHROUGH
    case META_COMMIT:
    case META_FAIL:
    *code++ = verbops[(meta - META_MARK) >> 16];
    break;
    case META_THEN:
    cb->external_flags |= PCRE2_HASTHEN;
    *code++ = OP_THEN;
    break;
    case META_THEN_ARG:
    cb->external_flags |= PCRE2_HASTHEN;
    goto VERB_ARG;
    case META_PRUNE_ARG:
    case META_SKIP_ARG:
    cb->had_pruneorskip = TRUE;
    PCRE2_FALLTHROUGH
    case META_MARK:
    case META_COMMIT_ARG:
    VERB_ARG:
    *code++ = verbops[(meta - META_MARK) >> 16];
    verbarglen = *(++pptr);
    verbculen = 0;
    tempcode = code++;
    for (int i = 0; i < (int)verbarglen; i++)
      {
      meta = *(++pptr);
#ifdef SUPPORT_UNICODE
      if (utf) mclength = PRIV(ord2utf)(meta, mcbuffer); else
#endif
        {
        mclength = 1;
        mcbuffer[0] = meta;
        }
      if (lengthptr != NULL) *lengthptr += mclength; else
        {
        memcpy(code, mcbuffer, CU2BYTES(mclength));
        code += mclength;
        verbculen += mclength;
        }
      }
    *tempcode = verbculen;
    *code++ = 0;
    break;
    case META_OPTIONS:
    *optionsptr = options = *(++pptr);
    *xoptionsptr = xoptions = *(++pptr);
    greedy_default = ((options & PCRE2_UNGREEDY) != 0);
    greedy_non_default = greedy_default ^ 1;
    req_caseopt = ((options & PCRE2_CASELESS) != 0)? REQ_CASELESS : 0;
    break;
    case META_OFFSET:
    if (lengthptr != NULL)
      {
      pptr = PRIV(compile_parse_scan_substr_args)(pptr, errorcodeptr, cb, lengthptr);
      if (pptr == NULL)
        return 0;
      break;
      }
    while (TRUE)
      {
      int count, index;
      named_group *ng;
      switch (META_CODE(*pptr))
        {
        case META_OFFSET:
        pptr++;
        SKIPOFFSET(pptr);
        continue;
        case META_CAPTURE_NAME:
        ng = cb->named_groups + pptr[1];
        pptr += 2;
        count = 0;
        index = 0;
        if (!PRIV(compile_find_dupname_details)(ng->name, ng->length, &index,
          &count, errorcodeptr, cb)) return 0;
        code[0] = OP_DNCREF;
        PUT2(code, 1, index);
        PUT2(code, 1 + IMM2_SIZE, count);
        code += 1 + 2 * IMM2_SIZE;
        continue;
        case META_CAPTURE_NUMBER:
        pptr += 2;
        if (pptr[-1] == 0) continue;
        code[0] = OP_CREF;
        PUT2(code, 1, pptr[-1]);
        code += 1 + IMM2_SIZE;
        continue;
        default:
        break;
        }
      break;
      }
    --pptr;
    break;
    case META_SCS:
    bravalue = OP_ASSERT_SCS;
    cb->assert_depth += 1;
    goto GROUP_PROCESS;
    case META_COND_RNUMBER:
    case META_COND_NAME:
    case META_COND_RNAME:
    bravalue = OP_COND;
    if (lengthptr != NULL)
      {
      uint32_t i;
      PCRE2_SPTR name;
      named_group *ng;
      uint32_t *start_pptr = pptr;
      uint32_t length = *(++pptr);
      GETPLUSOFFSET(offset, pptr);
      name = cb->start_pattern + offset;
      ng = PRIV(compile_find_named_group)(name, length, cb);
      if (ng == NULL)
        {
        groupnumber = 0;
        if (meta == META_COND_RNUMBER)
          {
          for (i = 1; i < length; i++)
            {
            groupnumber = groupnumber * 10 + (name[i] - CHAR_0);
            if (groupnumber > MAX_GROUP_NUMBER)
              {
              *errorcodeptr = ERR61;
              cb->erroroffset = offset + i;
              return 0;
              }
            }
          }
        if (meta != META_COND_RNUMBER || groupnumber > cb->bracount)
          {
          *errorcodeptr = ERR15;
          cb->erroroffset = offset;
          return 0;
          }
        if (groupnumber == 0) groupnumber = RREF_ANY;
        PCRE2_ASSERT(start_pptr[0] == META_COND_RNUMBER);
        start_pptr[1] = groupnumber;
        skipunits = 1+IMM2_SIZE;
        goto GROUP_PROCESS_NOTE_EMPTY;
        }
      if (meta == META_COND_RNUMBER) meta = META_COND_NAME;
      if ((ng->hash_dup & NAMED_GROUP_IS_DUPNAME) == 0)
        {
        if (ng->number > cb->top_backref) cb->top_backref = ng->number;
        start_pptr[0] = meta;
        start_pptr[1] = ng->number;
        skipunits = 1 + IMM2_SIZE;
        goto GROUP_PROCESS_NOTE_EMPTY;
        }
      start_pptr[0] = meta | 1;
      start_pptr[1] = (uint32_t)(ng - cb->named_groups);
      skipunits = 1 + 2 * IMM2_SIZE;
      }
    else
      {
      int count, index;
      named_group *ng;
      if (meta == META_COND_RNUMBER)
        {
        code[1+LINK_SIZE] = OP_RREF;
        PUT2(code, 2 + LINK_SIZE, pptr[1]);
        skipunits = 1 + IMM2_SIZE;
        pptr += 1 + SIZEOFFSET;
        goto GROUP_PROCESS_NOTE_EMPTY;
        }
      if (meta_arg == 0)
        {
        code[1+LINK_SIZE] = (meta == META_COND_RNAME)? OP_RREF : OP_CREF;
        PUT2(code, 2 + LINK_SIZE, pptr[1]);
        skipunits = 1 + IMM2_SIZE;
        pptr += 1 + SIZEOFFSET;
        goto GROUP_PROCESS_NOTE_EMPTY;
        }
      ng = cb->named_groups + pptr[1];
      count = 0;
      index = 0;
      if (!PRIV(compile_find_dupname_details)(ng->name, ng->length, &index,
            &count, errorcodeptr, cb)) return 0;
      code[1 + LINK_SIZE] = (meta == META_COND_RNAME)? OP_DNRREF : OP_DNCREF;
      PUT2(code, 2 + LINK_SIZE, index);
      PUT2(code, 2 + LINK_SIZE + IMM2_SIZE, count);
      skipunits = 1 + 2 * IMM2_SIZE;
      pptr += 1 + SIZEOFFSET;
      }
    PCRE2_ASSERT(meta != META_CAPTURE_NAME);
    goto GROUP_PROCESS_NOTE_EMPTY;
    case META_COND_DEFINE:
    bravalue = OP_COND;
    GETPLUSOFFSET(offset, pptr);
    code[1+LINK_SIZE] = OP_DEFINE;
    skipunits = 1;
    goto GROUP_PROCESS;
    case META_COND_NUMBER:
    bravalue = OP_COND;
    GETPLUSOFFSET(offset, pptr);
    groupnumber = *(++pptr);
    if (groupnumber > cb->bracount)
      {
      *errorcodeptr = ERR15;
      cb->erroroffset = offset;
      return 0;
      }
    if (groupnumber > cb->top_backref) cb->top_backref = groupnumber;
    offset -= 2;
    code[1+LINK_SIZE] = OP_CREF;
    skipunits = 1+IMM2_SIZE;
    PUT2(code, 2+LINK_SIZE, groupnumber);
    goto GROUP_PROCESS_NOTE_EMPTY;
    case META_COND_VERSION:
    bravalue = OP_COND;
    if (pptr[1] > 0)
      code[1+LINK_SIZE] = ((PCRE2_MAJOR > pptr[2]) ||
        (PCRE2_MAJOR == pptr[2] && PCRE2_MINOR >= pptr[3]))?
          OP_TRUE : OP_FALSE;
    else
      code[1+LINK_SIZE] = (PCRE2_MAJOR == pptr[2] && PCRE2_MINOR == pptr[3])?
        OP_TRUE : OP_FALSE;
    skipunits = 1;
    pptr += 3;
    goto GROUP_PROCESS_NOTE_EMPTY;
    case META_COND_ASSERT:
    bravalue = OP_COND;
    goto GROUP_PROCESS_NOTE_EMPTY;
    case META_LOOKAHEAD:
    bravalue = OP_ASSERT;
    cb->assert_depth += 1;
    goto GROUP_PROCESS;
    case META_LOOKAHEAD_NA:
    bravalue = OP_ASSERT_NA;
    cb->assert_depth += 1;
    goto GROUP_PROCESS;
    case META_LOOKAHEADNOT:
    if (pptr[1] == META_KET &&
         (pptr[2] < META_ASTERISK || pptr[2] > META_MINMAX_QUERY))
      {
      *code++ = OP_FAIL;
      pptr++;
      }
    else
      {
      bravalue = OP_ASSERT_NOT;
      cb->assert_depth += 1;
      goto GROUP_PROCESS;
      }
    break;
    case META_LOOKBEHIND:
    bravalue = OP_ASSERTBACK;
    cb->assert_depth += 1;
    goto GROUP_PROCESS;
    case META_LOOKBEHINDNOT:
    bravalue = OP_ASSERTBACK_NOT;
    cb->assert_depth += 1;
    goto GROUP_PROCESS;
    case META_LOOKBEHIND_NA:
    bravalue = OP_ASSERTBACK_NA;
    cb->assert_depth += 1;
    goto GROUP_PROCESS;
    case META_ATOMIC:
    bravalue = OP_ONCE;
    goto GROUP_PROCESS_NOTE_EMPTY;
    case META_SCRIPT_RUN:
    bravalue = OP_SCRIPT_RUN;
    goto GROUP_PROCESS_NOTE_EMPTY;
    case META_NOCAPTURE:
    bravalue = OP_BRA;
    GROUP_PROCESS_NOTE_EMPTY:
    note_group_empty = TRUE;
    GROUP_PROCESS:
    cb->parens_depth += 1;
    *code = bravalue;
    pptr++;
    tempcode = code;
    tempreqvary = cb->req_varyopt;
    length_prevgroup = 0;
    if ((group_return =
         compile_regex(
         options,
         xoptions,
         &tempcode,
         &pptr,
         errorcodeptr,
         skipunits,
         &subfirstcu,
         &subfirstcuflags,
         &subreqcu,
         &subreqcuflags,
         bcptr,
         open_caps,
         cb,
         (lengthptr == NULL)? NULL :
           &length_prevgroup
         )) == 0)
      return 0;
    cb->parens_depth -= 1;
    if (note_group_empty && bravalue != OP_COND && group_return > 0)
      matched_char = TRUE;
    if (bravalue >= OP_ASSERT && bravalue <= OP_ASSERT_SCS)
      cb->assert_depth -= 1;
    if (bravalue == OP_COND && lengthptr == NULL)
      {
      PCRE2_UCHAR *tc = code;
      int condcount = 0;
      do {
         condcount++;
         tc += GET(tc,1);
         }
      while (*tc != OP_KET);
      if (code[LINK_SIZE+1] == OP_DEFINE)
        {
        if (condcount > 1)
          {
          cb->erroroffset = offset;
          *errorcodeptr = ERR54;
          return 0;
          }
        code[LINK_SIZE+1] = OP_FALSE;
        bravalue = OP_DEFINE;
        }
      else
        {
        if (condcount > 2)
          {
          cb->erroroffset = offset;
          *errorcodeptr = ERR27;
          return 0;
          }
        if (condcount == 1) subfirstcuflags = subreqcuflags = REQ_NONE;
          else if (group_return > 0) matched_char = TRUE;
        }
      }
    if (lengthptr != NULL)
      {
      if (OFLOW_MAX - *lengthptr < length_prevgroup - 2 - 2*LINK_SIZE)
        {
        *errorcodeptr = ERR20;
        return 0;
        }
      *lengthptr += length_prevgroup - 2 - 2*LINK_SIZE;
      code++;
      PUTINC(code, 0, 1 + LINK_SIZE);
      *code++ = OP_KET;
      PUTINC(code, 0, 1 + LINK_SIZE);
      break;
      }
    code = tempcode;
    if (bravalue == OP_DEFINE) break;
    zeroreqcu = reqcu;
    zeroreqcuflags = reqcuflags;
    zerofirstcu = firstcu;
    zerofirstcuflags = firstcuflags;
    groupsetfirstcu = FALSE;
    if (bravalue >= OP_ONCE)
      {
      if (firstcuflags == REQ_UNSET && subfirstcuflags != REQ_UNSET)
        {
        if (subfirstcuflags < REQ_NONE)
          {
          firstcu = subfirstcu;
          firstcuflags = subfirstcuflags;
          groupsetfirstcu = TRUE;
          }
        else firstcuflags = REQ_NONE;
        zerofirstcuflags = REQ_NONE;
        }
      else if (subfirstcuflags < REQ_NONE && subreqcuflags >= REQ_NONE)
        {
        subreqcu = subfirstcu;
        subreqcuflags = subfirstcuflags | tempreqvary;
        }
      if (subreqcuflags < REQ_NONE)
        {
        reqcu = subreqcu;
        reqcuflags = subreqcuflags;
        }
      }
    else if ((bravalue == OP_ASSERT || bravalue == OP_ASSERT_NA) &&
             subreqcuflags < REQ_NONE && subfirstcuflags < REQ_NONE)
      {
      reqcu = subreqcu;
      reqcuflags = subreqcuflags;
      }
    break;
    case META_BACKREF_BYNAME:
    case META_RECURSE_BYNAME:
      {
      int count, index;
      PCRE2_SPTR name;
      named_group *ng;
      uint32_t length = *(++pptr);
      GETPLUSOFFSET(offset, pptr);
      name = cb->start_pattern + offset;
      ng = PRIV(compile_find_named_group)(name, length, cb);
      if (ng == NULL)
        {
        *errorcodeptr = ERR15;
        cb->erroroffset = offset;
        return 0;
        }
      groupnumber = ng->number;
      if (meta == META_RECURSE_BYNAME)
        {
        meta_arg = groupnumber;
        goto HANDLE_NUMERICAL_RECURSION;
        }
      cb->backref_map |= (groupnumber < 32)? (1u << groupnumber) : 1;
      if (groupnumber > cb->top_backref)
        cb->top_backref = groupnumber;
      if ((ng->hash_dup & NAMED_GROUP_IS_DUPNAME) == 0)
        {
        meta_arg = groupnumber;
        goto HANDLE_SINGLE_REFERENCE;
        }
      count = 0;
      index = 0;
      if (lengthptr == NULL && !PRIV(compile_find_dupname_details)(name, length,
            &index, &count, errorcodeptr, cb)) return 0;
      if (firstcuflags == REQ_UNSET) firstcuflags = REQ_NONE;
      *code++ = ((options & PCRE2_CASELESS) != 0)? OP_DNREFI : OP_DNREF;
      PUT2INC(code, 0, index);
      PUT2INC(code, 0, count);
      if ((options & PCRE2_CASELESS) != 0)
        *code++ = (((xoptions & PCRE2_EXTRA_CASELESS_RESTRICT) != 0)?
                   REFI_FLAG_CASELESS_RESTRICT : 0) |
                  (((xoptions & PCRE2_EXTRA_TURKISH_CASING) != 0)?
                   REFI_FLAG_TURKISH_CASING : 0);
      }
    break;
    case META_CALLOUT_NUMBER:
    code[0] = OP_CALLOUT;
    PUT(code, 1, pptr[1]);
    PUT(code, 1 + LINK_SIZE, pptr[2]);
    code[1 + 2*LINK_SIZE] = pptr[3];
    pptr += 3;
    code += PRIV(OP_lengths)[OP_CALLOUT];
    break;
    case META_CALLOUT_STRING:
    if (lengthptr != NULL)
      {
      *lengthptr += pptr[3] + (1 + 4*LINK_SIZE);
      pptr += 3;
      SKIPOFFSET(pptr);
      }
    else
      {
      PCRE2_SPTR pp;
      uint32_t delimiter;
      uint32_t length = pptr[3];
      PCRE2_UCHAR *callout_string = code + (1 + 4*LINK_SIZE);
      code[0] = OP_CALLOUT_STR;
      PUT(code, 1, pptr[1]);
      PUT(code, 1 + LINK_SIZE, pptr[2]);
      pptr += 3;
      GETPLUSOFFSET(offset, pptr);
      pp = cb->start_pattern + offset;
      delimiter = *callout_string++ = *pp++;
      if (delimiter == CHAR_LEFT_CURLY_BRACKET)
        delimiter = CHAR_RIGHT_CURLY_BRACKET;
      PUT(code, 1 + 3*LINK_SIZE, (int)(offset + 1));
      while (--length > 1)
        {
        if (*pp == delimiter && pp[1] == delimiter)
          {
          *callout_string++ = delimiter;
          pp += 2;
          length--;
          }
        else *callout_string++ = *pp++;
        }
      *callout_string++ = CHAR_NUL;
      PUT(code, 1 + 2*LINK_SIZE, (int)(callout_string - code));
      code = callout_string;
      }
    break;
    case META_MINMAX_PLUS:
    case META_MINMAX_QUERY:
    case META_MINMAX:
    repeat_min = *(++pptr);
    repeat_max = *(++pptr);
    goto REPEAT;
    case META_ASTERISK:
    case META_ASTERISK_PLUS:
    case META_ASTERISK_QUERY:
    repeat_min = 0;
    repeat_max = REPEAT_UNLIMITED;
    goto REPEAT;
    case META_PLUS:
    case META_PLUS_PLUS:
    case META_PLUS_QUERY:
    repeat_min = 1;
    repeat_max = REPEAT_UNLIMITED;
    goto REPEAT;
    case META_QUERY:
    case META_QUERY_PLUS:
    case META_QUERY_QUERY:
    repeat_min = 0;
    repeat_max = 1;
    REPEAT:
    if (previous_matched_char && repeat_min > 0) matched_char = TRUE;
    reqvary = (repeat_min == repeat_max)? 0 : REQ_VARY;
    if (repeat_min == 0)
      {
      firstcu = zerofirstcu;
      firstcuflags = zerofirstcuflags;
      reqcu = zeroreqcu;
      reqcuflags = zeroreqcuflags;
      }
    switch (meta)
      {
      case META_MINMAX_PLUS:
      case META_ASTERISK_PLUS:
      case META_PLUS_PLUS:
      case META_QUERY_PLUS:
      repeat_type = 0;
      possessive_quantifier = TRUE;
      break;
      case META_MINMAX_QUERY:
      case META_ASTERISK_QUERY:
      case META_PLUS_QUERY:
      case META_QUERY_QUERY:
      repeat_type = greedy_non_default;
      possessive_quantifier = FALSE;
      break;
      default:
      repeat_type = greedy_default;
      possessive_quantifier = FALSE;
      break;
      }
    PCRE2_ASSERT(previous != NULL);
    tempcode = previous;
    op_previous = *previous;
    switch (op_previous)
      {
      case OP_CHAR:
      case OP_CHARI:
      case OP_NOT:
      case OP_NOTI:
      if (repeat_max == 1 && repeat_min == 1) goto END_REPEAT;
      op_type = chartypeoffset[op_previous - OP_CHAR];
#ifdef MAYBE_UTF_MULTI
      if (utf && NOT_FIRSTCU(code[-1]))
        {
        PCRE2_UCHAR *lastchar = code - 1;
        BACKCHAR(lastchar);
        mclength = (uint32_t)(code - lastchar);
        memcpy(mcbuffer, lastchar, CU2BYTES(mclength));
        }
      else
#endif
        {
        mcbuffer[0] = code[-1];
        mclength = 1;
        if (op_previous <= OP_CHARI && repeat_min > 1)
          {
          reqcu = mcbuffer[0];
          reqcuflags = cb->req_varyopt;
          if (op_previous == OP_CHARI) reqcuflags |= REQ_CASELESS;
          }
        }
      goto OUTPUT_SINGLE_REPEAT;
#ifdef SUPPORT_WIDE_CHARS
      case OP_XCLASS:
      case OP_ECLASS:
#endif
      case OP_CLASS:
      case OP_NCLASS:
      case OP_REF:
      case OP_REFI:
      case OP_DNREF:
      case OP_DNREFI:
      if (repeat_max == 0)
        {
        code = previous;
        goto END_REPEAT;
        }
      if (repeat_max == 1 && repeat_min == 1) goto END_REPEAT;
      if (repeat_min == 0 && repeat_max == REPEAT_UNLIMITED)
        *code++ = OP_CRSTAR + repeat_type;
      else if (repeat_min == 1 && repeat_max == REPEAT_UNLIMITED)
        *code++ = OP_CRPLUS + repeat_type;
      else if (repeat_min == 0 && repeat_max == 1)
        *code++ = OP_CRQUERY + repeat_type;
      else
        {
        *code++ = OP_CRRANGE + repeat_type;
        PUT2INC(code, 0, repeat_min);
        if (repeat_max == REPEAT_UNLIMITED) repeat_max = 0;
        PUT2INC(code, 0, repeat_max);
        }
      break;
      case OP_RECURSE:
      if (repeat_max == 1 && repeat_min == 1 && !possessive_quantifier)
        goto END_REPEAT;
      if (repeat_min > 0 && (repeat_min != 1 || repeat_max != REPEAT_UNLIMITED))
        {
        int replicate = repeat_min;
        if (repeat_min == repeat_max) replicate--;
        if (lengthptr != NULL)
          {
          PCRE2_SIZE delta;
          if (PRIV(ckd_smul)(&delta, replicate, (int)length_prevgroup) ||
              OFLOW_MAX - *lengthptr < delta)
            {
            *errorcodeptr = ERR20;
            return 0;
            }
          *lengthptr += delta;
          }
        else for (int i = 0; i < replicate; i++)
          {
          memcpy(code, previous, CU2BYTES(length_prevgroup));
          previous = code;
          code += length_prevgroup;
          }
        if (repeat_min == repeat_max) break;
        if (repeat_max != REPEAT_UNLIMITED) repeat_max -= repeat_min;
        repeat_min = 0;
        }
        {
        PCRE2_SIZE length = (lengthptr != NULL) ? 1 + LINK_SIZE : length_prevgroup;
        (void)memmove(previous + 1 + LINK_SIZE, previous, CU2BYTES(length));
        op_previous = *previous = OP_BRA;
        PUT(previous, 1, 1 + LINK_SIZE + length);
        previous[1 + LINK_SIZE + length] = OP_KET;
        PUT(previous, 2 + LINK_SIZE + length, 1 + LINK_SIZE + length);
        }
      code += 2 + 2 * LINK_SIZE;
      length_prevgroup += 2 + 2 * LINK_SIZE;
      group_return = -1;
      PCRE2_FALLTHROUGH
      case OP_ASSERT:
      case OP_ASSERT_NOT:
      case OP_ASSERT_NA:
      case OP_ASSERTBACK:
      case OP_ASSERTBACK_NOT:
      case OP_ASSERTBACK_NA:
      case OP_ASSERT_SCS:
      case OP_ONCE:
      case OP_SCRIPT_RUN:
      case OP_BRA:
      case OP_CBRA:
      case OP_COND:
        {
        int len = (int)(code - previous);
        PCRE2_UCHAR *bralink = NULL;
        PCRE2_UCHAR *brazeroptr = NULL;
        if (repeat_max == 1 && repeat_min == 1 && !possessive_quantifier)
          goto END_REPEAT;
        if (op_previous == OP_COND && previous[LINK_SIZE+1] == OP_FALSE &&
            previous[GET(previous, 1)] != OP_ALT)
          goto END_REPEAT;
        if (op_previous < OP_ONCE)
          {
          if (repeat_max == REPEAT_UNLIMITED) repeat_max = repeat_min + 1;
          }
        if (repeat_min == 0)
          {
          if (repeat_max <= 1 || repeat_max == REPEAT_UNLIMITED)
            {
            (void)memmove(previous + 1, previous, CU2BYTES(len));
            code++;
            if (repeat_max == 0)
              {
              *previous++ = OP_SKIPZERO;
              goto END_REPEAT;
              }
            brazeroptr = previous;
            *previous++ = OP_BRAZERO + repeat_type;
            }
          else
            {
            int linkoffset;
            (void)memmove(previous + 2 + LINK_SIZE, previous, CU2BYTES(len));
            code += 2 + LINK_SIZE;
            *previous++ = OP_BRAZERO + repeat_type;
            *previous++ = OP_BRA;
            linkoffset = (bralink == NULL)? 0 : (int)(previous - bralink);
            bralink = previous;
            PUTINC(previous, 0, linkoffset);
            }
          if (repeat_max != REPEAT_UNLIMITED) repeat_max--;
          }
        else
          {
          if (repeat_min > 1)
            {
            if (lengthptr != NULL)
              {
              PCRE2_SIZE delta;
              if (PRIV(ckd_smul)(&delta, repeat_min - 1,
                                 (int)length_prevgroup) ||
                  OFLOW_MAX - *lengthptr < delta)
                {
                *errorcodeptr = ERR20;
                return 0;
                }
              *lengthptr += delta;
              }
            else
              {
              if (groupsetfirstcu && reqcuflags >= REQ_NONE)
                {
                reqcu = firstcu;
                reqcuflags = firstcuflags;
                }
              for (uint32_t i = 1; i < repeat_min; i++)
                {
                memcpy(code, previous, CU2BYTES(len));
                code += len;
                }
              }
            }
          if (repeat_max != REPEAT_UNLIMITED) repeat_max -= repeat_min;
          }
        if (repeat_max != REPEAT_UNLIMITED)
          {
          if (lengthptr != NULL && repeat_max > 0)
            {
            PCRE2_SIZE delta;
            if (PRIV(ckd_smul)(&delta, repeat_max,
                               (int)length_prevgroup + 1 + 2 + 2*LINK_SIZE) ||
                OFLOW_MAX + (2 + 2*LINK_SIZE) - *lengthptr < delta)
              {
              *errorcodeptr = ERR20;
              return 0;
              }
            delta -= (2 + 2*LINK_SIZE);
            *lengthptr += delta;
            }
          else for (uint32_t i = repeat_max; i >= 1; i--)
            {
            *code++ = OP_BRAZERO + repeat_type;
            if (i != 1)
              {
              int linkoffset;
              *code++ = OP_BRA;
              linkoffset = (bralink == NULL)? 0 : (int)(code - bralink);
              bralink = code;
              PUTINC(code, 0, linkoffset);
              }
            memcpy(code, previous, CU2BYTES(len));
            code += len;
            }
          while (bralink != NULL)
            {
            int oldlinkoffset;
            int linkoffset = (int)(code - bralink + 1);
            PCRE2_UCHAR *bra = code - linkoffset;
            oldlinkoffset = GET(bra, 1);
            bralink = (oldlinkoffset == 0)? NULL : bralink - oldlinkoffset;
            *code++ = OP_KET;
            PUTINC(code, 0, linkoffset);
            PUT(bra, 1, linkoffset);
            }
          }
        else
          {
          PCRE2_UCHAR *ketcode = code - 1 - LINK_SIZE;
          PCRE2_UCHAR *bracode = ketcode - GET(ketcode, 1);
          if (*bracode == OP_ONCE && possessive_quantifier) *bracode = OP_BRA;
          if (*bracode == OP_ONCE || *bracode == OP_SCRIPT_RUN)
            *ketcode = OP_KETRMAX + repeat_type;
          else
            {
            if (lengthptr == NULL)
              {
              if (group_return < 0) *bracode += OP_SBRA - OP_BRA;
              if (*bracode == OP_COND && bracode[GET(bracode,1)] != OP_ALT)
                *bracode = OP_SCOND;
              }
            if (possessive_quantifier)
              {
              if (*bracode == OP_COND || *bracode == OP_SCOND)
                {
                int nlen = (int)(code - bracode);
                (void)memmove(bracode + 1 + LINK_SIZE, bracode, CU2BYTES(nlen));
                code += 1 + LINK_SIZE;
                nlen += 1 + LINK_SIZE;
                *bracode = (*bracode == OP_COND)? OP_BRAPOS : OP_SBRAPOS;
                *code++ = OP_KETRPOS;
                PUTINC(code, 0, nlen);
                PUT(bracode, 1, nlen);
                }
              else
                {
                *bracode += 1;
                *ketcode = OP_KETRPOS;
                }
              if (brazeroptr != NULL) *brazeroptr = OP_BRAPOSZERO;
              if (repeat_min < 2) possessive_quantifier = FALSE;
              }
            else *ketcode = OP_KETRMAX + repeat_type;
            }
          }
        }
      break;
      default:
      if (op_previous >= OP_EODN || op_previous <= OP_WORD_BOUNDARY)
        {
        PCRE2_DEBUG_UNREACHABLE();
        *errorcodeptr = ERR10;
        return 0;
        }
        {
        int prop_type, prop_value;
        PCRE2_UCHAR *oldcode;
        if (repeat_max == 1 && repeat_min == 1) goto END_REPEAT;
        op_type = OP_TYPESTAR - OP_STAR;
        mclength = 0;
        if (op_previous == OP_PROP || op_previous == OP_NOTPROP)
          {
          prop_type = previous[1];
          prop_value = previous[2];
          }
        else
          {
          OUTPUT_SINGLE_REPEAT:
          prop_type = prop_value = -1;
          }
        oldcode = code;
        code = previous;
        if (repeat_max == 0) goto END_REPEAT;
        repeat_type += op_type;
        if (repeat_min == 0)
          {
          if (repeat_max == REPEAT_UNLIMITED) *code++ = OP_STAR + repeat_type;
            else if (repeat_max == 1) *code++ = OP_QUERY + repeat_type;
          else
            {
            *code++ = OP_UPTO + repeat_type;
            PUT2INC(code, 0, repeat_max);
            }
          }
        else if (repeat_min == 1)
          {
          if (repeat_max == REPEAT_UNLIMITED)
            *code++ = OP_PLUS + repeat_type;
          else
            {
            code = oldcode;
            if (repeat_max == 1) goto END_REPEAT;
            *code++ = OP_UPTO + repeat_type;
            PUT2INC(code, 0, repeat_max - 1);
            }
          }
        else
          {
          *code++ = OP_EXACT + op_type;
          PUT2INC(code, 0, repeat_min);
          if (repeat_max != repeat_min)
            {
            if (mclength > 0)
              {
              memcpy(code, mcbuffer, CU2BYTES(mclength));
              code += mclength;
              }
            else
              {
              *code++ = op_previous;
              if (prop_type >= 0)
                {
                *code++ = prop_type;
                *code++ = prop_value;
                }
              }
            if (repeat_max == REPEAT_UNLIMITED)
              *code++ = OP_STAR + repeat_type;
            else
              {
              repeat_max -= repeat_min;
              if (repeat_max == 1)
                {
                *code++ = OP_QUERY + repeat_type;
                }
              else
                {
                *code++ = OP_UPTO + repeat_type;
                PUT2INC(code, 0, repeat_max);
                }
              }
            }
          }
        if (mclength > 0)
          {
          memcpy(code, mcbuffer, CU2BYTES(mclength));
          code += mclength;
          }
        else
          {
          *code++ = op_previous;
          if (prop_type >= 0)
            {
            *code++ = prop_type;
            *code++ = prop_value;
            }
          }
        }
      break;
      }
    if (possessive_quantifier)
      {
      int len;
      switch(*tempcode)
        {
        case OP_TYPEEXACT:
        tempcode += PRIV(OP_lengths)[*tempcode] +
          ((tempcode[1 + IMM2_SIZE] == OP_PROP
          || tempcode[1 + IMM2_SIZE] == OP_NOTPROP)? 2 : 0);
        break;
        case OP_CHAR:
        case OP_CHARI:
        case OP_NOT:
        case OP_NOTI:
        case OP_EXACT:
        case OP_EXACTI:
        case OP_NOTEXACT:
        case OP_NOTEXACTI:
        tempcode += PRIV(OP_lengths)[*tempcode];
#ifdef SUPPORT_UNICODE
        if (utf && HAS_EXTRALEN(tempcode[-1]))
          tempcode += GET_EXTRALEN(tempcode[-1]);
#endif
        break;
        case OP_CLASS:
        case OP_NCLASS:
        tempcode += 1 + 32/sizeof(PCRE2_UCHAR);
        break;
#ifdef SUPPORT_WIDE_CHARS
        case OP_XCLASS:
        case OP_ECLASS:
        tempcode += GET(tempcode, 1);
        break;
#endif
        }
      len = (int)(code - tempcode);
      if (len > 0)
        {
        unsigned int repcode = *tempcode;
        if (repcode < OP_CALLOUT && opcode_possessify[repcode] > 0)
          *tempcode = opcode_possessify[repcode];
        else
          {
          (void)memmove(tempcode + 1 + LINK_SIZE, tempcode, CU2BYTES(len));
          code += 1 + LINK_SIZE;
          len += 1 + LINK_SIZE;
          tempcode[0] = OP_ONCE;
          *code++ = OP_KET;
          PUTINC(code, 0, len);
          PUT(tempcode, 1, len);
          }
        }
      }
    END_REPEAT:
    cb->req_varyopt |= reqvary;
    break;
    case META_BIGVALUE:
    pptr++;
    goto NORMAL_CHAR;
    case META_BACKREF:
    if (meta_arg < 10) offset = cb->small_ref_offset[meta_arg];
      else GETPLUSOFFSET(offset, pptr);
    if (meta_arg > cb->bracount)
      {
      cb->erroroffset = offset;
      *errorcodeptr = ERR15;
      return 0;
      }
    HANDLE_SINGLE_REFERENCE:
    if (firstcuflags == REQ_UNSET) zerofirstcuflags = firstcuflags = REQ_NONE;
    *code++ = ((options & PCRE2_CASELESS) != 0)? OP_REFI : OP_REF;
    PUT2INC(code, 0, meta_arg);
    if ((options & PCRE2_CASELESS) != 0)
      *code++ = (((xoptions & PCRE2_EXTRA_CASELESS_RESTRICT) != 0)?
                 REFI_FLAG_CASELESS_RESTRICT : 0) |
                (((xoptions & PCRE2_EXTRA_TURKISH_CASING) != 0)?
                 REFI_FLAG_TURKISH_CASING : 0);
    cb->backref_map |= (meta_arg < 32)? (1u << meta_arg) : 1;
    if (meta_arg > cb->top_backref) cb->top_backref = meta_arg;
    break;
    case META_RECURSE:
    GETPLUSOFFSET(offset, pptr);
    if (meta_arg > cb->bracount)
      {
      cb->erroroffset = offset;
      *errorcodeptr = ERR15;
      return 0;
      }
    HANDLE_NUMERICAL_RECURSION:
    *code = OP_RECURSE;
    PUT(code, 1, meta_arg);
    code += 1 + LINK_SIZE;
    length_prevgroup = 1 + LINK_SIZE;
    if (META_CODE(pptr[1]) == META_OFFSET ||
        META_CODE(pptr[1]) == META_CAPTURE_NAME ||
        META_CODE(pptr[1]) == META_CAPTURE_NUMBER)
      {
      recurse_arguments *args;
      if (lengthptr != NULL)
        {
        if (!PRIV(compile_parse_recurse_args)(pptr, offset, errorcodeptr, cb))
          return 0;
        args = (recurse_arguments*)cb->last_data;
        length_prevgroup += (args->size * (1 + IMM2_SIZE));
        *lengthptr += (args->size * (1 + IMM2_SIZE));
        pptr += args->skip_size;
        }
      else
        {
        uint16_t *current, *end;
        args = (recurse_arguments*)cb->first_data;
        PCRE2_ASSERT(args != NULL && args->header.type == CDATA_RECURSE_ARGS);
        current = (uint16_t*)(args + 1);
        end = current + args->size;
        PCRE2_ASSERT(end > current);
        do
          {
          code[0] = OP_CREF;
          PUT2(code, 1, *current);
          code += 1 + IMM2_SIZE;
          }
        while (++current < end);
        length_prevgroup += (args->size * (1 + IMM2_SIZE));
        pptr += args->skip_size;
        cb->first_data = args->header.next;
        cb->cx->memctl.free(args, cb->cx->memctl.memory_data);
        }
      }
    groupsetfirstcu = FALSE;
    cb->had_recurse = TRUE;
    if (firstcuflags == REQ_UNSET) firstcuflags = REQ_NONE;
    zerofirstcu = firstcu;
    zerofirstcuflags = firstcuflags;
    break;
    case META_CAPTURE:
    bravalue = OP_CBRA;
    skipunits = IMM2_SIZE;
    PUT2(code, 1+LINK_SIZE, meta_arg);
    cb->lastcapture = meta_arg;
    goto GROUP_PROCESS_NOTE_EMPTY;
    case META_ESCAPE:
    if (meta_arg > ESC_b && meta_arg < ESC_Z)
      {
      matched_char = TRUE;
      if (firstcuflags == REQ_UNSET) firstcuflags = REQ_NONE;
      }
    zerofirstcu = firstcu;
    zerofirstcuflags = firstcuflags;
    zeroreqcu = reqcu;
    zeroreqcuflags = reqcuflags;
#ifdef SUPPORT_UNICODE
    if (meta_arg == ESC_P || meta_arg == ESC_p)
      {
      uint32_t ptype = *(++pptr) >> 16;
      uint32_t pdata = *pptr & 0xffff;
      if ((options & PCRE2_CASELESS) != 0 && ptype == PT_PC &&
          (pdata == ucp_Lu || pdata == ucp_Ll || pdata == ucp_Lt))
        {
        ptype = PT_LAMP;
        pdata = 0;
        }
      if (ptype == PT_ANY)
        {
        if (meta_arg == ESC_P)
          {
          *code++ = OP_CLASS;
          memset(code, 0, 32);
          code += 32 / sizeof(PCRE2_UCHAR);
          }
        else
          *code++ = OP_ALLANY;
        }
      else
        {
        *code++ = (meta_arg == ESC_p)? OP_PROP : OP_NOTPROP;
        *code++ = ptype;
        *code++ = pdata;
        }
      break;
      }
#endif
    if (cb->assert_depth > 0 && meta_arg == ESC_K &&
        (xoptions & PCRE2_EXTRA_ALLOW_LOOKAROUND_BSK) == 0)
      {
      *errorcodeptr = ERR99;
      return 0;
      }
    switch(meta_arg)
      {
      case ESC_C:
      cb->external_flags |= PCRE2_HASBKC;
#if PCRE2_CODE_UNIT_WIDTH == 32
      meta_arg = OP_ALLANY;
      (void)utf;
#else
      if (!utf) meta_arg = OP_ALLANY;
#endif
      break;
      case ESC_B:
      case ESC_b:
      if ((options & PCRE2_UCP) != 0 && (xoptions & PCRE2_EXTRA_ASCII_BSW) == 0)
        meta_arg = (meta_arg == ESC_B)? OP_NOT_UCP_WORD_BOUNDARY :
          OP_UCP_WORD_BOUNDARY;
      PCRE2_FALLTHROUGH
      case ESC_A:
      if (cb->max_lookbehind == 0) cb->max_lookbehind = 1;
      break;
      case ESC_K:
      cb->external_flags |= PCRE2_HASBSK;
      break;
      }
    *code++ = meta_arg;
    break;
    default:
    if (meta >= META_END)
      {
      PCRE2_DEBUG_UNREACHABLE();
      *errorcodeptr = ERR89;
      return 0;
      }
    NORMAL_CHAR:
    meta = *pptr;
    NORMAL_CHAR_SET:
    matched_char = TRUE;
#ifdef SUPPORT_UNICODE
    if ((utf||ucp) && (options & PCRE2_CASELESS) != 0)
      {
      uint32_t caseset;
      if ((xoptions & (PCRE2_EXTRA_TURKISH_CASING|PCRE2_EXTRA_CASELESS_RESTRICT)) ==
            PCRE2_EXTRA_TURKISH_CASING &&
          UCD_ANY_I(meta))
        {
        caseset = PRIV(ucd_turkish_dotted_i_caseset) + (UCD_DOTTED_I(meta)? 0 : 3);
        }
      else if ((caseset = UCD_CASESET(meta)) != 0 &&
               (xoptions & PCRE2_EXTRA_CASELESS_RESTRICT) != 0 &&
               PRIV(ucd_caseless_sets)[caseset] < 128)
        {
        caseset = 0;
        }
      if (caseset != 0)
        {
        *code++ = OP_PROP;
        *code++ = PT_CLIST;
        *code++ = caseset;
        if (firstcuflags == REQ_UNSET)
          firstcuflags = zerofirstcuflags = REQ_NONE;
        break;
        }
      }
#endif
    CLASS_CASELESS_CHAR:
#ifdef SUPPORT_UNICODE
    if (utf) mclength = PRIV(ord2utf)(meta, mcbuffer); else
#endif
      {
      mclength = 1;
      mcbuffer[0] = meta;
      }
    *code++ = ((options & PCRE2_CASELESS) != 0)? OP_CHARI : OP_CHAR;
    memcpy(code, mcbuffer, CU2BYTES(mclength));
    code += mclength;
    if (mcbuffer[0] == CHAR_CR || mcbuffer[0] == CHAR_NL)
      cb->external_flags |= PCRE2_HASCRORLF;
    if (firstcuflags == REQ_UNSET)
      {
      zerofirstcuflags = REQ_NONE;
      zeroreqcu = reqcu;
      zeroreqcuflags = reqcuflags;
      if (mclength == 1 || req_caseopt == 0)
        {
        firstcu = mcbuffer[0];
        firstcuflags = req_caseopt;
        if (mclength != 1)
          {
          reqcu = code[-1];
          reqcuflags = cb->req_varyopt;
          }
        }
      else firstcuflags = reqcuflags = REQ_NONE;
      }
    else
      {
      zerofirstcu = firstcu;
      zerofirstcuflags = firstcuflags;
      zeroreqcu = reqcu;
      zeroreqcuflags = reqcuflags;
      if (mclength == 1 || req_caseopt == 0)
        {
        reqcu = code[-1];
        reqcuflags = req_caseopt | cb->req_varyopt;
        }
      }
    if (reset_caseful)
      {
      options &= ~PCRE2_CASELESS;
      req_caseopt = 0;
      reset_caseful = FALSE;
      }
    break;
    }
  }
PCRE2_DEBUG_UNREACHABLE();
return 0;
}
static int
compile_regex(uint32_t options, uint32_t xoptions, PCRE2_UCHAR **codeptr,
  uint32_t **pptrptr, int *errorcodeptr, uint32_t skipunits,
  uint32_t *firstcuptr, uint32_t *firstcuflagsptr, uint32_t *reqcuptr,
  uint32_t *reqcuflagsptr, branch_chain *bcptr, open_capitem *open_caps,
  compile_block *cb, PCRE2_SIZE *lengthptr)
{
PCRE2_UCHAR *code = *codeptr;
PCRE2_UCHAR *last_branch = code;
PCRE2_UCHAR *start_bracket = code;
BOOL lookbehind;
open_capitem capitem;
int capnumber = 0;
int okreturn = 1;
uint32_t *pptr = *pptrptr;
uint32_t firstcu, reqcu;
uint32_t lookbehindlength;
uint32_t lookbehindminlength;
uint32_t firstcuflags, reqcuflags;
PCRE2_SIZE length;
branch_chain bc;
if (cb->cx->stack_guard != NULL &&
    cb->cx->stack_guard(cb->parens_depth, cb->cx->stack_guard_data))
  {
  *errorcodeptr= ERR33;
  cb->erroroffset = 0;
  return 0;
  }
bc.outer = bcptr;
bc.current_branch = code;
firstcu = reqcu = 0;
firstcuflags = reqcuflags = REQ_UNSET;
length = 2 + 2*LINK_SIZE + skipunits;
lookbehind = *code == OP_ASSERTBACK ||
             *code == OP_ASSERTBACK_NOT ||
             *code == OP_ASSERTBACK_NA;
if (lookbehind)
  {
  lookbehindlength = META_DATA(pptr[-1]);
  lookbehindminlength = *pptr;
  pptr += SIZEOFFSET;
  }
else lookbehindlength = lookbehindminlength = 0;
if (*code == OP_CBRA)
  {
  capnumber = GET2(code, 1 + LINK_SIZE);
  capitem.number = capnumber;
  capitem.next = open_caps;
  capitem.assert_depth = cb->assert_depth;
  open_caps = &capitem;
  }
PUT(code, 1, 0);
code += 1 + LINK_SIZE + skipunits;
for (;;)
  {
  int branch_return;
  uint32_t branchfirstcu = 0, branchreqcu = 0;
  uint32_t branchfirstcuflags = REQ_UNSET, branchreqcuflags = REQ_UNSET;
  if (lookbehind && lookbehindlength > 0)
    {
    if (lookbehindminlength == LOOKBEHIND_MAX ||
        lookbehindminlength == lookbehindlength)
      {
      *code++ = OP_REVERSE;
      PUT2INC(code, 0, lookbehindlength);
      length += 1 + IMM2_SIZE;
      }
    else
      {
      *code++ = OP_VREVERSE;
      PUT2INC(code, 0, lookbehindminlength);
      PUT2INC(code, 0, lookbehindlength);
      length += 1 + 2*IMM2_SIZE;
      }
    }
  if ((branch_return =
        compile_branch(&options, &xoptions, &code, &pptr, errorcodeptr,
          &branchfirstcu, &branchfirstcuflags, &branchreqcu, &branchreqcuflags,
          &bc, open_caps, cb, (lengthptr == NULL)? NULL : &length)) == 0)
    return 0;
  if (branch_return < 0) okreturn = -1;
  if (lengthptr == NULL)
    {
    if (*last_branch != OP_ALT)
      {
      firstcu = branchfirstcu;
      firstcuflags = branchfirstcuflags;
      reqcu = branchreqcu;
      reqcuflags = branchreqcuflags;
      }
    else
      {
      if (firstcuflags != branchfirstcuflags || firstcu != branchfirstcu)
        {
        if (firstcuflags < REQ_NONE)
          {
          if (reqcuflags >= REQ_NONE)
            {
            reqcu = firstcu;
            reqcuflags = firstcuflags;
            }
          }
        firstcuflags = REQ_NONE;
        }
      if (firstcuflags >= REQ_NONE && branchfirstcuflags < REQ_NONE &&
          branchreqcuflags >= REQ_NONE)
        {
        branchreqcu = branchfirstcu;
        branchreqcuflags = branchfirstcuflags;
        }
      if (((reqcuflags & ~REQ_VARY) != (branchreqcuflags & ~REQ_VARY)) ||
          reqcu != branchreqcu)
        reqcuflags = REQ_NONE;
      else
        {
        reqcu = branchreqcu;
        reqcuflags |= branchreqcuflags;
        }
      }
    }
  if (META_CODE(*pptr) != META_ALT)
    {
    if (lengthptr == NULL)
      {
      uint32_t branch_length = (uint32_t)(code - last_branch);
      do
        {
        uint32_t prev_length = GET(last_branch, 1);
        PUT(last_branch, 1, branch_length);
        branch_length = prev_length;
        last_branch -= branch_length;
        }
      while (branch_length > 0);
      }
    *code = OP_KET;
    PUT(code, 1, (uint32_t)(code - start_bracket));
    code += 1 + LINK_SIZE;
    *codeptr = code;
    *pptrptr = pptr;
    *firstcuptr = firstcu;
    *firstcuflagsptr = firstcuflags;
    *reqcuptr = reqcu;
    *reqcuflagsptr = reqcuflags;
    if (lengthptr != NULL)
      {
      if (OFLOW_MAX - *lengthptr < length)
        {
        *errorcodeptr = ERR20;
        return 0;
        }
      *lengthptr += length;
      }
    return okreturn;
    }
  if (lengthptr != NULL)
    {
    code = *codeptr + 1 + LINK_SIZE + skipunits;
    length += 1 + LINK_SIZE;
    }
  else
    {
    *code = OP_ALT;
    PUT(code, 1, (int)(code - last_branch));
    bc.current_branch = last_branch = code;
    code += 1 + LINK_SIZE;
    }
  lookbehindlength = META_DATA(*pptr);
  pptr++;
  }
PCRE2_DEBUG_UNREACHABLE();
return 0;
}
static BOOL
is_anchored(PCRE2_SPTR code, uint32_t bracket_map, compile_block *cb,
  int atomcount, BOOL inassert, BOOL dotstar_anchor)
{
do {
   PCRE2_SPTR scode = first_significant_code(
     code + PRIV(OP_lengths)[*code], FALSE);
   int op = *scode;
   if (op == OP_BRA  || op == OP_BRAPOS ||
       op == OP_SBRA || op == OP_SBRAPOS)
     {
     if (!is_anchored(scode, bracket_map, cb, atomcount, inassert, dotstar_anchor))
       return FALSE;
     }
   else if (op == OP_CBRA  || op == OP_CBRAPOS ||
            op == OP_SCBRA || op == OP_SCBRAPOS)
     {
     int n = GET2(scode, 1+LINK_SIZE);
     uint32_t new_map = bracket_map | ((n < 32)? (1u << n) : 1);
     if (!is_anchored(scode, new_map, cb, atomcount, inassert, dotstar_anchor)) return FALSE;
     }
   else if (op == OP_ASSERT || op == OP_ASSERT_NA)
     {
     if (!is_anchored(scode, bracket_map, cb, atomcount, TRUE, dotstar_anchor)) return FALSE;
     }
   else if (op == OP_COND || op == OP_SCOND)
     {
     if (scode[GET(scode,1)] != OP_ALT) return FALSE;
     if (!is_anchored(scode, bracket_map, cb, atomcount, inassert, dotstar_anchor))
       return FALSE;
     }
   else if (op == OP_ONCE)
     {
     if (!is_anchored(scode, bracket_map, cb, atomcount + 1, inassert, dotstar_anchor))
       return FALSE;
     }
   else if ((op == OP_TYPESTAR || op == OP_TYPEMINSTAR ||
             op == OP_TYPEPOSSTAR))
     {
     if (scode[1] != OP_ALLANY || (bracket_map & cb->backref_map) != 0 ||
         atomcount > 0 || cb->had_pruneorskip || inassert || !dotstar_anchor)
       return FALSE;
     }
   else if (op != OP_SOD && op != OP_SOM && op != OP_CIRC) return FALSE;
   code += GET(code, 1);
   }
while (*code == OP_ALT);
return TRUE;
}
static BOOL
is_startline(PCRE2_SPTR code, unsigned int bracket_map, compile_block *cb,
  int atomcount, BOOL inassert, BOOL dotstar_anchor)
{
do {
   PCRE2_SPTR scode = first_significant_code(
     code + PRIV(OP_lengths)[*code], FALSE);
   int op = *scode;
   if (op == OP_COND)
     {
     scode += 1 + LINK_SIZE;
     if (*scode == OP_CALLOUT) scode += PRIV(OP_lengths)[OP_CALLOUT];
       else if (*scode == OP_CALLOUT_STR) scode += GET(scode, 1 + 2*LINK_SIZE);
     switch (*scode)
       {
       case OP_CREF:
       case OP_DNCREF:
       case OP_RREF:
       case OP_DNRREF:
       case OP_FAIL:
       case OP_FALSE:
       case OP_TRUE:
       return FALSE;
       default:
       if (!is_startline(scode, bracket_map, cb, atomcount, TRUE, dotstar_anchor))
         return FALSE;
       do scode += GET(scode, 1); while (*scode == OP_ALT);
       scode += 1 + LINK_SIZE;
       break;
       }
     scode = first_significant_code(scode, FALSE);
     op = *scode;
     }
   if (op == OP_BRA  || op == OP_BRAPOS ||
       op == OP_SBRA || op == OP_SBRAPOS)
     {
     if (!is_startline(scode, bracket_map, cb, atomcount, inassert, dotstar_anchor))
       return FALSE;
     }
   else if (op == OP_CBRA  || op == OP_CBRAPOS ||
            op == OP_SCBRA || op == OP_SCBRAPOS)
     {
     int n = GET2(scode, 1+LINK_SIZE);
     unsigned int new_map = bracket_map | ((n < 32)? (1u << n) : 1);
     if (!is_startline(scode, new_map, cb, atomcount, inassert, dotstar_anchor))
       return FALSE;
     }
   else if (op == OP_ASSERT || op == OP_ASSERT_NA)
     {
     if (!is_startline(scode, bracket_map, cb, atomcount, TRUE, dotstar_anchor))
       return FALSE;
     }
   else if (op == OP_ONCE)
     {
     if (!is_startline(scode, bracket_map, cb, atomcount + 1, inassert, dotstar_anchor))
       return FALSE;
     }
   else if (op == OP_TYPESTAR || op == OP_TYPEMINSTAR || op == OP_TYPEPOSSTAR)
     {
     if (scode[1] != OP_ANY || (bracket_map & cb->backref_map) != 0 ||
         atomcount > 0 || cb->had_pruneorskip || inassert || !dotstar_anchor)
       return FALSE;
     }
   else if (op != OP_CIRC && op != OP_CIRCM) return FALSE;
   code += GET(code, 1);
   }
while (*code == OP_ALT);
return TRUE;
}
static PCRE2_UCHAR *
find_recurse(PCRE2_UCHAR *code, BOOL utf)
{
for (;;)
  {
  PCRE2_UCHAR c = *code;
  if (c == OP_END) return NULL;
  if (c == OP_RECURSE) return code;
  if (c == OP_XCLASS || c == OP_ECLASS) code += GET(code, 1);
  else if (c == OP_CALLOUT_STR) code += GET(code, 1 + 2*LINK_SIZE);
  else
    {
    switch(c)
      {
      case OP_TYPESTAR:
      case OP_TYPEMINSTAR:
      case OP_TYPEPLUS:
      case OP_TYPEMINPLUS:
      case OP_TYPEQUERY:
      case OP_TYPEMINQUERY:
      case OP_TYPEPOSSTAR:
      case OP_TYPEPOSPLUS:
      case OP_TYPEPOSQUERY:
      if (code[1] == OP_PROP || code[1] == OP_NOTPROP) code += 2;
      break;
      case OP_TYPEPOSUPTO:
      case OP_TYPEUPTO:
      case OP_TYPEMINUPTO:
      case OP_TYPEEXACT:
      if (code[1 + IMM2_SIZE] == OP_PROP || code[1 + IMM2_SIZE] == OP_NOTPROP)
        code += 2;
      break;
      case OP_MARK:
      case OP_COMMIT_ARG:
      case OP_PRUNE_ARG:
      case OP_SKIP_ARG:
      case OP_THEN_ARG:
      code += code[1];
      break;
      }
    code += PRIV(OP_lengths)[c];
#ifdef MAYBE_UTF_MULTI
    if (utf) switch(c)
      {
      case OP_CHAR:
      case OP_CHARI:
      case OP_NOT:
      case OP_NOTI:
      case OP_EXACT:
      case OP_EXACTI:
      case OP_NOTEXACT:
      case OP_NOTEXACTI:
      case OP_UPTO:
      case OP_UPTOI:
      case OP_NOTUPTO:
      case OP_NOTUPTOI:
      case OP_MINUPTO:
      case OP_MINUPTOI:
      case OP_NOTMINUPTO:
      case OP_NOTMINUPTOI:
      case OP_POSUPTO:
      case OP_POSUPTOI:
      case OP_NOTPOSUPTO:
      case OP_NOTPOSUPTOI:
      case OP_STAR:
      case OP_STARI:
      case OP_NOTSTAR:
      case OP_NOTSTARI:
      case OP_MINSTAR:
      case OP_MINSTARI:
      case OP_NOTMINSTAR:
      case OP_NOTMINSTARI:
      case OP_POSSTAR:
      case OP_POSSTARI:
      case OP_NOTPOSSTAR:
      case OP_NOTPOSSTARI:
      case OP_PLUS:
      case OP_PLUSI:
      case OP_NOTPLUS:
      case OP_NOTPLUSI:
      case OP_MINPLUS:
      case OP_MINPLUSI:
      case OP_NOTMINPLUS:
      case OP_NOTMINPLUSI:
      case OP_POSPLUS:
      case OP_POSPLUSI:
      case OP_NOTPOSPLUS:
      case OP_NOTPOSPLUSI:
      case OP_QUERY:
      case OP_QUERYI:
      case OP_NOTQUERY:
      case OP_NOTQUERYI:
      case OP_MINQUERY:
      case OP_MINQUERYI:
      case OP_NOTMINQUERY:
      case OP_NOTMINQUERYI:
      case OP_POSQUERY:
      case OP_POSQUERYI:
      case OP_NOTPOSQUERY:
      case OP_NOTPOSQUERYI:
      if (HAS_EXTRALEN(code[-1])) code += GET_EXTRALEN(code[-1]);
      break;
      }
#else
    (void)(utf);
#endif
    }
  }
}
static uint32_t
find_firstassertedcu(PCRE2_SPTR code, uint32_t *flags, uint32_t inassert)
{
uint32_t c = 0;
uint32_t cflags = REQ_NONE;
*flags = REQ_NONE;
do {
   uint32_t d;
   uint32_t dflags;
   int xl = (*code == OP_CBRA || *code == OP_SCBRA ||
             *code == OP_CBRAPOS || *code == OP_SCBRAPOS)? IMM2_SIZE:0;
   PCRE2_SPTR scode = first_significant_code(code + 1+LINK_SIZE + xl, TRUE);
   PCRE2_UCHAR op = *scode;
   switch(op)
     {
     default:
     return 0;
     case OP_BRA:
     case OP_BRAPOS:
     case OP_CBRA:
     case OP_SCBRA:
     case OP_CBRAPOS:
     case OP_SCBRAPOS:
     case OP_ASSERT:
     case OP_ASSERT_NA:
     case OP_ONCE:
     case OP_SCRIPT_RUN:
     d = find_firstassertedcu(scode, &dflags, inassert +
       ((op == OP_ASSERT || op == OP_ASSERT_NA)?1:0));
     if (dflags >= REQ_NONE) return 0;
     if (cflags >= REQ_NONE) { c = d; cflags = dflags; }
       else if (c != d || cflags != dflags) return 0;
     break;
     case OP_EXACT:
     scode += IMM2_SIZE;
     PCRE2_FALLTHROUGH
     case OP_CHAR:
     case OP_PLUS:
     case OP_MINPLUS:
     case OP_POSPLUS:
     if (inassert == 0) return 0;
     if (cflags >= REQ_NONE) { c = scode[1]; cflags = 0; }
       else if (c != scode[1]) return 0;
     break;
     case OP_EXACTI:
     scode += IMM2_SIZE;
     PCRE2_FALLTHROUGH
     case OP_CHARI:
     case OP_PLUSI:
     case OP_MINPLUSI:
     case OP_POSPLUSI:
     if (inassert == 0) return 0;
#ifdef SUPPORT_UNICODE
#if PCRE2_CODE_UNIT_WIDTH == 8
     if (scode[1] >= 0x80) return 0;
#elif PCRE2_CODE_UNIT_WIDTH == 16
     if (scode[1] >= 0xd800 && scode[1] <= 0xdfff) return 0;
#endif
#endif
     if (cflags >= REQ_NONE) { c = scode[1]; cflags = REQ_CASELESS; }
       else if (c != scode[1]) return 0;
     break;
     }
   code += GET(code, 1);
   }
while (*code == OP_ALT);
*flags = cflags;
return c;
}
static uint32_t *
parsed_skip(uint32_t *pptr, uint32_t skiptype)
{
uint32_t nestlevel = 0;
for (;; pptr++)
  {
  uint32_t meta = META_CODE(*pptr);
  switch(meta)
    {
    default:
    if (meta < META_END) continue;
    break;
    case META_END:
    PCRE2_DEBUG_UNREACHABLE();
    return NULL;
    case META_BACKREF:
    if (META_DATA(*pptr) >= 10) pptr += SIZEOFFSET;
    break;
    case META_ESCAPE:
    if (*pptr - META_ESCAPE == ESC_P || *pptr - META_ESCAPE == ESC_p)
      pptr += 1;
    break;
    case META_MARK:
    case META_COMMIT_ARG:
    case META_PRUNE_ARG:
    case META_SKIP_ARG:
    case META_THEN_ARG:
    pptr += pptr[1];
    break;
    case META_CLASS_END:
    if (skiptype == PSKIP_CLASS) return pptr;
    break;
    case META_ATOMIC:
    case META_CAPTURE:
    case META_COND_ASSERT:
    case META_COND_DEFINE:
    case META_COND_NAME:
    case META_COND_NUMBER:
    case META_COND_RNAME:
    case META_COND_RNUMBER:
    case META_COND_VERSION:
    case META_SCS:
    case META_LOOKAHEAD:
    case META_LOOKAHEADNOT:
    case META_LOOKAHEAD_NA:
    case META_LOOKBEHIND:
    case META_LOOKBEHINDNOT:
    case META_LOOKBEHIND_NA:
    case META_NOCAPTURE:
    case META_SCRIPT_RUN:
    nestlevel++;
    break;
    case META_ALT:
    if (nestlevel == 0 && skiptype == PSKIP_ALT) return pptr;
    break;
    case META_KET:
    if (nestlevel == 0) return pptr;
    nestlevel--;
    break;
    }
  meta = (meta >> 16) & 0x7fff;
  if (meta >= sizeof(meta_extra_lengths)) return NULL;
  pptr += meta_extra_lengths[meta];
  }
PCRE2_UNREACHABLE();
}
static int
get_grouplength(uint32_t **pptrptr, int *minptr, BOOL isinline, int *errcodeptr,
  int *lcptr, int group, parsed_recurse_check *recurses, compile_block *cb)
{
uint32_t *gi = cb->groupinfo + 2 * group;
int branchlength, branchminlength;
int grouplength = -1;
int groupminlength = INT_MAX;
if (group > 0 && (cb->external_flags & PCRE2_DUPCAPUSED) == 0)
  {
  uint32_t groupinfo = gi[0];
  if ((groupinfo & GI_NOT_FIXED_LENGTH) != 0) return -1;
  if ((groupinfo & GI_SET_FIXED_LENGTH) != 0)
    {
    if (isinline) *pptrptr = parsed_skip(*pptrptr, PSKIP_KET);
    *minptr = gi[1];
    return groupinfo & GI_FIXED_LENGTH_MASK;
    }
  }
for(;;)
  {
  branchlength = get_branchlength(pptrptr, &branchminlength, errcodeptr, lcptr,
    recurses, cb);
  if (branchlength < 0) goto ISNOTFIXED;
  if (branchlength > grouplength) grouplength = branchlength;
  if (branchminlength < groupminlength) groupminlength = branchminlength;
  if (**pptrptr == META_KET) break;
  *pptrptr += 1;
  }
if (group > 0)
  {
  gi[0] |= (uint32_t)(GI_SET_FIXED_LENGTH | grouplength);
  gi[1] = groupminlength;
  }
*minptr = groupminlength;
return grouplength;
ISNOTFIXED:
if (group > 0) gi[0] |= GI_NOT_FIXED_LENGTH;
return -1;
}
static int
get_branchlength(uint32_t **pptrptr, int *minptr, int *errcodeptr, int *lcptr,
  parsed_recurse_check *recurses, compile_block *cb)
{
int branchlength = 0;
int branchminlength = 0;
int grouplength, groupminlength;
uint32_t lastitemlength = 0;
uint32_t lastitemminlength = 0;
uint32_t *pptr = *pptrptr;
PCRE2_SIZE offset;
parsed_recurse_check this_recurse;
if ((*lcptr)++ > 2000)
  {
  *errcodeptr = ERR35;
  return -1;
  }
for (;; pptr++)
  {
  parsed_recurse_check *r;
  uint32_t *gptr, *gptrend;
  uint32_t escape;
  uint32_t min, max;
  uint32_t group = 0;
  uint32_t itemlength = 0;
  uint32_t itemminlength = 0;
  if (*pptr < META_END)
    {
    itemlength = itemminlength = 1;
    }
  else switch (META_CODE(*pptr))
    {
    case META_KET:
    case META_ALT:
    goto EXIT;
    case META_ACCEPT:
    case META_FAIL:
    pptr = parsed_skip(pptr, PSKIP_ALT);
    if (pptr == NULL) goto PARSED_SKIP_FAILED;
    goto EXIT;
    case META_MARK:
    case META_COMMIT_ARG:
    case META_PRUNE_ARG:
    case META_SKIP_ARG:
    case META_THEN_ARG:
    pptr += pptr[1] + 1;
    break;
    case META_CIRCUMFLEX:
    case META_COMMIT:
    case META_DOLLAR:
    case META_PRUNE:
    case META_SKIP:
    case META_THEN:
    break;
    case META_OPTIONS:
    pptr += 2;
    break;
    case META_BIGVALUE:
    itemlength = itemminlength = 1;
    pptr += 1;
    break;
    case META_CLASS:
    case META_CLASS_NOT:
    itemlength = itemminlength = 1;
    pptr = parsed_skip(pptr, PSKIP_CLASS);
    if (pptr == NULL) goto PARSED_SKIP_FAILED;
    break;
    case META_CLASS_EMPTY_NOT:
    case META_DOT:
    itemlength = itemminlength = 1;
    break;
    case META_CALLOUT_NUMBER:
    pptr += 3;
    break;
    case META_CALLOUT_STRING:
    pptr += 3 + SIZEOFFSET;
    break;
    case META_ESCAPE:
    escape = META_DATA(*pptr);
    if (escape == ESC_X) return -1;
    if (escape == ESC_R)
      {
      itemminlength = 1;
      itemlength = 2;
      }
    else if (escape > ESC_b && escape < ESC_Z)
      {
#if PCRE2_CODE_UNIT_WIDTH != 32
      if ((cb->external_options & PCRE2_UTF) != 0 && escape == ESC_C)
        {
        *errcodeptr = ERR36;
        return -1;
        }
#endif
      itemlength = itemminlength = 1;
      if (escape == ESC_p || escape == ESC_P) pptr++;
      }
    break;
    case META_LOOKAHEAD:
    case META_LOOKAHEADNOT:
    case META_LOOKAHEAD_NA:
    case META_SCS:
    *errcodeptr = check_lookbehinds(pptr + 1, &pptr, recurses, cb, lcptr);
    if (*errcodeptr != 0) return -1;
    switch (pptr[1])
      {
      case META_ASTERISK:
      case META_ASTERISK_PLUS:
      case META_ASTERISK_QUERY:
      case META_PLUS:
      case META_PLUS_PLUS:
      case META_PLUS_QUERY:
      case META_QUERY:
      case META_QUERY_PLUS:
      case META_QUERY_QUERY:
      pptr++;
      break;
      case META_MINMAX:
      case META_MINMAX_PLUS:
      case META_MINMAX_QUERY:
      pptr += 3;
      break;
      default:
      break;
      }
    break;
    case META_LOOKBEHIND:
    case META_LOOKBEHINDNOT:
    case META_LOOKBEHIND_NA:
    if (!set_lookbehind_lengths(&pptr, errcodeptr, lcptr, recurses, cb))
      return -1;
    break;
    case META_BACKREF_BYNAME:
    if ((cb->external_options & PCRE2_MATCH_UNSET_BACKREF) != 0)
      goto ISNOTFIXED;
    PCRE2_FALLTHROUGH
    case META_RECURSE_BYNAME:
      {
      PCRE2_SPTR name;
      BOOL is_dupname = FALSE;
      named_group *ng;
      uint32_t meta_code = META_CODE(*pptr);
      uint32_t length = *(++pptr);
      GETPLUSOFFSET(offset, pptr);
      name = cb->start_pattern + offset;
      ng = PRIV(compile_find_named_group)(name, length, cb);
      if (ng == NULL)
        {
        *errcodeptr = ERR15;
        cb->erroroffset = offset;
        return -1;
        }
      group = ng->number;
      is_dupname = (ng->hash_dup & NAMED_GROUP_IS_DUPNAME) != 0;
      if (meta_code == META_RECURSE_BYNAME ||
          (!is_dupname && (cb->external_flags & PCRE2_DUPCAPUSED) == 0))
        goto RECURSE_OR_BACKREF_LENGTH;
      }
    goto ISNOTFIXED;
    case META_BACKREF:
    if ((cb->external_options & PCRE2_MATCH_UNSET_BACKREF) != 0 ||
        (cb->external_flags & PCRE2_DUPCAPUSED) != 0)
      goto ISNOTFIXED;
    group = META_DATA(*pptr);
    if (group < 10)
      {
      offset = cb->small_ref_offset[group];
      goto RECURSE_OR_BACKREF_LENGTH;
      }
    PCRE2_FALLTHROUGH
    case META_RECURSE:
    group = META_DATA(*pptr);
    GETPLUSOFFSET(offset, pptr);
    RECURSE_OR_BACKREF_LENGTH:
    if (group > cb->bracount)
      {
      cb->erroroffset = offset;
      *errcodeptr = ERR15;
      return -1;
      }
    if (group == 0) goto ISNOTFIXED;
    for (gptr = cb->parsed_pattern; *gptr != META_END; gptr++)
      {
      if (META_CODE(*gptr) == META_BIGVALUE) gptr++;
        else if (*gptr == (META_CAPTURE | group)) break;
      }
    gptrend = parsed_skip(gptr + 1, PSKIP_KET);
    if (gptrend == NULL) goto PARSED_SKIP_FAILED;
    if (pptr > gptr && pptr < gptrend) goto ISNOTFIXED;
    for (r = recurses; r != NULL; r = r->prev) if (r->groupptr == gptr) break;
    if (r != NULL) goto ISNOTFIXED;
    this_recurse.prev = recurses;
    this_recurse.groupptr = gptr;
    gptr++;
    grouplength = get_grouplength(&gptr, &groupminlength, FALSE, errcodeptr,
      lcptr, group, &this_recurse, cb);
    if (grouplength < 0)
      {
      if (*errcodeptr == 0) goto ISNOTFIXED;
      return -1;
      }
    itemlength = grouplength;
    itemminlength = groupminlength;
    break;
    case META_COND_DEFINE:
    pptr = parsed_skip(pptr + 1, PSKIP_KET);
    break;
    case META_COND_NAME:
    case META_COND_NUMBER:
    case META_COND_RNAME:
    case META_COND_RNUMBER:
    pptr += 2 + SIZEOFFSET;
    goto CHECK_GROUP;
    case META_COND_ASSERT:
    pptr += 1;
    goto CHECK_GROUP;
    case META_COND_VERSION:
    pptr += 4;
    goto CHECK_GROUP;
    case META_CAPTURE:
    group = META_DATA(*pptr);
    PCRE2_FALLTHROUGH
    case META_ATOMIC:
    case META_NOCAPTURE:
    case META_SCRIPT_RUN:
    pptr++;
    CHECK_GROUP:
    grouplength = get_grouplength(&pptr, &groupminlength, TRUE, errcodeptr,
      lcptr, group, recurses, cb);
    if (grouplength < 0) return -1;
    itemlength = grouplength;
    itemminlength = groupminlength;
    break;
    case META_QUERY:
    case META_QUERY_PLUS:
    case META_QUERY_QUERY:
    min = 0;
    max = 1;
    goto REPETITION;
    case META_MINMAX:
    case META_MINMAX_PLUS:
    case META_MINMAX_QUERY:
    min = pptr[1];
    max = pptr[2];
    pptr += 2;
    REPETITION:
    if (max != REPEAT_UNLIMITED)
      {
      if (lastitemlength != 0 &&
          max != 0 &&
          (INT_MAX - branchlength)/lastitemlength < max - 1)
        {
        *errcodeptr = ERR87;
        return -1;
        }
      if (min == 0) branchminlength -= lastitemminlength;
        else itemminlength = (min - 1) * lastitemminlength;
      if (max == 0) branchlength -= lastitemlength;
        else itemlength = (max - 1) * lastitemlength;
      break;
      }
    PCRE2_FALLTHROUGH
    default:
    ISNOTFIXED:
    *errcodeptr = ERR25;
    return -1;
    }
  if (INT_MAX - branchlength < (int)itemlength ||
      (branchlength += itemlength) > LOOKBEHIND_MAX)
    {
    *errcodeptr = ERR87;
    return -1;
    }
  branchminlength += itemminlength;
  lastitemlength = itemlength;
  lastitemminlength = itemminlength;
  }
EXIT:
*pptrptr = pptr;
*minptr = branchminlength;
return branchlength;
PARSED_SKIP_FAILED:
PCRE2_DEBUG_UNREACHABLE();
*errcodeptr = ERR90;
return -1;
}
static BOOL
set_lookbehind_lengths(uint32_t **pptrptr, int *errcodeptr, int *lcptr,
  parsed_recurse_check *recurses, compile_block *cb)
{
PCRE2_SIZE offset;
uint32_t *bptr = *pptrptr;
uint32_t *gbptr = bptr;
int maxlength = 0;
int minlength = INT_MAX;
BOOL variable = FALSE;
READPLUSOFFSET(offset, bptr);
*pptrptr += SIZEOFFSET;
do
  {
  int branchlength, branchminlength;
  *pptrptr += 1;
  branchlength = get_branchlength(pptrptr, &branchminlength, errcodeptr, lcptr,
    recurses, cb);
  if (branchlength < 0)
    {
    if (*errcodeptr == 0) *errcodeptr = ERR25;
    if (cb->erroroffset == PCRE2_UNSET) cb->erroroffset = offset;
    return FALSE;
    }
  if (branchlength != branchminlength) variable = TRUE;
  if (branchminlength < minlength) minlength = branchminlength;
  if (branchlength > maxlength) maxlength = branchlength;
  if (branchlength > cb->max_lookbehind) cb->max_lookbehind = branchlength;
  *bptr |= branchlength;
  bptr = *pptrptr;
  }
while (META_CODE(*bptr) == META_ALT);
if (variable)
  {
  gbptr[1] = minlength;
  if ((PCRE2_SIZE)maxlength > cb->max_varlookbehind)
    {
    *errcodeptr = ERR100;
    cb->erroroffset = offset;
    return FALSE;
    }
  }
else gbptr[1] = LOOKBEHIND_MAX;
return TRUE;
}
static int
check_lookbehinds(uint32_t *pptr, uint32_t **retptr,
  parsed_recurse_check *recurses, compile_block *cb, int *lcptr)
{
int errorcode = 0;
int nestlevel = 0;
cb->erroroffset = PCRE2_UNSET;
for (; *pptr != META_END; pptr++)
  {
  if (*pptr < META_END) continue;
  switch (META_CODE(*pptr))
    {
    default:
    PCRE2_DEBUG_UNREACHABLE();
    cb->erroroffset = 0;
    return ERR70;
    case META_ESCAPE:
    if (*pptr - META_ESCAPE == ESC_P || *pptr - META_ESCAPE == ESC_p)
      pptr += 1;
    break;
    case META_KET:
    if (--nestlevel < 0)
      {
      if (retptr != NULL) *retptr = pptr;
      return 0;
      }
    break;
    case META_ATOMIC:
    case META_CAPTURE:
    case META_COND_ASSERT:
    case META_SCS:
    case META_LOOKAHEAD:
    case META_LOOKAHEADNOT:
    case META_LOOKAHEAD_NA:
    case META_NOCAPTURE:
    case META_SCRIPT_RUN:
    nestlevel++;
    break;
    case META_ACCEPT:
    case META_ALT:
    case META_ASTERISK:
    case META_ASTERISK_PLUS:
    case META_ASTERISK_QUERY:
    case META_BACKREF:
    case META_CIRCUMFLEX:
    case META_CLASS:
    case META_CLASS_EMPTY:
    case META_CLASS_EMPTY_NOT:
    case META_CLASS_END:
    case META_CLASS_NOT:
    case META_COMMIT:
    case META_DOLLAR:
    case META_DOT:
    case META_FAIL:
    case META_PLUS:
    case META_PLUS_PLUS:
    case META_PLUS_QUERY:
    case META_PRUNE:
    case META_QUERY:
    case META_QUERY_PLUS:
    case META_QUERY_QUERY:
    case META_RANGE_ESCAPED:
    case META_RANGE_LITERAL:
    case META_SKIP:
    case META_THEN:
    break;
    case META_OFFSET:
    case META_RECURSE:
    pptr += SIZEOFFSET;
    break;
    case META_BACKREF_BYNAME:
    case META_RECURSE_BYNAME:
    pptr += 1 + SIZEOFFSET;
    break;
    case META_COND_DEFINE:
    pptr += SIZEOFFSET;
    nestlevel++;
    break;
    case META_COND_NAME:
    case META_COND_NUMBER:
    case META_COND_RNAME:
    case META_COND_RNUMBER:
    pptr += 1 + SIZEOFFSET;
    nestlevel++;
    break;
    case META_COND_VERSION:
    pptr += 3;
    nestlevel++;
    break;
    case META_CALLOUT_STRING:
    pptr += 3 + SIZEOFFSET;
    break;
    case META_BIGVALUE:
    case META_POSIX:
    case META_POSIX_NEG:
    case META_CAPTURE_NAME:
    case META_CAPTURE_NUMBER:
    pptr += 1;
    break;
    case META_MINMAX:
    case META_MINMAX_QUERY:
    case META_MINMAX_PLUS:
    case META_OPTIONS:
    pptr += 2;
    break;
    case META_CALLOUT_NUMBER:
    pptr += 3;
    break;
    case META_MARK:
    case META_COMMIT_ARG:
    case META_PRUNE_ARG:
    case META_SKIP_ARG:
    case META_THEN_ARG:
    pptr += 1 + pptr[1];
    break;
    case META_LOOKBEHIND:
    case META_LOOKBEHINDNOT:
    case META_LOOKBEHIND_NA:
    if (!set_lookbehind_lengths(&pptr, &errorcode, lcptr, recurses, cb))
      return errorcode;
    break;
    }
  }
return 0;
}
PCRE2_EXP_DEFN pcre2_code * PCRE2_CALL_CONVENTION
pcre2_compile(PCRE2_SPTR pattern, PCRE2_SIZE patlen, uint32_t options,
   int *errorptr, PCRE2_SIZE *erroroffset, pcre2_compile_context *ccontext)
{
BOOL utf;
BOOL ucp;
BOOL has_lookbehind = FALSE;
BOOL zero_terminated;
pcre2_real_code *re = NULL;
compile_block cb;
const uint8_t *tables;
PCRE2_UCHAR null_str[1] = { 0xcd };
PCRE2_UCHAR *code;
PCRE2_UCHAR *codestart;
PCRE2_SPTR ptr;
uint32_t *pptr;
PCRE2_SIZE length = 1;
PCRE2_SIZE usedlength;
PCRE2_SIZE re_blocksize;
PCRE2_SIZE parsed_size_needed;
uint32_t firstcuflags, reqcuflags;
uint32_t firstcu, reqcu;
uint32_t setflags = 0;
uint32_t xoptions;
uint32_t skipatstart;
uint32_t limit_heap  = UINT32_MAX;
uint32_t limit_match = UINT32_MAX;
uint32_t limit_depth = UINT32_MAX;
int newline = 0;
int bsr = 0;
int errorcode = 0;
int regexrc;
uint32_t i;
uint32_t optim_flags = ccontext != NULL ? ccontext->optimization_flags :
                                          PCRE2_OPTIMIZATION_ALL;
uint32_t stack_groupinfo[GROUPINFO_DEFAULT_SIZE];
uint32_t stack_parsed_pattern[PARSED_PATTERN_DEFAULT_SIZE];
named_group named_groups[NAMED_GROUP_LIST_SIZE];
uint32_t c16workspace[C16_WORK_SIZE];
PCRE2_UCHAR *cworkspace = (PCRE2_UCHAR *)c16workspace;
if (errorptr == NULL)
  {
  if (erroroffset != NULL) *erroroffset = 0;
  return NULL;
  }
if (erroroffset == NULL)
  {
  if (errorptr != NULL) *errorptr = ERR120;
  return NULL;
  }
*errorptr = ERR0;
*erroroffset = 0;
if (pattern == NULL)
  {
  if (patlen == 0)
    pattern = null_str;
  else
    {
    *errorptr = ERR16;
    return NULL;
    }
  }
if (ccontext == NULL)
  ccontext = (pcre2_compile_context *)(&PRIV(default_compile_context));
if ((options & PCRE2_MATCH_INVALID_UTF) != 0) options |= PCRE2_UTF;
if ((options & ~PUBLIC_COMPILE_OPTIONS) != 0 ||
    (ccontext->extra_options & ~PUBLIC_COMPILE_EXTRA_OPTIONS) != 0)
  {
  *errorptr = ERR17;
  return NULL;
  }
if ((options & PCRE2_LITERAL) != 0 &&
    ((options & ~PUBLIC_LITERAL_COMPILE_OPTIONS) != 0 ||
     (ccontext->extra_options & ~PUBLIC_LITERAL_COMPILE_EXTRA_OPTIONS) != 0))
  {
  *errorptr = ERR92;
  return NULL;
  }
if ((zero_terminated = (patlen == PCRE2_ZERO_TERMINATED)))
  patlen = PRIV(strlen)(pattern);
(void)zero_terminated;
if (patlen > ccontext->max_pattern_length)
  {
  *errorptr = ERR88;
  return NULL;
  }
if ((options & PCRE2_NO_AUTO_POSSESS) != 0)
  optim_flags &= ~PCRE2_OPTIM_AUTO_POSSESS;
if ((options & PCRE2_NO_DOTSTAR_ANCHOR) != 0)
  optim_flags &= ~PCRE2_OPTIM_DOTSTAR_ANCHOR;
if ((options & PCRE2_NO_START_OPTIMIZE) != 0)
  optim_flags &= ~PCRE2_OPTIM_START_OPTIMIZE;
tables = (ccontext->tables != NULL)? ccontext->tables : PRIV(default_tables);
cb.lcc = tables + lcc_offset;
cb.fcc = tables + fcc_offset;
cb.cbits = tables + cbits_offset;
cb.ctypes = tables + ctypes_offset;
cb.assert_depth = 0;
cb.bracount = 0;
cb.cx = ccontext;
cb.dupnames = FALSE;
cb.end_pattern = pattern + patlen;
cb.erroroffset = 0;
cb.external_flags = 0;
cb.external_options = options;
cb.groupinfo = stack_groupinfo;
cb.had_recurse = FALSE;
cb.lastcapture = 0;
cb.max_lookbehind = 0;
cb.max_varlookbehind = ccontext->max_varlookbehind;
cb.name_entry_size = 0;
cb.name_table = NULL;
cb.named_groups = named_groups;
cb.named_group_list_size = NAMED_GROUP_LIST_SIZE;
cb.names_found = 0;
cb.parens_depth = 0;
cb.parsed_pattern = stack_parsed_pattern;
cb.req_varyopt = 0;
cb.start_code = cworkspace;
cb.start_pattern = pattern;
cb.start_workspace = cworkspace;
cb.workspace_size = COMPILE_WORK_SIZE;
cb.first_data = NULL;
cb.last_data = NULL;
#ifdef SUPPORT_WIDE_CHARS
cb.char_lists_size = 0;
#endif
cb.top_backref = 0;
cb.backref_map = 0;
for (i = 0; i < 10; i++) cb.small_ref_offset[i] = PCRE2_UNSET;
#ifdef SUPPORT_VALGRIND
if (zero_terminated) VALGRIND_MAKE_MEM_NOACCESS(pattern + patlen, CU2BYTES(1));
#endif
xoptions = ccontext->extra_options;
ptr = pattern;
skipatstart = 0;
if ((options & PCRE2_LITERAL) == 0)
  {
  while (patlen - skipatstart >= 2 &&
         ptr[skipatstart] == CHAR_LEFT_PARENTHESIS &&
         ptr[skipatstart+1] == CHAR_ASTERISK)
    {
    for (i = 0; i < sizeof(pso_list)/sizeof(pso); i++)
      {
      const pso *p = pso_list + i;
      if (patlen - skipatstart - 2 >= p->length &&
          PRIV(strncmp_c8)(ptr + skipatstart + 2, p->name, p->length) == 0)
        {
        uint32_t c, pp;
        skipatstart += p->length + 2;
        switch(p->type)
          {
          case PSO_OPT:
          cb.external_options |= p->value;
          break;
          case PSO_XOPT:
          xoptions |= p->value;
          break;
          case PSO_FLG:
          setflags |= p->value;
          break;
          case PSO_NL:
          newline = p->value;
          setflags |= PCRE2_NL_SET;
          break;
          case PSO_BSR:
          bsr = p->value;
          setflags |= PCRE2_BSR_SET;
          break;
          case PSO_LIMM:
          case PSO_LIMD:
          case PSO_LIMH:
          c = 0;
          pp = skipatstart;
          while (pp < patlen && IS_DIGIT(ptr[pp]))
            {
            if (c > UINT32_MAX / 10 - 1) break;
            c = c*10 + (ptr[pp++] - CHAR_0);
            }
          if (pp >= patlen || pp == skipatstart || ptr[pp] != CHAR_RIGHT_PARENTHESIS)
            {
            errorcode = ERR60;
            ptr += pp;
            utf = FALSE;
            goto HAD_EARLY_ERROR;
            }
          if (p->type == PSO_LIMH) limit_heap = c;
            else if (p->type == PSO_LIMM) limit_match = c;
            else limit_depth = c;
          skipatstart = ++pp;
          break;
          case PSO_OPTMZ:
          optim_flags &= ~(p->value);
          switch(p->value)
            {
            case PCRE2_OPTIM_AUTO_POSSESS:
            cb.external_options |= PCRE2_NO_AUTO_POSSESS;
            break;
            case PCRE2_OPTIM_DOTSTAR_ANCHOR:
            cb.external_options |= PCRE2_NO_DOTSTAR_ANCHOR;
            break;
            case PCRE2_OPTIM_START_OPTIMIZE:
            cb.external_options |= PCRE2_NO_START_OPTIMIZE;
            break;
            }
          break;
          default:
          PCRE2_DEBUG_UNREACHABLE();
          }
        break;
        }
      }
    if (i >= sizeof(pso_list)/sizeof(pso)) break;
    }
    PCRE2_ASSERT(skipatstart <= patlen);
  }
ptr += skipatstart;
#ifndef SUPPORT_UNICODE
if ((cb.external_options & (PCRE2_UTF|PCRE2_UCP)) != 0)
  {
  errorcode = ERR32;
  goto HAD_EARLY_ERROR;
  }
#endif
utf = (cb.external_options & PCRE2_UTF) != 0;
if (utf)
  {
  if ((options & PCRE2_NEVER_UTF) != 0)
    {
    errorcode = ERR74;
    goto HAD_EARLY_ERROR;
    }
  if ((options & PCRE2_NO_UTF_CHECK) == 0 &&
       (errorcode = PRIV(valid_utf)(pattern, patlen, erroroffset)) != 0)
    goto HAD_ERROR;
#if PCRE2_CODE_UNIT_WIDTH == 16
  if ((ccontext->extra_options & PCRE2_EXTRA_ALLOW_SURROGATE_ESCAPES) != 0)
    {
    errorcode = ERR91;
    goto HAD_EARLY_ERROR;
    }
#endif
  }
ucp = (cb.external_options & PCRE2_UCP) != 0;
if (ucp && (cb.external_options & PCRE2_NEVER_UCP) != 0)
  {
  errorcode = ERR75;
  goto HAD_EARLY_ERROR;
  }
if ((xoptions & PCRE2_EXTRA_TURKISH_CASING) != 0)
  {
  if (!utf && !ucp)
    {
    errorcode = ERR104;
    goto HAD_EARLY_ERROR;
    }
#if PCRE2_CODE_UNIT_WIDTH == 8
  if (!utf)
    {
    errorcode = ERR105;
    goto HAD_EARLY_ERROR;
    }
#endif
  if ((xoptions & PCRE2_EXTRA_CASELESS_RESTRICT) != 0)
    {
    errorcode = ERR106;
    goto HAD_EARLY_ERROR;
    }
  }
if (bsr == 0) bsr = ccontext->bsr_convention;
if (newline == 0) newline = ccontext->newline_convention;
cb.nltype = NLTYPE_FIXED;
switch(newline)
  {
  case PCRE2_NEWLINE_CR:
  cb.nllen = 1;
  cb.nl[0] = CHAR_CR;
  break;
  case PCRE2_NEWLINE_LF:
  cb.nllen = 1;
  cb.nl[0] = CHAR_NL;
  break;
  case PCRE2_NEWLINE_NUL:
  cb.nllen = 1;
  cb.nl[0] = CHAR_NUL;
  break;
  case PCRE2_NEWLINE_CRLF:
  cb.nllen = 2;
  cb.nl[0] = CHAR_CR;
  cb.nl[1] = CHAR_NL;
  break;
  case PCRE2_NEWLINE_ANY:
  cb.nltype = NLTYPE_ANY;
  break;
  case PCRE2_NEWLINE_ANYCRLF:
  cb.nltype = NLTYPE_ANYCRLF;
  break;
  default:
  PCRE2_DEBUG_UNREACHABLE();
  errorcode = ERR56;
  goto HAD_EARLY_ERROR;
  }
parsed_size_needed = max_parsed_pattern(ptr, cb.end_pattern, utf, options);
if ((ccontext->extra_options &
     (PCRE2_EXTRA_MATCH_WORD|PCRE2_EXTRA_MATCH_LINE)) != 0)
  parsed_size_needed += 4;
if ((options & PCRE2_AUTO_CALLOUT) != 0)
  parsed_size_needed += 4;
parsed_size_needed += 1;
if (parsed_size_needed > PARSED_PATTERN_DEFAULT_SIZE)
  {
  uint32_t *heap_parsed_pattern = ccontext->memctl.malloc(
    parsed_size_needed * sizeof(uint32_t), ccontext->memctl.memory_data);
  if (heap_parsed_pattern == NULL)
    {
    *errorptr = ERR21;
    goto EXIT;
    }
  cb.parsed_pattern = heap_parsed_pattern;
  }
cb.parsed_pattern_end = cb.parsed_pattern + parsed_size_needed;
errorcode = parse_regex(ptr, cb.external_options, xoptions, &has_lookbehind, &cb);
if (errorcode != 0) goto HAD_CB_ERROR;
if (has_lookbehind)
  {
  int loopcount = 0;
  if (cb.bracount >= GROUPINFO_DEFAULT_SIZE/2)
    {
    cb.groupinfo = ccontext->memctl.malloc(
      (2 * (cb.bracount + 1))*sizeof(uint32_t), ccontext->memctl.memory_data);
    if (cb.groupinfo == NULL)
      {
      errorcode = ERR21;
      cb.erroroffset = 0;
      goto HAD_CB_ERROR;
      }
    }
  memset(cb.groupinfo, 0, (2 * cb.bracount + 1) * sizeof(uint32_t));
  errorcode = check_lookbehinds(cb.parsed_pattern, NULL, NULL, &cb, &loopcount);
  if (errorcode != 0) goto HAD_CB_ERROR;
  }
#ifdef DEBUG_SHOW_PARSED
fprintf(stderr, "+++ Pre-scan complete:\n");
show_parsed(&cb);
#endif
#ifdef DEBUG_SHOW_CAPTURES
  {
  named_group *ng = cb.named_groups;
  fprintf(stderr, "+++Captures: %d\n", cb.bracount);
  for (i = 0; i < cb.names_found; i++, ng++)
    {
    fprintf(stderr, "+++%3d %.*s\n", ng->number, ng->length, ng->name);
    }
  }
#endif
cb.erroroffset = patlen;
pptr = cb.parsed_pattern;
code = cworkspace;
*code = OP_BRA;
(void)compile_regex(cb.external_options, xoptions, &code, &pptr,
   &errorcode, 0, &firstcu, &firstcuflags, &reqcu, &reqcuflags, NULL, NULL,
   &cb, &length);
if (errorcode != 0) goto HAD_CB_ERROR;
#if defined SUPPORT_WIDE_CHARS
PCRE2_ASSERT((cb.char_lists_size & 0x3) == 0);
if (length > MAX_PATTERN_SIZE ||
    MAX_PATTERN_SIZE - length < (cb.char_lists_size / sizeof(PCRE2_UCHAR)))
#else
if (length > MAX_PATTERN_SIZE)
#endif
  {
  errorcode = ERR20;
  cb.erroroffset = 0;
  goto HAD_CB_ERROR;
  }
re_blocksize =
  CU2BYTES((PCRE2_SIZE)cb.names_found * (PCRE2_SIZE)cb.name_entry_size);
#if defined SUPPORT_WIDE_CHARS
if (cb.char_lists_size != 0)
  {
#if PCRE2_CODE_UNIT_WIDTH != 32
  re_blocksize = (PCRE2_SIZE)CLIST_ALIGN_TO(re_blocksize, sizeof(uint32_t));
#endif
  re_blocksize += cb.char_lists_size;
  }
#endif
re_blocksize += CU2BYTES(length);
if (re_blocksize > ccontext->max_pattern_compiled_length)
  {
  errorcode = ERR101;
  cb.erroroffset = 0;
  goto HAD_CB_ERROR;
  }
re_blocksize += sizeof(pcre2_real_code);
re = (pcre2_real_code *)
  ccontext->memctl.malloc(re_blocksize, ccontext->memctl.memory_data);
if (re == NULL)
  {
  errorcode = ERR21;
  cb.erroroffset = 0;
  goto HAD_CB_ERROR;
  }
memset((char *)re + sizeof(pcre2_real_code) - 8, 0, 8);
re->memctl = ccontext->memctl;
re->tables = tables;
re->executable_jit = NULL;
memset(re->start_bitmap, 0, 32 * sizeof(uint8_t));
re->blocksize = re_blocksize;
re->code_start = re_blocksize - CU2BYTES(length);
re->magic_number = MAGIC_NUMBER;
re->compile_options = options;
re->overall_options = cb.external_options;
re->extra_options = xoptions;
re->flags = PCRE2_CODE_UNIT_WIDTH/8 | cb.external_flags | setflags;
re->limit_heap = limit_heap;
re->limit_match = limit_match;
re->limit_depth = limit_depth;
re->first_codeunit = 0;
re->last_codeunit = 0;
re->bsr_convention = bsr;
re->newline_convention = newline;
re->max_lookbehind = 0;
re->minlength = 0;
re->top_bracket = 0;
re->top_backref = 0;
re->name_entry_size = cb.name_entry_size;
re->name_count = cb.names_found;
re->optimization_flags = optim_flags;
codestart = (PCRE2_UCHAR *)((uint8_t *)re + re->code_start);
cb.parens_depth = 0;
cb.assert_depth = 0;
cb.lastcapture = 0;
cb.name_table = (PCRE2_UCHAR *)((uint8_t *)re + sizeof(pcre2_real_code));
cb.start_code = codestart;
cb.req_varyopt = 0;
cb.had_accept = FALSE;
cb.had_pruneorskip = FALSE;
#ifdef SUPPORT_WIDE_CHARS
cb.char_lists_size = 0;
#endif
if (cb.names_found > 0)
  {
  named_group *ng = cb.named_groups;
  uint32_t tablecount = 0;
  for (i = 0; i < cb.names_found; i++, ng++)
    if (ng->length > 0)
      tablecount = PRIV(compile_add_name_to_table)(&cb, ng, tablecount);
  PCRE2_ASSERT(tablecount == cb.names_found);
  }
pptr = cb.parsed_pattern;
code = (PCRE2_UCHAR *)codestart;
*code = OP_BRA;
regexrc = compile_regex(re->overall_options, re->extra_options, &code,
  &pptr, &errorcode, 0, &firstcu, &firstcuflags, &reqcu, &reqcuflags, NULL,
  NULL, &cb, NULL);
if (regexrc < 0) re->flags |= PCRE2_MATCH_EMPTY;
re->top_bracket = cb.bracount;
re->top_backref = cb.top_backref;
re->max_lookbehind = cb.max_lookbehind;
if (cb.had_accept)
  {
  reqcu = 0;
  reqcuflags = REQ_NONE;
  re->flags |= PCRE2_HASACCEPT;
  }
*code++ = OP_END;
usedlength = code - codestart;
if (usedlength > length)
  {
  PCRE2_DEBUG_UNREACHABLE();
  errorcode = ERR23;
  cb.erroroffset = 0;
  goto HAD_CB_ERROR;
  }
re->blocksize -= CU2BYTES(length - usedlength);
#ifdef SUPPORT_VALGRIND
VALGRIND_MAKE_MEM_NOACCESS(code, CU2BYTES(length - usedlength));
#endif
#define RSCAN_CACHE_SIZE 8
if (errorcode == 0 && cb.had_recurse)
  {
  PCRE2_UCHAR *rcode;
  PCRE2_SPTR rgroup;
  unsigned int ccount = 0;
  int start = RSCAN_CACHE_SIZE;
  recurse_cache rc[RSCAN_CACHE_SIZE];
  for (rcode = find_recurse(codestart, utf);
       rcode != NULL;
       rcode = find_recurse(rcode + 1 + LINK_SIZE, utf))
    {
    int p, groupnumber;
    groupnumber = (int)GET(rcode, 1);
    if (groupnumber == 0) rgroup = codestart; else
      {
      PCRE2_SPTR search_from = codestart;
      rgroup = NULL;
      for (i = 0, p = start; i < ccount; i++, p = (p + 1) & 7)
        {
        if (groupnumber == rc[p].groupnumber)
          {
          rgroup = rc[p].group;
          break;
          }
        if (groupnumber > rc[p].groupnumber) search_from = rc[p].group;
        }
      if (rgroup == NULL)
        {
        rgroup = PRIV(find_bracket)(search_from, utf, groupnumber);
        if (rgroup == NULL)
          {
          PCRE2_DEBUG_UNREACHABLE();
          errorcode = ERR53;
          break;
          }
        if (--start < 0) start = RSCAN_CACHE_SIZE - 1;
        rc[start].groupnumber = groupnumber;
        rc[start].group = rgroup;
        if (ccount < RSCAN_CACHE_SIZE) ccount++;
        }
      }
    PUT(rcode, 1, (uint32_t)(rgroup - codestart));
    }
  }
#ifdef DEBUG_CALL_PRINTINT
pcre2_printint(re, stderr, TRUE);
fprintf(stderr, "Length=%lu Used=%lu\n", length, usedlength);
#endif
if (errorcode == 0 && (optim_flags & PCRE2_OPTIM_AUTO_POSSESS) != 0)
  {
  PCRE2_UCHAR *temp = (PCRE2_UCHAR *)codestart;
  int possessify_rc = PRIV(auto_possessify)(temp, &cb);
  if (possessify_rc != 0)
    {
    PCRE2_DEBUG_UNREACHABLE();
    errorcode = ERR80;
    cb.erroroffset = 0;
    }
  }
if (errorcode != 0) goto HAD_CB_ERROR;
if ((re->overall_options & PCRE2_ANCHORED) == 0)
  {
  BOOL dotstar_anchor = ((optim_flags & PCRE2_OPTIM_DOTSTAR_ANCHOR) != 0);
  if (is_anchored(codestart, 0, &cb, 0, FALSE, dotstar_anchor))
    re->overall_options |= PCRE2_ANCHORED;
  }
if ((optim_flags & PCRE2_OPTIM_START_OPTIMIZE) != 0)
  {
  int minminlength = 0;
  int study_rc;
  if (firstcuflags >= REQ_NONE) {
    uint32_t assertedcuflags = 0;
    uint32_t assertedcu = find_firstassertedcu(codestart, &assertedcuflags, 0);
    if (assertedcuflags < REQ_NONE && assertedcu != reqcu) {
      firstcu = assertedcu;
      firstcuflags = assertedcuflags;
    }
  }
  if (firstcuflags < REQ_NONE)
    {
    re->first_codeunit = firstcu;
    re->flags |= PCRE2_FIRSTSET;
    minminlength++;
    if ((firstcuflags & REQ_CASELESS) != 0)
      {
      if (firstcu < 128 || (!utf && !ucp && firstcu < 255))
        {
        if (cb.fcc[firstcu] != firstcu) re->flags |= PCRE2_FIRSTCASELESS;
        }
#ifdef SUPPORT_UNICODE
#if PCRE2_CODE_UNIT_WIDTH == 8
      else if (ucp && !utf && UCD_OTHERCASE(firstcu) != firstcu)
        re->flags |= PCRE2_FIRSTCASELESS;
#else
      else if ((utf || ucp) && firstcu <= MAX_UTF_CODE_POINT &&
               UCD_OTHERCASE(firstcu) != firstcu)
        re->flags |= PCRE2_FIRSTCASELESS;
#endif
#endif
      }
    }
  else if ((re->overall_options & PCRE2_ANCHORED) == 0)
    {
    BOOL dotstar_anchor = ((optim_flags & PCRE2_OPTIM_DOTSTAR_ANCHOR) != 0);
    if (is_startline(codestart, 0, &cb, 0, FALSE, dotstar_anchor))
      re->flags |= PCRE2_STARTLINE;
    }
  if (reqcuflags < REQ_NONE)
    {
#if PCRE2_CODE_UNIT_WIDTH == 16
    if ((re->overall_options & PCRE2_UTF) == 0 ||
        firstcuflags >= REQ_NONE ||
        (firstcu & 0xf800) != 0xd800 ||
        (reqcu & 0xfc00) != 0xdc00)
#elif PCRE2_CODE_UNIT_WIDTH == 8
    if ((re->overall_options & PCRE2_UTF) == 0 ||
        firstcuflags >= REQ_NONE ||
        (firstcu & 0x80) == 0 ||
        (reqcu & 0x80) == 0)
#endif
      {
      minminlength++;
      }
    if ((re->overall_options & PCRE2_ANCHORED) == 0 ||
        (reqcuflags & REQ_VARY) != 0)
      {
      re->last_codeunit = reqcu;
      re->flags |= PCRE2_LASTSET;
      if ((reqcuflags & REQ_CASELESS) != 0)
        {
        if (reqcu < 128 || (!utf && !ucp && reqcu < 255))
          {
          if (cb.fcc[reqcu] != reqcu) re->flags |= PCRE2_LASTCASELESS;
          }
#ifdef SUPPORT_UNICODE
#if PCRE2_CODE_UNIT_WIDTH == 8
      else if (ucp && !utf && UCD_OTHERCASE(reqcu) != reqcu)
        re->flags |= PCRE2_LASTCASELESS;
#else
      else if ((utf || ucp) && reqcu <= MAX_UTF_CODE_POINT &&
               UCD_OTHERCASE(reqcu) != reqcu)
        re->flags |= PCRE2_LASTCASELESS;
#endif
#endif
        }
      }
    }
  study_rc = PRIV(study)(re);
  if (study_rc != 0)
    {
    PCRE2_DEBUG_UNREACHABLE();
    errorcode = ERR31;
    cb.erroroffset = 0;
    goto HAD_CB_ERROR;
    }
  if ((re->flags & PCRE2_FIRSTMAPSET) != 0 && minminlength == 0)
    minminlength = 1;
  if (re->minlength < minminlength) re->minlength = minminlength;
  }
#ifdef SUPPORT_UNICODE
PCRE2_ASSERT(cb.first_data == NULL);
#endif
EXIT:
#ifdef SUPPORT_VALGRIND
if (zero_terminated) VALGRIND_MAKE_MEM_DEFINED(pattern + patlen, CU2BYTES(1));
#endif
if (cb.parsed_pattern != stack_parsed_pattern)
  ccontext->memctl.free(cb.parsed_pattern, ccontext->memctl.memory_data);
if (cb.named_group_list_size > NAMED_GROUP_LIST_SIZE)
  ccontext->memctl.free((void *)cb.named_groups, ccontext->memctl.memory_data);
if (cb.groupinfo != stack_groupinfo)
  ccontext->memctl.free((void *)cb.groupinfo, ccontext->memctl.memory_data);
return re;
HAD_CB_ERROR:
ptr = pattern + cb.erroroffset;
HAD_EARLY_ERROR:
PCRE2_ASSERT(ptr >= pattern);
PCRE2_ASSERT(ptr <= (pattern + patlen));
#if defined PCRE2_DEBUG && defined SUPPORT_UNICODE
if (ptr > pattern && utf)
  {
  PCRE2_SPTR prev = ptr - 1;
  PCRE2_SIZE dummyoffset;
  BACKCHAR(prev);
  PCRE2_ASSERT(prev >= pattern);
  PCRE2_ASSERT(PRIV(valid_utf)(prev, ptr - prev, &dummyoffset) == 0);
  }
#endif
*erroroffset = ptr - pattern;
HAD_ERROR:
*errorptr = errorcode;
pcre2_code_free(re);
re = NULL;
if (cb.first_data != NULL)
  {
  compile_data* current_data = cb.first_data;
  do
    {
    compile_data* next_data = current_data->next;
    cb.cx->memctl.free(current_data, cb.cx->memctl.memory_data);
    current_data = next_data;
    }
  while (current_data != NULL);
  }
goto EXIT;
}
#undef NLBLOCK
#undef PSSTART
#undef PSEND
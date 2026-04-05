#ifndef PCRE2_COMPILE_H_IDEMPOTENT_GUARD
#define PCRE2_COMPILE_H_IDEMPOTENT_GUARD
#include "pcre2_internal.h"
enum { ERR0 = COMPILE_ERROR_BASE,
       ERR1,   ERR2,   ERR3,   ERR4,   ERR5,   ERR6,   ERR7,   ERR8,   ERR9,   ERR10,
       ERR11,  ERR12,  ERR13,  ERR14,  ERR15,  ERR16,  ERR17,  ERR18,  ERR19,  ERR20,
       ERR21,  ERR22,  ERR23,  ERR24,  ERR25,  ERR26,  ERR27,  ERR28,  ERR29,  ERR30,
       ERR31,  ERR32,  ERR33,  ERR34,  ERR35,  ERR36,  ERR37,  ERR38,  ERR39,  ERR40,
       ERR41,  ERR42,  ERR43,  ERR44,  ERR45,  ERR46,  ERR47,  ERR48,  ERR49,  ERR50,
       ERR51,  ERR52,  ERR53,  ERR54,  ERR55,  ERR56,  ERR57,  ERR58,  ERR59,  ERR60,
       ERR61,  ERR62,  ERR63,  ERR64,  ERR65,  ERR66,  ERR67,  ERR68,  ERR69,  ERR70,
       ERR71,  ERR72,  ERR73,  ERR74,  ERR75,  ERR76,  ERR77,  ERR78,  ERR79,  ERR80,
       ERR81,  ERR82,  ERR83,  ERR84,  ERR85,  ERR86,  ERR87,  ERR88,  ERR89,  ERR90,
       ERR91,  ERR92,  ERR93,  ERR94,  ERR95,  ERR96,  ERR97,  ERR98,  ERR99,  ERR100,
       ERR101, ERR102, ERR103, ERR104, ERR105, ERR106, ERR107, ERR108, ERR109, ERR110,
       ERR111, ERR112, ERR113, ERR114, ERR115, ERR116, ERR117, ERR118, ERR119, ERR120 };
#define META_END              0x80000000u
#define META_ALT              0x80010000u
#define META_ATOMIC           0x80020000u
#define META_BACKREF          0x80030000u
#define META_BACKREF_BYNAME   0x80040000u
#define META_BIGVALUE         0x80050000u
#define META_CALLOUT_NUMBER   0x80060000u
#define META_CALLOUT_STRING   0x80070000u
#define META_CAPTURE          0x80080000u
#define META_CIRCUMFLEX       0x80090000u
#define META_CLASS            0x800a0000u
#define META_CLASS_EMPTY      0x800b0000u
#define META_CLASS_EMPTY_NOT  0x800c0000u
#define META_CLASS_END        0x800d0000u
#define META_CLASS_NOT        0x800e0000u
#define META_COND_ASSERT      0x800f0000u
#define META_COND_DEFINE      0x80100000u
#define META_COND_NAME        0x80110000u
#define META_COND_NUMBER      0x80120000u
#define META_COND_RNAME       0x80130000u
#define META_COND_RNUMBER     0x80140000u
#define META_COND_VERSION     0x80150000u
#define META_OFFSET           0x80160000u
#define META_SCS              0x80170000u
#define META_CAPTURE_NAME     0x80180000u
#define META_CAPTURE_NUMBER   0x80190000u
#define META_DOLLAR           0x801a0000u
#define META_DOT              0x801b0000u
#define META_ESCAPE           0x801c0000u
#define META_KET              0x801d0000u
#define META_NOCAPTURE        0x801e0000u
#define META_OPTIONS          0x801f0000u
#define META_POSIX            0x80200000u
#define META_POSIX_NEG        0x80210000u
#define META_RANGE_ESCAPED    0x80220000u
#define META_RANGE_LITERAL    0x80230000u
#define META_RECURSE          0x80240000u
#define META_RECURSE_BYNAME   0x80250000u
#define META_SCRIPT_RUN       0x80260000u
#define META_LOOKAHEAD        0x80270000u
#define META_LOOKAHEADNOT     0x80280000u
#define META_LOOKBEHIND       0x80290000u
#define META_LOOKBEHINDNOT    0x802a0000u
#define META_LOOKAHEAD_NA     0x802b0000u
#define META_LOOKBEHIND_NA    0x802c0000u
#define META_MARK             0x802d0000u
#define META_ACCEPT           0x802e0000u
#define META_FAIL             0x802f0000u
#define META_COMMIT           0x80300000u
#define META_COMMIT_ARG       0x80310000u
#define META_PRUNE            0x80320000u
#define META_PRUNE_ARG        0x80330000u
#define META_SKIP             0x80340000u
#define META_SKIP_ARG         0x80350000u
#define META_THEN             0x80360000u
#define META_THEN_ARG         0x80370000u
#define META_ASTERISK         0x80380000u
#define META_ASTERISK_PLUS    0x80390000u
#define META_ASTERISK_QUERY   0x803a0000u
#define META_PLUS             0x803b0000u
#define META_PLUS_PLUS        0x803c0000u
#define META_PLUS_QUERY       0x803d0000u
#define META_QUERY            0x803e0000u
#define META_QUERY_PLUS       0x803f0000u
#define META_QUERY_QUERY      0x80400000u
#define META_MINMAX           0x80410000u
#define META_MINMAX_PLUS      0x80420000u
#define META_MINMAX_QUERY     0x80430000u
#define META_ECLASS_AND       0x80440000u
#define META_ECLASS_OR        0x80450000u
#define META_ECLASS_SUB       0x80460000u
#define META_ECLASS_XOR       0x80470000u
#define META_ECLASS_NOT       0x80480000u
#define META_FIRST_QUANTIFIER META_ASTERISK
#define META_LAST_QUANTIFIER  META_MINMAX_QUERY
#define META_ATOMIC_SCRIPT_RUN 0x8fff0000u
#define META_CODE(x)   (x & 0xffff0000u)
#define META_DATA(x)   (x & 0x0000ffffu)
#define META_DIFF(x,y) ((x-y)>>16)
#if PCRE2_SIZE_MAX <= UINT32_MAX
#define PUTOFFSET(s,p) *p++ = s
#define GETOFFSET(s,p) s = *p++
#define GETPLUSOFFSET(s,p) s = *(++p)
#define READPLUSOFFSET(s,p) s = p[1]
#define SKIPOFFSET(p) p++
#define SIZEOFFSET 1
#else
#define PUTOFFSET(s,p) \
  { *p++ = (uint32_t)(s >> 32); *p++ = (uint32_t)(s & 0xffffffff); }
#define GETOFFSET(s,p) \
  { s = ((PCRE2_SIZE)p[0] << 32) | (PCRE2_SIZE)p[1]; p += 2; }
#define GETPLUSOFFSET(s,p) \
  { s = ((PCRE2_SIZE)p[1] << 32) | (PCRE2_SIZE)p[2]; p += 2; }
#define READPLUSOFFSET(s,p) \
  { s = ((PCRE2_SIZE)p[1] << 32) | (PCRE2_SIZE)p[2]; }
#define SKIPOFFSET(p) p += 2
#define SIZEOFFSET 2
#endif
#ifdef PCRE2_DEBUG
#define CDATA_RECURSE_ARGS       0
#define CDATA_CRANGE             1
#endif
#define CLASS_IS_ECLASS 0x1
#if PCRE2_CODE_UNIT_WIDTH == 8
#define MAX_UCHAR_VALUE 0xffu
#elif PCRE2_CODE_UNIT_WIDTH == 16
#define MAX_UCHAR_VALUE 0xffffu
#else
#define MAX_UCHAR_VALUE 0xffffffffu
#endif
#define GET_MAX_CHAR_VALUE(utf) \
  ((utf) ? MAX_UTF_CODE_POINT : MAX_UCHAR_VALUE)
#define SETBIT(a,b) a[(b) >> 3] |= (uint8_t)(1u << ((b) & 0x7))
#if PCRE2_CODE_UNIT_WIDTH == 8
#define SELECT_VALUE8(value8, value) (value8)
#else
#define SELECT_VALUE8(value8, value) (value)
#endif
#define CLIST_ALIGN_TO(base, align) \
  ((base + ((size_t)(align) - 1)) & ~((size_t)(align) - 1))
typedef struct {
  PCRE2_UCHAR *code_start;
  PCRE2_SIZE length;
  uint8_t op_single_type;
  class_bits_storage bits;
} eclass_op_info;
#define _pcre2_posix_class_maps                PCRE2_SUFFIX(_pcre2_posix_class_maps)
#define _pcre2_update_classbits                PCRE2_SUFFIX(_pcre2_update_classbits_)
#define _pcre2_compile_class_nested            PCRE2_SUFFIX(_pcre2_compile_class_nested_)
#define _pcre2_compile_class_not_nested        PCRE2_SUFFIX(_pcre2_compile_class_not_nested_)
#define _pcre2_compile_get_hash_from_name      PCRE2_SUFFIX(_pcre2_compile_get_hash_from_name)
#define _pcre2_compile_find_named_group        PCRE2_SUFFIX(_pcre2_compile_find_named_group)
#define _pcre2_compile_find_dupname_details    PCRE2_SUFFIX(_pcre2_compile_find_dupname_details)
#define _pcre2_compile_add_name_to_table       PCRE2_SUFFIX(_pcre2_compile_add_name_to_table)
#define _pcre2_compile_parse_scan_substr_args  PCRE2_SUFFIX(_pcre2_compile_parse_scan_substr_args)
#define _pcre2_compile_parse_recurse_args      PCRE2_SUFFIX(_pcre2_compile_parse_recurse_args)
#define PC_DIGIT   7
#define PC_GRAPH   8
#define PC_PRINT   9
#define PC_PUNCT  10
#define PC_XDIGIT 13
extern const int PRIV(posix_class_maps)[];
#define NAMED_GROUP_HASH_MASK      ((uint16_t)0x7fff)
#define NAMED_GROUP_IS_DUPNAME     ((uint16_t)0x8000)
#define NAMED_GROUP_GET_HASH(ng)   ((ng)->hash_dup & NAMED_GROUP_HASH_MASK)
void PRIV(update_classbits)(uint32_t ptype, uint32_t pdata, BOOL negated,
  uint8_t *classbits);
uint32_t *PRIV(compile_class_not_nested)(uint32_t options, uint32_t xoptions,
  uint32_t *start_ptr, PCRE2_UCHAR **pcode, BOOL negate_class, BOOL* has_bitmap,
  int *errorcodeptr, compile_block *cb, PCRE2_SIZE *lengthptr);
BOOL PRIV(compile_class_nested)(uint32_t options, uint32_t xoptions,
  uint32_t **pptr, PCRE2_UCHAR **pcode, int *errorcodeptr,
  compile_block *cb, PCRE2_SIZE *lengthptr);
uint16_t PRIV(compile_get_hash_from_name)(PCRE2_SPTR name, uint32_t length);
named_group *PRIV(compile_find_named_group)(PCRE2_SPTR name,
  uint32_t length, compile_block *cb);
uint32_t PRIV(compile_add_name_to_table)(compile_block *cb,
  named_group *ng, uint32_t tablecount);
BOOL PRIV(compile_find_dupname_details)(PCRE2_SPTR name, uint32_t length,
  int *indexptr, int *countptr, int *errorcodeptr, compile_block *cb);
uint32_t * PRIV(compile_parse_scan_substr_args)(uint32_t *pptr,
  int *errorcodeptr, compile_block *cb, PCRE2_SIZE *lengthptr);
BOOL PRIV(compile_parse_recurse_args)(uint32_t *pptr_start,
  PCRE2_SIZE offset, int *errorcodeptr, compile_block *cb);
#endif
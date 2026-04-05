#ifndef PCRE2_INTERNAL_H_IDEMPOTENT_GUARD
#define PCRE2_INTERNAL_H_IDEMPOTENT_GUARD
#if defined HAVE_CONFIG_H && !defined PCRE2_CONFIG_H_IDEMPOTENT_GUARD
#define PCRE2_CONFIG_H_IDEMPOTENT_GUARD
#include "config.h"
#endif
#ifdef ERLANG_INTEGRATION
#include "local_config.h"
#endif
#if defined EBCDIC && defined SUPPORT_UNICODE
#error The use of both EBCDIC and SUPPORT_UNICODE is not supported.
#endif
#if (!defined PCRE2_PCRE2TEST && !defined PCRE2_DFTABLES) && \
  (!defined PCRE2_CODE_UNIT_WIDTH ||     \
    (PCRE2_CODE_UNIT_WIDTH != 8 &&       \
     PCRE2_CODE_UNIT_WIDTH != 16 &&      \
     PCRE2_CODE_UNIT_WIDTH != 32))
#error PCRE2_CODE_UNIT_WIDTH must be defined as 8, 16, or 32.
#endif
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int BOOL;
#ifndef FALSE
#define FALSE   0
#define TRUE    1
#endif
#define STATIC_ASSERT_JOIN(a,b) a ## b
#define STATIC_ASSERT(cond, msg) \
  typedef int STATIC_ASSERT_JOIN(static_assertion_,msg)[(cond)?1:-1]
#ifdef SUPPORT_VALGRIND
#include <valgrind/memcheck.h>
#endif
#ifdef HAVE_ATTRIBUTE_UNINITIALIZED
#define PCRE2_KEEP_UNINITIALIZED __attribute__((uninitialized))
#else
#define PCRE2_KEEP_UNINITIALIZED
#endif
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define snprintf _snprintf
#endif
#if defined __cplusplus
#error This project uses C99. C++ is not supported.
#endif
#ifndef PCRE2_EXP_DECL
#  if defined(_WIN32) && !defined(PCRE2_STATIC)
#    define PCRE2_EXP_DECL  extern __declspec(dllexport)
#  else
#    define PCRE2_EXP_DECL  extern PCRE2_EXPORT
#  endif
#endif
#ifndef PCRE2_EXP_DEFN
#  if defined(_WIN32) && !defined(PCRE2_STATIC)
#    define PCRE2_EXP_DEFN  extern __declspec(dllexport)
#  else
#    define PCRE2_EXP_DEFN  extern PCRE2_EXPORT
#  endif
#endif
#include "pcre2.h"
#include "pcre2_ucp.h"
#if defined INT64_MAX || defined int64_t
#define INT64_OR_DOUBLE int64_t
#else
#define INT64_OR_DOUBLE double
#endif
#ifndef PRIV
#define PRIV(name) _pcre2_##name
#endif
#if defined(ERLANG_INTEGRATION) && !defined(PCRE2_BUILDING_PCRE2TEST)
struct PRIV(valid_utf_ystate) {
    int32_t loops_left;
    int length;
    int yielded;
    PCRE2_SPTR p;
};
extern int               PRIV(yielding_valid_utf)(PCRE2_SPTR, PCRE2_SIZE, PCRE2_SIZE *,
                                                  struct PRIV(valid_utf_ystate) *);
#endif
#define NOTACHAR 0xffffffff
#define MAX_UTF_CODE_POINT 0x10ffff
#define COMPILE_ERROR_BASE 100
#define START_FRAMES_SIZE 20480
#define DFA_START_RWS_SIZE 30720
#ifdef BSR_ANYCRLF
#define BSR_DEFAULT PCRE2_BSR_ANYCRLF
#else
#define BSR_DEFAULT PCRE2_BSR_UNICODE
#endif
#define HASUTF8EXTRALEN(c) ((c) >= 0xc0)
#define GETUTF8(c, eptr) \
    { \
    if ((c & 0x20u) == 0) \
      c = ((c & 0x1fu) << 6) | (eptr[1] & 0x3fu); \
    else if ((c & 0x10u) == 0) \
      c = ((c & 0x0fu) << 12) | ((eptr[1] & 0x3fu) << 6) | (eptr[2] & 0x3fu); \
    else if ((c & 0x08u) == 0) \
      c = ((c & 0x07u) << 18) | ((eptr[1] & 0x3fu) << 12) | \
      ((eptr[2] & 0x3fu) << 6) | (eptr[3] & 0x3fu); \
    else if ((c & 0x04u) == 0) \
      c = ((c & 0x03u) << 24) | ((eptr[1] & 0x3fu) << 18) | \
          ((eptr[2] & 0x3fu) << 12) | ((eptr[3] & 0x3fu) << 6) | \
          (eptr[4] & 0x3fu); \
    else \
      c = ((c & 0x01u) << 30) | ((eptr[1] & 0x3fu) << 24) | \
          ((eptr[2] & 0x3fu) << 18) | ((eptr[3] & 0x3fu) << 12) | \
          ((eptr[4] & 0x3fu) << 6) | (eptr[5] & 0x3fu); \
    }
#define GETUTF8INC(c, eptr) \
    { \
    if ((c & 0x20u) == 0) \
      c = ((c & 0x1fu) << 6) | (*eptr++ & 0x3fu); \
    else if ((c & 0x10u) == 0) \
      { \
      c = ((c & 0x0fu) << 12) | ((*eptr & 0x3fu) << 6) | (eptr[1] & 0x3fu); \
      eptr += 2; \
      } \
    else if ((c & 0x08u) == 0) \
      { \
      c = ((c & 0x07u) << 18) | ((*eptr & 0x3fu) << 12) | \
          ((eptr[1] & 0x3fu) << 6) | (eptr[2] & 0x3fu); \
      eptr += 3; \
      } \
    else if ((c & 0x04u) == 0) \
      { \
      c = ((c & 0x03u) << 24) | ((*eptr & 0x3fu) << 18) | \
          ((eptr[1] & 0x3fu) << 12) | ((eptr[2] & 0x3fu) << 6) | \
          (eptr[3] & 0x3fu); \
      eptr += 4; \
      } \
    else \
      { \
      c = ((c & 0x01u) << 30) | ((*eptr & 0x3fu) << 24) | \
          ((eptr[1] & 0x3fu) << 18) | ((eptr[2] & 0x3fu) << 12) | \
          ((eptr[3] & 0x3fu) << 6) | (eptr[4] & 0x3fu); \
      eptr += 5; \
      } \
    }
#define GETUTF8LEN(c, eptr, len) \
    { \
    if ((c & 0x20u) == 0) \
      { \
      c = ((c & 0x1fu) << 6) | (eptr[1] & 0x3fu); \
      len++; \
      } \
    else if ((c & 0x10u)  == 0) \
      { \
      c = ((c & 0x0fu) << 12) | ((eptr[1] & 0x3fu) << 6) | (eptr[2] & 0x3fu); \
      len += 2; \
      } \
    else if ((c & 0x08u)  == 0) \
      {\
      c = ((c & 0x07u) << 18) | ((eptr[1] & 0x3fu) << 12) | \
          ((eptr[2] & 0x3fu) << 6) | (eptr[3] & 0x3fu); \
      len += 3; \
      } \
    else if ((c & 0x04u)  == 0) \
      { \
      c = ((c & 0x03u) << 24) | ((eptr[1] & 0x3fu) << 18) | \
          ((eptr[2] & 0x3fu) << 12) | ((eptr[3] & 0x3fu) << 6) | \
          (eptr[4] & 0x3fu); \
      len += 4; \
      } \
    else \
      {\
      c = ((c & 0x01u) << 30) | ((eptr[1] & 0x3fu) << 24) | \
          ((eptr[2] & 0x3fu) << 18) | ((eptr[3] & 0x3fu) << 12) | \
          ((eptr[4] & 0x3fu) << 6) | (eptr[5] & 0x3fu); \
      len += 5; \
      } \
    }
#ifndef EBCDIC
#define HSPACE_LIST \
  CHAR_HT, CHAR_SPACE, CHAR_NBSP, \
  0x1680, 0x180e, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, \
  0x2006, 0x2007, 0x2008, 0x2009, 0x200a, 0x202f, 0x205f, 0x3000, \
  NOTACHAR
#define HSPACE_MULTIBYTE_CASES \
  case 0x1680:   \
  case 0x180e:   \
  case 0x2000:   \
  case 0x2001:   \
  case 0x2002:   \
  case 0x2003:   \
  case 0x2004:   \
  case 0x2005:   \
  case 0x2006:   \
  case 0x2007:   \
  case 0x2008:   \
  case 0x2009:   \
  case 0x200a:   \
  case 0x202f:   \
  case 0x205f:   \
  case 0x3000
#define HSPACE_BYTE_CASES \
  case CHAR_HT: \
  case CHAR_SPACE: \
  case CHAR_NBSP
#define HSPACE_CASES \
  HSPACE_BYTE_CASES: \
  HSPACE_MULTIBYTE_CASES
#define VSPACE_LIST \
  CHAR_LF, CHAR_VT, CHAR_FF, CHAR_CR, CHAR_NEL, 0x2028, 0x2029, NOTACHAR
#define VSPACE_MULTIBYTE_CASES \
  case 0x2028:     \
  case 0x2029
#define VSPACE_BYTE_CASES \
  case CHAR_LF: \
  case CHAR_VT: \
  case CHAR_FF: \
  case CHAR_CR: \
  case CHAR_NEL
#define VSPACE_CASES \
  VSPACE_BYTE_CASES: \
  VSPACE_MULTIBYTE_CASES
#else
#define HSPACE_LIST CHAR_HT, CHAR_SPACE, CHAR_NBSP, NOTACHAR
#define HSPACE_BYTE_CASES \
  case CHAR_HT: \
  case CHAR_SPACE: \
  case CHAR_NBSP
#define HSPACE_CASES HSPACE_BYTE_CASES
#ifdef EBCDIC_NL25
#define VSPACE_LIST \
  CHAR_VT, CHAR_FF, CHAR_CR, CHAR_NEL, CHAR_LF, NOTACHAR
#else
#define VSPACE_LIST \
  CHAR_VT, CHAR_FF, CHAR_CR, CHAR_LF, CHAR_NEL, NOTACHAR
#endif
#define VSPACE_BYTE_CASES \
  case CHAR_LF: \
  case CHAR_VT: \
  case CHAR_FF: \
  case CHAR_CR: \
  case CHAR_NEL
#define VSPACE_CASES VSPACE_BYTE_CASES
#endif
#define NLTYPE_FIXED    0
#define NLTYPE_ANY      1
#define NLTYPE_ANYCRLF  2
#define IS_NEWLINE(p) \
  ((NLBLOCK->nltype != NLTYPE_FIXED)? \
    ((p) < NLBLOCK->PSEND && \
     PRIV(is_newline)((p), NLBLOCK->nltype, NLBLOCK->PSEND, \
       &(NLBLOCK->nllen), utf)) \
    : \
    ((p) <= NLBLOCK->PSEND - NLBLOCK->nllen && \
     UCHAR21TEST(p) == NLBLOCK->nl[0] && \
     (NLBLOCK->nllen == 1 || UCHAR21TEST(p+1) == NLBLOCK->nl[1])       \
    ) \
  )
#define WAS_NEWLINE(p) \
  ((NLBLOCK->nltype != NLTYPE_FIXED)? \
    ((p) > NLBLOCK->PSSTART && \
     PRIV(was_newline)((p), NLBLOCK->nltype, NLBLOCK->PSSTART, \
       &(NLBLOCK->nllen), utf)) \
    : \
    ((p) >= NLBLOCK->PSSTART + NLBLOCK->nllen && \
     UCHAR21TEST(p - NLBLOCK->nllen) == NLBLOCK->nl[0] &&              \
     (NLBLOCK->nllen == 1 || UCHAR21TEST(p - NLBLOCK->nllen + 1) == NLBLOCK->nl[1]) \
    ) \
  )
#define PCRE2_MODE8         0x00000001u
#define PCRE2_MODE16        0x00000002u
#define PCRE2_MODE32        0x00000004u
#define PCRE2_FIRSTSET      0x00000010u
#define PCRE2_FIRSTCASELESS 0x00000020u
#define PCRE2_FIRSTMAPSET   0x00000040u
#define PCRE2_LASTSET       0x00000080u
#define PCRE2_LASTCASELESS  0x00000100u
#define PCRE2_STARTLINE     0x00000200u
#define PCRE2_JCHANGED      0x00000400u
#define PCRE2_HASCRORLF     0x00000800u
#define PCRE2_HASTHEN       0x00001000u
#define PCRE2_MATCH_EMPTY   0x00002000u
#define PCRE2_BSR_SET       0x00004000u
#define PCRE2_NL_SET        0x00008000u
#define PCRE2_NOTEMPTY_SET  0x00010000u
#define PCRE2_NE_ATST_SET   0x00020000u
#define PCRE2_DEREF_TABLES  0x00040000u
#define PCRE2_NOJIT         0x00080000u
#define PCRE2_HASBKPORX     0x00100000u
#define PCRE2_DUPCAPUSED    0x00200000u
#define PCRE2_HASBKC        0x00400000u
#define PCRE2_HASACCEPT     0x00800000u
#define PCRE2_HASBSK        0x01000000u
#define PCRE2_MODE_MASK     (PCRE2_MODE8 | PCRE2_MODE16 | PCRE2_MODE32)
enum { PCRE2_MATCHEDBY_INTERPRETER,
       PCRE2_MATCHEDBY_DFA_INTERPRETER,
       PCRE2_MATCHEDBY_JIT };
#define PCRE2_MD_COPIED_SUBJECT  0x01u
#define MAGIC_NUMBER  0x50435245UL
#if PCRE2_CODE_UNIT_WIDTH == 8
#define REQ_CU_MAX       5000
#else
#define REQ_CU_MAX       2000
#endif
#define ECLASS_NEST_LIMIT  15
#define cbit_space     0
#define cbit_xdigit   32
#define cbit_digit    64
#define cbit_upper    96
#define cbit_lower   128
#define cbit_word    160
#define cbit_graph   192
#define cbit_print   224
#define cbit_punct   256
#define cbit_cntrl   288
#define cbit_length  320
#define ctype_space    0x01
#define ctype_letter   0x02
#define ctype_lcletter 0x04
#define ctype_digit    0x08
#define ctype_word     0x10
#define lcc_offset      0
#define fcc_offset    256
#define cbits_offset  512
#define ctypes_offset (cbits_offset + cbit_length)
#define TABLES_LENGTH (ctypes_offset + 256)
#define PCRE2_OPTIM_AUTO_POSSESS    0x00000001u
#define PCRE2_OPTIM_DOTSTAR_ANCHOR  0x00000002u
#define PCRE2_OPTIM_START_OPTIMIZE  0x00000004u
#define PCRE2_OPTIMIZATION_ALL      0x00000007u
#ifndef SUPPORT_UNICODE
#ifdef EBCDIC
#ifndef EBCDIC_NL25
#define CHAR_NL                     '\x15'
#define CHAR_NEL                    '\x25'
#define STR_NL                      "\x15"
#define STR_NEL                     "\x25"
#else
#define CHAR_NL                     '\x25'
#define CHAR_NEL                    '\x15'
#define STR_NL                      "\x25"
#define STR_NEL                     "\x15"
#endif
#define CHAR_LF                     CHAR_NL
#define STR_LF                      STR_NL
#define CHAR_ESC                    '\047'
#define CHAR_DEL                    '\007'
#define CHAR_NBSP                   ((unsigned char)'\x41')
#define STR_ESC                     "\047"
#define STR_DEL                     "\007"
#else
#if '\n' != 0x0a
#error "ASCII character '\n' is not 0x0a"
#endif
#define CHAR_LF                     '\n'
#define CHAR_NL                     CHAR_LF
#define CHAR_NEL                    ((unsigned char)'\x85')
#define CHAR_ESC                    '\033'
#define CHAR_DEL                    '\177'
#define CHAR_NBSP                   ((unsigned char)'\xa0')
#define STR_LF                      "\n"
#define STR_NL                      STR_LF
#define STR_NEL                     "\x85"
#define STR_ESC                     "\033"
#define STR_DEL                     "\177"
#endif
#ifdef EBCDIC_IGNORING_COMPILER
#define CHAR_NUL                    '\000'
#define CHAR_HT                     '\005'
#define CHAR_VT                     '\013'
#define CHAR_FF                     '\014'
#define CHAR_CR                     '\015'
#define CHAR_BS                     '\026'
#define CHAR_BEL                    '\057'
#define CHAR_SPACE                  '\100'
#define CHAR_EXCLAMATION_MARK       '\132'
#define CHAR_QUOTATION_MARK         '\177'
#define CHAR_NUMBER_SIGN            '\173'
#define CHAR_DOLLAR_SIGN            '\133'
#define CHAR_PERCENT_SIGN           '\154'
#define CHAR_AMPERSAND              '\120'
#define CHAR_APOSTROPHE             '\175'
#define CHAR_LEFT_PARENTHESIS       '\115'
#define CHAR_RIGHT_PARENTHESIS      '\135'
#define CHAR_ASTERISK               '\134'
#define CHAR_PLUS                   '\116'
#define CHAR_COMMA                  '\153'
#define CHAR_MINUS                  '\140'
#define CHAR_DOT                    '\113'
#define CHAR_SLASH                  '\141'
#define CHAR_0                      ((unsigned char)'\xf0')
#define CHAR_1                      ((unsigned char)'\xf1')
#define CHAR_2                      ((unsigned char)'\xf2')
#define CHAR_3                      ((unsigned char)'\xf3')
#define CHAR_4                      ((unsigned char)'\xf4')
#define CHAR_5                      ((unsigned char)'\xf5')
#define CHAR_6                      ((unsigned char)'\xf6')
#define CHAR_7                      ((unsigned char)'\xf7')
#define CHAR_8                      ((unsigned char)'\xf8')
#define CHAR_9                      ((unsigned char)'\xf9')
#define CHAR_COLON                  '\172'
#define CHAR_SEMICOLON              '\136'
#define CHAR_LESS_THAN_SIGN         '\114'
#define CHAR_EQUALS_SIGN            '\176'
#define CHAR_GREATER_THAN_SIGN      '\156'
#define CHAR_QUESTION_MARK          '\157'
#define CHAR_COMMERCIAL_AT          '\174'
#define CHAR_A                      ((unsigned char)'\xc1')
#define CHAR_B                      ((unsigned char)'\xc2')
#define CHAR_C                      ((unsigned char)'\xc3')
#define CHAR_D                      ((unsigned char)'\xc4')
#define CHAR_E                      ((unsigned char)'\xc5')
#define CHAR_F                      ((unsigned char)'\xc6')
#define CHAR_G                      ((unsigned char)'\xc7')
#define CHAR_H                      ((unsigned char)'\xc8')
#define CHAR_I                      ((unsigned char)'\xc9')
#define CHAR_J                      ((unsigned char)'\xd1')
#define CHAR_K                      ((unsigned char)'\xd2')
#define CHAR_L                      ((unsigned char)'\xd3')
#define CHAR_M                      ((unsigned char)'\xd4')
#define CHAR_N                      ((unsigned char)'\xd5')
#define CHAR_O                      ((unsigned char)'\xd6')
#define CHAR_P                      ((unsigned char)'\xd7')
#define CHAR_Q                      ((unsigned char)'\xd8')
#define CHAR_R                      ((unsigned char)'\xd9')
#define CHAR_S                      ((unsigned char)'\xe2')
#define CHAR_T                      ((unsigned char)'\xe3')
#define CHAR_U                      ((unsigned char)'\xe4')
#define CHAR_V                      ((unsigned char)'\xe5')
#define CHAR_W                      ((unsigned char)'\xe6')
#define CHAR_X                      ((unsigned char)'\xe7')
#define CHAR_Y                      ((unsigned char)'\xe8')
#define CHAR_Z                      ((unsigned char)'\xe9')
#define CHAR_LEFT_SQUARE_BRACKET    ((unsigned char)'\xad')
#define CHAR_BACKSLASH              ((unsigned char)'\xe0')
#define CHAR_RIGHT_SQUARE_BRACKET   ((unsigned char)'\xbd')
#define CHAR_CIRCUMFLEX_ACCENT      '\137'
#define CHAR_UNDERSCORE             '\155'
#define CHAR_GRAVE_ACCENT           '\171'
#define CHAR_a                      ((unsigned char)'\x81')
#define CHAR_b                      ((unsigned char)'\x82')
#define CHAR_c                      ((unsigned char)'\x83')
#define CHAR_d                      ((unsigned char)'\x84')
#define CHAR_e                      ((unsigned char)'\x85')
#define CHAR_f                      ((unsigned char)'\x86')
#define CHAR_g                      ((unsigned char)'\x87')
#define CHAR_h                      ((unsigned char)'\x88')
#define CHAR_i                      ((unsigned char)'\x89')
#define CHAR_j                      ((unsigned char)'\x91')
#define CHAR_k                      ((unsigned char)'\x92')
#define CHAR_l                      ((unsigned char)'\x93')
#define CHAR_m                      ((unsigned char)'\x94')
#define CHAR_n                      ((unsigned char)'\x95')
#define CHAR_o                      ((unsigned char)'\x96')
#define CHAR_p                      ((unsigned char)'\x97')
#define CHAR_q                      ((unsigned char)'\x98')
#define CHAR_r                      ((unsigned char)'\x99')
#define CHAR_s                      ((unsigned char)'\xa2')
#define CHAR_t                      ((unsigned char)'\xa3')
#define CHAR_u                      ((unsigned char)'\xa4')
#define CHAR_v                      ((unsigned char)'\xa5')
#define CHAR_w                      ((unsigned char)'\xa6')
#define CHAR_x                      ((unsigned char)'\xa7')
#define CHAR_y                      ((unsigned char)'\xa8')
#define CHAR_z                      ((unsigned char)'\xa9')
#define CHAR_LEFT_CURLY_BRACKET     ((unsigned char)'\xc0')
#define CHAR_VERTICAL_LINE          '\117'
#define CHAR_RIGHT_CURLY_BRACKET    ((unsigned char)'\xd0')
#define CHAR_TILDE                  ((unsigned char)'\xa1')
#define STR_HT                      "\005"
#define STR_VT                      "\013"
#define STR_FF                      "\014"
#define STR_CR                      "\015"
#define STR_BS                      "\026"
#define STR_BEL                     "\057"
#define STR_SPACE                   "\100"
#define STR_EXCLAMATION_MARK        "\132"
#define STR_QUOTATION_MARK          "\177"
#define STR_NUMBER_SIGN             "\173"
#define STR_DOLLAR_SIGN             "\133"
#define STR_PERCENT_SIGN            "\154"
#define STR_AMPERSAND               "\120"
#define STR_APOSTROPHE              "\175"
#define STR_LEFT_PARENTHESIS        "\115"
#define STR_RIGHT_PARENTHESIS       "\135"
#define STR_ASTERISK                "\134"
#define STR_PLUS                    "\116"
#define STR_COMMA                   "\153"
#define STR_MINUS                   "\140"
#define STR_DOT                     "\113"
#define STR_SLASH                   "\141"
#define STR_0                       "\360"
#define STR_1                       "\361"
#define STR_2                       "\362"
#define STR_3                       "\363"
#define STR_4                       "\364"
#define STR_5                       "\365"
#define STR_6                       "\366"
#define STR_7                       "\367"
#define STR_8                       "\370"
#define STR_9                       "\371"
#define STR_COLON                   "\172"
#define STR_SEMICOLON               "\136"
#define STR_LESS_THAN_SIGN          "\114"
#define STR_EQUALS_SIGN             "\176"
#define STR_GREATER_THAN_SIGN       "\156"
#define STR_QUESTION_MARK           "\157"
#define STR_COMMERCIAL_AT           "\174"
#define STR_A                       "\301"
#define STR_B                       "\302"
#define STR_C                       "\303"
#define STR_D                       "\304"
#define STR_E                       "\305"
#define STR_F                       "\306"
#define STR_G                       "\307"
#define STR_H                       "\310"
#define STR_I                       "\311"
#define STR_J                       "\321"
#define STR_K                       "\322"
#define STR_L                       "\323"
#define STR_M                       "\324"
#define STR_N                       "\325"
#define STR_O                       "\326"
#define STR_P                       "\327"
#define STR_Q                       "\330"
#define STR_R                       "\331"
#define STR_S                       "\342"
#define STR_T                       "\343"
#define STR_U                       "\344"
#define STR_V                       "\345"
#define STR_W                       "\346"
#define STR_X                       "\347"
#define STR_Y                       "\350"
#define STR_Z                       "\351"
#define STR_LEFT_SQUARE_BRACKET     "\255"
#define STR_BACKSLASH               "\340"
#define STR_RIGHT_SQUARE_BRACKET    "\275"
#define STR_CIRCUMFLEX_ACCENT       "\137"
#define STR_UNDERSCORE              "\155"
#define STR_GRAVE_ACCENT            "\171"
#define STR_a                       "\201"
#define STR_b                       "\202"
#define STR_c                       "\203"
#define STR_d                       "\204"
#define STR_e                       "\205"
#define STR_f                       "\206"
#define STR_g                       "\207"
#define STR_h                       "\210"
#define STR_i                       "\211"
#define STR_j                       "\221"
#define STR_k                       "\222"
#define STR_l                       "\223"
#define STR_m                       "\224"
#define STR_n                       "\225"
#define STR_o                       "\226"
#define STR_p                       "\227"
#define STR_q                       "\230"
#define STR_r                       "\231"
#define STR_s                       "\242"
#define STR_t                       "\243"
#define STR_u                       "\244"
#define STR_v                       "\245"
#define STR_w                       "\246"
#define STR_x                       "\247"
#define STR_y                       "\250"
#define STR_z                       "\251"
#define STR_LEFT_CURLY_BRACKET      "\300"
#define STR_VERTICAL_LINE           "\117"
#define STR_RIGHT_CURLY_BRACKET     "\320"
#define STR_TILDE                   "\241"
#else
#ifdef EBCDIC
#if 'a' != 0x81
#error "EBCDIC character 'a' is not 0x81"
#endif
#else
#if 'a' != 0x61
#error "ASCII character 'a' is not 0x61"
#endif
#endif
#define CHAR_NUL                    '\0'
#define CHAR_HT                     '\t'
#define CHAR_VT                     '\v'
#define CHAR_FF                     '\f'
#define CHAR_CR                     '\r'
#define CHAR_BS                     '\b'
#define CHAR_BEL                    '\a'
#define CHAR_SPACE                  ' '
#define CHAR_EXCLAMATION_MARK       '!'
#define CHAR_QUOTATION_MARK         '"'
#define CHAR_NUMBER_SIGN            '#'
#define CHAR_DOLLAR_SIGN            '$'
#define CHAR_PERCENT_SIGN           '%'
#define CHAR_AMPERSAND              '&'
#define CHAR_APOSTROPHE             '\''
#define CHAR_LEFT_PARENTHESIS       '('
#define CHAR_RIGHT_PARENTHESIS      ')'
#define CHAR_ASTERISK               '*'
#define CHAR_PLUS                   '+'
#define CHAR_COMMA                  ','
#define CHAR_MINUS                  '-'
#define CHAR_DOT                    '.'
#define CHAR_SLASH                  '/'
#define CHAR_0                      '0'
#define CHAR_1                      '1'
#define CHAR_2                      '2'
#define CHAR_3                      '3'
#define CHAR_4                      '4'
#define CHAR_5                      '5'
#define CHAR_6                      '6'
#define CHAR_7                      '7'
#define CHAR_8                      '8'
#define CHAR_9                      '9'
#define CHAR_COLON                  ':'
#define CHAR_SEMICOLON              ';'
#define CHAR_LESS_THAN_SIGN         '<'
#define CHAR_EQUALS_SIGN            '='
#define CHAR_GREATER_THAN_SIGN      '>'
#define CHAR_QUESTION_MARK          '?'
#define CHAR_COMMERCIAL_AT          '@'
#define CHAR_A                      'A'
#define CHAR_B                      'B'
#define CHAR_C                      'C'
#define CHAR_D                      'D'
#define CHAR_E                      'E'
#define CHAR_F                      'F'
#define CHAR_G                      'G'
#define CHAR_H                      'H'
#define CHAR_I                      'I'
#define CHAR_J                      'J'
#define CHAR_K                      'K'
#define CHAR_L                      'L'
#define CHAR_M                      'M'
#define CHAR_N                      'N'
#define CHAR_O                      'O'
#define CHAR_P                      'P'
#define CHAR_Q                      'Q'
#define CHAR_R                      'R'
#define CHAR_S                      'S'
#define CHAR_T                      'T'
#define CHAR_U                      'U'
#define CHAR_V                      'V'
#define CHAR_W                      'W'
#define CHAR_X                      'X'
#define CHAR_Y                      'Y'
#define CHAR_Z                      'Z'
#define CHAR_LEFT_SQUARE_BRACKET    '['
#define CHAR_BACKSLASH              '\\'
#define CHAR_RIGHT_SQUARE_BRACKET   ']'
#define CHAR_CIRCUMFLEX_ACCENT      '^'
#define CHAR_UNDERSCORE             '_'
#define CHAR_GRAVE_ACCENT           '`'
#define CHAR_a                      'a'
#define CHAR_b                      'b'
#define CHAR_c                      'c'
#define CHAR_d                      'd'
#define CHAR_e                      'e'
#define CHAR_f                      'f'
#define CHAR_g                      'g'
#define CHAR_h                      'h'
#define CHAR_i                      'i'
#define CHAR_j                      'j'
#define CHAR_k                      'k'
#define CHAR_l                      'l'
#define CHAR_m                      'm'
#define CHAR_n                      'n'
#define CHAR_o                      'o'
#define CHAR_p                      'p'
#define CHAR_q                      'q'
#define CHAR_r                      'r'
#define CHAR_s                      's'
#define CHAR_t                      't'
#define CHAR_u                      'u'
#define CHAR_v                      'v'
#define CHAR_w                      'w'
#define CHAR_x                      'x'
#define CHAR_y                      'y'
#define CHAR_z                      'z'
#define CHAR_LEFT_CURLY_BRACKET     '{'
#define CHAR_VERTICAL_LINE          '|'
#define CHAR_RIGHT_CURLY_BRACKET    '}'
#define CHAR_TILDE                  '~'
#define STR_HT                      "\t"
#define STR_VT                      "\v"
#define STR_FF                      "\f"
#define STR_CR                      "\r"
#define STR_BS                      "\b"
#define STR_BEL                     "\a"
#define STR_SPACE                   " "
#define STR_EXCLAMATION_MARK        "!"
#define STR_QUOTATION_MARK          "\""
#define STR_NUMBER_SIGN             "#"
#define STR_DOLLAR_SIGN             "$"
#define STR_PERCENT_SIGN            "%"
#define STR_AMPERSAND               "&"
#define STR_APOSTROPHE              "'"
#define STR_LEFT_PARENTHESIS        "("
#define STR_RIGHT_PARENTHESIS       ")"
#define STR_ASTERISK                "*"
#define STR_PLUS                    "+"
#define STR_COMMA                   ","
#define STR_MINUS                   "-"
#define STR_DOT                     "."
#define STR_SLASH                   "/"
#define STR_0                       "0"
#define STR_1                       "1"
#define STR_2                       "2"
#define STR_3                       "3"
#define STR_4                       "4"
#define STR_5                       "5"
#define STR_6                       "6"
#define STR_7                       "7"
#define STR_8                       "8"
#define STR_9                       "9"
#define STR_COLON                   ":"
#define STR_SEMICOLON               ";"
#define STR_LESS_THAN_SIGN          "<"
#define STR_EQUALS_SIGN             "="
#define STR_GREATER_THAN_SIGN       ">"
#define STR_QUESTION_MARK           "?"
#define STR_COMMERCIAL_AT           "@"
#define STR_A                       "A"
#define STR_B                       "B"
#define STR_C                       "C"
#define STR_D                       "D"
#define STR_E                       "E"
#define STR_F                       "F"
#define STR_G                       "G"
#define STR_H                       "H"
#define STR_I                       "I"
#define STR_J                       "J"
#define STR_K                       "K"
#define STR_L                       "L"
#define STR_M                       "M"
#define STR_N                       "N"
#define STR_O                       "O"
#define STR_P                       "P"
#define STR_Q                       "Q"
#define STR_R                       "R"
#define STR_S                       "S"
#define STR_T                       "T"
#define STR_U                       "U"
#define STR_V                       "V"
#define STR_W                       "W"
#define STR_X                       "X"
#define STR_Y                       "Y"
#define STR_Z                       "Z"
#define STR_LEFT_SQUARE_BRACKET     "["
#define STR_BACKSLASH               "\\"
#define STR_RIGHT_SQUARE_BRACKET    "]"
#define STR_CIRCUMFLEX_ACCENT       "^"
#define STR_UNDERSCORE              "_"
#define STR_GRAVE_ACCENT            "`"
#define STR_a                       "a"
#define STR_b                       "b"
#define STR_c                       "c"
#define STR_d                       "d"
#define STR_e                       "e"
#define STR_f                       "f"
#define STR_g                       "g"
#define STR_h                       "h"
#define STR_i                       "i"
#define STR_j                       "j"
#define STR_k                       "k"
#define STR_l                       "l"
#define STR_m                       "m"
#define STR_n                       "n"
#define STR_o                       "o"
#define STR_p                       "p"
#define STR_q                       "q"
#define STR_r                       "r"
#define STR_s                       "s"
#define STR_t                       "t"
#define STR_u                       "u"
#define STR_v                       "v"
#define STR_w                       "w"
#define STR_x                       "x"
#define STR_y                       "y"
#define STR_z                       "z"
#define STR_LEFT_CURLY_BRACKET      "{"
#define STR_VERTICAL_LINE           "|"
#define STR_RIGHT_CURLY_BRACKET     "}"
#define STR_TILDE                   "~"
#endif
#else
#define CHAR_HT                     '\011'
#define CHAR_VT                     '\013'
#define CHAR_FF                     '\014'
#define CHAR_CR                     '\015'
#define CHAR_LF                     '\012'
#define CHAR_NL                     CHAR_LF
#define CHAR_NEL                    ((unsigned char)'\x85')
#define CHAR_BS                     '\010'
#define CHAR_BEL                    '\007'
#define CHAR_ESC                    '\033'
#define CHAR_DEL                    '\177'
#define CHAR_NUL                    '\0'
#define CHAR_SPACE                  '\040'
#define CHAR_EXCLAMATION_MARK       '\041'
#define CHAR_QUOTATION_MARK         '\042'
#define CHAR_NUMBER_SIGN            '\043'
#define CHAR_DOLLAR_SIGN            '\044'
#define CHAR_PERCENT_SIGN           '\045'
#define CHAR_AMPERSAND              '\046'
#define CHAR_APOSTROPHE             '\047'
#define CHAR_LEFT_PARENTHESIS       '\050'
#define CHAR_RIGHT_PARENTHESIS      '\051'
#define CHAR_ASTERISK               '\052'
#define CHAR_PLUS                   '\053'
#define CHAR_COMMA                  '\054'
#define CHAR_MINUS                  '\055'
#define CHAR_DOT                    '\056'
#define CHAR_SLASH                  '\057'
#define CHAR_0                      '\060'
#define CHAR_1                      '\061'
#define CHAR_2                      '\062'
#define CHAR_3                      '\063'
#define CHAR_4                      '\064'
#define CHAR_5                      '\065'
#define CHAR_6                      '\066'
#define CHAR_7                      '\067'
#define CHAR_8                      '\070'
#define CHAR_9                      '\071'
#define CHAR_COLON                  '\072'
#define CHAR_SEMICOLON              '\073'
#define CHAR_LESS_THAN_SIGN         '\074'
#define CHAR_EQUALS_SIGN            '\075'
#define CHAR_GREATER_THAN_SIGN      '\076'
#define CHAR_QUESTION_MARK          '\077'
#define CHAR_COMMERCIAL_AT          '\100'
#define CHAR_A                      '\101'
#define CHAR_B                      '\102'
#define CHAR_C                      '\103'
#define CHAR_D                      '\104'
#define CHAR_E                      '\105'
#define CHAR_F                      '\106'
#define CHAR_G                      '\107'
#define CHAR_H                      '\110'
#define CHAR_I                      '\111'
#define CHAR_J                      '\112'
#define CHAR_K                      '\113'
#define CHAR_L                      '\114'
#define CHAR_M                      '\115'
#define CHAR_N                      '\116'
#define CHAR_O                      '\117'
#define CHAR_P                      '\120'
#define CHAR_Q                      '\121'
#define CHAR_R                      '\122'
#define CHAR_S                      '\123'
#define CHAR_T                      '\124'
#define CHAR_U                      '\125'
#define CHAR_V                      '\126'
#define CHAR_W                      '\127'
#define CHAR_X                      '\130'
#define CHAR_Y                      '\131'
#define CHAR_Z                      '\132'
#define CHAR_LEFT_SQUARE_BRACKET    '\133'
#define CHAR_BACKSLASH              '\134'
#define CHAR_RIGHT_SQUARE_BRACKET   '\135'
#define CHAR_CIRCUMFLEX_ACCENT      '\136'
#define CHAR_UNDERSCORE             '\137'
#define CHAR_GRAVE_ACCENT           '\140'
#define CHAR_a                      '\141'
#define CHAR_b                      '\142'
#define CHAR_c                      '\143'
#define CHAR_d                      '\144'
#define CHAR_e                      '\145'
#define CHAR_f                      '\146'
#define CHAR_g                      '\147'
#define CHAR_h                      '\150'
#define CHAR_i                      '\151'
#define CHAR_j                      '\152'
#define CHAR_k                      '\153'
#define CHAR_l                      '\154'
#define CHAR_m                      '\155'
#define CHAR_n                      '\156'
#define CHAR_o                      '\157'
#define CHAR_p                      '\160'
#define CHAR_q                      '\161'
#define CHAR_r                      '\162'
#define CHAR_s                      '\163'
#define CHAR_t                      '\164'
#define CHAR_u                      '\165'
#define CHAR_v                      '\166'
#define CHAR_w                      '\167'
#define CHAR_x                      '\170'
#define CHAR_y                      '\171'
#define CHAR_z                      '\172'
#define CHAR_LEFT_CURLY_BRACKET     '\173'
#define CHAR_VERTICAL_LINE          '\174'
#define CHAR_RIGHT_CURLY_BRACKET    '\175'
#define CHAR_TILDE                  '\176'
#define CHAR_NBSP                   ((unsigned char)'\xa0')
#define STR_HT                      "\011"
#define STR_VT                      "\013"
#define STR_FF                      "\014"
#define STR_CR                      "\015"
#define STR_NL                      "\012"
#define STR_BS                      "\010"
#define STR_BEL                     "\007"
#define STR_ESC                     "\033"
#define STR_DEL                     "\177"
#define STR_SPACE                   "\040"
#define STR_EXCLAMATION_MARK        "\041"
#define STR_QUOTATION_MARK          "\042"
#define STR_NUMBER_SIGN             "\043"
#define STR_DOLLAR_SIGN             "\044"
#define STR_PERCENT_SIGN            "\045"
#define STR_AMPERSAND               "\046"
#define STR_APOSTROPHE              "\047"
#define STR_LEFT_PARENTHESIS        "\050"
#define STR_RIGHT_PARENTHESIS       "\051"
#define STR_ASTERISK                "\052"
#define STR_PLUS                    "\053"
#define STR_COMMA                   "\054"
#define STR_MINUS                   "\055"
#define STR_DOT                     "\056"
#define STR_SLASH                   "\057"
#define STR_0                       "\060"
#define STR_1                       "\061"
#define STR_2                       "\062"
#define STR_3                       "\063"
#define STR_4                       "\064"
#define STR_5                       "\065"
#define STR_6                       "\066"
#define STR_7                       "\067"
#define STR_8                       "\070"
#define STR_9                       "\071"
#define STR_COLON                   "\072"
#define STR_SEMICOLON               "\073"
#define STR_LESS_THAN_SIGN          "\074"
#define STR_EQUALS_SIGN             "\075"
#define STR_GREATER_THAN_SIGN       "\076"
#define STR_QUESTION_MARK           "\077"
#define STR_COMMERCIAL_AT           "\100"
#define STR_A                       "\101"
#define STR_B                       "\102"
#define STR_C                       "\103"
#define STR_D                       "\104"
#define STR_E                       "\105"
#define STR_F                       "\106"
#define STR_G                       "\107"
#define STR_H                       "\110"
#define STR_I                       "\111"
#define STR_J                       "\112"
#define STR_K                       "\113"
#define STR_L                       "\114"
#define STR_M                       "\115"
#define STR_N                       "\116"
#define STR_O                       "\117"
#define STR_P                       "\120"
#define STR_Q                       "\121"
#define STR_R                       "\122"
#define STR_S                       "\123"
#define STR_T                       "\124"
#define STR_U                       "\125"
#define STR_V                       "\126"
#define STR_W                       "\127"
#define STR_X                       "\130"
#define STR_Y                       "\131"
#define STR_Z                       "\132"
#define STR_LEFT_SQUARE_BRACKET     "\133"
#define STR_BACKSLASH               "\134"
#define STR_RIGHT_SQUARE_BRACKET    "\135"
#define STR_CIRCUMFLEX_ACCENT       "\136"
#define STR_UNDERSCORE              "\137"
#define STR_GRAVE_ACCENT            "\140"
#define STR_a                       "\141"
#define STR_b                       "\142"
#define STR_c                       "\143"
#define STR_d                       "\144"
#define STR_e                       "\145"
#define STR_f                       "\146"
#define STR_g                       "\147"
#define STR_h                       "\150"
#define STR_i                       "\151"
#define STR_j                       "\152"
#define STR_k                       "\153"
#define STR_l                       "\154"
#define STR_m                       "\155"
#define STR_n                       "\156"
#define STR_o                       "\157"
#define STR_p                       "\160"
#define STR_q                       "\161"
#define STR_r                       "\162"
#define STR_s                       "\163"
#define STR_t                       "\164"
#define STR_u                       "\165"
#define STR_v                       "\166"
#define STR_w                       "\167"
#define STR_x                       "\170"
#define STR_y                       "\171"
#define STR_z                       "\172"
#define STR_LEFT_CURLY_BRACKET      "\173"
#define STR_VERTICAL_LINE           "\174"
#define STR_RIGHT_CURLY_BRACKET     "\175"
#define STR_TILDE                   "\176"
#endif
#define STRING_ACCEPT0               STR_A STR_C STR_C STR_E STR_P STR_T "\0"
#define STRING_COMMIT0               STR_C STR_O STR_M STR_M STR_I STR_T "\0"
#define STRING_F0                    STR_F "\0"
#define STRING_FAIL0                 STR_F STR_A STR_I STR_L "\0"
#define STRING_MARK0                 STR_M STR_A STR_R STR_K "\0"
#define STRING_PRUNE0                STR_P STR_R STR_U STR_N STR_E "\0"
#define STRING_SKIP0                 STR_S STR_K STR_I STR_P "\0"
#define STRING_THEN                  STR_T STR_H STR_E STR_N
#define STRING_atomic0               STR_a STR_t STR_o STR_m STR_i STR_c "\0"
#define STRING_pla0                  STR_p STR_l STR_a "\0"
#define STRING_plb0                  STR_p STR_l STR_b "\0"
#define STRING_napla0                STR_n STR_a STR_p STR_l STR_a "\0"
#define STRING_naplb0                STR_n STR_a STR_p STR_l STR_b "\0"
#define STRING_nla0                  STR_n STR_l STR_a "\0"
#define STRING_nlb0                  STR_n STR_l STR_b "\0"
#define STRING_scs0                  STR_s STR_c STR_s "\0"
#define STRING_sr0                   STR_s STR_r "\0"
#define STRING_asr0                  STR_a STR_s STR_r "\0"
#define STRING_positive_lookahead0   STR_p STR_o STR_s STR_i STR_t STR_i STR_v STR_e STR_UNDERSCORE STR_l STR_o STR_o STR_k STR_a STR_h STR_e STR_a STR_d "\0"
#define STRING_positive_lookbehind0  STR_p STR_o STR_s STR_i STR_t STR_i STR_v STR_e STR_UNDERSCORE STR_l STR_o STR_o STR_k STR_b STR_e STR_h STR_i STR_n STR_d "\0"
#define STRING_non_atomic_positive_lookahead0   STR_n STR_o STR_n STR_UNDERSCORE STR_a STR_t STR_o STR_m STR_i STR_c STR_UNDERSCORE STR_p STR_o STR_s STR_i STR_t STR_i STR_v STR_e STR_UNDERSCORE STR_l STR_o STR_o STR_k STR_a STR_h STR_e STR_a STR_d "\0"
#define STRING_non_atomic_positive_lookbehind0  STR_n STR_o STR_n STR_UNDERSCORE STR_a STR_t STR_o STR_m STR_i STR_c STR_UNDERSCORE STR_p STR_o STR_s STR_i STR_t STR_i STR_v STR_e STR_UNDERSCORE STR_l STR_o STR_o STR_k STR_b STR_e STR_h STR_i STR_n STR_d "\0"
#define STRING_negative_lookahead0   STR_n STR_e STR_g STR_a STR_t STR_i STR_v STR_e STR_UNDERSCORE STR_l STR_o STR_o STR_k STR_a STR_h STR_e STR_a STR_d "\0"
#define STRING_negative_lookbehind0  STR_n STR_e STR_g STR_a STR_t STR_i STR_v STR_e STR_UNDERSCORE STR_l STR_o STR_o STR_k STR_b STR_e STR_h STR_i STR_n STR_d "\0"
#define STRING_script_run0           STR_s STR_c STR_r STR_i STR_p STR_t STR_UNDERSCORE STR_r STR_u STR_n "\0"
#define STRING_atomic_script_run     STR_a STR_t STR_o STR_m STR_i STR_c STR_UNDERSCORE STR_s STR_c STR_r STR_i STR_p STR_t STR_UNDERSCORE STR_r STR_u STR_n
#define STRING_scan_substring0       STR_s STR_c STR_a STR_n STR_UNDERSCORE STR_s STR_u STR_b STR_s STR_t STR_r STR_i STR_n STR_g "\0"
#define STRING_alpha0                STR_a STR_l STR_p STR_h STR_a "\0"
#define STRING_lower0                STR_l STR_o STR_w STR_e STR_r "\0"
#define STRING_upper0                STR_u STR_p STR_p STR_e STR_r "\0"
#define STRING_alnum0                STR_a STR_l STR_n STR_u STR_m "\0"
#define STRING_ascii0                STR_a STR_s STR_c STR_i STR_i "\0"
#define STRING_blank0                STR_b STR_l STR_a STR_n STR_k "\0"
#define STRING_cntrl0                STR_c STR_n STR_t STR_r STR_l "\0"
#define STRING_digit0                STR_d STR_i STR_g STR_i STR_t "\0"
#define STRING_graph0                STR_g STR_r STR_a STR_p STR_h "\0"
#define STRING_print0                STR_p STR_r STR_i STR_n STR_t "\0"
#define STRING_punct0                STR_p STR_u STR_n STR_c STR_t "\0"
#define STRING_space0                STR_s STR_p STR_a STR_c STR_e "\0"
#define STRING_word0                 STR_w STR_o STR_r STR_d       "\0"
#define STRING_xdigit                STR_x STR_d STR_i STR_g STR_i STR_t
#define STRING_DEFINE                STR_D STR_E STR_F STR_I STR_N STR_E
#define STRING_VERSION               STR_V STR_E STR_R STR_S STR_I STR_O STR_N
#define STRING_WEIRD_STARTWORD       STR_LEFT_SQUARE_BRACKET STR_COLON STR_LESS_THAN_SIGN STR_COLON STR_RIGHT_SQUARE_BRACKET STR_RIGHT_SQUARE_BRACKET
#define STRING_WEIRD_ENDWORD         STR_LEFT_SQUARE_BRACKET STR_COLON STR_GREATER_THAN_SIGN STR_COLON STR_RIGHT_SQUARE_BRACKET STR_RIGHT_SQUARE_BRACKET
#define STRING_CR_RIGHTPAR                STR_C STR_R STR_RIGHT_PARENTHESIS
#define STRING_LF_RIGHTPAR                STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_CRLF_RIGHTPAR              STR_C STR_R STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_ANY_RIGHTPAR               STR_A STR_N STR_Y STR_RIGHT_PARENTHESIS
#define STRING_ANYCRLF_RIGHTPAR           STR_A STR_N STR_Y STR_C STR_R STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_NUL_RIGHTPAR               STR_N STR_U STR_L STR_RIGHT_PARENTHESIS
#define STRING_BSR_ANYCRLF_RIGHTPAR       STR_B STR_S STR_R STR_UNDERSCORE STR_A STR_N STR_Y STR_C STR_R STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_BSR_UNICODE_RIGHTPAR       STR_B STR_S STR_R STR_UNDERSCORE STR_U STR_N STR_I STR_C STR_O STR_D STR_E STR_RIGHT_PARENTHESIS
#define STRING_UTF8_RIGHTPAR              STR_U STR_T STR_F STR_8 STR_RIGHT_PARENTHESIS
#define STRING_UTF16_RIGHTPAR             STR_U STR_T STR_F STR_1 STR_6 STR_RIGHT_PARENTHESIS
#define STRING_UTF32_RIGHTPAR             STR_U STR_T STR_F STR_3 STR_2 STR_RIGHT_PARENTHESIS
#define STRING_UTF_RIGHTPAR               STR_U STR_T STR_F STR_RIGHT_PARENTHESIS
#define STRING_UCP_RIGHTPAR               STR_U STR_C STR_P STR_RIGHT_PARENTHESIS
#define STRING_NO_AUTO_POSSESS_RIGHTPAR   STR_N STR_O STR_UNDERSCORE STR_A STR_U STR_T STR_O STR_UNDERSCORE STR_P STR_O STR_S STR_S STR_E STR_S STR_S STR_RIGHT_PARENTHESIS
#define STRING_NO_DOTSTAR_ANCHOR_RIGHTPAR STR_N STR_O STR_UNDERSCORE STR_D STR_O STR_T STR_S STR_T STR_A STR_R STR_UNDERSCORE STR_A STR_N STR_C STR_H STR_O STR_R STR_RIGHT_PARENTHESIS
#define STRING_NO_JIT_RIGHTPAR            STR_N STR_O STR_UNDERSCORE STR_J STR_I STR_T STR_RIGHT_PARENTHESIS
#define STRING_NO_START_OPT_RIGHTPAR      STR_N STR_O STR_UNDERSCORE STR_S STR_T STR_A STR_R STR_T STR_UNDERSCORE STR_O STR_P STR_T STR_RIGHT_PARENTHESIS
#define STRING_NOTEMPTY_RIGHTPAR          STR_N STR_O STR_T STR_E STR_M STR_P STR_T STR_Y STR_RIGHT_PARENTHESIS
#define STRING_NOTEMPTY_ATSTART_RIGHTPAR  STR_N STR_O STR_T STR_E STR_M STR_P STR_T STR_Y STR_UNDERSCORE STR_A STR_T STR_S STR_T STR_A STR_R STR_T STR_RIGHT_PARENTHESIS
#define STRING_CASELESS_RESTRICT_RIGHTPAR STR_C STR_A STR_S STR_E STR_L STR_E STR_S STR_S STR_UNDERSCORE STR_R STR_E STR_S STR_T STR_R STR_I STR_C STR_T STR_RIGHT_PARENTHESIS
#define STRING_TURKISH_CASING_RIGHTPAR    STR_T STR_U STR_R STR_K STR_I STR_S STR_H STR_UNDERSCORE STR_C STR_A STR_S STR_I STR_N STR_G STR_RIGHT_PARENTHESIS
#define STRING_LIMIT_HEAP_EQ              STR_L STR_I STR_M STR_I STR_T STR_UNDERSCORE STR_H STR_E STR_A STR_P STR_EQUALS_SIGN
#define STRING_LIMIT_MATCH_EQ             STR_L STR_I STR_M STR_I STR_T STR_UNDERSCORE STR_M STR_A STR_T STR_C STR_H STR_EQUALS_SIGN
#define STRING_LIMIT_DEPTH_EQ             STR_L STR_I STR_M STR_I STR_T STR_UNDERSCORE STR_D STR_E STR_P STR_T STR_H STR_EQUALS_SIGN
#define STRING_LIMIT_RECURSION_EQ         STR_L STR_I STR_M STR_I STR_T STR_UNDERSCORE STR_R STR_E STR_C STR_U STR_R STR_S STR_I STR_O STR_N STR_EQUALS_SIGN
#define STRING_MARK                       STR_M STR_A STR_R STR_K
#define STRING_bc                         STR_b STR_c
#define STRING_bidiclass                  STR_b STR_i STR_d STR_i STR_c STR_l STR_a STR_s STR_s
#define STRING_sc                         STR_s STR_c
#define STRING_script                     STR_s STR_c STR_r STR_i STR_p STR_t
#define STRING_scriptextensions           STR_s STR_c STR_r STR_i STR_p STR_t STR_e STR_x STR_t STR_e STR_n STR_s STR_i STR_o STR_n STR_s
#define STRING_scx                        STR_s STR_c STR_x
#define PT_LAMP       0
#define PT_GC         1
#define PT_PC         2
#define PT_SC         3
#define PT_SCX        4
#define PT_ALNUM      5
#define PT_SPACE      6
#define PT_PXSPACE    7
#define PT_WORD       8
#define PT_CLIST      9
#define PT_UCNC      10
#define PT_BIDICL    11
#define PT_BOOL      12
#define PT_ANY       13
#define PT_TABSIZE PT_ANY
#define PT_PXGRAPH   14
#define PT_PXPRINT   15
#define PT_PXPUNCT   16
#define PT_PXXDIGIT  17
#define PT_NOTSCRIPT 255
#define XCL_NOT      0x01
#define XCL_MAP      0x02
#define XCL_HASPROP  0x04
#define XCL_END      0
#define XCL_SINGLE   1
#define XCL_RANGE    2
#define XCL_PROP     3
#define XCL_NOTPROP  4
#define XCL_LIST     (sizeof(PCRE2_UCHAR) == 1 ? 0x10 : 0x1000)
#define XCL_CHAR_LIST_LOW_16_START 0x100
#define XCL_CHAR_LIST_LOW_16_END 0x7fff
#define XCL_CHAR_LIST_LOW_16_ADD 0x0
#define XCL_CHAR_LIST_HIGH_16_START 0x8000
#define XCL_CHAR_LIST_HIGH_16_END 0xffff
#define XCL_CHAR_LIST_HIGH_16_ADD 0x8000
#define XCL_CHAR_LIST_LOW_32_START 0x10000
#define XCL_CHAR_LIST_LOW_32_END 0x7fffffff
#define XCL_CHAR_LIST_LOW_32_ADD 0x0
#define XCL_CHAR_LIST_HIGH_32_START 0x80000000
#define XCL_CHAR_LIST_HIGH_32_END 0xffffffff
#define XCL_CHAR_LIST_HIGH_32_ADD 0x80000000
#define XCL_TYPE_MASK 0xfff
#define XCL_TYPE_BIT_LEN 3
#define XCL_BEGIN_WITH_RANGE 0x4
#define XCL_ITEM_COUNT_MASK 0x3
#define XCL_CHAR_END 0x1
#define XCL_CHAR_SHIFT 1
#define ECL_MAP     0x01
#define ECL_AND     1
#define ECL_OR      2
#define ECL_XOR     3
#define ECL_NOT     4
#define ECL_XCLASS  5
#define ECL_ANY     6
#define ECL_NONE    7
enum { ESC_A = 1, ESC_G, ESC_K, ESC_B, ESC_b, ESC_D, ESC_d, ESC_S, ESC_s,
       ESC_W, ESC_w, ESC_N, ESC_dum, ESC_C, ESC_P, ESC_p, ESC_R, ESC_H,
       ESC_h, ESC_V, ESC_v, ESC_X, ESC_Z, ESC_z,
       ESC_E, ESC_Q, ESC_g, ESC_k, ESC_ub };
#define FIRST_AUTOTAB_OP       OP_NOT_DIGIT
#define LAST_AUTOTAB_LEFT_OP   OP_EXTUNI
#define LAST_AUTOTAB_RIGHT_OP  OP_DOLLM
enum {
  OP_END,
  OP_SOD,
  OP_SOM,
  OP_SET_SOM,
  OP_NOT_WORD_BOUNDARY,
  OP_WORD_BOUNDARY,
  OP_NOT_DIGIT,
  OP_DIGIT,
  OP_NOT_WHITESPACE,
  OP_WHITESPACE,
  OP_NOT_WORDCHAR,
  OP_WORDCHAR,
  OP_ANY,
  OP_ALLANY,
  OP_ANYBYTE,
  OP_NOTPROP,
  OP_PROP,
  OP_ANYNL,
  OP_NOT_HSPACE,
  OP_HSPACE,
  OP_NOT_VSPACE,
  OP_VSPACE,
  OP_EXTUNI,
  OP_EODN,
  OP_EOD,
  OP_DOLL,
  OP_DOLLM,
  OP_CIRC,
  OP_CIRCM,
  OP_CHAR,
  OP_CHARI,
  OP_NOT,
  OP_NOTI,
  OP_STAR,
  OP_MINSTAR,
  OP_PLUS,
  OP_MINPLUS,
  OP_QUERY,
  OP_MINQUERY,
  OP_UPTO,
  OP_MINUPTO,
  OP_EXACT,
  OP_POSSTAR,
  OP_POSPLUS,
  OP_POSQUERY,
  OP_POSUPTO,
  OP_STARI,
  OP_MINSTARI,
  OP_PLUSI,
  OP_MINPLUSI,
  OP_QUERYI,
  OP_MINQUERYI,
  OP_UPTOI,
  OP_MINUPTOI,
  OP_EXACTI,
  OP_POSSTARI,
  OP_POSPLUSI,
  OP_POSQUERYI,
  OP_POSUPTOI,
  OP_NOTSTAR,
  OP_NOTMINSTAR,
  OP_NOTPLUS,
  OP_NOTMINPLUS,
  OP_NOTQUERY,
  OP_NOTMINQUERY,
  OP_NOTUPTO,
  OP_NOTMINUPTO,
  OP_NOTEXACT,
  OP_NOTPOSSTAR,
  OP_NOTPOSPLUS,
  OP_NOTPOSQUERY,
  OP_NOTPOSUPTO,
  OP_NOTSTARI,
  OP_NOTMINSTARI,
  OP_NOTPLUSI,
  OP_NOTMINPLUSI,
  OP_NOTQUERYI,
  OP_NOTMINQUERYI,
  OP_NOTUPTOI,
  OP_NOTMINUPTOI,
  OP_NOTEXACTI,
  OP_NOTPOSSTARI,
  OP_NOTPOSPLUSI,
  OP_NOTPOSQUERYI,
  OP_NOTPOSUPTOI,
  OP_TYPESTAR,
  OP_TYPEMINSTAR,
  OP_TYPEPLUS,
  OP_TYPEMINPLUS,
  OP_TYPEQUERY,
  OP_TYPEMINQUERY,
  OP_TYPEUPTO,
  OP_TYPEMINUPTO,
  OP_TYPEEXACT,
  OP_TYPEPOSSTAR,
  OP_TYPEPOSPLUS,
  OP_TYPEPOSQUERY,
  OP_TYPEPOSUPTO,
  OP_CRSTAR,
  OP_CRMINSTAR,
  OP_CRPLUS,
  OP_CRMINPLUS,
  OP_CRQUERY,
  OP_CRMINQUERY,
  OP_CRRANGE,
  OP_CRMINRANGE,
  OP_CRPOSSTAR,
  OP_CRPOSPLUS,
  OP_CRPOSQUERY,
  OP_CRPOSRANGE,
  OP_CLASS,
  OP_NCLASS,
  OP_XCLASS,
  OP_ECLASS,
  OP_REF,
  OP_REFI,
  OP_DNREF,
  OP_DNREFI,
  OP_RECURSE,
  OP_CALLOUT,
  OP_CALLOUT_STR,
  OP_ALT,
  OP_KET,
  OP_KETRMAX,
  OP_KETRMIN,
  OP_KETRPOS,
  OP_REVERSE,
  OP_VREVERSE,
  OP_ASSERT,
  OP_ASSERT_NOT,
  OP_ASSERTBACK,
  OP_ASSERTBACK_NOT,
  OP_ASSERT_NA,
  OP_ASSERTBACK_NA,
  OP_ASSERT_SCS,
  OP_ONCE,
  OP_SCRIPT_RUN,
  OP_BRA,
  OP_BRAPOS,
  OP_CBRA,
  OP_CBRAPOS,
  OP_COND,
  OP_SBRA,
  OP_SBRAPOS,
  OP_SCBRA,
  OP_SCBRAPOS,
  OP_SCOND,
  OP_CREF,
  OP_DNCREF,
  OP_RREF,
  OP_DNRREF,
  OP_FALSE,
  OP_TRUE,
  OP_BRAZERO,
  OP_BRAMINZERO,
  OP_BRAPOSZERO,
  OP_MARK,
  OP_PRUNE,
  OP_PRUNE_ARG,
  OP_SKIP,
  OP_SKIP_ARG,
  OP_THEN,
  OP_THEN_ARG,
  OP_COMMIT,
  OP_COMMIT_ARG,
  OP_FAIL,
  OP_ACCEPT,
  OP_ASSERT_ACCEPT,
  OP_CLOSE,
  OP_SKIPZERO,
  OP_DEFINE,
  OP_NOT_UCP_WORD_BOUNDARY,
  OP_UCP_WORD_BOUNDARY,
  OP_TABLE_LENGTH
};
#define OP_NAME_LIST \
  "End", "\\A", "\\G", "\\K", "\\B", "\\b", "\\D", "\\d",         \
  "\\S", "\\s", "\\W", "\\w", "Any", "AllAny", "Anybyte",         \
  "notprop", "prop", "\\R", "\\H", "\\h", "\\V", "\\v",           \
  "extuni",  "\\Z", "\\z",                                        \
  "$", "$", "^", "^", "char", "chari", "not", "noti",             \
  "*", "*?", "+", "+?", "?", "??",                                \
  "{", "{", "{",                                                  \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??",                                \
  "{", "{", "{",                                                  \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??",                                \
  "{", "{", "{",                                                  \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??",                                \
  "{", "{", "{",                                                  \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??", "{", "{", "{",                 \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??", "{", "{",                      \
  "*+","++", "?+", "{",                                           \
  "class", "nclass", "xclass", "eclass",                          \
  "Ref", "Refi", "DnRef", "DnRefi",                               \
  "Recurse", "Callout", "CalloutStr",                             \
  "Alt", "Ket", "KetRmax", "KetRmin", "KetRpos",                  \
  "Reverse", "VReverse", "Assert", "Assert not",                  \
  "Assert back", "Assert back not",                               \
  "Non-atomic assert", "Non-atomic assert back",                  \
  "Scan substring",                                               \
  "Once",                                                         \
  "Script run",                                                   \
  "Bra", "BraPos", "CBra", "CBraPos",                             \
  "Cond",                                                         \
  "SBra", "SBraPos", "SCBra", "SCBraPos",                         \
  "SCond",                                                        \
  "Capture ref", "Capture dnref", "Cond rec", "Cond dnrec",       \
  "Cond false", "Cond true",                                      \
  "Brazero", "Braminzero", "Braposzero",                          \
  "*MARK", "*PRUNE", "*PRUNE", "*SKIP", "*SKIP",                  \
  "*THEN", "*THEN", "*COMMIT", "*COMMIT", "*FAIL",                \
  "*ACCEPT", "*ASSERT_ACCEPT",                                    \
  "Close", "Skip zero", "Define", "\\B (ucp)", "\\b (ucp)"
#define OP_LENGTHS \
  1,                              \
  1, 1, 1, 1, 1,                  \
  1, 1, 1, 1, 1, 1,               \
  1, 1, 1,                        \
  3, 3,                           \
  1, 1, 1, 1, 1,                  \
  1,                              \
  1, 1, 1, 1, 1, 1,               \
  2,                              \
  2,                              \
  2,                              \
  2,                              \
   \
  2, 2, 2, 2, 2, 2,               \
  2+IMM2_SIZE, 2+IMM2_SIZE,       \
  2+IMM2_SIZE,                    \
  2, 2, 2, 2+IMM2_SIZE,           \
  2, 2, 2, 2, 2, 2,               \
  2+IMM2_SIZE, 2+IMM2_SIZE,       \
  2+IMM2_SIZE,                    \
  2, 2, 2, 2+IMM2_SIZE,           \
   \
  2, 2, 2, 2, 2, 2,               \
  2+IMM2_SIZE, 2+IMM2_SIZE,       \
  2+IMM2_SIZE,                    \
  2, 2, 2, 2+IMM2_SIZE,           \
  2, 2, 2, 2, 2, 2,               \
  2+IMM2_SIZE, 2+IMM2_SIZE,       \
  2+IMM2_SIZE,                    \
  2, 2, 2, 2+IMM2_SIZE,           \
   \
  2, 2, 2, 2, 2, 2,               \
  2+IMM2_SIZE, 2+IMM2_SIZE,       \
  2+IMM2_SIZE,                    \
  2, 2, 2, 2+IMM2_SIZE,           \
   \
  1, 1, 1, 1, 1, 1,               \
  1+2*IMM2_SIZE, 1+2*IMM2_SIZE,   \
  1, 1, 1, 1+2*IMM2_SIZE,         \
  1+(32/sizeof(PCRE2_UCHAR)),     \
  1+(32/sizeof(PCRE2_UCHAR)),     \
  0,                              \
  0,                              \
  1+IMM2_SIZE,                    \
  1+IMM2_SIZE+1,                  \
  1+2*IMM2_SIZE,                  \
  1+2*IMM2_SIZE+1,                \
  1+LINK_SIZE,                    \
  1+2*LINK_SIZE+1,                \
  0,                              \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+IMM2_SIZE,                    \
  1+2*IMM2_SIZE,                  \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE+IMM2_SIZE,          \
  1+LINK_SIZE+IMM2_SIZE,          \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE+IMM2_SIZE,          \
  1+LINK_SIZE+IMM2_SIZE,          \
  1+LINK_SIZE,                    \
  1+IMM2_SIZE, 1+2*IMM2_SIZE,     \
  1+IMM2_SIZE, 1+2*IMM2_SIZE,     \
  1, 1,                           \
  1, 1, 1,                        \
  3, 1, 3,                        \
  1, 3,                           \
  1, 3,                           \
  1, 3,                           \
  1, 1, 1,                        \
  1+IMM2_SIZE, 1,                 \
  1,                              \
  1, 1
#define RREF_ANY  0xffff
#define REFI_FLAG_CASELESS_RESTRICT  0x1
#define REFI_FLAG_TURKISH_CASING     0x2
typedef struct pcre2_memctl {
  void *    (*malloc)(size_t, void *);
  void      (*free)(void *, void *);
  void      *memory_data;
} pcre2_memctl;
typedef struct open_capitem {
  struct open_capitem *next;
  uint16_t number;
  uint16_t assert_depth;
} open_capitem;
typedef struct {
  uint16_t name_offset;
  uint16_t type;
  uint16_t value;
} ucp_type_table;
typedef struct {
  uint8_t script;
  uint8_t chartype;
  uint8_t gbprop;
  uint8_t caseset;
  int32_t other_case;
  uint16_t scriptx_bidiclass;
  uint16_t bprops;
} ucd_record;
#define UCD_BLOCK_SIZE 128
#define REAL_GET_UCD(ch) (PRIV(ucd_records) + \
        PRIV(ucd_stage2)[PRIV(ucd_stage1)[(int)(ch) / UCD_BLOCK_SIZE] * \
        UCD_BLOCK_SIZE + (int)(ch) % UCD_BLOCK_SIZE])
#if PCRE2_CODE_UNIT_WIDTH == 32
#define GET_UCD(ch) ((ch > MAX_UTF_CODE_POINT)? \
  PRIV(dummy_ucd_record) : REAL_GET_UCD(ch))
#else
#define GET_UCD(ch) REAL_GET_UCD(ch)
#endif
#define UCD_SCRIPTX_MASK 0x3ff
#define UCD_BIDICLASS_SHIFT 11
#define UCD_BPROPS_MASK 0xfff
#define UCD_SCRIPTX_PROP(prop) ((prop)->scriptx_bidiclass & UCD_SCRIPTX_MASK)
#define UCD_BIDICLASS_PROP(prop) ((prop)->scriptx_bidiclass >> UCD_BIDICLASS_SHIFT)
#define UCD_BPROPS_PROP(prop) ((prop)->bprops & UCD_BPROPS_MASK)
#define UCD_CHARTYPE(ch)    GET_UCD(ch)->chartype
#define UCD_SCRIPT(ch)      GET_UCD(ch)->script
#define UCD_CATEGORY(ch)    PRIV(ucp_gentype)[UCD_CHARTYPE(ch)]
#define UCD_GRAPHBREAK(ch)  GET_UCD(ch)->gbprop
#define UCD_CASESET(ch)     GET_UCD(ch)->caseset
#define UCD_OTHERCASE(ch)   ((uint32_t)((int)ch + (int)(GET_UCD(ch)->other_case)))
#define UCD_SCRIPTX(ch)     UCD_SCRIPTX_PROP(GET_UCD(ch))
#define UCD_BPROPS(ch)      UCD_BPROPS_PROP(GET_UCD(ch))
#define UCD_BIDICLASS(ch)   UCD_BIDICLASS_PROP(GET_UCD(ch))
#define UCD_ANY_I(ch) \
   \
  (((uint32_t)(ch) | 0x20u) == 0x69u || ((uint32_t)(ch) | 1u) == 0x0131u)
#define UCD_DOTTED_I(ch) \
  ((uint32_t)(ch) == 0x69u || (uint32_t)(ch) == 0x0130u)
#define UCD_FOLD_I_TURKISH(ch) \
  ((uint32_t)(ch) == 0x0130u ?   0x69u : \
   (uint32_t)(ch) ==   0x49u ? 0x0131u : (uint32_t)(ch))
#define MAPBIT(map,n) ((map)[(n)/32]&(1u<<((n)%32)))
#define MAPSET(map,n) ((map)[(n)/32]|=(1u<<((n)%32)))
typedef struct pcre2_serialized_data {
  uint32_t magic;
  uint32_t version;
  uint32_t config;
  int32_t  number_of_codes;
} pcre2_serialized_data;
#if defined PCRE2_CODE_UNIT_WIDTH && PCRE2_CODE_UNIT_WIDTH != 0
#if defined EBCDIC && PCRE2_CODE_UNIT_WIDTH != 8
#error EBCDIC is not supported for the 16-bit or 32-bit libraries
#endif
#define MAX_NON_UTF_CHAR (0xffffffffU >> (32 - PCRE2_CODE_UNIT_WIDTH))
#if PCRE2_CODE_UNIT_WIDTH == 8
extern const int              PRIV(utf8_table1)[];
extern const unsigned         PRIV(utf8_table1_size);
extern const int              PRIV(utf8_table2)[];
extern const int              PRIV(utf8_table3)[];
extern const uint8_t          PRIV(utf8_table4)[];
#endif
#define _pcre2_OP_lengths              PCRE2_SUFFIX(_pcre2_OP_lengths_)
#define _pcre2_callout_end_delims      PCRE2_SUFFIX(_pcre2_callout_end_delims_)
#define _pcre2_callout_start_delims    PCRE2_SUFFIX(_pcre2_callout_start_delims_)
#define _pcre2_default_compile_context PCRE2_SUFFIX(_pcre2_default_compile_context_)
#define _pcre2_default_convert_context PCRE2_SUFFIX(_pcre2_default_convert_context_)
#define _pcre2_default_match_context   PCRE2_SUFFIX(_pcre2_default_match_context_)
#define _pcre2_default_tables          PCRE2_SUFFIX(_pcre2_default_tables_)
#if PCRE2_CODE_UNIT_WIDTH == 32
#define _pcre2_dummy_ucd_record        PCRE2_SUFFIX(_pcre2_dummy_ucd_record_)
#endif
#define _pcre2_hspace_list             PCRE2_SUFFIX(_pcre2_hspace_list_)
#define _pcre2_vspace_list             PCRE2_SUFFIX(_pcre2_vspace_list_)
#define _pcre2_ucd_boolprop_sets       PCRE2_SUFFIX(_pcre2_ucd_boolprop_sets_)
#define _pcre2_ucd_caseless_sets       PCRE2_SUFFIX(_pcre2_ucd_caseless_sets_)
#define _pcre2_ucd_turkish_dotted_i_caseset  PCRE2_SUFFIX(_pcre2_ucd_turkish_dotted_i_caseset_)
#define _pcre2_ucd_nocase_ranges       PCRE2_SUFFIX(_pcre2_ucd_nocase_ranges_)
#define _pcre2_ucd_nocase_ranges_size  PCRE2_SUFFIX(_pcre2_ucd_nocase_ranges_size_)
#define _pcre2_ucd_digit_sets          PCRE2_SUFFIX(_pcre2_ucd_digit_sets_)
#define _pcre2_ucd_script_sets         PCRE2_SUFFIX(_pcre2_ucd_script_sets_)
#define _pcre2_ucd_records             PCRE2_SUFFIX(_pcre2_ucd_records_)
#define _pcre2_ucd_stage1              PCRE2_SUFFIX(_pcre2_ucd_stage1_)
#define _pcre2_ucd_stage2              PCRE2_SUFFIX(_pcre2_ucd_stage2_)
#define _pcre2_ucp_gbtable             PCRE2_SUFFIX(_pcre2_ucp_gbtable_)
#define _pcre2_ucp_gentype             PCRE2_SUFFIX(_pcre2_ucp_gentype_)
#define _pcre2_ucp_typerange           PCRE2_SUFFIX(_pcre2_ucp_typerange_)
#define _pcre2_unicode_version         PCRE2_SUFFIX(_pcre2_unicode_version_)
#define _pcre2_utt                     PCRE2_SUFFIX(_pcre2_utt_)
#define _pcre2_utt_names               PCRE2_SUFFIX(_pcre2_utt_names_)
#define _pcre2_utt_size                PCRE2_SUFFIX(_pcre2_utt_size_)
#define _pcre2_ebcdic_1047_to_ascii    PCRE2_SUFFIX(_pcre2_ebcdic_1047_to_ascii_)
#define _pcre2_ascii_to_ebcdic_1047    PCRE2_SUFFIX(_pcre2_ascii_to_ebcdic_1047_)
extern const uint8_t                   PRIV(OP_lengths)[];
extern const uint32_t                  PRIV(callout_end_delims)[];
extern const uint32_t                  PRIV(callout_start_delims)[];
extern pcre2_compile_context           PRIV(default_compile_context);
extern pcre2_convert_context           PRIV(default_convert_context);
extern pcre2_match_context             PRIV(default_match_context);
extern const uint8_t                   PRIV(default_tables)[];
extern const uint32_t                  PRIV(hspace_list)[];
extern const uint32_t                  PRIV(vspace_list)[];
extern const uint32_t                  PRIV(ucd_boolprop_sets)[];
extern const uint32_t                  PRIV(ucd_caseless_sets)[];
extern const uint32_t                  PRIV(ucd_turkish_dotted_i_caseset);
extern const uint32_t                  PRIV(ucd_nocase_ranges)[];
extern const uint32_t                  PRIV(ucd_nocase_ranges_size);
extern const uint32_t                  PRIV(ucd_digit_sets)[];
extern const uint32_t                  PRIV(ucd_script_sets)[];
extern const ucd_record                PRIV(ucd_records)[];
#if PCRE2_CODE_UNIT_WIDTH == 32
extern const ucd_record                PRIV(dummy_ucd_record)[];
#endif
extern const uint16_t                  PRIV(ucd_stage1)[];
extern const uint16_t                  PRIV(ucd_stage2)[];
extern const uint32_t                  PRIV(ucp_gbtable)[];
extern const uint32_t                  PRIV(ucp_gentype)[];
#ifdef SUPPORT_JIT
extern const int                       PRIV(ucp_typerange)[];
#endif
extern const char                     *PRIV(unicode_version);
extern const ucp_type_table            PRIV(utt)[];
extern const char                      PRIV(utt_names)[];
extern const size_t                    PRIV(utt_size);
extern const uint8_t                   PRIV(ebcdic_1047_to_ascii)[];
extern const uint8_t                   PRIV(ascii_to_ebcdic_1047)[];
#define branch_chain                 PCRE2_SUFFIX(branch_chain_)
#define compile_block                PCRE2_SUFFIX(compile_block_)
#define dfa_match_block              PCRE2_SUFFIX(dfa_match_block_)
#define match_block                  PCRE2_SUFFIX(match_block_)
#define named_group                  PCRE2_SUFFIX(named_group_)
#include "pcre2_intmodedep.h"
#define _pcre2_auto_possessify       PCRE2_SUFFIX(_pcre2_auto_possessify_)
#define _pcre2_check_escape          PCRE2_SUFFIX(_pcre2_check_escape_)
#define _pcre2_ckd_smul              PCRE2_SUFFIX(_pcre2_ckd_smul_)
#define _pcre2_extuni                PCRE2_SUFFIX(_pcre2_extuni_)
#define _pcre2_find_bracket          PCRE2_SUFFIX(_pcre2_find_bracket_)
#define _pcre2_is_newline            PCRE2_SUFFIX(_pcre2_is_newline_)
#define _pcre2_jit_free_rodata       PCRE2_SUFFIX(_pcre2_jit_free_rodata_)
#define _pcre2_jit_free              PCRE2_SUFFIX(_pcre2_jit_free_)
#define _pcre2_jit_get_size          PCRE2_SUFFIX(_pcre2_jit_get_size_)
#define _pcre2_jit_get_target        PCRE2_SUFFIX(_pcre2_jit_get_target_)
#define _pcre2_memctl_malloc         PCRE2_SUFFIX(_pcre2_memctl_malloc_)
#define _pcre2_ord2utf               PCRE2_SUFFIX(_pcre2_ord2utf_)
#define _pcre2_script_run            PCRE2_SUFFIX(_pcre2_script_run_)
#define _pcre2_strcmp                PCRE2_SUFFIX(_pcre2_strcmp_)
#define _pcre2_strcmp_c8             PCRE2_SUFFIX(_pcre2_strcmp_c8_)
#define _pcre2_strcpy_c8             PCRE2_SUFFIX(_pcre2_strcpy_c8_)
#define _pcre2_strlen                PCRE2_SUFFIX(_pcre2_strlen_)
#define _pcre2_strncmp               PCRE2_SUFFIX(_pcre2_strncmp_)
#define _pcre2_strncmp_c8            PCRE2_SUFFIX(_pcre2_strncmp_c8_)
#define _pcre2_study                 PCRE2_SUFFIX(_pcre2_study_)
#define _pcre2_valid_utf             PCRE2_SUFFIX(_pcre2_valid_utf_)
#define _pcre2_was_newline           PCRE2_SUFFIX(_pcre2_was_newline_)
#define _pcre2_xclass                PCRE2_SUFFIX(_pcre2_xclass_)
#define _pcre2_eclass                PCRE2_SUFFIX(_pcre2_eclass_)
extern int          _pcre2_auto_possessify(PCRE2_UCHAR *,
                      const compile_block *);
extern int          _pcre2_check_escape(PCRE2_SPTR *, PCRE2_SPTR, uint32_t *,
                      int *, uint32_t, uint32_t, uint32_t, BOOL, compile_block *);
extern BOOL         _pcre2_ckd_smul(PCRE2_SIZE *, int, int);
extern PCRE2_SPTR   _pcre2_extuni(uint32_t, PCRE2_SPTR, PCRE2_SPTR, PCRE2_SPTR,
                      BOOL, int *);
extern PCRE2_SPTR   _pcre2_find_bracket(PCRE2_SPTR, BOOL, int);
extern BOOL         _pcre2_is_newline(PCRE2_SPTR, uint32_t, PCRE2_SPTR,
                      uint32_t *, BOOL);
extern void         _pcre2_jit_free_rodata(void *, void *);
extern void         _pcre2_jit_free(void *, pcre2_memctl *);
extern size_t       _pcre2_jit_get_size(void *);
const char *        _pcre2_jit_get_target(void);
extern void *       _pcre2_memctl_malloc(size_t, pcre2_memctl *);
extern unsigned int _pcre2_ord2utf(uint32_t, PCRE2_UCHAR *);
extern BOOL         _pcre2_script_run(PCRE2_SPTR, PCRE2_SPTR, BOOL);
extern int          _pcre2_strcmp(PCRE2_SPTR, PCRE2_SPTR);
extern int          _pcre2_strcmp_c8(PCRE2_SPTR, const char *);
extern PCRE2_SIZE   _pcre2_strcpy_c8(PCRE2_UCHAR *, const char *);
extern PCRE2_SIZE   _pcre2_strlen(PCRE2_SPTR);
extern int          _pcre2_strncmp(PCRE2_SPTR, PCRE2_SPTR, size_t);
extern int          _pcre2_strncmp_c8(PCRE2_SPTR, const char *, size_t);
extern int          _pcre2_study(pcre2_real_code *);
extern int          _pcre2_valid_utf(PCRE2_SPTR, PCRE2_SIZE, PCRE2_SIZE *);
extern BOOL         _pcre2_was_newline(PCRE2_SPTR, uint32_t, PCRE2_SPTR,
                      uint32_t *, BOOL);
extern BOOL         _pcre2_xclass(uint32_t, PCRE2_SPTR, const uint8_t *, BOOL);
extern BOOL         _pcre2_eclass(uint32_t, PCRE2_SPTR, PCRE2_SPTR,
                      const uint8_t *, BOOL);
#endif
#include "pcre2_util.h"
#endif
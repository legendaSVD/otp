#include "pcre2_internal.h"
#define STRING(a)  # a
#define XSTRING(s) STRING(s)
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_config(uint32_t what, void *where)
{
if (where == NULL)
  {
  switch (what)
    {
    default:
    return PCRE2_ERROR_BADOPTION;
    case PCRE2_CONFIG_BSR:
    case PCRE2_CONFIG_COMPILED_WIDTHS:
    case PCRE2_CONFIG_DEPTHLIMIT:
    case PCRE2_CONFIG_EFFECTIVE_LINKSIZE:
    case PCRE2_CONFIG_HEAPLIMIT:
    case PCRE2_CONFIG_JIT:
    case PCRE2_CONFIG_LINKSIZE:
    case PCRE2_CONFIG_MATCHLIMIT:
    case PCRE2_CONFIG_NEVER_BACKSLASH_C:
    case PCRE2_CONFIG_NEWLINE:
    case PCRE2_CONFIG_PARENSLIMIT:
    case PCRE2_CONFIG_STACKRECURSE:
    case PCRE2_CONFIG_TABLES_LENGTH:
    case PCRE2_CONFIG_UNICODE:
    return sizeof(uint32_t);
    case PCRE2_CONFIG_JITTARGET:
    case PCRE2_CONFIG_UNICODE_VERSION:
    case PCRE2_CONFIG_VERSION:
    break;
    }
  }
switch (what)
  {
  default:
  return PCRE2_ERROR_BADOPTION;
  case PCRE2_CONFIG_BSR:
#ifdef BSR_ANYCRLF
  *((uint32_t *)where) = PCRE2_BSR_ANYCRLF;
#else
  *((uint32_t *)where) = PCRE2_BSR_UNICODE;
#endif
  break;
  case PCRE2_CONFIG_COMPILED_WIDTHS:
  *((uint32_t *)where) = 0
#ifdef SUPPORT_PCRE2_8
  + (1 << 0)
#endif
#ifdef SUPPORT_PCRE2_16
  + (1 << 1)
#endif
#ifdef SUPPORT_PCRE2_32
  + (1 << 2)
#endif
  ;
  break;
  case PCRE2_CONFIG_DEPTHLIMIT:
  *((uint32_t *)where) = MATCH_LIMIT_DEPTH;
  break;
  case PCRE2_CONFIG_EFFECTIVE_LINKSIZE:
  *((uint32_t *)where) = LINK_SIZE * sizeof(PCRE2_UCHAR);
  break;
  case PCRE2_CONFIG_HEAPLIMIT:
  *((uint32_t *)where) = HEAP_LIMIT;
  break;
  case PCRE2_CONFIG_JIT:
#ifdef SUPPORT_JIT
  *((uint32_t *)where) = 1;
#else
  *((uint32_t *)where) = 0;
#endif
  break;
  case PCRE2_CONFIG_JITTARGET:
#ifdef SUPPORT_JIT
    {
    const char *v = PRIV(jit_get_target)();
    return (int)(1 + ((where == NULL)?
      strlen(v) : PRIV(strcpy_c8)((PCRE2_UCHAR *)where, v)));
    }
#else
  return PCRE2_ERROR_BADOPTION;
#endif
  case PCRE2_CONFIG_LINKSIZE:
  *((uint32_t *)where) = (uint32_t)CONFIGURED_LINK_SIZE;
  break;
  case PCRE2_CONFIG_MATCHLIMIT:
  *((uint32_t *)where) = MATCH_LIMIT;
  break;
  case PCRE2_CONFIG_NEWLINE:
  *((uint32_t *)where) = NEWLINE_DEFAULT;
  break;
  case PCRE2_CONFIG_NEVER_BACKSLASH_C:
#ifdef NEVER_BACKSLASH_C
  *((uint32_t *)where) = 1;
#else
  *((uint32_t *)where) = 0;
#endif
  break;
  case PCRE2_CONFIG_PARENSLIMIT:
  *((uint32_t *)where) = PARENS_NEST_LIMIT;
  break;
  case PCRE2_CONFIG_STACKRECURSE:
  *((uint32_t *)where) = 0;
  break;
  case PCRE2_CONFIG_TABLES_LENGTH:
  *((uint32_t *)where) = TABLES_LENGTH;
  break;
  case PCRE2_CONFIG_UNICODE_VERSION:
    {
#if defined SUPPORT_UNICODE
    const char *v = PRIV(unicode_version);
#else
    const char *v = "Unicode not supported";
#endif
    return (int)(1 + ((where == NULL)?
      strlen(v) : PRIV(strcpy_c8)((PCRE2_UCHAR *)where, v)));
    }
  case PCRE2_CONFIG_UNICODE:
#if defined SUPPORT_UNICODE
  *((uint32_t *)where) = 1;
#else
  *((uint32_t *)where) = 0;
#endif
  break;
  case PCRE2_CONFIG_VERSION:
    {
    const char *v = (XSTRING(Z PCRE2_PRERELEASE)[1] == 0)?
      XSTRING(PCRE2_MAJOR.PCRE2_MINOR PCRE2_DATE) :
      XSTRING(PCRE2_MAJOR.PCRE2_MINOR) XSTRING(PCRE2_PRERELEASE PCRE2_DATE);
    return (int)(1 + ((where == NULL)?
      strlen(v) : PRIV(strcpy_c8)((PCRE2_UCHAR *)where, v)));
    }
  }
return 0;
}
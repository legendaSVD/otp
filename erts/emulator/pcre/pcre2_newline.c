#include "pcre2_internal.h"
BOOL
PRIV(is_newline)(PCRE2_SPTR ptr, uint32_t type, PCRE2_SPTR endptr,
  uint32_t *lenptr, BOOL utf)
{
uint32_t c;
#ifdef SUPPORT_UNICODE
if (utf) { GETCHAR(c, ptr); } else c = *ptr;
#else
(void)utf;
c = *ptr;
#endif
if (type == NLTYPE_ANYCRLF) switch(c)
  {
  case CHAR_LF:
  *lenptr = 1;
  return TRUE;
  case CHAR_CR:
  *lenptr = (ptr < endptr - 1 && ptr[1] == CHAR_LF)? 2 : 1;
  return TRUE;
  default:
  return FALSE;
  }
else switch(c)
  {
#ifdef EBCDIC
  case CHAR_NEL:
#endif
  case CHAR_LF:
  case CHAR_VT:
  case CHAR_FF:
  *lenptr = 1;
  return TRUE;
  case CHAR_CR:
  *lenptr = (ptr < endptr - 1 && ptr[1] == CHAR_LF)? 2 : 1;
  return TRUE;
#ifndef EBCDIC
#if PCRE2_CODE_UNIT_WIDTH == 8
  case CHAR_NEL:
  *lenptr = utf? 2 : 1;
  return TRUE;
  case 0x2028:
  case 0x2029:
  *lenptr = 3;
  return TRUE;
#else
  case CHAR_NEL:
  case 0x2028:
  case 0x2029:
  *lenptr = 1;
  return TRUE;
#endif
#endif
  default:
  return FALSE;
  }
}
BOOL
PRIV(was_newline)(PCRE2_SPTR ptr, uint32_t type, PCRE2_SPTR startptr,
  uint32_t *lenptr, BOOL utf)
{
uint32_t c;
ptr--;
#ifdef SUPPORT_UNICODE
if (utf)
  {
  BACKCHAR(ptr);
  GETCHAR(c, ptr);
  }
else c = *ptr;
#else
(void)utf;
c = *ptr;
#endif
if (type == NLTYPE_ANYCRLF) switch(c)
  {
  case CHAR_LF:
  *lenptr = (ptr > startptr && ptr[-1] == CHAR_CR)? 2 : 1;
  return TRUE;
  case CHAR_CR:
  *lenptr = 1;
  return TRUE;
  default:
  return FALSE;
  }
else switch(c)
  {
  case CHAR_LF:
  *lenptr = (ptr > startptr && ptr[-1] == CHAR_CR)? 2 : 1;
  return TRUE;
#ifdef EBCDIC
  case CHAR_NEL:
#endif
  case CHAR_VT:
  case CHAR_FF:
  case CHAR_CR:
  *lenptr = 1;
  return TRUE;
#ifndef EBCDIC
#if PCRE2_CODE_UNIT_WIDTH == 8
  case CHAR_NEL:
  *lenptr = utf? 2 : 1;
  return TRUE;
  case 0x2028:
  case 0x2029:
  *lenptr = 3;
  return TRUE;
#else
  case CHAR_NEL:
  case 0x2028:
  case 0x2029:
  *lenptr = 1;
  return TRUE;
#endif
#endif
  default:
  return FALSE;
  }
}
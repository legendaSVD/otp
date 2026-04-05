#include "pcre2_internal.h"
#ifndef SUPPORT_UNICODE
unsigned int
PRIV(ord2utf)(uint32_t cvalue, PCRE2_UCHAR *buffer)
{
(void)(cvalue);
(void)(buffer);
return 0;
}
#else
unsigned int
PRIV(ord2utf)(uint32_t cvalue, PCRE2_UCHAR *buffer)
{
#if PCRE2_CODE_UNIT_WIDTH == 8
unsigned int i;
for (i = 0; i < PRIV(utf8_table1_size); i++)
  if ((int)cvalue <= PRIV(utf8_table1)[i]) break;
buffer += i;
for (unsigned int j = i; j != 0; j--)
 {
 *buffer-- = 0x80 | (cvalue & 0x3f);
 cvalue >>= 6;
 }
*buffer = (PCRE2_UCHAR)(PRIV(utf8_table2)[i] | (int)cvalue);
return i + 1;
#elif PCRE2_CODE_UNIT_WIDTH == 16
if (cvalue <= 0xffff)
  {
  *buffer = (PCRE2_UCHAR)cvalue;
  return 1;
  }
cvalue -= 0x10000;
*buffer++ = 0xd800 | (cvalue >> 10);
*buffer = 0xdc00 | (cvalue & 0x3ff);
return 2;
#else
*buffer = (PCRE2_UCHAR)cvalue;
return 1;
#endif
}
#endif
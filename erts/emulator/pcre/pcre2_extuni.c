#include "pcre2_internal.h"
#ifndef SUPPORT_UNICODE
PCRE2_SPTR
PRIV(extuni)(uint32_t c, PCRE2_SPTR eptr, PCRE2_SPTR start_subject,
  PCRE2_SPTR end_subject, BOOL utf, int *xcount)
{
(void)c;
(void)eptr;
(void)start_subject;
(void)end_subject;
(void)utf;
(void)xcount;
return NULL;
}
#else
PCRE2_SPTR
PRIV(extuni)(uint32_t c, PCRE2_SPTR eptr, PCRE2_SPTR start_subject,
  PCRE2_SPTR end_subject, BOOL utf, int *xcount)
{
BOOL was_ep_ZWJ = FALSE;
int lgb = UCD_GRAPHBREAK(c);
while (eptr < end_subject)
  {
  int rgb;
  int len = 1;
  if (!utf) c = *eptr; else { GETCHARLEN(c, eptr, len); }
  rgb = UCD_GRAPHBREAK(c);
  if ((PRIV(ucp_gbtable)[lgb] & (1u << rgb)) == 0) break;
  if (lgb == ucp_gbZWJ && rgb == ucp_gbExtended_Pictographic && !was_ep_ZWJ)
    break;
  if (lgb == ucp_gbRegional_Indicator && rgb == ucp_gbRegional_Indicator)
    {
    int ricount = 0;
    PCRE2_SPTR bptr = eptr - 1;
    if (utf) BACKCHAR(bptr);
    while (bptr > start_subject)
      {
      bptr--;
      if (utf)
        {
        BACKCHAR(bptr);
        GETCHAR(c, bptr);
        }
      else
      c = *bptr;
      if (UCD_GRAPHBREAK(c) != ucp_gbRegional_Indicator) break;
      ricount++;
      }
    if ((ricount & 1) != 0) break;
    }
  was_ep_ZWJ = (lgb == ucp_gbExtended_Pictographic && rgb == ucp_gbZWJ);
  if (rgb != ucp_gbExtend || lgb != ucp_gbExtended_Pictographic) lgb = rgb;
  eptr += len;
  if (xcount != NULL) *xcount += 1;
  }
return eptr;
}
#endif
#include "pcre2_internal.h"
#define MAX_CACHE_BACKREF 128
#define SET_BIT(c) re->start_bitmap[(c)/8] |= (1u << ((c)&7))
enum { SSB_FAIL, SSB_DONE, SSB_CONTINUE, SSB_UNKNOWN, SSB_TOODEEP };
static int
find_minlength(const pcre2_real_code *re, PCRE2_SPTR code,
  PCRE2_SPTR startcode, BOOL utf, recurse_check *recurses, int *countptr,
  int *backref_cache)
{
int length = -1;
int branchlength = 0;
int prev_cap_recno = -1;
int prev_cap_d = 0;
int prev_recurse_recno = -1;
int prev_recurse_d = 0;
uint32_t once_fudge = 0;
BOOL had_recurse = FALSE;
BOOL dupcapused = (re->flags & PCRE2_DUPCAPUSED) != 0;
PCRE2_SPTR nextbranch = code + GET(code, 1);
PCRE2_SPTR cc = code + 1 + LINK_SIZE;
recurse_check this_recurse;
if (*code >= OP_SBRA && *code <= OP_SCOND) return 0;
if (*code == OP_CBRA || *code == OP_CBRAPOS) cc += IMM2_SIZE;
if ((*countptr)++ > 1000) return -1;
for (;;)
  {
  int d, min, recno;
  PCRE2_UCHAR op;
  PCRE2_SPTR cs, ce;
  if (branchlength >= (int)UINT16_MAX)
    {
    branchlength = UINT16_MAX;
    cc = nextbranch;
    }
  op = *cc;
  switch (op)
    {
    case OP_COND:
    case OP_SCOND:
    cs = cc + GET(cc, 1);
    if (*cs != OP_ALT)
      {
      cc = cs + 1 + LINK_SIZE;
      break;
      }
    goto PROCESS_NON_CAPTURE;
    case OP_BRA:
    if (cc[1+LINK_SIZE] == OP_RECURSE && cc[2*(1+LINK_SIZE)] == OP_KET)
      {
      once_fudge = 1 + LINK_SIZE;
      cc += 1 + LINK_SIZE;
      break;
      }
    PCRE2_FALLTHROUGH
    case OP_ONCE:
    case OP_SCRIPT_RUN:
    case OP_SBRA:
    case OP_BRAPOS:
    case OP_SBRAPOS:
    PROCESS_NON_CAPTURE:
    d = find_minlength(re, cc, startcode, utf, recurses, countptr,
      backref_cache);
    if (d < 0) return d;
    branchlength += d;
    do cc += GET(cc, 1); while (*cc == OP_ALT);
    cc += 1 + LINK_SIZE;
    break;
    case OP_CBRA:
    case OP_SCBRA:
    case OP_CBRAPOS:
    case OP_SCBRAPOS:
    recno = (int)GET2(cc, 1+LINK_SIZE);
    if (dupcapused || recno != prev_cap_recno)
      {
      prev_cap_recno = recno;
      prev_cap_d = find_minlength(re, cc, startcode, utf, recurses, countptr,
        backref_cache);
      if (prev_cap_d < 0) return prev_cap_d;
      }
    branchlength += prev_cap_d;
    do cc += GET(cc, 1); while (*cc == OP_ALT);
    cc += 1 + LINK_SIZE;
    break;
    case OP_ACCEPT:
    case OP_ASSERT_ACCEPT:
    return -1;
    case OP_ALT:
    case OP_KET:
    case OP_KETRMAX:
    case OP_KETRMIN:
    case OP_KETRPOS:
    case OP_END:
    if (length < 0 || (!had_recurse && branchlength < length))
      length = branchlength;
    if (op != OP_ALT || length == 0) return length;
    nextbranch = cc + GET(cc, 1);
    cc += 1 + LINK_SIZE;
    branchlength = 0;
    had_recurse = FALSE;
    break;
    case OP_ASSERT:
    case OP_ASSERT_NOT:
    case OP_ASSERTBACK:
    case OP_ASSERTBACK_NOT:
    case OP_ASSERT_NA:
    case OP_ASSERT_SCS:
    case OP_ASSERTBACK_NA:
    do cc += GET(cc, 1); while (*cc == OP_ALT);
    PCRE2_FALLTHROUGH
    case OP_REVERSE:
    case OP_VREVERSE:
    case OP_CREF:
    case OP_DNCREF:
    case OP_RREF:
    case OP_DNRREF:
    case OP_FALSE:
    case OP_TRUE:
    case OP_CALLOUT:
    case OP_SOD:
    case OP_SOM:
    case OP_EOD:
    case OP_EODN:
    case OP_CIRC:
    case OP_CIRCM:
    case OP_DOLL:
    case OP_DOLLM:
    case OP_NOT_WORD_BOUNDARY:
    case OP_WORD_BOUNDARY:
    case OP_NOT_UCP_WORD_BOUNDARY:
    case OP_UCP_WORD_BOUNDARY:
    cc += PRIV(OP_lengths)[*cc];
    break;
    case OP_CALLOUT_STR:
    cc += GET(cc, 1 + 2*LINK_SIZE);
    break;
    case OP_BRAZERO:
    case OP_BRAMINZERO:
    case OP_BRAPOSZERO:
    case OP_SKIPZERO:
    cc += PRIV(OP_lengths)[*cc];
    do cc += GET(cc, 1); while (*cc == OP_ALT);
    cc += 1 + LINK_SIZE;
    break;
    case OP_CHAR:
    case OP_CHARI:
    case OP_NOT:
    case OP_NOTI:
    case OP_PLUS:
    case OP_PLUSI:
    case OP_MINPLUS:
    case OP_MINPLUSI:
    case OP_POSPLUS:
    case OP_POSPLUSI:
    case OP_NOTPLUS:
    case OP_NOTPLUSI:
    case OP_NOTMINPLUS:
    case OP_NOTMINPLUSI:
    case OP_NOTPOSPLUS:
    case OP_NOTPOSPLUSI:
    branchlength++;
    cc += 2;
#ifdef SUPPORT_UNICODE
    if (utf && HAS_EXTRALEN(cc[-1])) cc += GET_EXTRALEN(cc[-1]);
#endif
    break;
    case OP_TYPEPLUS:
    case OP_TYPEMINPLUS:
    case OP_TYPEPOSPLUS:
    branchlength++;
    cc += (cc[1] == OP_PROP || cc[1] == OP_NOTPROP)? 4 : 2;
    break;
    case OP_EXACT:
    case OP_EXACTI:
    case OP_NOTEXACT:
    case OP_NOTEXACTI:
    branchlength += GET2(cc,1);
    cc += 2 + IMM2_SIZE;
#ifdef SUPPORT_UNICODE
    if (utf && HAS_EXTRALEN(cc[-1])) cc += GET_EXTRALEN(cc[-1]);
#endif
    break;
    case OP_TYPEEXACT:
    branchlength += GET2(cc,1);
    cc += 2 + IMM2_SIZE + ((cc[1 + IMM2_SIZE] == OP_PROP
      || cc[1 + IMM2_SIZE] == OP_NOTPROP)? 2 : 0);
    break;
    case OP_PROP:
    case OP_NOTPROP:
    cc += 2;
    PCRE2_FALLTHROUGH
    case OP_NOT_DIGIT:
    case OP_DIGIT:
    case OP_NOT_WHITESPACE:
    case OP_WHITESPACE:
    case OP_NOT_WORDCHAR:
    case OP_WORDCHAR:
    case OP_ANY:
    case OP_ALLANY:
    case OP_EXTUNI:
    case OP_HSPACE:
    case OP_NOT_HSPACE:
    case OP_VSPACE:
    case OP_NOT_VSPACE:
    branchlength++;
    cc++;
    break;
    case OP_ANYNL:
    branchlength += 1;
    cc++;
    break;
    case OP_ANYBYTE:
#ifdef SUPPORT_UNICODE
    if (utf) return -1;
#endif
    branchlength++;
    cc++;
    break;
    case OP_TYPESTAR:
    case OP_TYPEMINSTAR:
    case OP_TYPEQUERY:
    case OP_TYPEMINQUERY:
    case OP_TYPEPOSSTAR:
    case OP_TYPEPOSQUERY:
    if (cc[1] == OP_PROP || cc[1] == OP_NOTPROP) cc += 2;
    cc += PRIV(OP_lengths)[op];
    break;
    case OP_TYPEUPTO:
    case OP_TYPEMINUPTO:
    case OP_TYPEPOSUPTO:
    if (cc[1 + IMM2_SIZE] == OP_PROP
      || cc[1 + IMM2_SIZE] == OP_NOTPROP) cc += 2;
    cc += PRIV(OP_lengths)[op];
    break;
    case OP_CLASS:
    case OP_NCLASS:
#ifdef SUPPORT_WIDE_CHARS
    case OP_XCLASS:
    case OP_ECLASS:
    if (op == OP_XCLASS || op == OP_ECLASS)
      cc += GET(cc, 1);
    else
#endif
      cc += PRIV(OP_lengths)[OP_CLASS];
    switch (*cc)
      {
      case OP_CRPLUS:
      case OP_CRMINPLUS:
      case OP_CRPOSPLUS:
      branchlength++;
      PCRE2_FALLTHROUGH
      case OP_CRSTAR:
      case OP_CRMINSTAR:
      case OP_CRQUERY:
      case OP_CRMINQUERY:
      case OP_CRPOSSTAR:
      case OP_CRPOSQUERY:
      cc++;
      break;
      case OP_CRRANGE:
      case OP_CRMINRANGE:
      case OP_CRPOSRANGE:
      branchlength += GET2(cc,1);
      cc += 1 + 2 * IMM2_SIZE;
      break;
      default:
      branchlength++;
      break;
      }
    break;
    case OP_DNREF:
    case OP_DNREFI:
    if (!dupcapused && (re->overall_options & PCRE2_MATCH_UNSET_BACKREF) == 0)
      {
      int count = GET2(cc, 1+IMM2_SIZE);
      PCRE2_SPTR slot =
        (PCRE2_SPTR)((const uint8_t *)re + sizeof(pcre2_real_code)) +
          GET2(cc, 1) * re->name_entry_size;
      d = INT_MAX;
      while (count-- > 0)
        {
        int dd, i;
        recno = GET2(slot, 0);
        if (recno <= backref_cache[0] && backref_cache[recno] >= 0)
          dd = backref_cache[recno];
        else
          {
          ce = cs = PRIV(find_bracket)(startcode, utf, recno);
          if (cs == NULL) return -2;
          do ce += GET(ce, 1); while (*ce == OP_ALT);
          dd = 0;
          if (!dupcapused || PRIV(find_bracket)(ce, utf, recno) == NULL)
            {
            if (cc > cs && cc < ce)
              {
              had_recurse = TRUE;
              }
            else
              {
              recurse_check *r = recurses;
              for (r = recurses; r != NULL; r = r->prev)
                if (r->group == cs) break;
              if (r != NULL)
                {
                had_recurse = TRUE;
                }
              else
                {
                this_recurse.prev = recurses;
                this_recurse.group = cs;
                dd = find_minlength(re, cs, startcode, utf, &this_recurse,
                  countptr, backref_cache);
                if (dd < 0) return dd;
                }
              }
            }
          backref_cache[recno] = dd;
          for (i = backref_cache[0] + 1; i < recno; i++) backref_cache[i] = -1;
          backref_cache[0] = recno;
          }
        if (dd < d) d = dd;
        if (d <= 0) break;
        slot += re->name_entry_size;
        }
      }
    else d = 0;
    cc += PRIV(OP_lengths)[*cc];
    goto REPEAT_BACK_REFERENCE;
    case OP_REF:
    case OP_REFI:
    recno = GET2(cc, 1);
    if (recno <= backref_cache[0] && backref_cache[recno] >= 0)
      d = backref_cache[recno];
    else
      {
      int i;
      d = 0;
      if ((re->overall_options & PCRE2_MATCH_UNSET_BACKREF) == 0)
        {
        ce = cs = PRIV(find_bracket)(startcode, utf, recno);
        if (cs == NULL) return -2;
        do ce += GET(ce, 1); while (*ce == OP_ALT);
        if (!dupcapused || PRIV(find_bracket)(ce, utf, recno) == NULL)
          {
          if (cc > cs && cc < ce)
            {
            had_recurse = TRUE;
            }
          else
            {
            recurse_check *r = recurses;
            for (r = recurses; r != NULL; r = r->prev) if (r->group == cs) break;
            if (r != NULL)
              {
              had_recurse = TRUE;
              }
            else
              {
              this_recurse.prev = recurses;
              this_recurse.group = cs;
              d = find_minlength(re, cs, startcode, utf, &this_recurse, countptr,
                backref_cache);
              if (d < 0) return d;
              }
            }
          }
        }
      backref_cache[recno] = d;
      for (i = backref_cache[0] + 1; i < recno; i++) backref_cache[i] = -1;
      backref_cache[0] = recno;
      }
    cc += PRIV(OP_lengths)[*cc];
    REPEAT_BACK_REFERENCE:
    switch (*cc)
      {
      case OP_CRSTAR:
      case OP_CRMINSTAR:
      case OP_CRQUERY:
      case OP_CRMINQUERY:
      case OP_CRPOSSTAR:
      case OP_CRPOSQUERY:
      min = 0;
      cc++;
      break;
      case OP_CRPLUS:
      case OP_CRMINPLUS:
      case OP_CRPOSPLUS:
      min = 1;
      cc++;
      break;
      case OP_CRRANGE:
      case OP_CRMINRANGE:
      case OP_CRPOSRANGE:
      min = GET2(cc, 1);
      cc += 1 + 2 * IMM2_SIZE;
      break;
      default:
      min = 1;
      break;
      }
    if ((d > 0 && (INT_MAX/d) < min) || (int)UINT16_MAX - branchlength < min*d)
      branchlength = UINT16_MAX;
    else branchlength += min * d;
    break;
    case OP_RECURSE:
    cs = ce = startcode + GET(cc, 1);
    recno = GET2(cs, 1+LINK_SIZE);
    if (recno == prev_recurse_recno)
      {
      branchlength += prev_recurse_d;
      }
    else
      {
      do ce += GET(ce, 1); while (*ce == OP_ALT);
      if (cc > cs && cc < ce)
        had_recurse = TRUE;
      else
        {
        recurse_check *r = recurses;
        for (r = recurses; r != NULL; r = r->prev) if (r->group == cs) break;
        if (r != NULL)
          had_recurse = TRUE;
        else
          {
          this_recurse.prev = recurses;
          this_recurse.group = cs;
          prev_recurse_d = find_minlength(re, cs, startcode, utf, &this_recurse,
            countptr, backref_cache);
          if (prev_recurse_d < 0) return prev_recurse_d;
          prev_recurse_recno = recno;
          branchlength += prev_recurse_d;
          }
        }
      }
    cc += 1 + LINK_SIZE + once_fudge;
    once_fudge = 0;
    break;
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
    cc += PRIV(OP_lengths)[op];
#ifdef SUPPORT_UNICODE
    if (utf && HAS_EXTRALEN(cc[-1])) cc += GET_EXTRALEN(cc[-1]);
#endif
    break;
    case OP_MARK:
    case OP_COMMIT_ARG:
    case OP_PRUNE_ARG:
    case OP_SKIP_ARG:
    case OP_THEN_ARG:
    cc += PRIV(OP_lengths)[op] + cc[1];
    break;
    case OP_CLOSE:
    case OP_COMMIT:
    case OP_FAIL:
    case OP_PRUNE:
    case OP_SET_SOM:
    case OP_SKIP:
    case OP_THEN:
    cc += PRIV(OP_lengths)[op];
    break;
    default:
    PCRE2_DEBUG_UNREACHABLE();
    return -3;
    }
  }
PCRE2_DEBUG_UNREACHABLE();
return -3;
}
static PCRE2_SPTR
set_table_bit(pcre2_real_code *re, PCRE2_SPTR p, BOOL caseless, BOOL utf,
  BOOL ucp)
{
uint32_t c = *p++;
(void)utf;
(void)ucp;
#if PCRE2_CODE_UNIT_WIDTH != 8
if (c > 0xff) SET_BIT(0xff); else
#endif
SET_BIT(c);
#ifdef SUPPORT_UNICODE
if (utf)
  {
#if PCRE2_CODE_UNIT_WIDTH == 8
  if (c >= 0xc0) GETUTF8INC(c, p);
#elif PCRE2_CODE_UNIT_WIDTH == 16
  if ((c & 0xfc00) == 0xd800) GETUTF16INC(c, p);
#endif
  }
#endif
if (caseless)
  {
#ifdef SUPPORT_UNICODE
  if (utf || ucp)
    {
    c = UCD_OTHERCASE(c);
#if PCRE2_CODE_UNIT_WIDTH == 8
    if (utf)
      {
      PCRE2_UCHAR buff[6];
      (void)PRIV(ord2utf)(c, buff);
      SET_BIT(buff[0]);
      }
    else if (c < 256) SET_BIT(c);
#else
    if (c > 0xff) SET_BIT(0xff); else SET_BIT(c);
#endif
    }
  else
#endif
  if (MAX_255(c)) SET_BIT(re->tables[fcc_offset + c]);
  }
return p;
}
static void
set_type_bits(pcre2_real_code *re, int cbit_type, unsigned int table_limit)
{
uint32_t c;
for (c = 0; c < table_limit; c++)
  re->start_bitmap[c] |= re->tables[c+cbits_offset+cbit_type];
#if defined SUPPORT_UNICODE && PCRE2_CODE_UNIT_WIDTH == 8
if (table_limit == 32) return;
for (c = 128; c < 256; c++)
  {
  if ((re->tables[cbits_offset + c/8] & (1u << (c&7))) != 0)
    {
    PCRE2_UCHAR buff[6];
    (void)PRIV(ord2utf)(c, buff);
    SET_BIT(buff[0]);
    }
  }
#endif
}
static void
set_nottype_bits(pcre2_real_code *re, int cbit_type, unsigned int table_limit)
{
uint32_t c;
for (c = 0; c < table_limit; c++)
  re->start_bitmap[c] |= (uint8_t)(~(re->tables[c+cbits_offset+cbit_type]));
#if defined SUPPORT_UNICODE && PCRE2_CODE_UNIT_WIDTH == 8
if (table_limit != 32) for (c = 24; c < 32; c++) re->start_bitmap[c] = 0xff;
#endif
}
#if defined SUPPORT_UNICODE && PCRE2_CODE_UNIT_WIDTH == 8
static void
study_char_list(PCRE2_SPTR code, uint8_t *start_bitmap,
  const uint8_t *char_lists_end)
{
uint32_t type, list_ind;
uint32_t char_list_add = XCL_CHAR_LIST_LOW_16_ADD;
uint32_t range_start = ~(uint32_t)0, range_end = 0;
const uint8_t *next_char;
PCRE2_UCHAR start_buffer[6], end_buffer[6];
PCRE2_UCHAR start, end;
type = (uint32_t)(code[0] << 8) | code[1];
code += 2;
next_char = char_lists_end - (GET(code, 0) << 1);
type &= XCL_TYPE_MASK;
list_ind = 0;
if ((type & XCL_BEGIN_WITH_RANGE) != 0)
  range_start = XCL_CHAR_LIST_LOW_16_START;
while (type > 0)
  {
  uint32_t item_count = type & XCL_ITEM_COUNT_MASK;
  if (item_count == XCL_ITEM_COUNT_MASK)
    {
    if (list_ind <= 1)
      {
      item_count = *(const uint16_t*)next_char;
      next_char += 2;
      }
    else
      {
      item_count = *(const uint32_t*)next_char;
      next_char += 4;
      }
    }
  while (item_count > 0)
    {
    if (list_ind <= 1)
      {
      range_end = *(const uint16_t*)next_char;
      next_char += 2;
      }
    else
      {
      range_end = *(const uint32_t*)next_char;
      next_char += 4;
      }
    if ((range_end & XCL_CHAR_END) != 0)
      {
      range_end = char_list_add + (range_end >> XCL_CHAR_SHIFT);
      PRIV(ord2utf)(range_end, end_buffer);
      end = end_buffer[0];
      if (range_start < range_end)
        {
        PRIV(ord2utf)(range_start, start_buffer);
        for (start = start_buffer[0]; start <= end; start++)
          start_bitmap[start / 8] |= (1u << (start & 7));
        }
      else
        start_bitmap[end / 8] |= (1u << (end & 7));
      range_start = ~(uint32_t)0;
      }
    else
      range_start = char_list_add + (range_end >> XCL_CHAR_SHIFT);
    item_count--;
    }
  list_ind++;
  type >>= XCL_TYPE_BIT_LEN;
  if (range_start == ~(uint32_t)0)
    {
    if ((type & XCL_BEGIN_WITH_RANGE) != 0)
      {
      if (list_ind == 1) range_start = XCL_CHAR_LIST_HIGH_16_START;
      else range_start = XCL_CHAR_LIST_LOW_32_START;
      }
    }
  else if ((type & XCL_BEGIN_WITH_RANGE) == 0)
    {
    PRIV(ord2utf)(range_start, start_buffer);
    if (list_ind == 1) range_end = XCL_CHAR_LIST_LOW_16_END;
    else range_end = XCL_CHAR_LIST_HIGH_16_END;
    PRIV(ord2utf)(range_end, end_buffer);
    end = end_buffer[0];
    for (start = start_buffer[0]; start <= end; start++)
      start_bitmap[start / 8] |= (1u << (start & 7));
    range_start = ~(uint32_t)0;
    }
  if (list_ind == 1) char_list_add = XCL_CHAR_LIST_HIGH_16_ADD;
  else char_list_add = XCL_CHAR_LIST_LOW_32_ADD;
  }
}
#endif
static int
set_start_bits(pcre2_real_code *re, PCRE2_SPTR code, BOOL utf, BOOL ucp,
  int *depthptr)
{
uint32_t c;
int yield = SSB_DONE;
#if defined SUPPORT_UNICODE && PCRE2_CODE_UNIT_WIDTH == 8
int table_limit = utf? 16:32;
#else
int table_limit = 32;
#endif
*depthptr += 1;
if (*depthptr > 1000) return SSB_TOODEEP;
do
  {
  BOOL try_next = TRUE;
  PCRE2_SPTR tcode = code + 1 + LINK_SIZE;
  if (*code == OP_CBRA || *code == OP_SCBRA ||
      *code == OP_CBRAPOS || *code == OP_SCBRAPOS) tcode += IMM2_SIZE;
  while (try_next)
    {
    int rc;
    PCRE2_SPTR ncode;
    const uint8_t *classmap = NULL;
#ifdef SUPPORT_WIDE_CHARS
    PCRE2_UCHAR xclassflags;
#endif
    switch(*tcode)
      {
      default:
      return SSB_UNKNOWN;
      case OP_ACCEPT:
      case OP_ASSERT_ACCEPT:
      case OP_ALLANY:
      case OP_ANY:
      case OP_ANYBYTE:
      case OP_CIRCM:
      case OP_CLOSE:
      case OP_COMMIT:
      case OP_COMMIT_ARG:
      case OP_COND:
      case OP_CREF:
      case OP_FALSE:
      case OP_TRUE:
      case OP_DNCREF:
      case OP_DNREF:
      case OP_DNREFI:
      case OP_DNRREF:
      case OP_DOLL:
      case OP_DOLLM:
      case OP_END:
      case OP_EOD:
      case OP_EODN:
      case OP_EXTUNI:
      case OP_FAIL:
      case OP_MARK:
      case OP_NOT:
      case OP_NOTEXACT:
      case OP_NOTEXACTI:
      case OP_NOTI:
      case OP_NOTMINPLUS:
      case OP_NOTMINPLUSI:
      case OP_NOTMINQUERY:
      case OP_NOTMINQUERYI:
      case OP_NOTMINSTAR:
      case OP_NOTMINSTARI:
      case OP_NOTMINUPTO:
      case OP_NOTMINUPTOI:
      case OP_NOTPLUS:
      case OP_NOTPLUSI:
      case OP_NOTPOSPLUS:
      case OP_NOTPOSPLUSI:
      case OP_NOTPOSQUERY:
      case OP_NOTPOSQUERYI:
      case OP_NOTPOSSTAR:
      case OP_NOTPOSSTARI:
      case OP_NOTPOSUPTO:
      case OP_NOTPOSUPTOI:
      case OP_NOTPROP:
      case OP_NOTQUERY:
      case OP_NOTQUERYI:
      case OP_NOTSTAR:
      case OP_NOTSTARI:
      case OP_NOTUPTO:
      case OP_NOTUPTOI:
      case OP_NOT_HSPACE:
      case OP_NOT_VSPACE:
      case OP_PRUNE:
      case OP_PRUNE_ARG:
      case OP_RECURSE:
      case OP_REF:
      case OP_REFI:
      case OP_REVERSE:
      case OP_VREVERSE:
      case OP_RREF:
      case OP_SCOND:
      case OP_SET_SOM:
      case OP_SKIP:
      case OP_SKIP_ARG:
      case OP_SOD:
      case OP_SOM:
      case OP_THEN:
      case OP_THEN_ARG:
      return SSB_FAIL;
      case OP_CIRC:
      tcode += PRIV(OP_lengths)[OP_CIRC];
      break;
      case OP_PROP:
      if (tcode[1] != PT_CLIST) return SSB_FAIL;
        {
        const uint32_t *p = PRIV(ucd_caseless_sets) + tcode[2];
        while ((c = *p++) < NOTACHAR)
          {
#if defined SUPPORT_UNICODE && PCRE2_CODE_UNIT_WIDTH == 8
          if (utf)
            {
            PCRE2_UCHAR buff[6];
            (void)PRIV(ord2utf)(c, buff);
            c = buff[0];
            }
#endif
          if (c > 0xff) SET_BIT(0xff); else SET_BIT(c);
          }
        }
      try_next = FALSE;
      break;
      case OP_WORD_BOUNDARY:
      case OP_NOT_WORD_BOUNDARY:
      case OP_UCP_WORD_BOUNDARY:
      case OP_NOT_UCP_WORD_BOUNDARY:
      tcode++;
      break;
      case OP_ASSERT:
      case OP_ASSERT_NA:
      ncode = tcode + GET(tcode, 1);
      while (*ncode == OP_ALT) ncode += GET(ncode, 1);
      ncode += 1 + LINK_SIZE;
      for (BOOL done = FALSE; !done;)
        {
        switch (*ncode)
          {
          case OP_ASSERT:
          case OP_ASSERT_NOT:
          case OP_ASSERTBACK:
          case OP_ASSERTBACK_NOT:
          case OP_ASSERT_NA:
          case OP_ASSERTBACK_NA:
          case OP_ASSERT_SCS:
          ncode += GET(ncode, 1);
          while (*ncode == OP_ALT) ncode += GET(ncode, 1);
          ncode += 1 + LINK_SIZE;
          break;
          case OP_WORD_BOUNDARY:
          case OP_NOT_WORD_BOUNDARY:
          case OP_UCP_WORD_BOUNDARY:
          case OP_NOT_UCP_WORD_BOUNDARY:
          ncode++;
          break;
          case OP_CALLOUT:
          ncode += PRIV(OP_lengths)[OP_CALLOUT];
          break;
          case OP_CALLOUT_STR:
          ncode += GET(ncode, 1 + 2*LINK_SIZE);
          break;
          default:
          done = TRUE;
          break;
          }
        }
      switch(*ncode)
        {
        default:
        break;
        case OP_PROP:
        if (ncode[1] != PT_CLIST) break;
        PCRE2_FALLTHROUGH
        case OP_ANYNL:
        case OP_CHAR:
        case OP_CHARI:
        case OP_EXACT:
        case OP_EXACTI:
        case OP_HSPACE:
        case OP_MINPLUS:
        case OP_MINPLUSI:
        case OP_PLUS:
        case OP_PLUSI:
        case OP_POSPLUS:
        case OP_POSPLUSI:
        case OP_VSPACE:
        case OP_DIGIT:
        case OP_NOT_DIGIT:
        case OP_WORDCHAR:
        case OP_NOT_WORDCHAR:
        case OP_WHITESPACE:
        case OP_NOT_WHITESPACE:
        tcode = ncode;
        continue;
        }
      PCRE2_FALLTHROUGH
      case OP_BRA:
      case OP_SBRA:
      case OP_CBRA:
      case OP_SCBRA:
      case OP_BRAPOS:
      case OP_SBRAPOS:
      case OP_CBRAPOS:
      case OP_SCBRAPOS:
      case OP_ONCE:
      case OP_SCRIPT_RUN:
      rc = set_start_bits(re, tcode, utf, ucp, depthptr);
      if (rc == SSB_DONE)
        {
        try_next = FALSE;
        }
      else if (rc == SSB_CONTINUE)
        {
        do tcode += GET(tcode, 1); while (*tcode == OP_ALT);
        tcode += 1 + LINK_SIZE;
        }
      else return rc;
      break;
      case OP_ALT:
      yield = SSB_CONTINUE;
      try_next = FALSE;
      break;
      case OP_KET:
      case OP_KETRMAX:
      case OP_KETRMIN:
      case OP_KETRPOS:
      return SSB_CONTINUE;
      case OP_CALLOUT:
      tcode += PRIV(OP_lengths)[OP_CALLOUT];
      break;
      case OP_CALLOUT_STR:
      tcode += GET(tcode, 1 + 2*LINK_SIZE);
      break;
      case OP_ASSERT_NOT:
      case OP_ASSERTBACK:
      case OP_ASSERTBACK_NOT:
      case OP_ASSERTBACK_NA:
      case OP_ASSERT_SCS:
      do tcode += GET(tcode, 1); while (*tcode == OP_ALT);
      tcode += 1 + LINK_SIZE;
      break;
      case OP_BRAZERO:
      case OP_BRAMINZERO:
      case OP_BRAPOSZERO:
      rc = set_start_bits(re, ++tcode, utf, ucp, depthptr);
      if (rc == SSB_FAIL || rc == SSB_UNKNOWN || rc == SSB_TOODEEP) return rc;
      do tcode += GET(tcode,1); while (*tcode == OP_ALT);
      tcode += 1 + LINK_SIZE;
      break;
      case OP_SKIPZERO:
      tcode++;
      do tcode += GET(tcode,1); while (*tcode == OP_ALT);
      tcode += 1 + LINK_SIZE;
      break;
      case OP_STAR:
      case OP_MINSTAR:
      case OP_POSSTAR:
      case OP_QUERY:
      case OP_MINQUERY:
      case OP_POSQUERY:
      tcode = set_table_bit(re, tcode + 1, FALSE, utf, ucp);
      break;
      case OP_STARI:
      case OP_MINSTARI:
      case OP_POSSTARI:
      case OP_QUERYI:
      case OP_MINQUERYI:
      case OP_POSQUERYI:
      tcode = set_table_bit(re, tcode + 1, TRUE, utf, ucp);
      break;
      case OP_UPTO:
      case OP_MINUPTO:
      case OP_POSUPTO:
      tcode = set_table_bit(re, tcode + 1 + IMM2_SIZE, FALSE, utf, ucp);
      break;
      case OP_UPTOI:
      case OP_MINUPTOI:
      case OP_POSUPTOI:
      tcode = set_table_bit(re, tcode + 1 + IMM2_SIZE, TRUE, utf, ucp);
      break;
      case OP_EXACT:
      tcode += IMM2_SIZE;
      PCRE2_FALLTHROUGH
      case OP_CHAR:
      case OP_PLUS:
      case OP_MINPLUS:
      case OP_POSPLUS:
      (void)set_table_bit(re, tcode + 1, FALSE, utf, ucp);
      try_next = FALSE;
      break;
      case OP_EXACTI:
      tcode += IMM2_SIZE;
      PCRE2_FALLTHROUGH
      case OP_CHARI:
      case OP_PLUSI:
      case OP_MINPLUSI:
      case OP_POSPLUSI:
      (void)set_table_bit(re, tcode + 1, TRUE, utf, ucp);
      try_next = FALSE;
      break;
      case OP_HSPACE:
      SET_BIT(CHAR_HT);
      SET_BIT(CHAR_SPACE);
#if PCRE2_CODE_UNIT_WIDTH != 8
      SET_BIT(CHAR_NBSP);
      SET_BIT(0xFF);
#else
#ifdef SUPPORT_UNICODE
      if (utf)
        {
        SET_BIT(0xC2);
        SET_BIT(0xE1);
        SET_BIT(0xE2);
        SET_BIT(0xE3);
        }
      else
#endif
        {
        SET_BIT(CHAR_NBSP);
        }
#endif
      try_next = FALSE;
      break;
      case OP_ANYNL:
      case OP_VSPACE:
      SET_BIT(CHAR_LF);
      SET_BIT(CHAR_VT);
      SET_BIT(CHAR_FF);
      SET_BIT(CHAR_CR);
#if PCRE2_CODE_UNIT_WIDTH != 8
      SET_BIT(CHAR_NEL);
      SET_BIT(0xFF);
#else
#ifdef SUPPORT_UNICODE
      if (utf)
        {
        SET_BIT(0xC2);
        SET_BIT(0xE2);
        }
      else
#endif
        {
        SET_BIT(CHAR_NEL);
        }
#endif
      try_next = FALSE;
      break;
      case OP_NOT_DIGIT:
      set_nottype_bits(re, cbit_digit, table_limit);
      try_next = FALSE;
      break;
      case OP_DIGIT:
      set_type_bits(re, cbit_digit, table_limit);
      try_next = FALSE;
      break;
      case OP_NOT_WHITESPACE:
      set_nottype_bits(re, cbit_space, table_limit);
      try_next = FALSE;
      break;
      case OP_WHITESPACE:
      set_type_bits(re, cbit_space, table_limit);
      try_next = FALSE;
      break;
      case OP_NOT_WORDCHAR:
      set_nottype_bits(re, cbit_word, table_limit);
      try_next = FALSE;
      break;
      case OP_WORDCHAR:
      set_type_bits(re, cbit_word, table_limit);
      try_next = FALSE;
      break;
      case OP_TYPEPLUS:
      case OP_TYPEMINPLUS:
      case OP_TYPEPOSPLUS:
      tcode++;
      break;
      case OP_TYPEEXACT:
      tcode += 1 + IMM2_SIZE;
      break;
      case OP_TYPEUPTO:
      case OP_TYPEMINUPTO:
      case OP_TYPEPOSUPTO:
      tcode += IMM2_SIZE;
      PCRE2_FALLTHROUGH
      case OP_TYPESTAR:
      case OP_TYPEMINSTAR:
      case OP_TYPEPOSSTAR:
      case OP_TYPEQUERY:
      case OP_TYPEMINQUERY:
      case OP_TYPEPOSQUERY:
      switch(tcode[1])
        {
        default:
        case OP_ANY:
        case OP_ALLANY:
        return SSB_FAIL;
        case OP_HSPACE:
        SET_BIT(CHAR_HT);
        SET_BIT(CHAR_SPACE);
#if PCRE2_CODE_UNIT_WIDTH != 8
        SET_BIT(CHAR_NBSP);
        SET_BIT(0xFF);
#else
#ifdef SUPPORT_UNICODE
        if (utf)
          {
          SET_BIT(0xC2);
          SET_BIT(0xE1);
          SET_BIT(0xE2);
          SET_BIT(0xE3);
          }
        else
#endif
          {
          SET_BIT(CHAR_NBSP);
          }
#endif
        break;
        case OP_ANYNL:
        case OP_VSPACE:
        SET_BIT(CHAR_LF);
        SET_BIT(CHAR_VT);
        SET_BIT(CHAR_FF);
        SET_BIT(CHAR_CR);
#if PCRE2_CODE_UNIT_WIDTH != 8
        SET_BIT(CHAR_NEL);
        SET_BIT(0xFF);
#else
#ifdef SUPPORT_UNICODE
        if (utf)
          {
          SET_BIT(0xC2);
          SET_BIT(0xE2);
          }
        else
#endif
          {
          SET_BIT(CHAR_NEL);
          }
#endif
        break;
        case OP_NOT_DIGIT:
        set_nottype_bits(re, cbit_digit, table_limit);
        break;
        case OP_DIGIT:
        set_type_bits(re, cbit_digit, table_limit);
        break;
        case OP_NOT_WHITESPACE:
        set_nottype_bits(re, cbit_space, table_limit);
        break;
        case OP_WHITESPACE:
        set_type_bits(re, cbit_space, table_limit);
        break;
        case OP_NOT_WORDCHAR:
        set_nottype_bits(re, cbit_word, table_limit);
        break;
        case OP_WORDCHAR:
        set_type_bits(re, cbit_word, table_limit);
        break;
        }
      tcode += 2;
      break;
#ifdef SUPPORT_WIDE_CHARS
      case OP_ECLASS:
      return SSB_FAIL;
#endif
#ifdef SUPPORT_WIDE_CHARS
      case OP_XCLASS:
      xclassflags = tcode[1 + LINK_SIZE];
      if ((xclassflags & XCL_HASPROP) != 0 ||
          (xclassflags & (XCL_MAP|XCL_NOT)) == XCL_NOT)
        return SSB_FAIL;
      classmap = ((xclassflags & XCL_MAP) == 0)? NULL :
        (const uint8_t *)(tcode + 1 + LINK_SIZE + 1);
#if PCRE2_CODE_UNIT_WIDTH == 8
      if (utf && (xclassflags & XCL_NOT) == 0)
        {
        PCRE2_UCHAR b, e;
        PCRE2_SPTR p = tcode + 1 + LINK_SIZE + 1 + ((classmap == NULL)? 0:32);
        tcode += GET(tcode, 1);
        if (*p >= XCL_LIST)
          {
          study_char_list(p, re->start_bitmap,
            ((const uint8_t *)re + re->code_start));
          goto HANDLE_CLASSMAP;
          }
        for (;;) switch (*p++)
          {
          case XCL_SINGLE:
          b = *p++;
          while ((*p & 0xc0) == 0x80) p++;
          re->start_bitmap[b/8] |= (1u << (b&7));
          break;
          case XCL_RANGE:
          b = *p++;
          while ((*p & 0xc0) == 0x80) p++;
          e = *p++;
          while ((*p & 0xc0) == 0x80) p++;
          for (; b <= e; b++)
            re->start_bitmap[b/8] |= (1u << (b&7));
          break;
          case XCL_END:
          goto HANDLE_CLASSMAP;
          default:
          PCRE2_DEBUG_UNREACHABLE();
          return SSB_UNKNOWN;
          }
        }
#endif
#endif
      PCRE2_FALLTHROUGH
      case OP_NCLASS:
#if defined SUPPORT_UNICODE && PCRE2_CODE_UNIT_WIDTH == 8
      if (utf)
        {
        re->start_bitmap[24] |= 0xf0;
        memset(re->start_bitmap+25, 0xff, 7);
        }
      PCRE2_FALLTHROUGH
#elif PCRE2_CODE_UNIT_WIDTH != 8
      SET_BIT(0xFF);
      PCRE2_FALLTHROUGH
#endif
      case OP_CLASS:
      if (*tcode == OP_XCLASS) tcode += GET(tcode, 1); else
        {
        classmap = (const uint8_t *)(++tcode);
        tcode += 32 / sizeof(PCRE2_UCHAR);
        }
#if defined SUPPORT_WIDE_CHARS && PCRE2_CODE_UNIT_WIDTH == 8
      HANDLE_CLASSMAP:
#endif
      if (classmap != NULL)
        {
#if defined SUPPORT_UNICODE && PCRE2_CODE_UNIT_WIDTH == 8
        if (utf)
          {
          for (c = 0; c < 16; c++) re->start_bitmap[c] |= classmap[c];
          for (c = 128; c < 256; c++)
            {
            if ((classmap[c/8] & (1u << (c&7))) != 0)
              {
              int d = (c >> 6) | 0xc0;
              re->start_bitmap[d/8] |= (1u << (d&7));
              c = (c & 0xc0) + 0x40 - 1;
              }
            }
          }
        else
#endif
          {
          for (c = 0; c < 32; c++) re->start_bitmap[c] |= classmap[c];
          }
        }
      switch (*tcode)
        {
        case OP_CRSTAR:
        case OP_CRMINSTAR:
        case OP_CRQUERY:
        case OP_CRMINQUERY:
        case OP_CRPOSSTAR:
        case OP_CRPOSQUERY:
        tcode++;
        break;
        case OP_CRRANGE:
        case OP_CRMINRANGE:
        case OP_CRPOSRANGE:
        if (GET2(tcode, 1) == 0) tcode += 1 + 2 * IMM2_SIZE;
          else try_next = FALSE;
        break;
        default:
        try_next = FALSE;
        break;
        }
      break;
      }
    }
  code += GET(code, 1);
  }
while (*code == OP_ALT);
return yield;
}
int
PRIV(study)(pcre2_real_code *re)
{
int count = 0;
PCRE2_UCHAR *code;
BOOL utf = (re->overall_options & PCRE2_UTF) != 0;
BOOL ucp = (re->overall_options & PCRE2_UCP) != 0;
code = (PCRE2_UCHAR *)((uint8_t *)re + re->code_start);
if ((re->flags & (PCRE2_FIRSTSET|PCRE2_STARTLINE)) == 0)
  {
  int depth = 0;
  int rc = set_start_bits(re, code, utf, ucp, &depth);
  if (rc == SSB_UNKNOWN)
    {
    PCRE2_DEBUG_UNREACHABLE();
    return 1;
    }
  if (rc == SSB_DONE)
    {
    int i;
    int a = -1;
    int b = -1;
    uint8_t *p = re->start_bitmap;
    uint32_t flags = PCRE2_FIRSTMAPSET;
    for (i = 0; i < 256; p++, i += 8)
      {
      uint8_t x = *p;
      if (x != 0)
        {
        int c;
        uint8_t y = x & (~x + 1);
        if (y != x) goto DONE;
#if PCRE2_CODE_UNIT_WIDTH != 8
        if (i == 248 && x == 0x80) goto DONE;
#endif
        c = i;
        switch (x)
          {
          case 1:   break;
          case 2:   c += 1; break;  case 4:  c += 2; break;
          case 8:   c += 3; break;  case 16: c += 4; break;
          case 32:  c += 5; break;  case 64: c += 6; break;
          case 128: c += 7; break;
          }
#if PCRE2_CODE_UNIT_WIDTH == 8
        if (utf && c > 127) goto DONE;
#endif
        if (a < 0) a = c;
        else if (b < 0)
          {
          int d = TABLE_GET((unsigned int)c, re->tables + fcc_offset, c);
#ifdef SUPPORT_UNICODE
          if (utf || ucp)
            {
            if (UCD_CASESET(c) != 0) goto DONE;
            if (c > 127) d = UCD_OTHERCASE(c);
            }
#endif
          if (d != a) goto DONE;
          b = c;
#ifdef EBCDIC
          if (TABLE_GET((unsigned int)a, re->tables + lcc_offset, a) == a)
            {
            b = a;
            a = c;
            }
#endif
          }
        else goto DONE;
        }
      }
    if (a >= 0) {
      if ((re->flags & PCRE2_LASTSET) && (re->last_codeunit == (uint32_t)a || (b >= 0 && re->last_codeunit == (uint32_t)b))) {
        re->flags &= ~(PCRE2_LASTSET | PCRE2_LASTCASELESS);
        re->last_codeunit = 0;
      }
      re->first_codeunit = a;
      flags = PCRE2_FIRSTSET;
      if (b >= 0) flags |= PCRE2_FIRSTCASELESS;
    }
    DONE:
    re->flags |= flags;
    }
  }
if ((re->flags & (PCRE2_MATCH_EMPTY|PCRE2_HASACCEPT)) == 0 &&
     re->top_backref <= MAX_CACHE_BACKREF)
  {
  int min;
  int backref_cache[MAX_CACHE_BACKREF+1];
  backref_cache[0] = 0;
  min = find_minlength(re, code, code, utf, NULL, &count, backref_cache);
  switch(min)
    {
    case -1:
    break;
    case -2:
    PCRE2_DEBUG_UNREACHABLE();
    return 2;
    case -3:
    PCRE2_DEBUG_UNREACHABLE();
    return 3;
    default:
    re->minlength = (min > (int)UINT16_MAX)? (int)UINT16_MAX : min;
    break;
    }
  }
return 0;
}
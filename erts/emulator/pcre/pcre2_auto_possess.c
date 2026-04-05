#include "pcre2_internal.h"
#define MAX_LIST 8
#define APTROWS (LAST_AUTOTAB_LEFT_OP - FIRST_AUTOTAB_OP + 1)
#define APTCOLS (LAST_AUTOTAB_RIGHT_OP - FIRST_AUTOTAB_OP + 1)
static const uint8_t autoposstab[APTROWS][APTCOLS] = {
  { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
  { 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1 },
  { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1 },
  { 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0 },
  { 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 }
};
#ifdef SUPPORT_UNICODE
static const uint8_t propposstab[PT_TABSIZE][PT_TABSIZE] = {
  { 3,  0,  0,  0,   0,    3,    1,      1,   0,    0,   0,    0,    0 },
  { 0,  2,  4,  0,   0,    9,   10,     10,  11,    0,   0,    0,    0 },
  { 0,  5,  2,  0,   0,   15,   16,     16,  17,    0,   0,    0,    0 },
  { 0,  0,  0,  2,   2,    0,    0,      0,   0,    0,   0,    0,    0 },
  { 0,  0,  0,  2,   2,    0,    0,      0,   0,    0,   0,    0,    0 },
  { 3,  6, 12,  0,   0,    3,    1,      1,   0,    0,   0,    0,    0 },
  { 1,  7, 13,  0,   0,    1,    3,      3,   1,    0,   0,    0,    0 },
  { 1,  7, 13,  0,   0,    1,    3,      3,   1,    0,   0,    0,    0 },
  { 0,  8, 14,  0,   0,    0,    1,      1,   3,    0,   0,    0,    0 },
  { 0,  0,  0,  0,   0,    0,    0,      0,   0,    0,   0,    0,    0 },
  { 0,  0,  0,  0,   0,    0,    0,      0,   0,    0,   3,    0,    0 },
  { 0,  0,  0,  0,   0,    0,    0,      0,   0,    0,   0,    0,    0 },
  { 0,  0,  0,  0,   0,    0,    0,      0,   0,    0,   0,    0,    0 }
};
static const uint8_t catposstab[7][30] = {
  { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 }
};
static const uint8_t posspropstab[3][4] = {
  { ucp_L, ucp_N, ucp_N, ucp_Nl },
  { ucp_Z, ucp_Z, ucp_C, ucp_Cc },
  { ucp_L, ucp_N, ucp_P, ucp_Po }
};
#endif
#ifdef SUPPORT_UNICODE
static BOOL
check_char_prop(uint32_t c, unsigned int ptype, unsigned int pdata,
  BOOL negated)
{
BOOL ok, rc;
const uint32_t *p;
const ucd_record *prop = GET_UCD(c);
switch(ptype)
  {
  case PT_LAMP:
  return (prop->chartype == ucp_Lu ||
          prop->chartype == ucp_Ll ||
          prop->chartype == ucp_Lt) == negated;
  case PT_GC:
  return (pdata == PRIV(ucp_gentype)[prop->chartype]) == negated;
  case PT_PC:
  return (pdata == prop->chartype) == negated;
  case PT_SC:
  return (pdata == prop->script) == negated;
  case PT_SCX:
  ok = (pdata == prop->script
        || MAPBIT(PRIV(ucd_script_sets) + UCD_SCRIPTX_PROP(prop), pdata) != 0);
  return ok == negated;
  case PT_ALNUM:
  return (PRIV(ucp_gentype)[prop->chartype] == ucp_L ||
          PRIV(ucp_gentype)[prop->chartype] == ucp_N) == negated;
  case PT_SPACE:
  case PT_PXSPACE:
  switch(c)
    {
    HSPACE_CASES:
    VSPACE_CASES:
    rc = negated;
    break;
    default:
    rc = (PRIV(ucp_gentype)[prop->chartype] == ucp_Z) == negated;
    }
  return rc;
  case PT_WORD:
  return (PRIV(ucp_gentype)[prop->chartype] == ucp_L ||
          PRIV(ucp_gentype)[prop->chartype] == ucp_N ||
          c == CHAR_UNDERSCORE) == negated;
  case PT_CLIST:
  p = PRIV(ucd_caseless_sets) + prop->caseset;
  for (;;)
    {
    if (c < *p) return !negated;
    if (c == *p++) return negated;
    }
  PCRE2_DEBUG_UNREACHABLE();
  break;
  case PT_BIDICL:
  return FALSE;
  case PT_BOOL:
  return FALSE;
  }
return FALSE;
}
#endif
static PCRE2_UCHAR
get_repeat_base(PCRE2_UCHAR c)
{
return (c > OP_TYPEPOSUPTO)? c :
       (c >= OP_TYPESTAR)?   OP_TYPESTAR :
       (c >= OP_NOTSTARI)?   OP_NOTSTARI :
       (c >= OP_NOTSTAR)?    OP_NOTSTAR :
       (c >= OP_STARI)?      OP_STARI :
                             OP_STAR;
}
static PCRE2_SPTR
get_chr_property_list(PCRE2_SPTR code, BOOL utf, BOOL ucp, const uint8_t *fcc,
  uint32_t *list)
{
PCRE2_UCHAR c = *code;
PCRE2_UCHAR base;
PCRE2_SPTR end;
PCRE2_SPTR class_end;
uint32_t chr;
#ifdef SUPPORT_UNICODE
uint32_t *clist_dest;
const uint32_t *clist_src;
#else
(void)utf;
(void)ucp;
#endif
list[0] = c;
list[1] = FALSE;
code++;
if (c >= OP_STAR && c <= OP_TYPEPOSUPTO)
  {
  base = get_repeat_base(c);
  c -= (base - OP_STAR);
  if (c == OP_UPTO || c == OP_MINUPTO || c == OP_EXACT || c == OP_POSUPTO)
    code += IMM2_SIZE;
  list[1] = (c != OP_PLUS && c != OP_MINPLUS && c != OP_EXACT &&
             c != OP_POSPLUS);
  switch(base)
    {
    case OP_STAR:
    list[0] = OP_CHAR;
    break;
    case OP_STARI:
    list[0] = OP_CHARI;
    break;
    case OP_NOTSTAR:
    list[0] = OP_NOT;
    break;
    case OP_NOTSTARI:
    list[0] = OP_NOTI;
    break;
    case OP_TYPESTAR:
    list[0] = *code;
    code++;
    break;
    }
  c = list[0];
  }
switch(c)
  {
  case OP_NOT_DIGIT:
  case OP_DIGIT:
  case OP_NOT_WHITESPACE:
  case OP_WHITESPACE:
  case OP_NOT_WORDCHAR:
  case OP_WORDCHAR:
  case OP_ANY:
  case OP_ALLANY:
  case OP_ANYNL:
  case OP_NOT_HSPACE:
  case OP_HSPACE:
  case OP_NOT_VSPACE:
  case OP_VSPACE:
  case OP_EXTUNI:
  case OP_EODN:
  case OP_EOD:
  case OP_DOLL:
  case OP_DOLLM:
  return code;
  case OP_CHAR:
  case OP_NOT:
  GETCHARINCTEST(chr, code);
  list[2] = chr;
  list[3] = NOTACHAR;
  return code;
  case OP_CHARI:
  case OP_NOTI:
  list[0] = (c == OP_CHARI) ? OP_CHAR : OP_NOT;
  GETCHARINCTEST(chr, code);
  list[2] = chr;
#ifdef SUPPORT_UNICODE
  if (chr < 128 || (chr < 256 && !utf && !ucp))
    list[3] = fcc[chr];
  else
    list[3] = UCD_OTHERCASE(chr);
#elif defined SUPPORT_WIDE_CHARS
  list[3] = (chr < 256) ? fcc[chr] : chr;
#else
  list[3] = fcc[chr];
#endif
  if (chr == list[3])
    list[3] = NOTACHAR;
  else
    list[4] = NOTACHAR;
  return code;
#ifdef SUPPORT_UNICODE
  case OP_PROP:
  case OP_NOTPROP:
  if (code[0] != PT_CLIST)
    {
    list[2] = code[0];
    list[3] = code[1];
    return code + 2;
    }
  clist_src = PRIV(ucd_caseless_sets) + code[1];
  clist_dest = list + 2;
  code += 2;
  do {
     if (clist_dest >= list + MAX_LIST)
       {
       PCRE2_DEBUG_UNREACHABLE();
       list[2] = code[0];
       list[3] = code[1];
       return code;
       }
     *clist_dest++ = *clist_src;
     }
  while(*clist_src++ != NOTACHAR);
  list[0] = (c == OP_PROP) ? OP_CHAR : OP_NOT;
  return code;
#endif
  case OP_NCLASS:
  case OP_CLASS:
#ifdef SUPPORT_WIDE_CHARS
  case OP_XCLASS:
  case OP_ECLASS:
  if (c == OP_XCLASS || c == OP_ECLASS)
    end = code + GET(code, 0) - 1;
  else
#endif
    end = code + 32 / sizeof(PCRE2_UCHAR);
  class_end = end;
  switch(*end)
    {
    case OP_CRSTAR:
    case OP_CRMINSTAR:
    case OP_CRQUERY:
    case OP_CRMINQUERY:
    case OP_CRPOSSTAR:
    case OP_CRPOSQUERY:
    list[1] = TRUE;
    end++;
    break;
    case OP_CRPLUS:
    case OP_CRMINPLUS:
    case OP_CRPOSPLUS:
    end++;
    break;
    case OP_CRRANGE:
    case OP_CRMINRANGE:
    case OP_CRPOSRANGE:
    list[1] = (GET2(end, 1) == 0);
    end += 1 + 2 * IMM2_SIZE;
    break;
    }
  list[2] = (uint32_t)(end - code);
  list[3] = (uint32_t)(end - class_end);
  return end;
  }
return NULL;
}
static BOOL
compare_opcodes(PCRE2_SPTR code, BOOL utf, BOOL ucp, const compile_block *cb,
  const uint32_t *base_list, PCRE2_SPTR base_end, int *rec_limit)
{
PCRE2_UCHAR c;
uint32_t list[MAX_LIST];
const uint32_t *chr_ptr;
const uint32_t *ochr_ptr;
const uint32_t *list_ptr;
PCRE2_SPTR next_code;
#ifdef SUPPORT_WIDE_CHARS
PCRE2_SPTR xclass_flags;
#endif
const uint8_t *class_bitset;
const uint8_t *set1, *set2, *set_end;
uint32_t chr;
BOOL accepted, invert_bits;
BOOL entered_a_group = FALSE;
if (--(*rec_limit) <= 0) return FALSE;
for(;;)
  {
  PCRE2_SPTR bracode;
  c = *code;
  if (c == OP_CALLOUT)
    {
    code += PRIV(OP_lengths)[c];
    continue;
    }
  if (c == OP_CALLOUT_STR)
    {
    code += GET(code, 1 + 2*LINK_SIZE);
    continue;
    }
  if (c == OP_ALT)
    {
    do code += GET(code, 1); while (*code == OP_ALT);
    c = *code;
    }
  switch(c)
    {
    case OP_END:
    return base_list[1] != 0;
    case OP_KET:
    case OP_KETRPOS:
    if (base_list[1] == 0) return FALSE;
    bracode = code - GET(code, 1);
    switch(*bracode)
      {
      case OP_CBRA:
      case OP_SCBRA:
      case OP_CBRAPOS:
      case OP_SCBRAPOS:
      if (cb->had_recurse) return FALSE;
      break;
      case OP_SCRIPT_RUN:
      if (base_list[0] != OP_CHAR && base_list[0] != OP_CHARI)
        return FALSE;
      break;
      case OP_ASSERT:
      case OP_ASSERT_NOT:
      case OP_ONCE:
      return !entered_a_group;
      case OP_ASSERTBACK:
      case OP_ASSERTBACK_NOT:
      do
        {
        if (bracode[1+LINK_SIZE] == OP_VREVERSE) return FALSE;
        bracode += GET(bracode, 1);
        }
      while (*bracode == OP_ALT);
      return !entered_a_group;
      case OP_ASSERT_NA:
      case OP_ASSERTBACK_NA:
      return FALSE;
      }
    code += PRIV(OP_lengths)[c];
    continue;
    case OP_ONCE:
    case OP_BRA:
    case OP_CBRA:
    next_code = code + GET(code, 1);
    code += PRIV(OP_lengths)[c];
    while (*next_code == OP_ALT)
      {
      if (!compare_opcodes(code, utf, ucp, cb, base_list, base_end, rec_limit))
        return FALSE;
      code = next_code + 1 + LINK_SIZE;
      next_code += GET(next_code, 1);
      }
    entered_a_group = TRUE;
    continue;
    case OP_BRAZERO:
    case OP_BRAMINZERO:
    next_code = code + 1;
    if (*next_code != OP_BRA && *next_code != OP_CBRA &&
        *next_code != OP_ONCE) return FALSE;
    do next_code += GET(next_code, 1); while (*next_code == OP_ALT);
    next_code += 1 + LINK_SIZE;
    if (!compare_opcodes(next_code, utf, ucp, cb, base_list, base_end,
         rec_limit))
      return FALSE;
    code += PRIV(OP_lengths)[c];
    continue;
    default:
    break;
    }
  code = get_chr_property_list(code, utf, ucp, cb->fcc, list);
  if (code == NULL) return FALSE;
  if (base_list[0] == OP_CHAR)
    {
    chr_ptr = base_list + 2;
    list_ptr = list;
    }
  else if (list[0] == OP_CHAR)
    {
    chr_ptr = list + 2;
    list_ptr = base_list;
    }
  else if (base_list[0] == OP_CLASS || list[0] == OP_CLASS
#if PCRE2_CODE_UNIT_WIDTH == 8
      || (!utf && (base_list[0] == OP_NCLASS || list[0] == OP_NCLASS))
#endif
      )
    {
#if PCRE2_CODE_UNIT_WIDTH == 8
    if (base_list[0] == OP_CLASS || (!utf && base_list[0] == OP_NCLASS))
#else
    if (base_list[0] == OP_CLASS)
#endif
      {
      set1 = (const uint8_t *)(base_end - base_list[2]);
      list_ptr = list;
      }
    else
      {
      set1 = (const uint8_t *)(code - list[2]);
      list_ptr = base_list;
      }
    invert_bits = FALSE;
    switch(list_ptr[0])
      {
      case OP_CLASS:
      case OP_NCLASS:
      set2 = (const uint8_t *)
        ((list_ptr == list ? code : base_end) - list_ptr[2]);
      break;
#ifdef SUPPORT_WIDE_CHARS
      case OP_XCLASS:
      xclass_flags = (list_ptr == list ? code : base_end) -
        list_ptr[2] + LINK_SIZE;
      if ((*xclass_flags & XCL_HASPROP) != 0) return FALSE;
      if ((*xclass_flags & XCL_MAP) == 0)
        {
        if (list[1] == 0) return (*xclass_flags & XCL_NOT) == 0;
        continue;
        }
      set2 = (const uint8_t *)(xclass_flags + 1);
      break;
#endif
      case OP_NOT_DIGIT:
      invert_bits = TRUE;
      PCRE2_FALLTHROUGH
      case OP_DIGIT:
      set2 = (const uint8_t *)(cb->cbits + cbit_digit);
      break;
      case OP_NOT_WHITESPACE:
      invert_bits = TRUE;
      PCRE2_FALLTHROUGH
      case OP_WHITESPACE:
      set2 = (const uint8_t *)(cb->cbits + cbit_space);
      break;
      case OP_NOT_WORDCHAR:
      invert_bits = TRUE;
      PCRE2_FALLTHROUGH
      case OP_WORDCHAR:
      set2 = (const uint8_t *)(cb->cbits + cbit_word);
      break;
      default:
      return FALSE;
      }
    set_end = set1 + 32;
    if (invert_bits)
      {
      do
        {
        if ((*set1++ & ~(*set2++)) != 0) return FALSE;
        }
      while (set1 < set_end);
      }
    else
      {
      do
        {
        if ((*set1++ & *set2++) != 0) return FALSE;
        }
      while (set1 < set_end);
      }
    if (list[1] == 0) return TRUE;
    continue;
    }
  else
    {
    uint32_t leftop, rightop;
    leftop = base_list[0];
    rightop = list[0];
#ifdef SUPPORT_UNICODE
    accepted = FALSE;
    if (leftop == OP_PROP || leftop == OP_NOTPROP)
      {
      if (rightop == OP_EOD)
        accepted = TRUE;
      else if (rightop == OP_PROP || rightop == OP_NOTPROP)
        {
        int n;
        const uint8_t *p;
        BOOL same = leftop == rightop;
        BOOL lisprop = leftop == OP_PROP;
        BOOL risprop = rightop == OP_PROP;
        BOOL bothprop = lisprop && risprop;
        n = propposstab[base_list[2]][list[2]];
        switch(n)
          {
          case 0: break;
          case 1: accepted = bothprop; break;
          case 2: accepted = (base_list[3] == list[3]) != same; break;
          case 3: accepted = !same; break;
          case 4:
          accepted = risprop && catposstab[base_list[3]][list[3]] == same;
          break;
          case 5:
          accepted = lisprop && catposstab[list[3]][base_list[3]] == same;
          break;
          case 6:
          case 7:
          case 8:
          p = posspropstab[n-6];
          accepted = risprop && lisprop ==
            (list[3] != p[0] &&
             list[3] != p[1] &&
            (list[3] != p[2] || !lisprop));
          break;
          case 9:
          case 10:
          case 11:
          p = posspropstab[n-9];
          accepted = lisprop && risprop ==
            (base_list[3] != p[0] &&
             base_list[3] != p[1] &&
            (base_list[3] != p[2] || !risprop));
          break;
          case 12:
          case 13:
          case 14:
          p = posspropstab[n-12];
          accepted = risprop && lisprop ==
            (catposstab[p[0]][list[3]] &&
             catposstab[p[1]][list[3]] &&
            (list[3] != p[3] || !lisprop));
          break;
          case 15:
          case 16:
          case 17:
          p = posspropstab[n-15];
          accepted = lisprop && risprop ==
            (catposstab[p[0]][base_list[3]] &&
             catposstab[p[1]][base_list[3]] &&
            (base_list[3] != p[3] || !risprop));
          break;
          }
        }
      }
    else
#endif
    accepted = leftop >= FIRST_AUTOTAB_OP && leftop <= LAST_AUTOTAB_LEFT_OP &&
           rightop >= FIRST_AUTOTAB_OP && rightop <= LAST_AUTOTAB_RIGHT_OP &&
           autoposstab[leftop - FIRST_AUTOTAB_OP][rightop - FIRST_AUTOTAB_OP];
    if (!accepted) return FALSE;
    if (list[1] == 0) return TRUE;
    continue;
    }
  do
    {
    chr = *chr_ptr;
    switch(list_ptr[0])
      {
      case OP_CHAR:
      ochr_ptr = list_ptr + 2;
      do
        {
        if (chr == *ochr_ptr) return FALSE;
        ochr_ptr++;
        }
      while(*ochr_ptr != NOTACHAR);
      break;
      case OP_NOT:
      ochr_ptr = list_ptr + 2;
      do
        {
        if (chr == *ochr_ptr)
          break;
        ochr_ptr++;
        }
      while(*ochr_ptr != NOTACHAR);
      if (*ochr_ptr == NOTACHAR) return FALSE;
      break;
      case OP_DIGIT:
      if (chr < 256 && (cb->ctypes[chr] & ctype_digit) != 0) return FALSE;
      break;
      case OP_NOT_DIGIT:
      if (chr > 255 || (cb->ctypes[chr] & ctype_digit) == 0) return FALSE;
      break;
      case OP_WHITESPACE:
      if (chr < 256 && (cb->ctypes[chr] & ctype_space) != 0) return FALSE;
      break;
      case OP_NOT_WHITESPACE:
      if (chr > 255 || (cb->ctypes[chr] & ctype_space) == 0) return FALSE;
      break;
      case OP_WORDCHAR:
      if (chr < 255 && (cb->ctypes[chr] & ctype_word) != 0) return FALSE;
      break;
      case OP_NOT_WORDCHAR:
      if (chr > 255 || (cb->ctypes[chr] & ctype_word) == 0) return FALSE;
      break;
      case OP_HSPACE:
      switch(chr)
        {
        HSPACE_CASES: return FALSE;
        default: break;
        }
      break;
      case OP_NOT_HSPACE:
      switch(chr)
        {
        HSPACE_CASES: break;
        default: return FALSE;
        }
      break;
      case OP_ANYNL:
      case OP_VSPACE:
      switch(chr)
        {
        VSPACE_CASES: return FALSE;
        default: break;
        }
      break;
      case OP_NOT_VSPACE:
      switch(chr)
        {
        VSPACE_CASES: break;
        default: return FALSE;
        }
      break;
      case OP_DOLL:
      case OP_EODN:
      switch (chr)
        {
        case CHAR_CR:
        case CHAR_LF:
        case CHAR_VT:
        case CHAR_FF:
        case CHAR_NEL:
#ifndef EBCDIC
        case 0x2028:
        case 0x2029:
#endif
        return FALSE;
        }
      break;
      case OP_EOD:
      break;
#ifdef SUPPORT_UNICODE
      case OP_PROP:
      case OP_NOTPROP:
      if (!check_char_prop(chr, list_ptr[2], list_ptr[3],
            list_ptr[0] == OP_NOTPROP))
        return FALSE;
      break;
#endif
      case OP_NCLASS:
      if (chr > 255) return FALSE;
      PCRE2_FALLTHROUGH
      case OP_CLASS:
      if (chr > 255) break;
      class_bitset = (const uint8_t *)
        ((list_ptr == list ? code : base_end) - list_ptr[2]);
      if ((class_bitset[chr >> 3] & (1u << (chr & 7))) != 0) return FALSE;
      break;
#ifdef SUPPORT_WIDE_CHARS
      case OP_XCLASS:
      if (PRIV(xclass)(chr, (list_ptr == list ? code : base_end) -
          list_ptr[2] + LINK_SIZE, (const uint8_t*)cb->start_code, utf))
        return FALSE;
      break;
      case OP_ECLASS:
      if (PRIV(eclass)(chr,
          (list_ptr == list ? code : base_end) - list_ptr[2] + LINK_SIZE,
          (list_ptr == list ? code : base_end) - list_ptr[3],
          (const uint8_t*)cb->start_code, utf))
        return FALSE;
      break;
#endif
      default:
      return FALSE;
      }
    chr_ptr++;
    }
  while(*chr_ptr != NOTACHAR);
  if (list[1] == 0) return TRUE;
  }
PCRE2_DEBUG_UNREACHABLE();
return FALSE;
}
int
PRIV(auto_possessify)(PCRE2_UCHAR *code, const compile_block *cb)
{
PCRE2_UCHAR c;
PCRE2_SPTR end;
PCRE2_UCHAR *repeat_opcode;
uint32_t list[MAX_LIST];
int rec_limit = 1000;
BOOL utf = (cb->external_options & PCRE2_UTF) != 0;
BOOL ucp = (cb->external_options & PCRE2_UCP) != 0;
for (;;)
  {
  c = *code;
  if (c >= OP_TABLE_LENGTH)
    {
    PCRE2_DEBUG_UNREACHABLE();
    return -1;
    }
  if (c >= OP_STAR && c <= OP_TYPEPOSUPTO)
    {
    c -= get_repeat_base(c) - OP_STAR;
    end = (c <= OP_MINUPTO) ?
      get_chr_property_list(code, utf, ucp, cb->fcc, list) : NULL;
    list[1] = c == OP_STAR || c == OP_PLUS || c == OP_QUERY || c == OP_UPTO;
    if (end != NULL && compare_opcodes(end, utf, ucp, cb, list, end,
        &rec_limit))
      {
      switch(c)
        {
        case OP_STAR:
        *code += OP_POSSTAR - OP_STAR;
        break;
        case OP_MINSTAR:
        *code += OP_POSSTAR - OP_MINSTAR;
        break;
        case OP_PLUS:
        *code += OP_POSPLUS - OP_PLUS;
        break;
        case OP_MINPLUS:
        *code += OP_POSPLUS - OP_MINPLUS;
        break;
        case OP_QUERY:
        *code += OP_POSQUERY - OP_QUERY;
        break;
        case OP_MINQUERY:
        *code += OP_POSQUERY - OP_MINQUERY;
        break;
        case OP_UPTO:
        *code += OP_POSUPTO - OP_UPTO;
        break;
        case OP_MINUPTO:
        *code += OP_POSUPTO - OP_MINUPTO;
        break;
        }
      }
    c = *code;
    }
  else if (c == OP_CLASS || c == OP_NCLASS
#ifdef SUPPORT_WIDE_CHARS
           || c == OP_XCLASS || c == OP_ECLASS
#endif
           )
    {
#ifdef SUPPORT_WIDE_CHARS
    if (c == OP_XCLASS || c == OP_ECLASS)
      repeat_opcode = code + GET(code, 1);
    else
#endif
      repeat_opcode = code + 1 + (32 / sizeof(PCRE2_UCHAR));
    c = *repeat_opcode;
    if (c >= OP_CRSTAR && c <= OP_CRMINRANGE)
      {
      end = get_chr_property_list(code, utf, ucp, cb->fcc, list);
      list[1] = (c & 1) == 0;
      if (end != NULL &&
          compare_opcodes(end, utf, ucp, cb, list, end, &rec_limit))
        {
        switch (c)
          {
          case OP_CRSTAR:
          case OP_CRMINSTAR:
          *repeat_opcode = OP_CRPOSSTAR;
          break;
          case OP_CRPLUS:
          case OP_CRMINPLUS:
          *repeat_opcode = OP_CRPOSPLUS;
          break;
          case OP_CRQUERY:
          case OP_CRMINQUERY:
          *repeat_opcode = OP_CRPOSQUERY;
          break;
          case OP_CRRANGE:
          case OP_CRMINRANGE:
          *repeat_opcode = OP_CRPOSRANGE;
          break;
          }
        }
      }
    c = *code;
    }
  switch(c)
    {
    case OP_END:
    return 0;
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
    case OP_TYPEUPTO:
    case OP_TYPEMINUPTO:
    case OP_TYPEEXACT:
    case OP_TYPEPOSUPTO:
    if (code[1 + IMM2_SIZE] == OP_PROP || code[1 + IMM2_SIZE] == OP_NOTPROP)
      code += 2;
    break;
    case OP_CALLOUT_STR:
    code += GET(code, 1 + 2*LINK_SIZE);
    break;
#ifdef SUPPORT_WIDE_CHARS
    case OP_XCLASS:
    case OP_ECLASS:
    code += GET(code, 1);
    break;
#endif
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
    case OP_STAR:
    case OP_MINSTAR:
    case OP_PLUS:
    case OP_MINPLUS:
    case OP_QUERY:
    case OP_MINQUERY:
    case OP_UPTO:
    case OP_MINUPTO:
    case OP_EXACT:
    case OP_POSSTAR:
    case OP_POSPLUS:
    case OP_POSQUERY:
    case OP_POSUPTO:
    case OP_STARI:
    case OP_MINSTARI:
    case OP_PLUSI:
    case OP_MINPLUSI:
    case OP_QUERYI:
    case OP_MINQUERYI:
    case OP_UPTOI:
    case OP_MINUPTOI:
    case OP_EXACTI:
    case OP_POSSTARI:
    case OP_POSPLUSI:
    case OP_POSQUERYI:
    case OP_POSUPTOI:
    case OP_NOTSTAR:
    case OP_NOTMINSTAR:
    case OP_NOTPLUS:
    case OP_NOTMINPLUS:
    case OP_NOTQUERY:
    case OP_NOTMINQUERY:
    case OP_NOTUPTO:
    case OP_NOTMINUPTO:
    case OP_NOTEXACT:
    case OP_NOTPOSSTAR:
    case OP_NOTPOSPLUS:
    case OP_NOTPOSQUERY:
    case OP_NOTPOSUPTO:
    case OP_NOTSTARI:
    case OP_NOTMINSTARI:
    case OP_NOTPLUSI:
    case OP_NOTMINPLUSI:
    case OP_NOTQUERYI:
    case OP_NOTMINQUERYI:
    case OP_NOTUPTOI:
    case OP_NOTMINUPTOI:
    case OP_NOTEXACTI:
    case OP_NOTPOSSTARI:
    case OP_NOTPOSPLUSI:
    case OP_NOTPOSQUERYI:
    case OP_NOTPOSUPTOI:
    if (HAS_EXTRALEN(code[-1])) code += GET_EXTRALEN(code[-1]);
    break;
    }
#else
  (void)(utf);
#endif
  }
}
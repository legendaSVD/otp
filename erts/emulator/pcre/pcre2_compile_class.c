#include "pcre2_compile.h"
typedef struct {
  uint32_t options;
  uint32_t xoptions;
  int *errorcodeptr;
  compile_block *cb;
  BOOL needs_bitmap;
} eclass_context;
#ifdef PCRE2_DEBUG
#define CLASS_END_CASES(meta) \
  default: \
  PCRE2_ASSERT((meta) <= META_END); \
  PCRE2_FALLTHROUGH  \
  case META_CLASS: \
  case META_CLASS_NOT: \
  case META_CLASS_EMPTY: \
  case META_CLASS_EMPTY_NOT: \
  case META_CLASS_END: \
  case META_ECLASS_AND: \
  case META_ECLASS_OR: \
  case META_ECLASS_SUB: \
  case META_ECLASS_XOR: \
  case META_ECLASS_NOT:
#else
#define CLASS_END_CASES(meta) \
  default:
#endif
#ifdef SUPPORT_WIDE_CHARS
static void do_heapify(uint32_t *buffer, size_t size, size_t i)
{
size_t max;
size_t left;
size_t right;
uint32_t tmp1, tmp2;
while (TRUE)
  {
  max = i;
  left = (i << 1) + 2;
  right = left + 2;
  if (left < size && buffer[left] > buffer[max]) max = left;
  if (right < size && buffer[right] > buffer[max]) max = right;
  if (i == max) return;
  tmp1 = buffer[i];
  tmp2 = buffer[i + 1];
  buffer[i] = buffer[max];
  buffer[i + 1] = buffer[max + 1];
  buffer[max] = tmp1;
  buffer[max + 1] = tmp2;
  i = max;
  }
}
#ifdef SUPPORT_UNICODE
#define PARSE_CLASS_UTF               0x1
#define PARSE_CLASS_CASELESS_UTF      0x2
#define PARSE_CLASS_RESTRICTED_UTF    0x4
#define PARSE_CLASS_TURKISH_UTF       0x8
static const uint32_t*
get_nocase_range(uint32_t c)
{
uint32_t left = 0;
uint32_t right = PRIV(ucd_nocase_ranges_size);
uint32_t middle;
if (c > MAX_UTF_CODE_POINT) return PRIV(ucd_nocase_ranges) + right;
while (TRUE)
  {
  middle = ((left + right) >> 1) | 0x1;
  if (PRIV(ucd_nocase_ranges)[middle] <= c)
    left = middle + 1;
  else if (middle > 1 && PRIV(ucd_nocase_ranges)[middle - 2] > c)
    right = middle - 1;
  else
    return PRIV(ucd_nocase_ranges) + (middle - 1);
  }
}
static size_t
utf_caseless_extend(uint32_t start, uint32_t end, uint32_t options,
  uint32_t *buffer)
{
uint32_t new_start = start;
uint32_t new_end = end;
uint32_t c = start;
const uint32_t *list;
uint32_t tmp[3];
size_t result = 2;
const uint32_t *skip_range = get_nocase_range(c);
uint32_t skip_start = skip_range[0];
#if PCRE2_CODE_UNIT_WIDTH == 8
PCRE2_ASSERT(options & PARSE_CLASS_UTF);
#endif
#if PCRE2_CODE_UNIT_WIDTH == 32
if (end > MAX_UTF_CODE_POINT) end = MAX_UTF_CODE_POINT;
#endif
while (c <= end)
  {
  uint32_t co;
  if (c > skip_start)
    {
    c = skip_range[1];
    skip_range += 2;
    skip_start = skip_range[0];
    continue;
    }
  if ((options & (PARSE_CLASS_TURKISH_UTF|PARSE_CLASS_RESTRICTED_UTF)) ==
        PARSE_CLASS_TURKISH_UTF &&
      UCD_ANY_I(c))
    {
    co = PRIV(ucd_turkish_dotted_i_caseset) + (UCD_DOTTED_I(c)? 0 : 3);
    }
  else if ((co = UCD_CASESET(c)) != 0 &&
           (options & PARSE_CLASS_RESTRICTED_UTF) != 0 &&
           PRIV(ucd_caseless_sets)[co] < 128)
    {
    co = 0;
    }
  if (co != 0)
    list = PRIV(ucd_caseless_sets) + co;
  else
    {
    co = UCD_OTHERCASE(c);
    list = tmp;
    tmp[0] = c;
    tmp[1] = NOTACHAR;
    if (co != c)
      {
      tmp[1] = co;
      tmp[2] = NOTACHAR;
      }
    }
  c++;
  do
    {
#if PCRE2_CODE_UNIT_WIDTH == 16
    if (!(options & PARSE_CLASS_UTF) && *list > 0xffff) continue;
#endif
    if (*list < new_start)
      {
      if (*list + 1 == new_start)
        {
        new_start--;
        continue;
        }
      }
    else if (*list > new_end)
      {
      if (*list - 1 == new_end)
        {
        new_end++;
        continue;
        }
      }
    else continue;
    result += 2;
    if (buffer != NULL)
      {
      buffer[0] = *list;
      buffer[1] = *list;
      buffer += 2;
      }
    }
  while (*(++list) != NOTACHAR);
  }
  if (buffer != NULL)
    {
    buffer[0] = new_start;
    buffer[1] = new_end;
    buffer += 2;
    (void)buffer;
    }
  return result;
}
#endif
static size_t
append_char_list(const uint32_t *p, uint32_t *buffer)
{
const uint32_t *n;
size_t result = 0;
while (*p != NOTACHAR)
  {
  n = p;
  while (n[0] == n[1] - 1) n++;
  PCRE2_ASSERT(*p < 0xffff);
  if (buffer != NULL)
    {
    buffer[0] = *p;
    buffer[1] = *n;
    buffer += 2;
    }
  result += 2;
  p = n + 1;
  }
  return result;
}
static uint32_t
get_highest_char(uint32_t options)
{
(void)options;
#if PCRE2_CODE_UNIT_WIDTH == 8
return MAX_UTF_CODE_POINT;
#else
#ifdef SUPPORT_UNICODE
return GET_MAX_CHAR_VALUE((options & PARSE_CLASS_UTF) != 0);
#else
return MAX_UCHAR_VALUE;
#endif
#endif
}
static size_t
append_negated_char_list(const uint32_t *p, uint32_t options, uint32_t *buffer)
{
const uint32_t *n;
uint32_t start = 0;
size_t result = 2;
PCRE2_ASSERT(*p > 0);
while (*p != NOTACHAR)
  {
  n = p;
  while (n[0] == n[1] - 1) n++;
  PCRE2_ASSERT(*p < 0xffff);
  if (buffer != NULL)
    {
    buffer[0] = start;
    buffer[1] = *p - 1;
    buffer += 2;
    }
  result += 2;
  start = *n + 1;
  p = n + 1;
  }
  if (buffer != NULL)
    {
    buffer[0] = start;
    buffer[1] = get_highest_char(options);
    buffer += 2;
    (void)buffer;
    }
  return result;
}
static uint32_t *
append_non_ascii_range(uint32_t options, uint32_t *buffer)
{
  if (buffer == NULL) return NULL;
  buffer[0] = 0x100;
  buffer[1] = get_highest_char(options);
  return buffer + 2;
}
static size_t
parse_class(uint32_t *ptr, uint32_t options, uint32_t *buffer)
{
size_t total_size = 0;
size_t size;
uint32_t meta_arg;
uint32_t start_char;
while (TRUE)
  {
  switch (META_CODE(*ptr))
    {
    case META_ESCAPE:
      meta_arg = META_DATA(*ptr);
      switch (meta_arg)
        {
        case ESC_D:
        case ESC_W:
        case ESC_S:
        buffer = append_non_ascii_range(options, buffer);
        total_size += 2;
        break;
        case ESC_h:
        size = append_char_list(PRIV(hspace_list), buffer);
        total_size += size;
        if (buffer != NULL) buffer += size;
        break;
        case ESC_H:
        size = append_negated_char_list(PRIV(hspace_list), options, buffer);
        total_size += size;
        if (buffer != NULL) buffer += size;
        break;
        case ESC_v:
        size = append_char_list(PRIV(vspace_list), buffer);
        total_size += size;
        if (buffer != NULL) buffer += size;
        break;
        case ESC_V:
        size = append_negated_char_list(PRIV(vspace_list), options, buffer);
        total_size += size;
        if (buffer != NULL) buffer += size;
        break;
        case ESC_p:
        case ESC_P:
        ptr++;
        if (meta_arg == ESC_p && (*ptr >> 16) == PT_ANY)
          {
          if (buffer != NULL)
            {
            buffer[0] = 0;
            buffer[1] = get_highest_char(options);
            buffer += 2;
            }
          total_size += 2;
          }
        break;
        }
      ptr++;
      continue;
    case META_POSIX_NEG:
      buffer = append_non_ascii_range(options, buffer);
      total_size += 2;
      ptr += 2;
      continue;
    case META_POSIX:
      ptr += 2;
      continue;
    case META_BIGVALUE:
      ptr++;
      break;
    CLASS_END_CASES(*ptr)
      if (*ptr >= META_END) return total_size;
      break;
    }
    start_char = *ptr;
    if (ptr[1] == META_RANGE_LITERAL || ptr[1] == META_RANGE_ESCAPED)
      {
      ptr += 2;
      PCRE2_ASSERT(*ptr < META_END || *ptr == META_BIGVALUE);
      if (*ptr == META_BIGVALUE) ptr++;
#ifdef EBCDIC
#error "Missing EBCDIC support"
#endif
      }
#ifdef SUPPORT_UNICODE
    if (options & PARSE_CLASS_CASELESS_UTF)
      {
      size = utf_caseless_extend(start_char, *ptr++, options, buffer);
      if (buffer != NULL) buffer += size;
      total_size += size;
      continue;
      }
#endif
    if (buffer != NULL)
      {
      buffer[0] = start_char;
      buffer[1] = *ptr;
      buffer += 2;
      }
    ptr++;
    total_size += 2;
  }
  return total_size;
}
#define CHAR_LIST_EXTRA_SIZE 3
static const uint32_t char_list_starts[] = {
#if PCRE2_CODE_UNIT_WIDTH == 32
  XCL_CHAR_LIST_HIGH_32_START,
#endif
#if PCRE2_CODE_UNIT_WIDTH == 32 || defined SUPPORT_UNICODE
  XCL_CHAR_LIST_LOW_32_START,
#endif
  XCL_CHAR_LIST_HIGH_16_START,
  XCL_CHAR_LIST_LOW_16_START,
};
static class_ranges *
compile_optimize_class(uint32_t *start_ptr, uint32_t options,
  uint32_t xoptions, compile_block *cb)
{
class_ranges* cranges;
uint32_t *ptr;
uint32_t *buffer;
uint32_t *dst;
uint32_t class_options = 0;
size_t range_list_size = 0, total_size, i;
uint32_t tmp1, tmp2;
const uint32_t *char_list_next;
uint16_t *next_char;
uint32_t char_list_start, char_list_end;
uint32_t range_start, range_end;
#ifdef SUPPORT_UNICODE
if (options & PCRE2_UTF)
  class_options |= PARSE_CLASS_UTF;
if ((options & PCRE2_CASELESS) && (options & (PCRE2_UTF|PCRE2_UCP)))
  class_options |= PARSE_CLASS_CASELESS_UTF;
if (xoptions & PCRE2_EXTRA_CASELESS_RESTRICT)
  class_options |= PARSE_CLASS_RESTRICTED_UTF;
if (xoptions & PCRE2_EXTRA_TURKISH_CASING)
  class_options |= PARSE_CLASS_TURKISH_UTF;
#else
(void)options;
(void)xoptions;
#endif
range_list_size = parse_class(start_ptr, class_options, NULL);
PCRE2_ASSERT((range_list_size & 0x1) == 0);
total_size = range_list_size +
   ((range_list_size >= 2) ? CHAR_LIST_EXTRA_SIZE : 0);
cranges = cb->cx->memctl.malloc(
  sizeof(class_ranges) + total_size * sizeof(uint32_t),
  cb->cx->memctl.memory_data);
if (cranges == NULL) return NULL;
cranges->header.next = NULL;
#ifdef PCRE2_DEBUG
cranges->header.type = CDATA_CRANGE;
#endif
cranges->range_list_size = (uint16_t)range_list_size;
cranges->char_lists_types = 0;
cranges->char_lists_size = 0;
cranges->char_lists_start = 0;
if (range_list_size == 0) return cranges;
buffer = (uint32_t*)(cranges + 1);
parse_class(start_ptr, class_options, buffer);
if (range_list_size <= 2) return cranges;
i = (((range_list_size >> 2) - 1) << 1);
while (TRUE)
  {
  do_heapify(buffer, range_list_size, i);
  if (i == 0) break;
  i -= 2;
  }
i = range_list_size - 2;
while (TRUE)
  {
  tmp1 = buffer[i];
  tmp2 = buffer[i + 1];
  buffer[i] = buffer[0];
  buffer[i + 1] = buffer[1];
  buffer[0] = tmp1;
  buffer[1] = tmp2;
  do_heapify(buffer, i, 0);
  if (i == 0) break;
  i -= 2;
  }
dst = buffer;
ptr = buffer + 2;
range_list_size -= 2;
while (range_list_size > 0 && dst[1] != ~(uint32_t)0)
  {
  if (dst[1] + 1 < ptr[0])
    {
    dst += 2;
    dst[0] = ptr[0];
    dst[1] = ptr[1];
    }
  else if (dst[1] < ptr[1]) dst[1] = ptr[1];
  ptr += 2;
  range_list_size -= 2;
  }
PCRE2_ASSERT(dst[1] <= get_highest_char(class_options));
ptr = buffer;
while (ptr < dst && ptr[1] < 0x100) ptr += 2;
if (dst - ptr < (2 * (6 - 1)))
  {
  cranges->range_list_size = (uint16_t)(dst + 2 - buffer);
  return cranges;
  }
char_list_next = char_list_starts;
char_list_start = *char_list_next++;
#if PCRE2_CODE_UNIT_WIDTH == 32
char_list_end = XCL_CHAR_LIST_HIGH_32_END;
#elif defined SUPPORT_UNICODE
char_list_end = XCL_CHAR_LIST_LOW_32_END;
#else
char_list_end = XCL_CHAR_LIST_HIGH_16_END;
#endif
next_char = (uint16_t*)(buffer + total_size);
tmp1 = 0;
tmp2 = ((sizeof(char_list_starts) / sizeof(uint32_t)) - 1) * XCL_TYPE_BIT_LEN;
PCRE2_ASSERT(tmp2 <= 3 * XCL_TYPE_BIT_LEN && tmp2 >= XCL_TYPE_BIT_LEN);
range_start = dst[0];
range_end = dst[1];
while (TRUE)
  {
  if (range_start >= char_list_start)
    {
    if (range_start == range_end || range_end < char_list_end)
      {
      tmp1++;
      next_char--;
      if (char_list_start < XCL_CHAR_LIST_LOW_32_START)
        *next_char = (uint16_t)((range_end << XCL_CHAR_SHIFT) | XCL_CHAR_END);
      else
        *(uint32_t*)(--next_char) =
          (range_end << XCL_CHAR_SHIFT) | XCL_CHAR_END;
      }
    if (range_start < range_end)
      {
      if (range_start > char_list_start)
        {
        tmp1++;
        next_char--;
        if (char_list_start < XCL_CHAR_LIST_LOW_32_START)
          *next_char = (uint16_t)(range_start << XCL_CHAR_SHIFT);
        else
          *(uint32_t*)(--next_char) = (range_start << XCL_CHAR_SHIFT);
        }
      else
        cranges->char_lists_types |= XCL_BEGIN_WITH_RANGE << tmp2;
      }
    PCRE2_ASSERT((uint32_t*)next_char >= dst + 2);
    if (dst > buffer)
      {
      dst -= 2;
      range_start = dst[0];
      range_end = dst[1];
      continue;
      }
    range_start = 0;
    range_end = 0;
    }
  if (range_end >= char_list_start)
    {
    PCRE2_ASSERT(range_start < char_list_start);
    if (range_end < char_list_end)
      {
      tmp1++;
      next_char--;
      if (char_list_start < XCL_CHAR_LIST_LOW_32_START)
        *next_char = (uint16_t)((range_end << XCL_CHAR_SHIFT) | XCL_CHAR_END);
      else
        *(uint32_t*)(--next_char) =
          (range_end << XCL_CHAR_SHIFT) | XCL_CHAR_END;
      PCRE2_ASSERT((uint32_t*)next_char >= dst + 2);
      }
    cranges->char_lists_types |= XCL_BEGIN_WITH_RANGE << tmp2;
    }
  if (tmp1 >= XCL_ITEM_COUNT_MASK)
    {
    cranges->char_lists_types |= XCL_ITEM_COUNT_MASK << tmp2;
    next_char--;
    if (char_list_start < XCL_CHAR_LIST_LOW_32_START)
      *next_char = (uint16_t)tmp1;
    else
      *(uint32_t*)(--next_char) = tmp1;
    }
  else
    cranges->char_lists_types |= tmp1 << tmp2;
  if (range_start < XCL_CHAR_LIST_LOW_16_START) break;
  PCRE2_ASSERT(tmp2 >= XCL_TYPE_BIT_LEN);
  char_list_end = char_list_start - 1;
  char_list_start = *char_list_next++;
  tmp1 = 0;
  tmp2 -= XCL_TYPE_BIT_LEN;
  }
if (dst[0] < XCL_CHAR_LIST_LOW_16_START) dst += 2;
PCRE2_ASSERT((uint16_t*)dst <= next_char);
cranges->char_lists_size =
  (size_t)((uint8_t*)(buffer + total_size) - (uint8_t*)next_char);
cranges->char_lists_start = (size_t)((uint8_t*)next_char - (uint8_t*)buffer);
cranges->range_list_size = (uint16_t)(dst - buffer);
return cranges;
}
#endif
#ifdef SUPPORT_UNICODE
void PRIV(update_classbits)(uint32_t ptype, uint32_t pdata, BOOL negated,
  uint8_t *classbits)
{
int c, chartype;
const ucd_record *prop;
uint32_t gentype;
BOOL set_bit;
if (ptype == PT_ANY)
  {
  if (!negated) memset(classbits, 0xff, 32);
  return;
  }
for (c = 0; c < 256; c++)
  {
  prop = GET_UCD(c);
  set_bit = FALSE;
  (void)set_bit;
  switch (ptype)
    {
    case PT_LAMP:
    chartype = prop->chartype;
    set_bit = (chartype == ucp_Lu || chartype == ucp_Ll || chartype == ucp_Lt);
    break;
    case PT_GC:
    set_bit = (PRIV(ucp_gentype)[prop->chartype] == pdata);
    break;
    case PT_PC:
    set_bit = (prop->chartype == pdata);
    break;
    case PT_SC:
    set_bit = (prop->script == pdata);
    break;
    case PT_SCX:
    set_bit = (prop->script == pdata ||
      MAPBIT(PRIV(ucd_script_sets) + UCD_SCRIPTX_PROP(prop), pdata) != 0);
    break;
    case PT_ALNUM:
    gentype = PRIV(ucp_gentype)[prop->chartype];
    set_bit = (gentype == ucp_L || gentype == ucp_N);
    break;
    case PT_SPACE:
    case PT_PXSPACE:
    switch(c)
      {
      HSPACE_BYTE_CASES:
      VSPACE_BYTE_CASES:
      set_bit = TRUE;
      break;
      default:
      set_bit = (PRIV(ucp_gentype)[prop->chartype] == ucp_Z);
      break;
      }
    break;
    case PT_WORD:
    chartype = prop->chartype;
    gentype = PRIV(ucp_gentype)[chartype];
    set_bit = (gentype == ucp_L || gentype == ucp_N ||
               chartype == ucp_Mn || chartype == ucp_Pc);
    break;
    case PT_UCNC:
    set_bit = (c == CHAR_DOLLAR_SIGN || c == CHAR_COMMERCIAL_AT ||
               c == CHAR_GRAVE_ACCENT || c >= 0xa0);
    break;
    case PT_BIDICL:
    set_bit = (UCD_BIDICLASS_PROP(prop) == pdata);
    break;
    case PT_BOOL:
    set_bit = MAPBIT(PRIV(ucd_boolprop_sets) +
                     UCD_BPROPS_PROP(prop), pdata) != 0;
    break;
    case PT_PXGRAPH:
    chartype = prop->chartype;
    gentype = PRIV(ucp_gentype)[chartype];
    set_bit = (gentype != ucp_Z && (gentype != ucp_C || chartype == ucp_Cf));
    break;
    case PT_PXPRINT:
    chartype = prop->chartype;
    set_bit = (chartype != ucp_Zl && chartype != ucp_Zp &&
       (PRIV(ucp_gentype)[chartype] != ucp_C || chartype == ucp_Cf));
    break;
    case PT_PXPUNCT:
    gentype = PRIV(ucp_gentype)[prop->chartype];
    set_bit = (gentype == ucp_P || (c < 128 && gentype == ucp_S));
    break;
    default:
    PCRE2_ASSERT(ptype == PT_PXXDIGIT);
    set_bit = (c >= CHAR_0 && c <= CHAR_9) ||
              (c >= CHAR_A && c <= CHAR_F) ||
              (c >= CHAR_a && c <= CHAR_f);
    break;
    }
  if (negated) set_bit = !set_bit;
  if (set_bit) *classbits |= (uint8_t)(1 << (c & 0x7));
  if ((c & 0x7) == 0x7) classbits++;
  }
}
#endif
#ifdef SUPPORT_WIDE_CHARS
#define XCLASS_REQUIRED 0x1
#define XCLASS_HAS_8BIT_CHARS 0x2
#define XCLASS_HAS_PROPS 0x4
#define XCLASS_HAS_CHAR_LISTS 0x8
#define XCLASS_HIGH_ANY 0x10
#endif
static void
add_to_class(uint32_t options, uint32_t xoptions, compile_block *cb,
  uint32_t start, uint32_t end)
{
uint8_t *classbits = cb->classbits.classbits;
uint32_t c, byte_start, byte_end;
uint32_t classbits_end = (end <= 0xff ? end : 0xff);
#ifndef SUPPORT_UNICODE
(void)xoptions;
#endif
if ((options & PCRE2_CASELESS) != 0)
  {
#ifdef SUPPORT_UNICODE
  if ((options & (PCRE2_UTF|PCRE2_UCP)) != 0)
    {
      BOOL turkish_i = (xoptions & (PCRE2_EXTRA_TURKISH_CASING|PCRE2_EXTRA_CASELESS_RESTRICT)) ==
        PCRE2_EXTRA_TURKISH_CASING;
      if (start < 128)
        {
        uint32_t lo_end = (classbits_end < 127 ? classbits_end : 127);
        for (c = start; c <= lo_end; c++)
          {
          if (turkish_i && UCD_ANY_I(c)) continue;
          SETBIT(classbits, cb->fcc[c]);
          }
        }
      if (classbits_end >= 128)
        {
        uint32_t hi_start = (start > 128 ? start : 128);
        for (c = hi_start; c <= classbits_end; c++)
          {
          uint32_t co = UCD_OTHERCASE(c);
          if (co <= 0xff) SETBIT(classbits, co);
          }
        }
    }
  else
#endif
    {
    for (c = start; c <= classbits_end; c++)
      SETBIT(classbits, cb->fcc[c]);
    }
  }
byte_start = (start + 7) >> 3;
byte_end = (classbits_end + 1) >> 3;
if (byte_start >= byte_end)
  {
  for (c = start; c <= classbits_end; c++)
    SETBIT(classbits, c);
  return;
  }
for (c = byte_start; c < byte_end; c++)
  classbits[c] = 0xff;
byte_start <<= 3;
byte_end <<= 3;
for (c = start; c < byte_start; c++)
  SETBIT(classbits, c);
for (c = byte_end; c <= classbits_end; c++)
  SETBIT(classbits, c);
}
#if PCRE2_CODE_UNIT_WIDTH == 8
static void
add_list_to_class(uint32_t options, uint32_t xoptions, compile_block *cb,
  const uint32_t *p)
{
while (p[0] < 256)
  {
  unsigned int n = 0;
  while(p[n+1] == p[0] + n + 1) n++;
  add_to_class(options, xoptions, cb, p[0], p[n]);
  p += n + 1;
  }
}
static void
add_not_list_to_class(uint32_t options, uint32_t xoptions, compile_block *cb,
  const uint32_t *p)
{
if (p[0] > 0)
  add_to_class(options, xoptions, cb, 0, p[0] - 1);
while (p[0] < 256)
  {
  while (p[1] == p[0] + 1) p++;
  add_to_class(options, xoptions, cb, p[0] + 1, (p[1] > 255) ? 255 : p[1] - 1);
  p++;
  }
}
#endif
uint32_t *
PRIV(compile_class_not_nested)(uint32_t options, uint32_t xoptions,
  uint32_t *start_ptr, PCRE2_UCHAR **pcode, BOOL negate_class, BOOL* has_bitmap,
  int *errorcodeptr, compile_block *cb, PCRE2_SIZE *lengthptr)
{
uint32_t *pptr = start_ptr;
PCRE2_UCHAR *code = *pcode;
BOOL should_flip_negation;
const uint8_t *cbits = cb->cbits;
uint8_t *const classbits = cb->classbits.classbits;
#ifdef SUPPORT_UNICODE
BOOL utf = (options & PCRE2_UTF) != 0;
#else
BOOL utf = FALSE;
#endif
#ifdef SUPPORT_WIDE_CHARS
uint32_t xclass_props;
PCRE2_UCHAR *class_uchardata;
class_ranges* cranges;
#else
(void)has_bitmap;
(void)errorcodeptr;
(void)lengthptr;
#endif
should_flip_negation = FALSE;
#ifdef SUPPORT_WIDE_CHARS
xclass_props = 0;
#if PCRE2_CODE_UNIT_WIDTH == 8
cranges = NULL;
if (utf)
#endif
  {
  if (lengthptr != NULL)
    {
    cranges = compile_optimize_class(pptr, options, xoptions, cb);
    if (cranges == NULL)
      {
      *errorcodeptr = ERR21;
      return NULL;
      }
    if (cb->last_data != NULL)
      cb->last_data->next = &cranges->header;
    else
      cb->first_data = &cranges->header;
    cb->last_data = &cranges->header;
    }
  else
    {
    cranges = (class_ranges*)cb->first_data;
    PCRE2_ASSERT(cranges != NULL && cranges->header.type == CDATA_CRANGE);
    cb->first_data = cranges->header.next;
    }
  if (cranges->range_list_size > 0)
    {
    const uint32_t *ranges = (const uint32_t*)(cranges + 1);
    if (ranges[0] <= 255)
      xclass_props |= XCLASS_HAS_8BIT_CHARS;
    if (ranges[cranges->range_list_size - 1] == GET_MAX_CHAR_VALUE(utf) &&
        ranges[cranges->range_list_size - 2] <= 256)
      xclass_props |= XCLASS_HIGH_ANY;
    }
  }
class_uchardata = code + LINK_SIZE + 2;
#endif
memset(classbits, 0, 32);
while (TRUE)
  {
  uint32_t meta = *(pptr++);
  BOOL local_negate;
  int posix_class;
  int taboffset, tabopt;
  class_bits_storage pbits;
  uint32_t escape, c;
  switch (META_CODE(meta))
    {
    case META_POSIX:
    case META_POSIX_NEG:
    local_negate = (meta == META_POSIX_NEG);
    posix_class = *(pptr++);
    if (local_negate) should_flip_negation = TRUE;
    if ((options & PCRE2_CASELESS) != 0 && posix_class <= 2)
      posix_class = 0;
#ifdef SUPPORT_UNICODE
    if ((options & PCRE2_UCP) != 0 &&
        (xoptions & PCRE2_EXTRA_ASCII_POSIX) == 0)
      {
      uint32_t ptype;
      switch(posix_class)
        {
        case PC_GRAPH:
        case PC_PRINT:
        case PC_PUNCT:
        ptype = (posix_class == PC_GRAPH)? PT_PXGRAPH :
                (posix_class == PC_PRINT)? PT_PXPRINT : PT_PXPUNCT;
        PRIV(update_classbits)(ptype, 0, local_negate, classbits);
        if ((xclass_props & XCLASS_HIGH_ANY) == 0)
          {
          if (lengthptr != NULL)
            *lengthptr += 3;
          else
            {
            *class_uchardata++ = local_negate? XCL_NOTPROP : XCL_PROP;
            *class_uchardata++ = (PCRE2_UCHAR)ptype;
            *class_uchardata++ = 0;
            }
          xclass_props |= XCLASS_REQUIRED | XCLASS_HAS_PROPS;
          }
        continue;
        default:
        break;
        }
      }
#endif
    posix_class *= 3;
    memcpy(pbits.classbits, cbits + PRIV(posix_class_maps)[posix_class], 32);
    taboffset = PRIV(posix_class_maps)[posix_class + 1];
    tabopt = PRIV(posix_class_maps)[posix_class + 2];
    if (taboffset >= 0)
      {
      if (tabopt >= 0)
        for (int i = 0; i < 32; i++)
          pbits.classbits[i] |= cbits[i + taboffset];
      else
        for (int i = 0; i < 32; i++)
          pbits.classbits[i] &= (uint8_t)(~cbits[i + taboffset]);
      }
    if (tabopt < 0) tabopt = -tabopt;
#ifdef EBCDIC
      {
      uint8_t posix_vertical[4] = { CHAR_LF, CHAR_VT, CHAR_FF, CHAR_CR };
      uint8_t posix_underscore = CHAR_UNDERSCORE;
      uint8_t *chars = NULL;
      int n = 0;
      if (tabopt == 1) { chars = posix_vertical; n = 4; }
      else if (tabopt == 2) { chars = &posix_underscore; n = 1; }
      for (; n > 0; ++chars, --n)
        pbits.classbits[*chars/8] &= ~(1u << (*chars&7));
      }
#else
    if (tabopt == 1) pbits.classbits[1] &= ~0x3c;
    else if (tabopt == 2) pbits.classbits[11] &= 0x7f;
#endif
      {
      uint32_t *classwords = cb->classbits.classwords;
      if (local_negate)
        for (int i = 0; i < 8; i++)
          classwords[i] |= (uint32_t)(~pbits.classwords[i]);
      else
        for (int i = 0; i < 8; i++)
          classwords[i] |= pbits.classwords[i];
      }
#ifdef SUPPORT_WIDE_CHARS
    xclass_props |= XCLASS_HAS_8BIT_CHARS;
#endif
    continue;
    case META_BIGVALUE:
    meta = *(pptr++);
    break;
    case META_ESCAPE:
    escape = META_DATA(meta);
    switch(escape)
      {
      case ESC_d:
      for (int i = 0; i < 32; i++) classbits[i] |= cbits[i+cbit_digit];
      break;
      case ESC_D:
      should_flip_negation = TRUE;
      for (int i = 0; i < 32; i++)
        classbits[i] |= (uint8_t)(~cbits[i+cbit_digit]);
      break;
      case ESC_w:
      for (int i = 0; i < 32; i++) classbits[i] |= cbits[i+cbit_word];
      break;
      case ESC_W:
      should_flip_negation = TRUE;
      for (int i = 0; i < 32; i++)
        classbits[i] |= (uint8_t)(~cbits[i+cbit_word]);
      break;
      case ESC_s:
      for (int i = 0; i < 32; i++) classbits[i] |= cbits[i+cbit_space];
      break;
      case ESC_S:
      should_flip_negation = TRUE;
      for (int i = 0; i < 32; i++)
        classbits[i] |= (uint8_t)(~cbits[i+cbit_space]);
      break;
      case ESC_h:
#if PCRE2_CODE_UNIT_WIDTH == 8
#ifdef SUPPORT_UNICODE
      if (cranges != NULL) break;
#endif
      add_list_to_class(options & ~PCRE2_CASELESS, xoptions,
        cb, PRIV(hspace_list));
#else
      PCRE2_ASSERT(cranges != NULL);
#endif
      break;
      case ESC_H:
#if PCRE2_CODE_UNIT_WIDTH == 8
#ifdef SUPPORT_UNICODE
      if (cranges != NULL) break;
#endif
      add_not_list_to_class(options & ~PCRE2_CASELESS, xoptions,
        cb, PRIV(hspace_list));
#else
      PCRE2_ASSERT(cranges != NULL);
#endif
      break;
      case ESC_v:
#if PCRE2_CODE_UNIT_WIDTH == 8
#ifdef SUPPORT_UNICODE
      if (cranges != NULL) break;
#endif
      add_list_to_class(options & ~PCRE2_CASELESS, xoptions,
        cb, PRIV(vspace_list));
#else
      PCRE2_ASSERT(cranges != NULL);
#endif
      break;
      case ESC_V:
#if PCRE2_CODE_UNIT_WIDTH == 8
#ifdef SUPPORT_UNICODE
      if (cranges != NULL) break;
#endif
      add_not_list_to_class(options & ~PCRE2_CASELESS, xoptions,
        cb, PRIV(vspace_list));
#else
      PCRE2_ASSERT(cranges != NULL);
#endif
      break;
#ifdef SUPPORT_UNICODE
      case ESC_p:
      case ESC_P:
        {
        uint32_t ptype = *pptr >> 16;
        uint32_t pdata = *(pptr++) & 0xffff;
        if (ptype == PT_ANY)
          {
#if PCRE2_CODE_UNIT_WIDTH == 8
          if (!utf && escape == ESC_p) memset(classbits, 0xff, 32);
#endif
          continue;
          }
        PRIV(update_classbits)(ptype, pdata, (escape == ESC_P), classbits);
        if ((xclass_props & XCLASS_HIGH_ANY) == 0)
          {
          if (lengthptr != NULL)
            *lengthptr += 3;
          else
            {
            *class_uchardata++ = (escape == ESC_p)? XCL_PROP : XCL_NOTPROP;
            *class_uchardata++ = ptype;
            *class_uchardata++ = pdata;
            }
          xclass_props |= XCLASS_REQUIRED | XCLASS_HAS_PROPS;
          }
        }
      continue;
#endif
      }
#ifdef SUPPORT_WIDE_CHARS
    xclass_props |= XCLASS_HAS_8BIT_CHARS;
#endif
    continue;
    CLASS_END_CASES(meta)
    if (meta < META_END) break;
    goto END_PROCESSING;
    }
  c = meta;
  if (c == CHAR_CR || c == CHAR_NL) cb->external_flags |= PCRE2_HASCRORLF;
  if (*pptr == META_RANGE_LITERAL || *pptr == META_RANGE_ESCAPED)
    {
    uint32_t d;
#ifdef EBCDIC
    BOOL range_is_literal = (*pptr == META_RANGE_LITERAL);
#endif
    ++pptr;
    d = *(pptr++);
    if (d == META_BIGVALUE) d = *(pptr++);
    if (d == CHAR_CR || d == CHAR_NL) cb->external_flags |= PCRE2_HASCRORLF;
#if PCRE2_CODE_UNIT_WIDTH == 8
#ifdef SUPPORT_UNICODE
    if (cranges != NULL) continue;
    xclass_props |= XCLASS_HAS_8BIT_CHARS;
#endif
#ifdef EBCDIC
    if (range_is_literal &&
         (cb->ctypes[c] & ctype_letter) != 0 &&
         (cb->ctypes[d] & ctype_letter) != 0 &&
         (c <= CHAR_z) == (d <= CHAR_z))
      {
      uint32_t uc = (d <= CHAR_z)? 0 : 64;
      uint32_t C = c - uc;
      uint32_t D = d - uc;
      if (C <= CHAR_i)
        {
        add_to_class(options, xoptions, cb, C + uc,
          ((D < CHAR_i)? D : CHAR_i) + uc);
        C = CHAR_j;
        }
      if (C <= D && C <= CHAR_r)
        {
        add_to_class(options, xoptions, cb, C + uc,
          ((D < CHAR_r)? D : CHAR_r) + uc);
        C = CHAR_s;
        }
      if (C <= D)
        add_to_class(options, xoptions, cb, C + uc, D + uc);
      }
    else
#endif
    add_to_class(options, xoptions, cb, c, d);
#else
    PCRE2_ASSERT(cranges != NULL);
#endif
    continue;
    }
#if PCRE2_CODE_UNIT_WIDTH == 8
#ifdef SUPPORT_UNICODE
  if (cranges != NULL) continue;
  xclass_props |= XCLASS_HAS_8BIT_CHARS;
#endif
  add_to_class(options, xoptions, cb, meta, meta);
#else
  PCRE2_ASSERT(cranges != NULL);
#endif
  }
END_PROCESSING:
#ifdef SUPPORT_WIDE_CHARS
PCRE2_ASSERT((xclass_props & XCLASS_HAS_PROPS) == 0 ||
             (xclass_props & XCLASS_HIGH_ANY) == 0);
if (cranges != NULL)
  {
  uint32_t *range = (uint32_t*)(cranges + 1);
  uint32_t *end = range + cranges->range_list_size;
  while (range < end && range[0] < 256)
    {
    PCRE2_ASSERT((xclass_props & XCLASS_HAS_8BIT_CHARS) != 0);
    add_to_class(((options & (PCRE2_UTF|PCRE2_UCP)) != 0)?
        (options & ~PCRE2_CASELESS) : options, xoptions, cb, range[0], range[1]);
    if (range[1] > 255) break;
    range += 2;
    }
  if (cranges->char_lists_size > 0)
    {
    PCRE2_ASSERT((xclass_props & XCLASS_HIGH_ANY) == 0);
    xclass_props |= XCLASS_REQUIRED | XCLASS_HAS_CHAR_LISTS;
    }
  else
    {
    if ((xclass_props & XCLASS_HIGH_ANY) != 0)
      {
      PCRE2_ASSERT(range + 2 == end && range[0] <= 256 &&
        range[1] >= GET_MAX_CHAR_VALUE(utf));
      should_flip_negation = TRUE;
      range = end;
      }
    while (range < end)
      {
      uint32_t range_start = range[0];
      uint32_t range_end = range[1];
      range += 2;
      xclass_props |= XCLASS_REQUIRED;
      if (range_start < 256) range_start = 256;
      if (lengthptr != NULL)
        {
#ifdef SUPPORT_UNICODE
        if (utf)
          {
          *lengthptr += 1;
          if (range_start < range_end)
            *lengthptr += PRIV(ord2utf)(range_start, class_uchardata);
          *lengthptr += PRIV(ord2utf)(range_end, class_uchardata);
          continue;
          }
#endif
        *lengthptr += range_start < range_end ? 3 : 2;
        continue;
        }
#ifdef SUPPORT_UNICODE
      if (utf)
        {
        if (range_start < range_end)
          {
          *class_uchardata++ = XCL_RANGE;
          class_uchardata += PRIV(ord2utf)(range_start, class_uchardata);
          }
        else
          *class_uchardata++ = XCL_SINGLE;
        class_uchardata += PRIV(ord2utf)(range_end, class_uchardata);
        continue;
        }
#endif
#if PCRE2_CODE_UNIT_WIDTH != 8
      if (range_start < range_end)
        {
        *class_uchardata++ = XCL_RANGE;
        *class_uchardata++ = range_start;
        }
      else
        *class_uchardata++ = XCL_SINGLE;
      *class_uchardata++ = range_end;
#endif
      }
    if (lengthptr == NULL)
      cb->cx->memctl.free(cranges, cb->cx->memctl.memory_data);
    }
  }
#endif
#ifdef SUPPORT_WIDE_CHARS
if ((xclass_props & XCLASS_REQUIRED) != 0)
  {
  PCRE2_UCHAR *previous = code;
  if ((xclass_props & XCLASS_HAS_CHAR_LISTS) == 0)
    *class_uchardata++ = XCL_END;
  *code++ = OP_XCLASS;
  code += LINK_SIZE;
  *code = negate_class? XCL_NOT:0;
  if ((xclass_props & XCLASS_HAS_PROPS) != 0) *code |= XCL_HASPROP;
  if ((xclass_props & XCLASS_HAS_8BIT_CHARS) != 0 || has_bitmap != NULL)
    {
    if (negate_class)
      {
      uint32_t *classwords = cb->classbits.classwords;
      for (int i = 0; i < 8; i++) classwords[i] = ~classwords[i];
      }
    if (has_bitmap == NULL)
      {
      *code++ |= XCL_MAP;
      (void)memmove(code + (32 / sizeof(PCRE2_UCHAR)), code,
        CU2BYTES(class_uchardata - code));
      memcpy(code, classbits, 32);
      code = class_uchardata + (32 / sizeof(PCRE2_UCHAR));
      }
    else
      {
      code = class_uchardata;
      if ((xclass_props & XCLASS_HAS_8BIT_CHARS) != 0)
        *has_bitmap = TRUE;
      }
    }
  else code = class_uchardata;
  if ((xclass_props & XCLASS_HAS_CHAR_LISTS) != 0)
    {
    size_t char_lists_size = cranges->char_lists_size;
    PCRE2_ASSERT((char_lists_size & 0x1) == 0 &&
                 (cb->char_lists_size & 0x3) == 0);
    if (lengthptr != NULL)
      {
      char_lists_size = CLIST_ALIGN_TO(char_lists_size, sizeof(uint32_t));
#if PCRE2_CODE_UNIT_WIDTH == 8
      *lengthptr += 2 + LINK_SIZE;
#else
      *lengthptr += 1 + LINK_SIZE;
#endif
      cb->char_lists_size += char_lists_size;
      char_lists_size /= sizeof(PCRE2_UCHAR);
      if (*lengthptr > MAX_PATTERN_SIZE ||
          MAX_PATTERN_SIZE - *lengthptr < char_lists_size)
        {
        *errorcodeptr = ERR20;
        return NULL;
        }
      }
    else
      {
      uint8_t *data;
      PCRE2_ASSERT(cranges->char_lists_types <= XCL_TYPE_MASK);
#if PCRE2_CODE_UNIT_WIDTH == 8
      code[0] = (uint8_t)(XCL_LIST |
        (cranges->char_lists_types >> 8));
      code[1] = (uint8_t)cranges->char_lists_types;
      code += 2;
#else
      *code++ = (PCRE2_UCHAR)(XCL_LIST | cranges->char_lists_types);
#endif
      cb->char_lists_size += char_lists_size;
      data = (uint8_t*)cb->start_code - cb->char_lists_size;
      memcpy(data, (uint8_t*)(cranges + 1) + cranges->char_lists_start,
        char_lists_size);
      char_lists_size = cb->char_lists_size;
      PUT(code, 0, (uint32_t)(char_lists_size >> 1));
      code += LINK_SIZE;
      if ((char_lists_size & 0x2) != 0)
        ((uint16_t*)data)[-1] = 0xdead;
      cb->char_lists_size =
        CLIST_ALIGN_TO(char_lists_size, sizeof(uint32_t));
      cb->cx->memctl.free(cranges, cb->cx->memctl.memory_data);
      }
    }
  PUT(previous, 1, (int)(code - previous));
  goto DONE;
  }
#endif
if (negate_class)
  {
  uint32_t *classwords = cb->classbits.classwords;
  for (int i = 0; i < 8; i++) classwords[i] = ~classwords[i];
  }
if ((SELECT_VALUE8(!utf, 0) || negate_class != should_flip_negation) &&
    cb->classbits.classwords[0] == ~(uint32_t)0)
  {
  const uint32_t *classwords = cb->classbits.classwords;
  int i;
  for (i = 0; i < 8; i++)
    if (classwords[i] != ~(uint32_t)0) break;
  if (i == 8)
    {
    *code++ = OP_ALLANY;
    goto DONE;
    }
  }
*code++ = (negate_class == should_flip_negation) ? OP_CLASS : OP_NCLASS;
memcpy(code, classbits, 32);
code += 32 / sizeof(PCRE2_UCHAR);
DONE:
*pcode = code;
return pptr - 1;
}
static void
fold_negation(eclass_op_info *pop_info, PCRE2_SIZE *lengthptr,
  BOOL preserve_classbits)
{
if (pop_info->op_single_type == 0)
  {
  if (lengthptr != NULL)
    *lengthptr += 1;
  else
    pop_info->code_start[pop_info->length] = ECL_NOT;
  pop_info->length += 1;
  }
else if (pop_info->op_single_type == ECL_ANY ||
         pop_info->op_single_type == ECL_NONE)
  {
  pop_info->op_single_type = (pop_info->op_single_type == ECL_NONE)?
      ECL_ANY : ECL_NONE;
  if (lengthptr == NULL)
    *(pop_info->code_start) = pop_info->op_single_type;
  }
else
  {
  PCRE2_ASSERT(pop_info->op_single_type == ECL_XCLASS &&
               pop_info->length >= 1 + LINK_SIZE + 1);
  if (lengthptr == NULL)
    pop_info->code_start[1 + LINK_SIZE] ^= XCL_NOT;
  }
if (!preserve_classbits)
  {
  for (int i = 0; i < 8; i++)
    pop_info->bits.classwords[i] = ~pop_info->bits.classwords[i];
  }
}
static void
fold_binary(int op, eclass_op_info *lhs_op_info, eclass_op_info *rhs_op_info,
  PCRE2_SIZE *lengthptr)
{
switch (op)
  {
  case ECL_AND:
  if (rhs_op_info->op_single_type == ECL_ANY)
    {
    }
  else if (lhs_op_info->op_single_type == ECL_ANY)
    {
    if (lengthptr == NULL)
      memmove(lhs_op_info->code_start, rhs_op_info->code_start,
              CU2BYTES(rhs_op_info->length));
    lhs_op_info->length = rhs_op_info->length;
    lhs_op_info->op_single_type = rhs_op_info->op_single_type;
    }
  else if (rhs_op_info->op_single_type == ECL_NONE)
    {
    if (lengthptr == NULL)
      lhs_op_info->code_start[0] = ECL_NONE;
    lhs_op_info->length = 1;
    lhs_op_info->op_single_type = ECL_NONE;
    }
  else if (lhs_op_info->op_single_type == ECL_NONE)
    {
    }
  else
    {
    if (lengthptr != NULL)
      *lengthptr += 1;
    else
      {
      PCRE2_ASSERT(rhs_op_info->code_start ==
          lhs_op_info->code_start + lhs_op_info->length);
      rhs_op_info->code_start[rhs_op_info->length] = ECL_AND;
      }
    lhs_op_info->length += rhs_op_info->length + 1;
    lhs_op_info->op_single_type = 0;
    }
  for (int i = 0; i < 8; i++)
    lhs_op_info->bits.classwords[i] &= rhs_op_info->bits.classwords[i];
  break;
  case ECL_OR:
  if (rhs_op_info->op_single_type == ECL_NONE)
    {
    }
  else if (lhs_op_info->op_single_type == ECL_NONE)
    {
    if (lengthptr == NULL)
      memmove(lhs_op_info->code_start, rhs_op_info->code_start,
              CU2BYTES(rhs_op_info->length));
    lhs_op_info->length = rhs_op_info->length;
    lhs_op_info->op_single_type = rhs_op_info->op_single_type;
    }
  else if (rhs_op_info->op_single_type == ECL_ANY)
    {
    if (lengthptr == NULL)
      lhs_op_info->code_start[0] = ECL_ANY;
    lhs_op_info->length = 1;
    lhs_op_info->op_single_type = ECL_ANY;
    }
  else if (lhs_op_info->op_single_type == ECL_ANY)
    {
    }
  else
    {
    if (lengthptr != NULL)
      *lengthptr += 1;
    else
      {
      PCRE2_ASSERT(rhs_op_info->code_start ==
          lhs_op_info->code_start + lhs_op_info->length);
      rhs_op_info->code_start[rhs_op_info->length] = ECL_OR;
      }
    lhs_op_info->length += rhs_op_info->length + 1;
    lhs_op_info->op_single_type = 0;
    }
  for (int i = 0; i < 8; i++)
    lhs_op_info->bits.classwords[i] |= rhs_op_info->bits.classwords[i];
  break;
  case ECL_XOR:
  if (rhs_op_info->op_single_type == ECL_NONE)
    {
    }
  else if (lhs_op_info->op_single_type == ECL_NONE)
    {
    if (lengthptr == NULL)
      memmove(lhs_op_info->code_start, rhs_op_info->code_start,
              CU2BYTES(rhs_op_info->length));
    lhs_op_info->length = rhs_op_info->length;
    lhs_op_info->op_single_type = rhs_op_info->op_single_type;
    }
  else if (rhs_op_info->op_single_type == ECL_ANY)
    {
    fold_negation(lhs_op_info, lengthptr, TRUE);
    }
  else if (lhs_op_info->op_single_type == ECL_ANY)
    {
    if (lengthptr == NULL)
      memmove(lhs_op_info->code_start, rhs_op_info->code_start,
              CU2BYTES(rhs_op_info->length));
    lhs_op_info->length = rhs_op_info->length;
    lhs_op_info->op_single_type = rhs_op_info->op_single_type;
    fold_negation(lhs_op_info, lengthptr, TRUE);
    }
  else
    {
    if (lengthptr != NULL)
      *lengthptr += 1;
    else
      {
      PCRE2_ASSERT(rhs_op_info->code_start ==
          lhs_op_info->code_start + lhs_op_info->length);
      rhs_op_info->code_start[rhs_op_info->length] = ECL_XOR;
      }
    lhs_op_info->length += rhs_op_info->length + 1;
    lhs_op_info->op_single_type = 0;
    }
  for (int i = 0; i < 8; i++)
    lhs_op_info->bits.classwords[i] ^= rhs_op_info->bits.classwords[i];
  break;
  default:
  PCRE2_DEBUG_UNREACHABLE();
  break;
  }
}
static BOOL
compile_eclass_nested(eclass_context *context, BOOL negated,
  uint32_t **pptr, PCRE2_UCHAR **pcode,
  eclass_op_info *pop_info, PCRE2_SIZE *lengthptr);
static BOOL
compile_class_operand(eclass_context *context, BOOL negated,
  uint32_t **pptr, PCRE2_UCHAR **pcode, eclass_op_info *pop_info,
  PCRE2_SIZE *lengthptr)
{
uint32_t *ptr = *pptr;
uint32_t *prev_ptr;
PCRE2_UCHAR *code = *pcode;
PCRE2_UCHAR *code_start = code;
PCRE2_SIZE prev_length = (lengthptr != NULL)? *lengthptr : 0;
PCRE2_SIZE extra_length;
uint32_t meta = META_CODE(*ptr);
switch (meta)
  {
  case META_CLASS_EMPTY_NOT:
  case META_CLASS_EMPTY:
  ++ptr;
  pop_info->length = 1;
  if ((meta == META_CLASS_EMPTY) == negated)
    {
    *code++ = pop_info->op_single_type = ECL_ANY;
    memset(pop_info->bits.classbits, 0xff, 32);
    }
  else
    {
    *code++ = pop_info->op_single_type = ECL_NONE;
    memset(pop_info->bits.classbits, 0, 32);
    }
  break;
  case META_CLASS:
  case META_CLASS_NOT:
  if ((*ptr & CLASS_IS_ECLASS) != 0)
    {
    if (!compile_eclass_nested(context, negated, &ptr, &code,
                               pop_info, lengthptr))
      return FALSE;
    PCRE2_ASSERT(*ptr == META_CLASS_END);
    ptr++;
    goto DONE;
    }
  ptr++;
  PCRE2_FALLTHROUGH
  default:
  prev_ptr = ptr;
  ptr = PRIV(compile_class_not_nested)(
    context->options, context->xoptions, ptr, &code,
    (meta != META_CLASS_NOT) == negated, &context->needs_bitmap,
    context->errorcodeptr, context->cb, lengthptr);
  if (ptr == NULL) return FALSE;
  if (ptr <= prev_ptr)
    {
    PCRE2_DEBUG_UNREACHABLE();
    return FALSE;
    }
  if (meta == META_CLASS || meta == META_CLASS_NOT)
    {
    PCRE2_ASSERT(*ptr == META_CLASS_END);
    ptr++;
    }
  PCRE2_ASSERT(code > code_start);
  extra_length = (lengthptr != NULL)? *lengthptr - prev_length : 0;
  if (*code_start == OP_ALLANY)
    {
    PCRE2_ASSERT(code - code_start == 1 && extra_length == 0);
    pop_info->length = 1;
    *code_start = pop_info->op_single_type = ECL_ANY;
    memset(pop_info->bits.classbits, 0xff, 32);
    }
  else if (*code_start == OP_CLASS || *code_start == OP_NCLASS)
    {
    PCRE2_ASSERT(code - code_start == 1 + 32 / sizeof(PCRE2_UCHAR) &&
                 extra_length == 0);
    pop_info->length = 1;
    *code_start = pop_info->op_single_type =
        (*code_start == OP_CLASS)? ECL_NONE : ECL_ANY;
    memcpy(pop_info->bits.classbits, code_start + 1, 32);
    if (lengthptr != NULL)
      *lengthptr += code - (code_start + 1);
    code = code_start + 1;
    if (!context->needs_bitmap && *code_start == ECL_NONE)
      {
      uint32_t *classwords = pop_info->bits.classwords;
      for (int i = 0; i < 8; i++)
        if (classwords[i] != 0)
          {
          context->needs_bitmap = TRUE;
          break;
          }
      }
    else
      context->needs_bitmap = TRUE;
    }
  else
    {
    PCRE2_ASSERT(*code_start == OP_XCLASS);
    *code_start = pop_info->op_single_type = ECL_XCLASS;
    PCRE2_ASSERT(code - code_start >= 1 + LINK_SIZE + 1);
    memcpy(pop_info->bits.classbits, context->cb->classbits.classbits, 32);
    pop_info->length = (code - code_start) + extra_length;
    }
  break;
  }
pop_info->code_start = (lengthptr == NULL)? code_start : NULL;
if (lengthptr != NULL)
  {
  *lengthptr += code - code_start;
  code = code_start;
  }
DONE:
PCRE2_ASSERT(lengthptr == NULL || (code == code_start));
*pptr = ptr;
*pcode = code;
return TRUE;
}
static BOOL
compile_class_juxtaposition(eclass_context *context, BOOL negated,
  uint32_t **pptr, PCRE2_UCHAR **pcode, eclass_op_info *pop_info,
  PCRE2_SIZE *lengthptr)
{
uint32_t *ptr = *pptr;
PCRE2_UCHAR *code = *pcode;
#ifdef PCRE2_DEBUG
PCRE2_UCHAR *start_code = *pcode;
#endif
if (!compile_class_operand(context, negated, &ptr, &code, pop_info, lengthptr))
  return FALSE;
while (*ptr != META_CLASS_END &&
       !(*ptr >= META_ECLASS_AND && *ptr <= META_ECLASS_NOT))
  {
  uint32_t op;
  BOOL rhs_negated;
  eclass_op_info rhs_op_info;
  if (negated)
    {
    op = ECL_AND;
    rhs_negated = TRUE;
    }
  else
    {
    op = ECL_OR;
    rhs_negated = FALSE;
    }
  if (!compile_class_operand(context, rhs_negated, &ptr, &code,
                             &rhs_op_info, lengthptr))
    return FALSE;
  fold_binary(op, pop_info, &rhs_op_info, lengthptr);
  if (lengthptr == NULL)
    code = pop_info->code_start + pop_info->length;
  }
PCRE2_ASSERT(lengthptr == NULL || code == start_code);
*pptr = ptr;
*pcode = code;
return TRUE;
}
static BOOL
compile_class_unary(eclass_context *context, BOOL negated,
  uint32_t **pptr, PCRE2_UCHAR **pcode, eclass_op_info *pop_info,
  PCRE2_SIZE *lengthptr)
{
uint32_t *ptr = *pptr;
#ifdef PCRE2_DEBUG
PCRE2_UCHAR *start_code = *pcode;
#endif
while (*ptr == META_ECLASS_NOT)
  {
  ++ptr;
  negated = !negated;
  }
*pptr = ptr;
if (!compile_class_juxtaposition(context, negated, pptr, pcode,
                                 pop_info, lengthptr))
  return FALSE;
PCRE2_ASSERT(lengthptr == NULL || *pcode == start_code);
return TRUE;
}
static BOOL
compile_class_binary_tight(eclass_context *context, BOOL negated,
  uint32_t **pptr, PCRE2_UCHAR **pcode, eclass_op_info *pop_info,
  PCRE2_SIZE *lengthptr)
{
uint32_t *ptr = *pptr;
PCRE2_UCHAR *code = *pcode;
#ifdef PCRE2_DEBUG
PCRE2_UCHAR *start_code = *pcode;
#endif
if (!compile_class_unary(context, negated, &ptr, &code, pop_info, lengthptr))
  return FALSE;
while (*ptr == META_ECLASS_AND)
  {
  uint32_t op;
  BOOL rhs_negated;
  eclass_op_info rhs_op_info;
  if (negated)
    {
    op = ECL_OR;
    rhs_negated = TRUE;
    }
  else
    {
    op = ECL_AND;
    rhs_negated = FALSE;
    }
  ++ptr;
  if (!compile_class_unary(context, rhs_negated, &ptr, &code,
                           &rhs_op_info, lengthptr))
    return FALSE;
  fold_binary(op, pop_info, &rhs_op_info, lengthptr);
  if (lengthptr == NULL)
    code = pop_info->code_start + pop_info->length;
  }
PCRE2_ASSERT(lengthptr == NULL || code == start_code);
*pptr = ptr;
*pcode = code;
return TRUE;
}
static BOOL
compile_class_binary_loose(eclass_context *context, BOOL negated,
  uint32_t **pptr, PCRE2_UCHAR **pcode, eclass_op_info *pop_info,
  PCRE2_SIZE *lengthptr)
{
uint32_t *ptr = *pptr;
PCRE2_UCHAR *code = *pcode;
#ifdef PCRE2_DEBUG
PCRE2_UCHAR *start_code = *pcode;
#endif
if (!compile_class_binary_tight(context, negated, &ptr, &code,
                                pop_info, lengthptr))
  return FALSE;
while (*ptr >= META_ECLASS_OR && *ptr <= META_ECLASS_XOR)
  {
  uint32_t op;
  BOOL op_neg;
  BOOL rhs_negated;
  eclass_op_info rhs_op_info;
  if (negated)
    {
    op = (*ptr == META_ECLASS_OR )? ECL_AND :
         (*ptr == META_ECLASS_SUB)? ECL_OR  :
          ECL_XOR;
    op_neg = (*ptr == META_ECLASS_XOR);
    rhs_negated = *ptr != META_ECLASS_SUB;
    }
  else
    {
    op = (*ptr == META_ECLASS_OR )? ECL_OR  :
         (*ptr == META_ECLASS_SUB)? ECL_AND :
          ECL_XOR;
    op_neg = FALSE;
    rhs_negated = *ptr == META_ECLASS_SUB;
    }
  ++ptr;
  if (!compile_class_binary_tight(context, rhs_negated, &ptr, &code,
                                  &rhs_op_info, lengthptr))
    return FALSE;
  fold_binary(op, pop_info, &rhs_op_info, lengthptr);
  if (op_neg) fold_negation(pop_info, lengthptr, FALSE);
  if (lengthptr == NULL)
    code = pop_info->code_start + pop_info->length;
  }
PCRE2_ASSERT(lengthptr == NULL || code == start_code);
*pptr = ptr;
*pcode = code;
return TRUE;
}
static BOOL
compile_eclass_nested(eclass_context *context, BOOL negated,
  uint32_t **pptr, PCRE2_UCHAR **pcode,
  eclass_op_info *pop_info, PCRE2_SIZE *lengthptr)
{
uint32_t *ptr = *pptr;
#ifdef PCRE2_DEBUG
PCRE2_UCHAR *start_code = *pcode;
#endif
PCRE2_ASSERT(*ptr == (META_CLASS | CLASS_IS_ECLASS) ||
             *ptr == (META_CLASS_NOT | CLASS_IS_ECLASS));
if (*ptr++ == (META_CLASS_NOT | CLASS_IS_ECLASS))
  negated = !negated;
(*pptr)++;
if (!compile_class_binary_loose(context, negated, pptr, pcode,
                                pop_info, lengthptr))
  return FALSE;
PCRE2_ASSERT(**pptr == META_CLASS_END);
PCRE2_ASSERT(lengthptr == NULL || *pcode == start_code);
return TRUE;
}
BOOL
PRIV(compile_class_nested)(uint32_t options, uint32_t xoptions,
  uint32_t **pptr, PCRE2_UCHAR **pcode, int *errorcodeptr,
  compile_block *cb, PCRE2_SIZE *lengthptr)
{
eclass_context context;
eclass_op_info op_info;
PCRE2_SIZE previous_length = (lengthptr != NULL)? *lengthptr : 0;
PCRE2_UCHAR *code = *pcode;
PCRE2_UCHAR *previous;
BOOL allbitsone = TRUE;
context.needs_bitmap = FALSE;
context.options = options;
context.xoptions = xoptions;
context.errorcodeptr = errorcodeptr;
context.cb = cb;
previous = code;
*code++ = OP_ECLASS;
code += LINK_SIZE;
*code++ = 0;
if (!compile_eclass_nested(&context, FALSE, pptr, &code, &op_info, lengthptr))
  return FALSE;
if (lengthptr != NULL)
  {
  *lengthptr += code - previous;
  code = previous;
  }
for (int i = 0; i < 8; i++)
  if (op_info.bits.classwords[i] != 0xffffffff)
    {
    allbitsone = FALSE;
    break;
    }
#ifndef SUPPORT_WIDE_CHARS
PCRE2_ASSERT(op_info.op_single_type != 0);
#else
if (op_info.op_single_type != 0)
#endif
  {
  code = previous;
  if (op_info.op_single_type == ECL_ANY && allbitsone)
    {
    if (lengthptr != NULL) *lengthptr -= 1;
    *code++ = OP_ALLANY;
    }
  else if (op_info.op_single_type == ECL_ANY ||
           op_info.op_single_type == ECL_NONE)
    {
    PCRE2_SIZE required_len = 1 + (32 / sizeof(PCRE2_UCHAR));
    if (lengthptr != NULL)
      {
      if (required_len > (*lengthptr - previous_length))
      *lengthptr = previous_length + required_len;
      }
    if (lengthptr != NULL) *lengthptr -= required_len;
    *code++ = (op_info.op_single_type == ECL_ANY)? OP_NCLASS : OP_CLASS;
    memcpy(code, op_info.bits.classbits, 32);
    code += 32 / sizeof(PCRE2_UCHAR);
    }
  else
    {
#ifndef SUPPORT_WIDE_CHARS
    PCRE2_DEBUG_UNREACHABLE();
#else
    BOOL need_map = context.needs_bitmap;
    PCRE2_SIZE required_len;
    PCRE2_ASSERT(op_info.op_single_type == ECL_XCLASS);
    required_len = op_info.length + (need_map? 32/sizeof(PCRE2_UCHAR) : 0);
    if (lengthptr != NULL)
      {
      if (required_len > (*lengthptr - previous_length))
        *lengthptr = previous_length + required_len;
      *lengthptr -= 1 + LINK_SIZE + 1;
      *code++ = OP_XCLASS;
      PUT(code, 0, 1 + LINK_SIZE + 1);
      code += LINK_SIZE;
      *code++ = 0;
      }
    else
      {
      PCRE2_UCHAR *rest;
      PCRE2_SIZE rest_len;
      PCRE2_UCHAR flags;
      PCRE2_ASSERT(op_info.length >= 1 + LINK_SIZE + 1);
      rest = op_info.code_start + 1 + LINK_SIZE + 1;
      rest_len = (op_info.code_start + op_info.length) - rest;
      flags = op_info.code_start[1 + LINK_SIZE];
      PCRE2_ASSERT((flags & XCL_MAP) == 0);
      memmove(code + 1 + LINK_SIZE + 1 + (need_map? 32/sizeof(PCRE2_UCHAR) : 0),
              rest, CU2BYTES(rest_len));
      *code++ = OP_XCLASS;
      PUT(code, 0, (int)required_len);
      code += LINK_SIZE;
      *code++ = flags | (need_map? XCL_MAP : 0);
      if (need_map)
        {
        memcpy(code, op_info.bits.classbits, 32);
        code += 32 / sizeof(PCRE2_UCHAR);
        }
      code += rest_len;
      }
#endif
    }
  }
#ifdef SUPPORT_WIDE_CHARS
else
  {
  BOOL need_map = context.needs_bitmap;
  PCRE2_SIZE required_len =
    1 + LINK_SIZE + 1 + (need_map? 32/sizeof(PCRE2_UCHAR) : 0) + op_info.length;
  if (lengthptr != NULL)
    {
    if (required_len > (*lengthptr - previous_length))
      *lengthptr = previous_length + required_len;
    *lengthptr -= 1 + LINK_SIZE + 1;
    *code++ = OP_ECLASS;
    PUT(code, 0, 1 + LINK_SIZE + 1);
    code += LINK_SIZE;
    *code++ = 0;
    }
  else
    {
    if (need_map)
      {
      PCRE2_UCHAR *map_start = previous + 1 + LINK_SIZE + 1;
      previous[1 + LINK_SIZE] |= ECL_MAP;
      memmove(map_start + 32/sizeof(PCRE2_UCHAR), map_start,
              CU2BYTES(code - map_start));
      memcpy(map_start, op_info.bits.classbits, 32);
      code += 32 / sizeof(PCRE2_UCHAR);
      }
    PUT(previous, 1, (int)(code - previous));
    }
  }
#endif
*pcode = code;
return TRUE;
}
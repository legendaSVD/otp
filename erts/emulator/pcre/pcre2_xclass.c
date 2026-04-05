#include "pcre2_internal.h"
BOOL
PRIV(xclass)(uint32_t c, PCRE2_SPTR data, const uint8_t *char_lists_end, BOOL utf)
{
PCRE2_UCHAR t;
BOOL not_negated = (*data & XCL_NOT) == 0;
uint32_t type, max_index, min_index, value;
const uint8_t *next_char;
#if PCRE2_CODE_UNIT_WIDTH == 8
utf = TRUE;
#endif
if ((*data++ & XCL_MAP) != 0)
  {
  if (c < 256)
    return (((const uint8_t *)data)[c/8] & (1u << (c&7))) != 0;
  data += 32 / sizeof(PCRE2_UCHAR);
  }
#ifdef SUPPORT_UNICODE
if (*data == XCL_PROP || *data == XCL_NOTPROP)
  {
  const ucd_record *prop = GET_UCD(c);
  do
    {
    int chartype;
    BOOL isprop = (*data++) == XCL_PROP;
    BOOL ok;
    switch(*data)
      {
      case PT_LAMP:
      chartype = prop->chartype;
      if ((chartype == ucp_Lu || chartype == ucp_Ll ||
           chartype == ucp_Lt) == isprop) return not_negated;
      break;
      case PT_GC:
      if ((data[1] == PRIV(ucp_gentype)[prop->chartype]) == isprop)
        return not_negated;
      break;
      case PT_PC:
      if ((data[1] == prop->chartype) == isprop) return not_negated;
      break;
      case PT_SC:
      if ((data[1] == prop->script) == isprop) return not_negated;
      break;
      case PT_SCX:
      ok = (data[1] == prop->script ||
            MAPBIT(PRIV(ucd_script_sets) + UCD_SCRIPTX_PROP(prop), data[1]) != 0);
      if (ok == isprop) return not_negated;
      break;
      case PT_ALNUM:
      chartype = prop->chartype;
      if ((PRIV(ucp_gentype)[chartype] == ucp_L ||
           PRIV(ucp_gentype)[chartype] == ucp_N) == isprop)
        return not_negated;
      break;
      case PT_SPACE:
      case PT_PXSPACE:
      switch(c)
        {
        HSPACE_CASES:
        VSPACE_CASES:
        if (isprop) return not_negated;
        break;
        default:
        if ((PRIV(ucp_gentype)[prop->chartype] == ucp_Z) == isprop)
          return not_negated;
        break;
        }
      break;
      case PT_WORD:
      chartype = prop->chartype;
      if ((PRIV(ucp_gentype)[chartype] == ucp_L ||
           PRIV(ucp_gentype)[chartype] == ucp_N ||
           chartype == ucp_Mn || chartype == ucp_Pc) == isprop)
        return not_negated;
      break;
      case PT_UCNC:
      if (c < 0xa0)
        {
        if ((c == CHAR_DOLLAR_SIGN || c == CHAR_COMMERCIAL_AT ||
             c == CHAR_GRAVE_ACCENT) == isprop)
          return not_negated;
        }
      else
        {
        if ((c < 0xd800 || c > 0xdfff) == isprop)
          return not_negated;
        }
      break;
      case PT_BIDICL:
      if ((UCD_BIDICLASS_PROP(prop) == data[1]) == isprop)
        return not_negated;
      break;
      case PT_BOOL:
      ok = MAPBIT(PRIV(ucd_boolprop_sets) +
        UCD_BPROPS_PROP(prop), data[1]) != 0;
      if (ok == isprop) return not_negated;
      break;
      case PT_PXGRAPH:
      chartype = prop->chartype;
      if ((PRIV(ucp_gentype)[chartype] != ucp_Z &&
            (PRIV(ucp_gentype)[chartype] != ucp_C ||
              (chartype == ucp_Cf &&
                c != 0x061c && c != 0x180e && (c < 0x2066 || c > 0x2069))
         )) == isprop)
        return not_negated;
      break;
      case PT_PXPRINT:
      chartype = prop->chartype;
      if ((chartype != ucp_Zl &&
           chartype != ucp_Zp &&
            (PRIV(ucp_gentype)[chartype] != ucp_C ||
              (chartype == ucp_Cf &&
                c != 0x061c && (c < 0x2066 || c > 0x2069))
         )) == isprop)
        return not_negated;
      break;
      case PT_PXPUNCT:
      chartype = prop->chartype;
      if ((PRIV(ucp_gentype)[chartype] == ucp_P ||
            (c < 128 && PRIV(ucp_gentype)[chartype] == ucp_S)) == isprop)
        return not_negated;
      break;
      case PT_PXXDIGIT:
      if (((c >= CHAR_0 && c <= CHAR_9) ||
           (c >= CHAR_A && c <= CHAR_F) ||
           (c >= CHAR_a && c <= CHAR_f) ||
           (c >= 0xff10 && c <= 0xff19) ||
           (c >= 0xff21 && c <= 0xff26) ||
           (c >= 0xff41 && c <= 0xff46)) == isprop)
        return not_negated;
      break;
      default:
      PCRE2_DEBUG_UNREACHABLE();
      return FALSE;
      }
    data += 2;
    }
  while (*data == XCL_PROP || *data == XCL_NOTPROP);
  }
#else
  (void)utf;
#endif
if (*data < XCL_LIST)
  {
  while ((t = *data++) != XCL_END)
    {
    uint32_t x, y;
#ifdef SUPPORT_UNICODE
    if (utf)
      {
      GETCHARINC(x, data);
      }
    else
#endif
      x = *data++;
    if (t == XCL_SINGLE)
      {
      if (c <= x) return (c == x) ? not_negated : !not_negated;
      continue;
      }
    PCRE2_ASSERT(t == XCL_RANGE);
#ifdef SUPPORT_UNICODE
    if (utf)
      {
      GETCHARINC(y, data);
      }
    else
#endif
      y = *data++;
    if (c <= y) return (c >= x) ? not_negated : !not_negated;
    }
  return !not_negated;
  }
#if PCRE2_CODE_UNIT_WIDTH == 8
type = (uint32_t)(data[0] << 8) | data[1];
data += 2;
#else
type = data[0];
data++;
#endif
next_char = char_lists_end - (GET(data, 0) << 1);
type &= XCL_TYPE_MASK;
PCRE2_ASSERT(((uintptr_t)next_char & 0x1) == 0);
if (c >= XCL_CHAR_LIST_HIGH_16_START)
  {
  max_index = type & XCL_ITEM_COUNT_MASK;
  if (max_index == XCL_ITEM_COUNT_MASK)
    {
    max_index = *(const uint16_t*)next_char;
    PCRE2_ASSERT(max_index >= XCL_ITEM_COUNT_MASK);
    next_char += 2;
    }
  next_char += max_index << 1;
  type >>= XCL_TYPE_BIT_LEN;
  }
if (c < XCL_CHAR_LIST_LOW_32_START)
  {
  max_index = type & XCL_ITEM_COUNT_MASK;
  c = (uint16_t)((c << XCL_CHAR_SHIFT) | XCL_CHAR_END);
  if (max_index == XCL_ITEM_COUNT_MASK)
    {
    max_index = *(const uint16_t*)next_char;
    PCRE2_ASSERT(max_index >= XCL_ITEM_COUNT_MASK);
    next_char += 2;
    }
  if (max_index == 0 || c < *(const uint16_t*)next_char)
    return ((type & XCL_BEGIN_WITH_RANGE) != 0) == not_negated;
  min_index = 0;
  value = ((const uint16_t*)next_char)[--max_index];
  if (c >= value)
    return (value == c || (value & XCL_CHAR_END) == 0) == not_negated;
  max_index--;
  while (TRUE)
    {
    uint32_t mid_index = (min_index + max_index) >> 1;
    value = ((const uint16_t*)next_char)[mid_index];
    if (c < value)
      max_index = mid_index - 1;
    else if (((const uint16_t*)next_char)[mid_index + 1] <= c)
      min_index = mid_index + 1;
    else
      return (value == c || (value & XCL_CHAR_END) == 0) == not_negated;
    }
  }
max_index = type & XCL_ITEM_COUNT_MASK;
if (max_index == XCL_ITEM_COUNT_MASK)
  {
  max_index = *(const uint16_t*)next_char;
  PCRE2_ASSERT(max_index >= XCL_ITEM_COUNT_MASK);
  next_char += 2;
  }
next_char += (max_index << 1);
type >>= XCL_TYPE_BIT_LEN;
PCRE2_ASSERT(((uintptr_t)next_char & 0x3) == 0);
max_index = type & XCL_ITEM_COUNT_MASK;
#if PCRE2_CODE_UNIT_WIDTH == 32
if (c >= XCL_CHAR_LIST_HIGH_32_START)
  {
  if (max_index == XCL_ITEM_COUNT_MASK)
    {
    max_index = *(const uint32_t*)next_char;
    PCRE2_ASSERT(max_index >= XCL_ITEM_COUNT_MASK);
    next_char += 4;
    }
  next_char += max_index << 2;
  type >>= XCL_TYPE_BIT_LEN;
  max_index = type & XCL_ITEM_COUNT_MASK;
  }
#endif
c = (uint32_t)((c << XCL_CHAR_SHIFT) | XCL_CHAR_END);
if (max_index == XCL_ITEM_COUNT_MASK)
  {
  max_index = *(const uint32_t*)next_char;
  next_char += 4;
  }
if (max_index == 0 || c < *(const uint32_t*)next_char)
  return ((type & XCL_BEGIN_WITH_RANGE) != 0) == not_negated;
min_index = 0;
value = ((const uint32_t*)next_char)[--max_index];
if (c >= value)
  return (value == c || (value & XCL_CHAR_END) == 0) == not_negated;
max_index--;
while (TRUE)
  {
  uint32_t mid_index = (min_index + max_index) >> 1;
  value = ((const uint32_t*)next_char)[mid_index];
  if (c < value)
    max_index = mid_index - 1;
  else if (((const uint32_t*)next_char)[mid_index + 1] <= c)
    min_index = mid_index + 1;
  else
    return (value == c || (value & XCL_CHAR_END) == 0) == not_negated;
  }
}
BOOL
PRIV(eclass)(uint32_t c, PCRE2_SPTR data_start, PCRE2_SPTR data_end,
  const uint8_t *char_lists_end, BOOL utf)
{
PCRE2_SPTR ptr = data_start;
PCRE2_UCHAR flags;
uint32_t stack = 0;
int stack_depth = 0;
PCRE2_ASSERT(data_start < data_end);
flags = *ptr++;
PCRE2_ASSERT((flags & ECL_MAP) == 0 ||
             (data_end - ptr) >= 32 / (int)sizeof(PCRE2_UCHAR));
if ((flags & ECL_MAP) != 0)
  {
  if (c < 256)
    return (((const uint8_t *)ptr)[c/8] & (1u << (c&7))) != 0;
  ptr += 32 / sizeof(PCRE2_UCHAR);
  }
while (ptr < data_end)
  {
  switch (*ptr)
    {
    case ECL_AND:
    ++ptr;
    stack = (stack >> 1) & (stack | ~(uint32_t)1u);
    PCRE2_ASSERT(stack_depth >= 2);
    --stack_depth;
    break;
    case ECL_OR:
    ++ptr;
    stack = (stack >> 1) | (stack & (uint32_t)1u);
    PCRE2_ASSERT(stack_depth >= 2);
    --stack_depth;
    break;
    case ECL_XOR:
    ++ptr;
    stack = (stack >> 1) ^ (stack & (uint32_t)1u);
    PCRE2_ASSERT(stack_depth >= 2);
    --stack_depth;
    break;
    case ECL_NOT:
    ++ptr;
    stack ^= (uint32_t)1u;
    PCRE2_ASSERT(stack_depth >= 1);
    break;
    case ECL_XCLASS:
      {
      uint32_t matched = PRIV(xclass)(c, ptr + 1 + LINK_SIZE, char_lists_end, utf);
      ptr += GET(ptr, 1);
      stack = (stack << 1) | matched;
      ++stack_depth;
      break;
      }
    default:
    PCRE2_DEBUG_UNREACHABLE();
    return FALSE;
    }
  }
PCRE2_ASSERT(stack_depth == 1);
(void)stack_depth;
return (stack & 1u) != 0;
}
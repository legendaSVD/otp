#include "pcre2_internal.h"
enum { SCRIPT_UNSET,
       SCRIPT_MAP,
       SCRIPT_HANPENDING,
       SCRIPT_HANHIRAKATA,
       SCRIPT_HANBOPOMOFO,
       SCRIPT_HANHANGUL
       };
#define UCD_MAPSIZE (ucp_Unknown/32 + 1)
#define FULL_MAPSIZE (ucp_Script_Count/32 + 1)
BOOL
PRIV(script_run)(PCRE2_SPTR ptr, PCRE2_SPTR endptr, BOOL utf)
{
#ifdef SUPPORT_UNICODE
uint32_t require_state = SCRIPT_UNSET;
uint32_t require_map[FULL_MAPSIZE];
uint32_t map[FULL_MAPSIZE];
uint32_t require_digitset = 0;
uint32_t c;
#if PCRE2_CODE_UNIT_WIDTH == 32
(void)utf;
#endif
if (ptr >= endptr) return TRUE;
GETCHARINCTEST(c, ptr);
if (ptr >= endptr) return TRUE;
for (int i = 0; i < FULL_MAPSIZE; i++) require_map[i] = 0;
for (;;)
  {
  const ucd_record *ucd = GET_UCD(c);
  uint32_t script = ucd->script;
  if (script == ucp_Unknown) return FALSE;
  if (UCD_SCRIPTX_PROP(ucd) != 0 || (script != ucp_Inherited && script != ucp_Common))
    {
    BOOL OK;
    memcpy(map, PRIV(ucd_script_sets) + UCD_SCRIPTX_PROP(ucd), UCD_MAPSIZE * sizeof(uint32_t));
    memset(map + UCD_MAPSIZE, 0, (FULL_MAPSIZE - UCD_MAPSIZE) * sizeof(uint32_t));
    if (script != ucp_Common && script != ucp_Inherited) MAPSET(map, script);
    switch(require_state)
      {
      case SCRIPT_UNSET:
      switch(script)
        {
        case ucp_Han:
        require_state = SCRIPT_HANPENDING;
        break;
        case ucp_Hiragana:
        case ucp_Katakana:
        require_state = SCRIPT_HANHIRAKATA;
        break;
        case ucp_Bopomofo:
        require_state = SCRIPT_HANBOPOMOFO;
        break;
        case ucp_Hangul:
        require_state = SCRIPT_HANHANGUL;
        break;
        default:
        memcpy(require_map, map, FULL_MAPSIZE * sizeof(uint32_t));
        require_state = SCRIPT_MAP;
        break;
        }
      break;
#define FOUND_BOPOMOFO 1
#define FOUND_HIRAGANA 2
#define FOUND_KATAKANA 4
#define FOUND_HANGUL   8
      case SCRIPT_HANPENDING:
      if (script != ucp_Han)
        {
        uint32_t chspecial = 0;
        if (MAPBIT(map, ucp_Bopomofo) != 0) chspecial |= FOUND_BOPOMOFO;
        if (MAPBIT(map, ucp_Hiragana) != 0) chspecial |= FOUND_HIRAGANA;
        if (MAPBIT(map, ucp_Katakana) != 0) chspecial |= FOUND_KATAKANA;
        if (MAPBIT(map, ucp_Hangul) != 0)   chspecial |= FOUND_HANGUL;
        if (chspecial == 0) return FALSE;
        if (chspecial == FOUND_BOPOMOFO)
          require_state = SCRIPT_HANBOPOMOFO;
        else if (chspecial == (FOUND_HIRAGANA|FOUND_KATAKANA))
          require_state = SCRIPT_HANHIRAKATA;
        }
      break;
      case SCRIPT_HANHIRAKATA:
      if (MAPBIT(map, ucp_Han) + MAPBIT(map, ucp_Hiragana) +
          MAPBIT(map, ucp_Katakana) == 0) return FALSE;
      break;
      case SCRIPT_HANBOPOMOFO:
      if (MAPBIT(map, ucp_Han) + MAPBIT(map, ucp_Bopomofo) == 0) return FALSE;
      break;
      case SCRIPT_HANHANGUL:
      if (MAPBIT(map, ucp_Han) + MAPBIT(map, ucp_Hangul) == 0) return FALSE;
      break;
      case SCRIPT_MAP:
      OK = FALSE;
      for (int i = 0; i < FULL_MAPSIZE; i++)
        {
        if ((require_map[i] & map[i]) != 0)
          {
          OK = TRUE;
          break;
          }
        }
      if (!OK) return FALSE;
      switch(script)
        {
        case ucp_Han:
        require_state = SCRIPT_HANPENDING;
        break;
        case ucp_Hiragana:
        case ucp_Katakana:
        require_state = SCRIPT_HANHIRAKATA;
        break;
        case ucp_Bopomofo:
        require_state = SCRIPT_HANBOPOMOFO;
        break;
        case ucp_Hangul:
        require_state = SCRIPT_HANHANGUL;
        break;
        default:
        for (int i = 0; i < FULL_MAPSIZE; i++) require_map[i] &= map[i];
        break;
        }
      break;
      }
    }
  if (ucd->chartype == ucp_Nd)
    {
    uint32_t digitset;
    if (c <= PRIV(ucd_digit_sets)[1]) digitset = 1; else
      {
      int mid;
      int bot = 1;
      int top = PRIV(ucd_digit_sets)[0];
      for (;;)
        {
        if (top <= bot + 1)
          {
          digitset = top;
          break;
          }
        mid = (top + bot) / 2;
        if (c <= PRIV(ucd_digit_sets)[mid]) top = mid; else bot = mid;
        }
      }
    if (require_digitset == 0) require_digitset = digitset;
      else if (digitset != require_digitset) return FALSE;
    }
  if (ptr >= endptr) return TRUE;
  GETCHARINCTEST(c, ptr);
  }
#else
(void)ptr;
(void)endptr;
(void)utf;
return TRUE;
#endif
}
#include "pcre2_internal.h"
PCRE2_SPTR
PRIV(find_bracket)(PCRE2_SPTR code, BOOL utf, int number)
{
for (;;)
  {
  PCRE2_UCHAR c = *code;
  if (c == OP_END) return NULL;
  if (c == OP_XCLASS || c == OP_ECLASS) code += GET(code, 1);
  else if (c == OP_CALLOUT_STR) code += GET(code, 1 + 2*LINK_SIZE);
  else if (c == OP_REVERSE || c == OP_VREVERSE)
    {
    if (number < 0) return code;
    code += PRIV(OP_lengths)[c];
    }
  else if (c == OP_CBRA || c == OP_SCBRA ||
           c == OP_CBRAPOS || c == OP_SCBRAPOS)
    {
    int n = (int)GET2(code, 1+LINK_SIZE);
    if (n == number) return code;
    code += PRIV(OP_lengths)[c];
    }
  else
    {
    switch(c)
      {
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
      case OP_EXACT:
      case OP_EXACTI:
      case OP_NOTEXACT:
      case OP_NOTEXACTI:
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
      case OP_PLUS:
      case OP_PLUSI:
      case OP_NOTPLUS:
      case OP_NOTPLUSI:
      case OP_MINPLUS:
      case OP_MINPLUSI:
      case OP_NOTMINPLUS:
      case OP_NOTMINPLUSI:
      case OP_POSPLUS:
      case OP_POSPLUSI:
      case OP_NOTPOSPLUS:
      case OP_NOTPOSPLUSI:
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
      if (HAS_EXTRALEN(code[-1])) code += GET_EXTRALEN(code[-1]);
      break;
      }
#else
    (void)(utf);
#endif
    }
  }
}
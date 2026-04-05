#include "pcre2_internal.h"
static void *default_malloc(size_t size, void *data)
{
(void)data;
return malloc(size);
}
static void default_free(void *block, void *data)
{
(void)data;
free(block);
}
extern void *
PRIV(memctl_malloc)(size_t size, pcre2_memctl *memctl)
{
pcre2_memctl *newmemctl;
void *yield = (memctl == NULL)? malloc(size) :
  memctl->malloc(size, memctl->memory_data);
if (yield == NULL) return NULL;
newmemctl = (pcre2_memctl *)yield;
if (memctl == NULL)
  {
  newmemctl->malloc = default_malloc;
  newmemctl->free = default_free;
  newmemctl->memory_data = NULL;
  }
else *newmemctl = *memctl;
return yield;
}
PCRE2_EXP_DEFN pcre2_general_context * PCRE2_CALL_CONVENTION
pcre2_general_context_create(void *(*private_malloc)(size_t, void *),
  void (*private_free)(void *, void *), void *memory_data)
{
pcre2_general_context *gcontext;
if (private_malloc == NULL) private_malloc = default_malloc;
if (private_free == NULL) private_free = default_free;
gcontext = private_malloc(sizeof(pcre2_real_general_context), memory_data);
if (gcontext == NULL) return NULL;
gcontext->memctl.malloc = private_malloc;
gcontext->memctl.free = private_free;
gcontext->memctl.memory_data = memory_data;
return gcontext;
}
pcre2_compile_context PRIV(default_compile_context) = {
  { default_malloc, default_free, NULL },
  NULL,
  NULL,
  PRIV(default_tables),
  PCRE2_UNSET,
  PCRE2_UNSET,
  BSR_DEFAULT,
  NEWLINE_DEFAULT,
  PARENS_NEST_LIMIT,
  0,
  MAX_VARLOOKBEHIND,
  PCRE2_OPTIMIZATION_ALL
  };
PCRE2_EXP_DEFN pcre2_compile_context * PCRE2_CALL_CONVENTION
pcre2_compile_context_create(pcre2_general_context *gcontext)
{
pcre2_compile_context *ccontext = PRIV(memctl_malloc)(
  sizeof(pcre2_real_compile_context), (pcre2_memctl *)gcontext);
if (ccontext == NULL) return NULL;
*ccontext = PRIV(default_compile_context);
if (gcontext != NULL)
  *((pcre2_memctl *)ccontext) = *((pcre2_memctl *)gcontext);
return ccontext;
}
pcre2_match_context PRIV(default_match_context) = {
  { default_malloc, default_free, NULL },
#ifdef SUPPORT_JIT
  NULL,
  NULL,
#endif
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  PCRE2_UNSET,
  HEAP_LIMIT,
  MATCH_LIMIT,
  MATCH_LIMIT_DEPTH };
PCRE2_EXP_DEFN pcre2_match_context * PCRE2_CALL_CONVENTION
pcre2_match_context_create(pcre2_general_context *gcontext)
{
pcre2_match_context *mcontext = PRIV(memctl_malloc)(
  sizeof(pcre2_real_match_context), (pcre2_memctl *)gcontext);
if (mcontext == NULL) return NULL;
*mcontext = PRIV(default_match_context);
if (gcontext != NULL)
  *((pcre2_memctl *)mcontext) = *((pcre2_memctl *)gcontext);
return mcontext;
}
pcre2_convert_context PRIV(default_convert_context) = {
  { default_malloc, default_free, NULL },
#ifdef _WIN32
  CHAR_BACKSLASH,
  CHAR_GRAVE_ACCENT
#else
  CHAR_SLASH,
  CHAR_BACKSLASH
#endif
  };
PCRE2_EXP_DEFN pcre2_convert_context * PCRE2_CALL_CONVENTION
pcre2_convert_context_create(pcre2_general_context *gcontext)
{
pcre2_convert_context *ccontext = PRIV(memctl_malloc)(
  sizeof(pcre2_real_convert_context), (pcre2_memctl *)gcontext);
if (ccontext == NULL) return NULL;
*ccontext = PRIV(default_convert_context);
if (gcontext != NULL)
  *((pcre2_memctl *)ccontext) = *((pcre2_memctl *)gcontext);
return ccontext;
}
PCRE2_EXP_DEFN pcre2_general_context * PCRE2_CALL_CONVENTION
pcre2_general_context_copy(pcre2_general_context *gcontext)
{
pcre2_general_context *newcontext =
  gcontext->memctl.malloc(sizeof(pcre2_real_general_context),
  gcontext->memctl.memory_data);
if (newcontext == NULL) return NULL;
memcpy(newcontext, gcontext, sizeof(pcre2_real_general_context));
return newcontext;
}
PCRE2_EXP_DEFN pcre2_compile_context * PCRE2_CALL_CONVENTION
pcre2_compile_context_copy(pcre2_compile_context *ccontext)
{
pcre2_compile_context *newcontext =
  ccontext->memctl.malloc(sizeof(pcre2_real_compile_context),
  ccontext->memctl.memory_data);
if (newcontext == NULL) return NULL;
memcpy(newcontext, ccontext, sizeof(pcre2_real_compile_context));
return newcontext;
}
PCRE2_EXP_DEFN pcre2_match_context * PCRE2_CALL_CONVENTION
pcre2_match_context_copy(pcre2_match_context *mcontext)
{
pcre2_match_context *newcontext =
  mcontext->memctl.malloc(sizeof(pcre2_real_match_context),
  mcontext->memctl.memory_data);
if (newcontext == NULL) return NULL;
memcpy(newcontext, mcontext, sizeof(pcre2_real_match_context));
return newcontext;
}
PCRE2_EXP_DEFN pcre2_convert_context * PCRE2_CALL_CONVENTION
pcre2_convert_context_copy(pcre2_convert_context *ccontext)
{
pcre2_convert_context *newcontext =
  ccontext->memctl.malloc(sizeof(pcre2_real_convert_context),
  ccontext->memctl.memory_data);
if (newcontext == NULL) return NULL;
memcpy(newcontext, ccontext, sizeof(pcre2_real_convert_context));
return newcontext;
}
PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_general_context_free(pcre2_general_context *gcontext)
{
if (gcontext != NULL)
  gcontext->memctl.free(gcontext, gcontext->memctl.memory_data);
}
PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_compile_context_free(pcre2_compile_context *ccontext)
{
if (ccontext != NULL)
  ccontext->memctl.free(ccontext, ccontext->memctl.memory_data);
}
PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_match_context_free(pcre2_match_context *mcontext)
{
if (mcontext != NULL)
  mcontext->memctl.free(mcontext, mcontext->memctl.memory_data);
}
PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_convert_context_free(pcre2_convert_context *ccontext)
{
if (ccontext != NULL)
  ccontext->memctl.free(ccontext, ccontext->memctl.memory_data);
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_character_tables(pcre2_compile_context *ccontext,
  const uint8_t *tables)
{
ccontext->tables = tables;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_bsr(pcre2_compile_context *ccontext, uint32_t value)
{
switch(value)
  {
  case PCRE2_BSR_ANYCRLF:
  case PCRE2_BSR_UNICODE:
  ccontext->bsr_convention = value;
  return 0;
  default:
  return PCRE2_ERROR_BADDATA;
  }
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_max_pattern_length(pcre2_compile_context *ccontext, PCRE2_SIZE length)
{
ccontext->max_pattern_length = length;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_max_pattern_compiled_length(pcre2_compile_context *ccontext, PCRE2_SIZE length)
{
ccontext->max_pattern_compiled_length = length;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_newline(pcre2_compile_context *ccontext, uint32_t newline)
{
switch(newline)
  {
  case PCRE2_NEWLINE_CR:
  case PCRE2_NEWLINE_LF:
  case PCRE2_NEWLINE_CRLF:
  case PCRE2_NEWLINE_ANY:
  case PCRE2_NEWLINE_ANYCRLF:
  case PCRE2_NEWLINE_NUL:
  ccontext->newline_convention = newline;
  return 0;
  default:
  return PCRE2_ERROR_BADDATA;
  }
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_max_varlookbehind(pcre2_compile_context *ccontext, uint32_t limit)
{
ccontext->max_varlookbehind = limit;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_parens_nest_limit(pcre2_compile_context *ccontext, uint32_t limit)
{
ccontext->parens_nest_limit = limit;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_compile_extra_options(pcre2_compile_context *ccontext, uint32_t options)
{
ccontext->extra_options = options;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_compile_recursion_guard(pcre2_compile_context *ccontext,
  int (*guard)(uint32_t, void *), void *user_data)
{
ccontext->stack_guard = guard;
ccontext->stack_guard_data = user_data;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_optimize(pcre2_compile_context *ccontext, uint32_t directive)
{
if (ccontext == NULL)
  return PCRE2_ERROR_NULL;
switch (directive)
  {
  case PCRE2_OPTIMIZATION_NONE:
  ccontext->optimization_flags = 0;
  break;
  case PCRE2_OPTIMIZATION_FULL:
  ccontext->optimization_flags = PCRE2_OPTIMIZATION_ALL;
  break;
  default:
  if (directive >= PCRE2_AUTO_POSSESS && directive <= PCRE2_START_OPTIMIZE_OFF)
    {
    if ((directive & 1) != 0)
      ccontext->optimization_flags &= ~(1u << ((directive >> 1) - 32));
    else
      ccontext->optimization_flags |= 1u << ((directive >> 1) - 32);
    return 0;
    }
  return PCRE2_ERROR_BADOPTION;
  }
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_callout(pcre2_match_context *mcontext,
  int (*callout)(pcre2_callout_block *, void *), void *callout_data)
{
mcontext->callout = callout;
mcontext->callout_data = callout_data;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_substitute_callout(pcre2_match_context *mcontext,
  int (*substitute_callout)(pcre2_substitute_callout_block *, void *),
  void *substitute_callout_data)
{
mcontext->substitute_callout = substitute_callout;
mcontext->substitute_callout_data = substitute_callout_data;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_substitute_case_callout(pcre2_match_context *mcontext,
  PCRE2_SIZE (*substitute_case_callout)(PCRE2_SPTR, PCRE2_SIZE, PCRE2_UCHAR *,
                                        PCRE2_SIZE, int, void *),
  void *substitute_case_callout_data)
{
mcontext->substitute_case_callout = substitute_case_callout;
mcontext->substitute_case_callout_data = substitute_case_callout_data;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_heap_limit(pcre2_match_context *mcontext, uint32_t limit)
{
mcontext->heap_limit = limit;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_match_limit(pcre2_match_context *mcontext, uint32_t limit)
{
mcontext->match_limit = limit;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_depth_limit(pcre2_match_context *mcontext, uint32_t limit)
{
mcontext->depth_limit = limit;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_offset_limit(pcre2_match_context *mcontext, PCRE2_SIZE limit)
{
mcontext->offset_limit = limit;
return 0;
}
#ifdef ERLANG_INTEGRATION
PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_set_loops_left(pcre2_match_data *mdata, int budget)
{
mdata->loops_left = budget;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_get_loops_left(pcre2_match_data *mdata)
{
return mdata->loops_left;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_restart_data(pcre2_match_data *mdata, void **data)
{
mdata->restart_data = data;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_restart_flags(pcre2_match_data *mdata, uint32_t flags)
{
mdata->restart_flags = flags;
return 0;
}
#endif
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_recursion_limit(pcre2_match_context *mcontext, uint32_t limit)
{
return pcre2_set_depth_limit(mcontext, limit);
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_recursion_memory_management(pcre2_match_context *mcontext,
  void *(*mymalloc)(size_t, void *), void (*myfree)(void *, void *),
  void *mydata)
{
(void)mcontext;
(void)mymalloc;
(void)myfree;
(void)mydata;
return 0;
}
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_glob_separator(pcre2_convert_context *ccontext, uint32_t separator)
{
if (separator != CHAR_SLASH && separator != CHAR_BACKSLASH &&
    separator != CHAR_DOT) return PCRE2_ERROR_BADDATA;
ccontext->glob_separator = separator;
return 0;
}
static const char *globpunct =
  STR_EXCLAMATION_MARK STR_QUOTATION_MARK STR_NUMBER_SIGN STR_DOLLAR_SIGN
  STR_PERCENT_SIGN STR_AMPERSAND STR_APOSTROPHE STR_LEFT_PARENTHESIS
  STR_RIGHT_PARENTHESIS STR_ASTERISK STR_PLUS STR_COMMA STR_MINUS STR_DOT
  STR_SLASH STR_COLON STR_SEMICOLON STR_LESS_THAN_SIGN STR_EQUALS_SIGN
  STR_GREATER_THAN_SIGN STR_QUESTION_MARK STR_COMMERCIAL_AT
  STR_LEFT_SQUARE_BRACKET STR_BACKSLASH STR_RIGHT_SQUARE_BRACKET
  STR_CIRCUMFLEX_ACCENT STR_UNDERSCORE STR_GRAVE_ACCENT STR_LEFT_CURLY_BRACKET
  STR_VERTICAL_LINE STR_RIGHT_CURLY_BRACKET STR_TILDE;
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_glob_escape(pcre2_convert_context *ccontext, uint32_t escape)
{
if (escape > 255 || (escape != 0 && strchr(globpunct, escape) == NULL))
  return PCRE2_ERROR_BADDATA;
ccontext->glob_escape = escape;
return 0;
}
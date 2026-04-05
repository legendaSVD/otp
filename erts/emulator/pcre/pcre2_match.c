#include "pcre2_internal.h"
#ifdef DEBUG_FRAMES_DISPLAY
#include <stdarg.h>
#endif
#ifdef DEBUG_SHOW_OPS
static const char *OP_names[] = { OP_NAME_LIST };
#endif
#define NLBLOCK mb
#define PSSTART start_subject
#define PSEND   end_subject
#define RECURSE_UNSET 0xffffffffu
#define PUBLIC_MATCH_OPTIONS \
  (PCRE2_ANCHORED|PCRE2_ENDANCHORED|PCRE2_NOTBOL|PCRE2_NOTEOL|PCRE2_NOTEMPTY| \
   PCRE2_NOTEMPTY_ATSTART|PCRE2_NO_UTF_CHECK|PCRE2_PARTIAL_HARD| \
   PCRE2_PARTIAL_SOFT|PCRE2_NO_JIT|PCRE2_COPY_MATCHED_SUBJECT| \
   PCRE2_DISABLE_RECURSELOOP_CHECK)
#define PUBLIC_JIT_MATCH_OPTIONS \
   (PCRE2_NO_UTF_CHECK|PCRE2_NOTBOL|PCRE2_NOTEOL|PCRE2_NOTEMPTY|\
    PCRE2_NOTEMPTY_ATSTART|PCRE2_PARTIAL_SOFT|PCRE2_PARTIAL_HARD|\
    PCRE2_COPY_MATCHED_SUBJECT)
#define MATCH_MATCH        1
#define MATCH_NOMATCH      0
#define MATCH_ACCEPT       (-999)
#define MATCH_KETRPOS      (-998)
#define MATCH_COMMIT       (-997)
#define MATCH_PRUNE        (-996)
#define MATCH_SKIP         (-995)
#define MATCH_SKIP_ARG     (-994)
#define MATCH_THEN         (-993)
#define MATCH_BACKTRACK_MAX MATCH_THEN
#define MATCH_BACKTRACK_MIN MATCH_COMMIT
#define GF_CAPTURE     0x00010000u
#define GF_NOCAPTURE   0x00020000u
#define GF_CONDASSERT  0x00030000u
#define GF_RECURSE     0x00040000u
#define GF_IDMASK(a)   ((a) & 0xffff0000u)
#define GF_DATAMASK(a) ((a) & 0x0000ffffu)
enum { REPTYPE_MIN, REPTYPE_MAX, REPTYPE_POS };
static const uint32_t rep_min[] = {
  0, 0,
  1, 1,
  0, 0,
  0, 0,
  0, 1, 0 };
static const uint32_t rep_max[] = {
  UINT32_MAX, UINT32_MAX,
  UINT32_MAX, UINT32_MAX,
  1, 1,
  0, 0,
  UINT32_MAX, UINT32_MAX, 1 };
static const uint32_t rep_typ[] = {
  REPTYPE_MAX, REPTYPE_MIN,
  REPTYPE_MAX, REPTYPE_MIN,
  REPTYPE_MAX, REPTYPE_MIN,
  REPTYPE_MAX, REPTYPE_MIN,
  REPTYPE_POS, REPTYPE_POS,
  REPTYPE_POS, REPTYPE_POS };
enum { RM1=1, RM2,  RM3,  RM4,  RM5,  RM6,  RM7,  RM8,  RM9,  RM10,
       RM11,  RM12, RM13, RM14, RM15, RM16, RM17, RM18, RM19, RM20,
       RM21,  RM22, RM23, RM24, RM25, RM26, RM27, RM28, RM29, RM30,
       RM31,  RM32, RM33, RM34, RM35, RM36, RM37, RM38, RM39 };
#ifdef SUPPORT_WIDE_CHARS
enum { RM100=100, RM101, RM102, RM103 };
#endif
#ifdef SUPPORT_UNICODE
enum { RM200=200, RM201, RM202, RM203, RM204, RM205, RM206, RM207,
       RM208,     RM209, RM210, RM211, RM212, RM213, RM214, RM215,
       RM216,     RM217, RM218, RM219, RM220, RM221, RM222, RM223,
       RM224 };
#endif
#define Fback_frame        F->back_frame
#define Fcapture_last      F->capture_last
#define Fcurrent_recurse   F->current_recurse
#define Fecode             F->ecode
#define Feptr              F->eptr
#define Fgroup_frame_type  F->group_frame_type
#define Flast_group_offset F->last_group_offset
#define Flength            F->length
#define Fmark              F->mark
#define Frdepth            F->rdepth
#define Fstart_match       F->start_match
#define Foffset_top        F->offset_top
#define Foccu              F->occu
#define Fop                F->op
#define Fovector           F->ovector
#define Freturn_id         F->return_id
#ifdef DEBUG_FRAMES_DISPLAY
static void
display_frames(FILE *f, heapframe *F, heapframe *P, PCRE2_SIZE frame_size,
  match_block *mb, pcre2_match_data *match_data, const char *s, ...)
{
uint32_t i;
heapframe *Q;
va_list ap;
va_start(ap, s);
fprintf(f, "FRAMES ");
vfprintf(f, s, ap);
va_end(ap);
if (P != NULL) fprintf(f, " P=%lu",
  ((char *)P - (char *)(match_data->heapframes))/frame_size);
fprintf(f, "\n");
for (i = 0, Q = match_data->heapframes;
     Q <= F;
     i++, Q = (heapframe *)((char *)Q + frame_size))
  {
  fprintf(f, "Frame %d type=%x subj=%lu code=%d back=%lu id=%d",
    i, Q->group_frame_type, Q->eptr - mb->start_subject, *(Q->ecode),
    Q->back_frame, Q->return_id);
  if (Q->last_group_offset == PCRE2_UNSET)
    fprintf(f, " lgoffset=unset\n");
  else
    fprintf(f, " lgoffset=%lu\n",  Q->last_group_offset/frame_size);
  }
}
#endif
#ifdef ERLANG_INTEGRATION
#define NAME_XCAT(A,B) A##B
#define NAME_CAT(A,B) NAME_XCAT(A,B)
#ifdef DEBUG
const int erts_dbg_pcre_cost_chk_lines[] = {
#include "pcre2_match_yield_coverage.gen.h"
};
uint64_t erts_dbg_pcre_cost_chk_visits[ERLANG_YIELD_POINT_CNT];
uint64_t erts_dbg_pcre_cost_chk_yields[ERLANG_YIELD_POINT_CNT];
const int erts_dbg_pcre_cost_chk_cnt = ERLANG_YIELD_POINT_CNT;
# define DBG_COST_CHK_VISIT() ++(erts_dbg_pcre_cost_chk_visits[NAME_CAT(ERLANG_YIELD_POINT_,__LINE__)])
# define DBG_COST_CHK_YIELD() ++(erts_dbg_pcre_cost_chk_yields[NAME_CAT(ERLANG_YIELD_POINT_,__LINE__)])
# define DBG_FAKE_COST_CHK() (DBG_COST_CHK_VISIT(), DBG_COST_CHK_YIELD())
#else
# define DBG_COST_CHK_VISIT()
# define DBG_COST_CHK_YIELD()
# define DBG_FAKE_COST_CHK()
#endif
#ifdef ERLANG_DEBUG
#include <stdarg.h>
static void
edebug_printf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  fprintf(stderr, "PCRE2: ");
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\r\n");
}
#endif
#if !defined(__GNUC__) || defined(__e2k__)
#  define ERTS_AT_LEAST_GCC_VSN__(MAJ, MIN, PL) 0
#elif !defined(__GNUC_MINOR__)
#  define ERTS_AT_LEAST_GCC_VSN__(MAJ, MIN, PL) \
  ((__GNUC__ << 24) >= (((MAJ) << 24) | ((MIN) << 12) | (PL)))
#elif !defined(__GNUC_PATCHLEVEL__)
#  define ERTS_AT_LEAST_GCC_VSN__(MAJ, MIN, PL) \
  (((__GNUC__ << 24) | (__GNUC_MINOR__ << 12)) >= (((MAJ) << 24) | ((MIN) << 12) | (PL)))
#else
#  define ERTS_AT_LEAST_GCC_VSN__(MAJ, MIN, PL) \
  (((__GNUC__ << 24) | (__GNUC_MINOR__ << 12) | __GNUC_PATCHLEVEL__) >= (((MAJ) << 24) | ((MIN) << 12) | (PL)))
#endif
#endif
static int
do_callout(heapframe *F, match_block *mb, PCRE2_SIZE *lengthptr)
{
int rc;
PCRE2_SIZE save0, save1;
PCRE2_SIZE *callout_ovector;
pcre2_callout_block *cb;
*lengthptr = (*Fecode == OP_CALLOUT)?
  PRIV(OP_lengths)[OP_CALLOUT] : GET(Fecode, 1 + 2*LINK_SIZE);
if (mb->callout == NULL) return 0;
callout_ovector = (PCRE2_SIZE *)(Fovector) - 2;
cb = mb->cb;
cb->capture_top      = (uint32_t)Foffset_top/2 + 1;
cb->capture_last     = Fcapture_last;
cb->offset_vector    = callout_ovector;
cb->mark             = mb->nomatch_mark;
cb->current_position = (PCRE2_SIZE)(Feptr - mb->start_subject);
cb->pattern_position = GET(Fecode, 1);
cb->next_item_length = GET(Fecode, 1 + LINK_SIZE);
if (*Fecode == OP_CALLOUT)
  {
  cb->callout_number = Fecode[1 + 2*LINK_SIZE];
  cb->callout_string_offset = 0;
  cb->callout_string = NULL;
  cb->callout_string_length = 0;
  }
else
  {
  cb->callout_number = 0;
  cb->callout_string_offset = GET(Fecode, 1 + 3*LINK_SIZE);
  cb->callout_string = Fecode + (1 + 4*LINK_SIZE) + 1;
  cb->callout_string_length =
    *lengthptr - (1 + 4*LINK_SIZE) - 2;
  }
save0 = callout_ovector[0];
save1 = callout_ovector[1];
callout_ovector[0] = callout_ovector[1] = PCRE2_UNSET;
rc = mb->callout(cb, mb->callout_data);
callout_ovector[0] = save0;
callout_ovector[1] = save1;
cb->callout_flags = 0;
return rc;
}
static int
match_ref(PCRE2_SIZE offset, BOOL caseless, int caseopts, heapframe *F,
  match_block *mb, PCRE2_SIZE *lengthptr)
{
PCRE2_SPTR p;
PCRE2_SIZE length;
PCRE2_SPTR eptr;
PCRE2_SPTR eptr_start;
#ifndef SUPPORT_UNICODE
(void)caseopts;
#endif
if (offset >= Foffset_top || Fovector[offset] == PCRE2_UNSET)
  {
  if ((mb->poptions & PCRE2_MATCH_UNSET_BACKREF) != 0)
    {
    *lengthptr = 0;
    return 0;
    }
  else return -1;
  }
eptr = eptr_start = Feptr;
p = mb->start_subject + Fovector[offset];
length = Fovector[offset+1] - Fovector[offset];
PCRE2_ASSERT(eptr <= mb->end_subject);
if (caseless)
  {
#if defined SUPPORT_UNICODE
  BOOL utf = (mb->poptions & PCRE2_UTF) != 0;
  BOOL caseless_restrict = (caseopts & REFI_FLAG_CASELESS_RESTRICT) != 0;
  BOOL turkish_casing = !caseless_restrict && (caseopts & REFI_FLAG_TURKISH_CASING) != 0;
  if (utf || (mb->poptions & PCRE2_UCP) != 0)
    {
    PCRE2_SPTR endptr = p + length;
    while (p < endptr)
      {
      uint32_t c, d;
      const ucd_record *ur;
      if (eptr >= mb->end_subject) return 1;
      if (utf)
        {
        GETCHARINC(c, eptr);
        GETCHARINC(d, p);
        }
      else
        {
        c = *eptr++;
        d = *p++;
        }
      if (turkish_casing && UCD_ANY_I(d))
        {
        c = UCD_FOLD_I_TURKISH(c);
        d = UCD_FOLD_I_TURKISH(d);
        if (c != d) return -1;
        }
      else if (c != d && c != (uint32_t)((int)d + (ur = GET_UCD(d))->other_case))
        {
        const uint32_t *pp = PRIV(ucd_caseless_sets) + ur->caseset;
        if (caseless_restrict && *pp < 128) return -1;
        for (;;)
          {
          if (c < *pp) return -1;
          if (c == *pp++) break;
          }
        }
      }
    }
  else
#endif
    {
    for (; length > 0; length--)
      {
      uint32_t cc, cp;
      if (eptr >= mb->end_subject) return 1;
      cc = UCHAR21TEST(eptr);
      cp = UCHAR21TEST(p);
      if (TABLE_GET(cp, mb->lcc, cp) != TABLE_GET(cc, mb->lcc, cc))
        return -1;
      p++;
      eptr++;
      }
    }
  }
else
  {
  if (mb->partial != 0)
    {
    for (; length > 0; length--)
      {
      if (eptr >= mb->end_subject) return 1;
      if (UCHAR21INCTEST(p) != UCHAR21INCTEST(eptr)) return -1;
      }
    }
  else
    {
    if ((PCRE2_SIZE)(mb->end_subject - eptr) < length ||
        memcmp(p, eptr, CU2BYTES(length)) != 0) return -1;
    eptr += length;
    }
  }
*lengthptr = eptr - eptr_start;
return 0;
}
static void
recurse_update_offsets(heapframe *F, heapframe *P)
{
PCRE2_SIZE *dst = F->ovector;
PCRE2_SIZE *src = P->ovector;
PCRE2_SIZE offset = 2;
PCRE2_SIZE offset_top = Foffset_top + 2;
PCRE2_SIZE diff;
PCRE2_SPTR ecode = Fecode;
do
  {
  diff = (GET2(ecode, 1) << 1) - offset;
  ecode += 1 + IMM2_SIZE;
  if (offset + diff >= offset_top)
    {
    while (*ecode == OP_CREF) ecode += 1 + IMM2_SIZE;
    break;
    }
  if (diff == 2)
    {
    dst[0] = src[0];
    dst[1] = src[1];
    }
  else if (diff >= 4)
    memcpy(dst, src, diff * sizeof(PCRE2_SIZE));
  diff += 2;
  offset += diff;
  dst += diff;
  src += diff;
  }
while (*ecode == OP_CREF);
diff = offset_top - offset;
if (diff == 2)
  {
  dst[0] = src[0];
  dst[1] = src[1];
  }
else if (diff >= 4)
  memcpy(dst, src, diff * sizeof(PCRE2_SIZE));
Fecode = ecode;
Foffset_top = (offset <= P->offset_top) ? P->offset_top : (offset - 2);
}
#define CHECK_PARTIAL() \
  do { \
     if (Feptr >= mb->end_subject) \
       { \
       SCHECK_PARTIAL(); \
       } \
     } \
  while (0)
#define SCHECK_PARTIAL() \
  do { \
     if (mb->partial != 0 && \
         (Feptr > mb->start_used_ptr || mb->allowemptypartial)) \
       { \
       mb->hitend = TRUE; \
       if (mb->partial > 1) return PCRE2_ERROR_PARTIAL; \
       } \
     } \
  while (0)
#define RMATCH(ra,rb) \
  do { \
     start_ecode = ra; \
     Freturn_id = rb; \
     goto MATCH_RECURSE; \
     L_##rb:; \
     } \
  while (0)
#define RRETURN(ra) \
  do { \
     rrc = ra; \
     goto RETURN_SWITCH; \
     } \
  while (0)
static int
match(PCRE2_SPTR start_eptr, PCRE2_SPTR start_ecode, uint16_t top_bracket,
  PCRE2_SIZE frame_size, pcre2_match_data *match_data, match_block *mb)
{
heapframe *F;
heapframe *N = NULL;
heapframe *P = NULL;
heapframe *frames_top;
heapframe *assert_accept_frame = NULL;
PCRE2_SIZE frame_copy_size;
PCRE2_SPTR branch_end = NULL;
PCRE2_SPTR branch_start;
PCRE2_SPTR bracode;
PCRE2_SIZE offset;
PCRE2_SIZE length;
int rrc;
#ifdef SUPPORT_UNICODE
int proptype;
#endif
uint32_t i;
uint32_t fc;
uint32_t number;
uint32_t reptype = 0;
uint32_t group_frame_type;
BOOL condition;
BOOL cur_is_word;
BOOL prev_is_word;
#ifdef SUPPORT_UNICODE
BOOL notmatch;
#endif
#if defined(ERLANG_INTEGRATION)
BOOL samelengths;
int lgb;
int rgb;
#ifdef ERLANG_DEBUG
#define EDEBUGF(X) edebug_printf X
#else
#define EDEBUGF(X)
#endif
int32_t* restrict loops_left_p = &match_data->loops_left;
#define COST(N) (*loops_left_p -= (N))
#define COST_CHK(N) 				\
do {						\
  *loops_left_p -= (N);	                        \
  DBG_COST_CHK_VISIT();                         \
  if (*loops_left_p <= 0) {                     \
      DBG_COST_CHK_YIELD();                     \
      Freturn_id = __LINE__ ;	                \
      goto LOOP_COUNT_BREAK;			\
      NAME_CAT(L_LOOP_COUNT_,__LINE__):	        \
      ;                                         \
  }						\
} while (0)
#else
#define COST(N)
#define COST_CHK(N)
#endif
#ifdef SUPPORT_UNICODE
BOOL utf = (mb->poptions & PCRE2_UTF) != 0;
BOOL ucp = (mb->poptions & PCRE2_UCP) != 0;
#else
BOOL utf = FALSE;
#endif
frame_copy_size = frame_size - offsetof(heapframe, eptr);
#ifdef ERLANG_INTEGRATION
if (mb->state_save) {
  F = mb->state_save;
  EDEBUGF(("Break restore!"));
  goto LOOP_COUNT_RETURN;
}
#endif
F = match_data->heapframes;
frames_top = (heapframe *)((char *)F + match_data->heapframes_size);
Frdepth = 0;
Fcapture_last = 0;
Fcurrent_recurse = RECURSE_UNSET;
Fstart_match = Feptr = start_eptr;
Fmark = NULL;
Foffset_top = 0;
Flast_group_offset = PCRE2_UNSET;
group_frame_type = 0;
goto NEW_FRAME;
MATCH_RECURSE:
N = (heapframe *)((char *)F + frame_size);
if ((heapframe *)((char *)N + frame_size) >= frames_top)
  {
  heapframe *new;
  PCRE2_SIZE newsize;
  PCRE2_SIZE usedsize = (char *)N - (char *)(match_data->heapframes);
  if (match_data->heapframes_size >= PCRE2_SIZE_MAX / 2)
    {
    if (match_data->heapframes_size == PCRE2_SIZE_MAX - 1)
      return PCRE2_ERROR_NOMEMORY;
    newsize = PCRE2_SIZE_MAX - 1;
    }
  else
    newsize = match_data->heapframes_size * 2;
  if (newsize / 1024 >= mb->heap_limit)
    {
    PCRE2_SIZE old_size = match_data->heapframes_size / 1024;
    if (mb->heap_limit <= old_size)
      return PCRE2_ERROR_HEAPLIMIT;
    else
      {
      PCRE2_SIZE max_delta = 1024 * (mb->heap_limit - old_size);
      int over_bytes = match_data->heapframes_size % 1024;
      if (over_bytes) max_delta -= (1024 - over_bytes);
      newsize = match_data->heapframes_size + max_delta;
      }
    }
  if (newsize - usedsize < frame_size) return PCRE2_ERROR_HEAPLIMIT;
  new = match_data->memctl.malloc(newsize+frame_size, match_data->memctl.memory_data);
  if (new == NULL) return PCRE2_ERROR_NOMEMORY;
  memcpy(new, match_data->heapframes, usedsize);
  N = (heapframe *)((char *)new + usedsize);
  F = (heapframe *)((char *)N - frame_size);
  match_data->memctl.free(match_data->heapframes, match_data->memctl.memory_data);
  match_data->heapframes = new;
  match_data->heapframes_size = newsize;
  frames_top = (heapframe *)((char *)new + newsize);
  }
#ifdef DEBUG_SHOW_RMATCH
fprintf(stderr, "++ RMATCH %d frame=%d", Freturn_id, Frdepth + 1);
if (group_frame_type != 0)
  {
  fprintf(stderr, " type=%x ", group_frame_type);
  switch (GF_IDMASK(group_frame_type))
    {
    case GF_CAPTURE:
    fprintf(stderr, "capture=%d", GF_DATAMASK(group_frame_type));
    break;
    case GF_NOCAPTURE:
    fprintf(stderr, "nocapture op=%d", GF_DATAMASK(group_frame_type));
    break;
    case GF_CONDASSERT:
    fprintf(stderr, "condassert op=%d", GF_DATAMASK(group_frame_type));
    break;
    case GF_RECURSE:
    fprintf(stderr, "recurse=%d", GF_DATAMASK(group_frame_type));
    break;
    default:
    fprintf(stderr, "*** unknown ***");
    break;
    }
  }
fprintf(stderr, "\n");
#endif
memcpy((char *)N + offsetof(heapframe, eptr),
       (char *)F + offsetof(heapframe, eptr),
       frame_copy_size);
N->rdepth = Frdepth + 1;
F = N;
NEW_FRAME:
Fgroup_frame_type = group_frame_type;
Fecode = start_ecode;
Fback_frame = frame_size;
if (group_frame_type != 0)
  {
  Flast_group_offset = (char *)F - (char *)match_data->heapframes;
  if (GF_IDMASK(group_frame_type) == GF_RECURSE)
    Fcurrent_recurse = GF_DATAMASK(group_frame_type);
  group_frame_type = 0;
  }
if (mb->match_call_count++ >= mb->match_limit) return PCRE2_ERROR_MATCHLIMIT;
if (Frdepth >= mb->match_limit_depth) return PCRE2_ERROR_DEPTHLIMIT;
#ifdef DEBUG_SHOW_OPS
fprintf(stderr, "\n++ New frame: type=0x%x subject offset %ld\n",
  GF_IDMASK(Fgroup_frame_type), Feptr - mb->start_subject);
#endif
for (;;)
  {
#ifdef DEBUG_SHOW_OPS
fprintf(stderr, "++ %2ld op=%3d %s\n", Fecode - mb->start_code, *Fecode,
  OP_names[*Fecode]);
#endif
  COST_CHK(1);
  Fop = (uint8_t)(*Fecode);
  switch(Fop)
    {
    case OP_CLOSE:
    if (Fcurrent_recurse == RECURSE_UNSET)
      {
      number = GET2(Fecode, 1);
      offset = Flast_group_offset;
      for(;;)
        {
        PCRE2_ASSERT(offset != PCRE2_UNSET);
        if (offset == PCRE2_UNSET) return PCRE2_ERROR_INTERNAL;
        N = (heapframe *)((char *)match_data->heapframes + offset);
        P = (heapframe *)((char *)N - frame_size);
        if (N->group_frame_type == (GF_CAPTURE | number)) break;
        offset = P->last_group_offset;
        COST(1);
        }
      offset = (number << 1) - 2;
      Fcapture_last = number;
      Fovector[offset] = P->eptr - mb->start_subject;
      Fovector[offset+1] = Feptr - mb->start_subject;
      if (offset >= Foffset_top) Foffset_top = offset + 2;
      }
    Fecode += PRIV(OP_lengths)[*Fecode];
    break;
    case OP_ASSERT_ACCEPT:
    if (Feptr > mb->last_used_ptr) mb->last_used_ptr = Feptr;
    assert_accept_frame = F;
    RRETURN(MATCH_ACCEPT);
    case OP_ACCEPT:
    if (Fcurrent_recurse != RECURSE_UNSET)
      {
#ifdef DEBUG_SHOW_OPS
      fprintf(stderr, "++ Accept within recursion\n");
#endif
      offset = Flast_group_offset;
      for(;;)
        {
        PCRE2_ASSERT(offset != PCRE2_UNSET);
        if (offset == PCRE2_UNSET) return PCRE2_ERROR_INTERNAL;
        N = (heapframe *)((char *)match_data->heapframes + offset);
        P = (heapframe *)((char *)N - frame_size);
        if (GF_IDMASK(N->group_frame_type) == GF_RECURSE) break;
        offset = P->last_group_offset;
        COST(1);
        }
      P->eptr = Feptr;
      P->mark = Fmark;
      P->start_match = Fstart_match;
      F = P;
      Fecode += 1 + LINK_SIZE;
      continue;
      }
    PCRE2_FALLTHROUGH
    case OP_END:
    if (Feptr == Fstart_match &&
         ((mb->moptions & PCRE2_NOTEMPTY) != 0 ||
           ((mb->moptions & PCRE2_NOTEMPTY_ATSTART) != 0 &&
             Fstart_match == mb->start_subject + mb->start_offset)))
      {
#ifdef DEBUG_SHOW_OPS
      fprintf(stderr, "++ Backtrack because empty string\n");
#endif
      RRETURN(MATCH_NOMATCH);
      }
    if (Feptr < mb->end_subject &&
        ((mb->moptions | mb->poptions) & PCRE2_ENDANCHORED) != 0)
      {
      if (Fop == OP_END)
        {
#ifdef DEBUG_SHOW_OPS
        fprintf(stderr, "++ Backtrack because not at end (endanchored set)\n");
#endif
        RRETURN(MATCH_NOMATCH);
        }
#ifdef DEBUG_SHOW_OPS
      fprintf(stderr, "++ Failed ACCEPT not at end (endanchored set)\n");
#endif
      return MATCH_NOMATCH;
      }
    if (Fstart_match < mb->start_subject + mb->start_offset ||
        Fstart_match > Feptr)
      {
      PCRE2_ASSERT(mb->hasbsk);
      if (!mb->allowlookaroundbsk)
        return PCRE2_ERROR_BAD_BACKSLASH_K;
      }
    mb->end_match_ptr = Feptr;
    mb->end_offset_top = Foffset_top;
    mb->mark = Fmark;
    if (Feptr > mb->last_used_ptr) mb->last_used_ptr = Feptr;
    match_data->ovector[0] = Fstart_match - mb->start_subject;
    match_data->ovector[1] = Feptr - mb->start_subject;
    i = 2 * ((top_bracket + 1 > match_data->oveccount)?
      match_data->oveccount : top_bracket + 1);
    memcpy(match_data->ovector + 2, Fovector, (i - 2) * sizeof(PCRE2_SIZE));
    while (--i >= Foffset_top + 2) match_data->ovector[i] = PCRE2_UNSET;
    return MATCH_MATCH;
    case OP_ANY:
    if (IS_NEWLINE(Feptr)) RRETURN(MATCH_NOMATCH);
    if (mb->partial != 0 &&
        Feptr == mb->end_subject - 1 &&
        NLBLOCK->nltype == NLTYPE_FIXED &&
        NLBLOCK->nllen == 2 &&
        UCHAR21TEST(Feptr) == NLBLOCK->nl[0])
      {
      mb->hitend = TRUE;
      if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
      }
    PCRE2_FALLTHROUGH
    case OP_ALLANY:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    Feptr++;
#ifdef SUPPORT_UNICODE
    if (utf) ACROSSCHAR(Feptr < mb->end_subject, Feptr, Feptr++);
#endif
    Fecode++;
    break;
    case OP_ANYBYTE:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    Feptr++;
    Fecode++;
    break;
    case OP_CHAR:
#ifdef SUPPORT_UNICODE
    if (utf)
      {
      Flength = 1;
      Fecode++;
      GETCHARLEN(fc, Fecode, Flength);
      if (Flength > (PCRE2_SIZE)(mb->end_subject - Feptr))
        {
        CHECK_PARTIAL();
        RRETURN(MATCH_NOMATCH);
        }
      for (; Flength > 0; Flength--)
        {
        if (*Fecode++ != UCHAR21INC(Feptr)) RRETURN(MATCH_NOMATCH);
        COST(1);
        }
      }
    else
#endif
      {
      if (mb->end_subject - Feptr < 1)
        {
        SCHECK_PARTIAL();
        RRETURN(MATCH_NOMATCH);
        }
      if (Fecode[1] != *Feptr++) RRETURN(MATCH_NOMATCH);
      Fecode += 2;
      }
    break;
    case OP_CHARI:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
#ifdef SUPPORT_UNICODE
    if (utf)
      {
      Flength = 1;
      Fecode++;
      GETCHARLEN(fc, Fecode, Flength);
      if (fc < 128)
        {
        uint32_t cc = UCHAR21(Feptr);
        if (mb->lcc[fc] != TABLE_GET(cc, mb->lcc, cc)) RRETURN(MATCH_NOMATCH);
        Fecode++;
        Feptr++;
        }
      else
        {
        uint32_t dc;
        GETCHARINC(dc, Feptr);
        Fecode += Flength;
        if (dc != fc && dc != UCD_OTHERCASE(fc)) RRETURN(MATCH_NOMATCH);
        }
      }
    else if (ucp)
      {
      uint32_t cc = UCHAR21(Feptr);
      fc = Fecode[1];
      if (fc < 128)
        {
        if (mb->lcc[fc] != TABLE_GET(cc, mb->lcc, cc)) RRETURN(MATCH_NOMATCH);
        }
      else
        {
        if (cc != fc && cc != UCD_OTHERCASE(fc)) RRETURN(MATCH_NOMATCH);
        }
      Feptr++;
      Fecode += 2;
      }
    else
#endif
      {
      if (TABLE_GET(Fecode[1], mb->lcc, Fecode[1])
          != TABLE_GET(*Feptr, mb->lcc, *Feptr)) RRETURN(MATCH_NOMATCH);
      Feptr++;
      Fecode += 2;
      }
    break;
    case OP_NOT:
    case OP_NOTI:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
#ifdef SUPPORT_UNICODE
    if (utf)
      {
      uint32_t ch;
      Fecode++;
      GETCHARINC(ch, Fecode);
      GETCHARINC(fc, Feptr);
      if (ch == fc)
        {
        RRETURN(MATCH_NOMATCH);
        }
      else if (Fop == OP_NOTI)
        {
        if (ch > 127)
          ch = UCD_OTHERCASE(ch);
        else
          ch = (mb->fcc)[ch];
        if (ch == fc) RRETURN(MATCH_NOMATCH);
        }
      }
    else if (ucp)
      {
      uint32_t ch;
      fc = UCHAR21INC(Feptr);
      ch = Fecode[1];
      Fecode += 2;
      if (ch == fc)
        {
        RRETURN(MATCH_NOMATCH);
        }
      else if (Fop == OP_NOTI)
        {
        if (ch > 127)
          ch = UCD_OTHERCASE(ch);
        else
          ch = (mb->fcc)[ch];
        if (ch == fc) RRETURN(MATCH_NOMATCH);
        }
      }
    else
#endif
      {
      uint32_t ch = Fecode[1];
      fc = UCHAR21INC(Feptr);
      if (ch == fc || (Fop == OP_NOTI && TABLE_GET(ch, mb->fcc, ch) == fc))
        RRETURN(MATCH_NOMATCH);
      Fecode += 2;
      }
    break;
#define Loclength    F->temp_size
#define Lstart_eptr  F->temp_sptr[0]
#define Lcharptr     F->temp_sptr[1]
#define Lmin         F->temp_32[0]
#define Lmax         F->temp_32[1]
#define Lc           F->temp_32[2]
#define Loc          F->temp_32[3]
    case OP_EXACT:
    case OP_EXACTI:
    Lmin = Lmax = GET2(Fecode, 1);
    Fecode += 1 + IMM2_SIZE;
    goto REPEATCHAR;
    case OP_POSUPTO:
    case OP_POSUPTOI:
    reptype = REPTYPE_POS;
    Lmin = 0;
    Lmax = GET2(Fecode, 1);
    Fecode += 1 + IMM2_SIZE;
    goto REPEATCHAR;
    case OP_UPTO:
    case OP_UPTOI:
    reptype = REPTYPE_MAX;
    Lmin = 0;
    Lmax = GET2(Fecode, 1);
    Fecode += 1 + IMM2_SIZE;
    goto REPEATCHAR;
    case OP_MINUPTO:
    case OP_MINUPTOI:
    reptype = REPTYPE_MIN;
    Lmin = 0;
    Lmax = GET2(Fecode, 1);
    Fecode += 1 + IMM2_SIZE;
    goto REPEATCHAR;
    case OP_POSSTAR:
    case OP_POSSTARI:
    reptype = REPTYPE_POS;
    Lmin = 0;
    Lmax = UINT32_MAX;
    Fecode++;
    goto REPEATCHAR;
    case OP_POSPLUS:
    case OP_POSPLUSI:
    reptype = REPTYPE_POS;
    Lmin = 1;
    Lmax = UINT32_MAX;
    Fecode++;
    goto REPEATCHAR;
    case OP_POSQUERY:
    case OP_POSQUERYI:
    reptype = REPTYPE_POS;
    Lmin = 0;
    Lmax = 1;
    Fecode++;
    goto REPEATCHAR;
    case OP_STAR:
    case OP_STARI:
    case OP_MINSTAR:
    case OP_MINSTARI:
    case OP_PLUS:
    case OP_PLUSI:
    case OP_MINPLUS:
    case OP_MINPLUSI:
    case OP_QUERY:
    case OP_QUERYI:
    case OP_MINQUERY:
    case OP_MINQUERYI:
    fc = *Fecode++ - ((Fop < OP_STARI)? OP_STAR : OP_STARI);
    Lmin = rep_min[fc];
    Lmax = rep_max[fc];
    reptype = rep_typ[fc];
    REPEATCHAR:
#ifdef SUPPORT_UNICODE
    if (utf)
      {
      Flength = 1;
      Lcharptr = Fecode;
      GETCHARLEN(fc, Fecode, Flength);
      Fecode += Flength;
      if (Flength > 1)
        {
        uint32_t othercase;
        if (Fop >= OP_STARI &&
            (othercase = UCD_OTHERCASE(fc)) != fc)
          Loclength = PRIV(ord2utf)(othercase, Foccu);
        else Loclength = 0;
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr <= mb->end_subject - Flength &&
            memcmp(Feptr, Lcharptr, CU2BYTES(Flength)) == 0) Feptr += Flength;
          else if (Loclength > 0 &&
                   Feptr <= mb->end_subject - Loclength &&
                   memcmp(Feptr, Foccu, CU2BYTES(Loclength)) == 0)
            Feptr += Loclength;
          else
            {
            CHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          }
        if (Lmin == Lmax) continue;
        if (reptype == REPTYPE_MIN)
          {
          for (;;)
            {
            RMATCH(Fecode, RM202);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr <= mb->end_subject - Flength &&
              memcmp(Feptr, Lcharptr, CU2BYTES(Flength)) == 0) Feptr += Flength;
            else if (Loclength > 0 &&
                     Feptr <= mb->end_subject - Loclength &&
                     memcmp(Feptr, Foccu, CU2BYTES(Loclength)) == 0)
              Feptr += Loclength;
            else
              {
              CHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            }
          PCRE2_UNREACHABLE();
          }
        else
          {
          Lstart_eptr = Feptr;
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr <= mb->end_subject - Flength &&
                memcmp(Feptr, Lcharptr, CU2BYTES(Flength)) == 0)
              Feptr += Flength;
            else if (Loclength > 0 &&
                     Feptr <= mb->end_subject - Loclength &&
                     memcmp(Feptr, Foccu, CU2BYTES(Loclength)) == 0)
              Feptr += Loclength;
            else
              {
              CHECK_PARTIAL();
              break;
              }
            COST(1);
            }
          if (reptype != REPTYPE_POS) for(;;)
            {
            if (Feptr <= Lstart_eptr) break;
            RMATCH(Fecode, RM203);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            Feptr--;
            BACKCHAR(Feptr);
            }
          }
        break;
        }
      Lc = fc;
      }
    else
#endif
    Lc = *Fecode++;
    if (Fop >= OP_STARI)
      {
#if PCRE2_CODE_UNIT_WIDTH == 8
#ifdef SUPPORT_UNICODE
      if (ucp && !utf && Lc > 127) Loc = UCD_OTHERCASE(Lc);
      else
#endif
      Loc = mb->fcc[Lc];
#else
#ifdef SUPPORT_UNICODE
      if ((utf || ucp) && Lc > 127) Loc = UCD_OTHERCASE(Lc);
      else
#endif
      Loc = TABLE_GET(Lc, mb->fcc, Lc);
#endif
      for (i = 1; i <= Lmin; i++)
        {
        uint32_t cc;
        if (Feptr >= mb->end_subject)
          {
          SCHECK_PARTIAL();
          RRETURN(MATCH_NOMATCH);
          }
        cc = UCHAR21TEST(Feptr);
        if (Lc != cc && Loc != cc) RRETURN(MATCH_NOMATCH);
        Feptr++;
        COST_CHK(1);
        }
      if (Lmin == Lmax) continue;
      if (reptype == REPTYPE_MIN)
        {
        for (;;)
          {
          uint32_t cc;
          RMATCH(Fecode, RM25);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          cc = UCHAR21TEST(Feptr);
          if (Lc != cc && Loc != cc) RRETURN(MATCH_NOMATCH);
          Feptr++;
          }
        PCRE2_UNREACHABLE();
        }
      else
        {
        Lstart_eptr = Feptr;
        for (i = Lmin; i < Lmax; i++)
          {
          uint32_t cc;
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            break;
            }
          cc = UCHAR21TEST(Feptr);
          if (Lc != cc && Loc != cc) break;
          Feptr++;
          COST_CHK(1);
          }
        if (reptype != REPTYPE_POS) for (;;)
          {
          if (Feptr == Lstart_eptr) break;
          RMATCH(Fecode, RM26);
          Feptr--;
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          }
        }
      }
    else
      {
      for (i = 1; i <= Lmin; i++)
        {
        if (Feptr >= mb->end_subject)
          {
          SCHECK_PARTIAL();
          RRETURN(MATCH_NOMATCH);
          }
        if (Lc != UCHAR21INCTEST(Feptr)) RRETURN(MATCH_NOMATCH);
        COST(1);
        }
      if (Lmin == Lmax) continue;
      if (reptype == REPTYPE_MIN)
        {
        for (;;)
          {
          RMATCH(Fecode, RM27);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (Lc != UCHAR21INCTEST(Feptr)) RRETURN(MATCH_NOMATCH);
          }
        PCRE2_UNREACHABLE();
        }
      else
        {
        Lstart_eptr = Feptr;
        for (i = Lmin; i < Lmax; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            break;
            }
          if (Lc != UCHAR21TEST(Feptr)) break;
          Feptr++;
          COST(1);
          }
        if (reptype != REPTYPE_POS) for (;;)
          {
          if (Feptr <= Lstart_eptr) break;
          RMATCH(Fecode, RM28);
          Feptr--;
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          }
        }
      }
    break;
#undef Loclength
#undef Lstart_eptr
#undef Lcharptr
#undef Lmin
#undef Lmax
#undef Lc
#undef Loc
#define Lstart_eptr  F->temp_sptr[0]
#define Lmin         F->temp_32[0]
#define Lmax         F->temp_32[1]
#define Lc           F->temp_32[2]
#define Loc          F->temp_32[3]
    case OP_NOTEXACT:
    case OP_NOTEXACTI:
    Lmin = Lmax = GET2(Fecode, 1);
    Fecode += 1 + IMM2_SIZE;
    goto REPEATNOTCHAR;
    case OP_NOTUPTO:
    case OP_NOTUPTOI:
    Lmin = 0;
    Lmax = GET2(Fecode, 1);
    reptype = REPTYPE_MAX;
    Fecode += 1 + IMM2_SIZE;
    goto REPEATNOTCHAR;
    case OP_NOTMINUPTO:
    case OP_NOTMINUPTOI:
    Lmin = 0;
    Lmax = GET2(Fecode, 1);
    reptype = REPTYPE_MIN;
    Fecode += 1 + IMM2_SIZE;
    goto REPEATNOTCHAR;
    case OP_NOTPOSSTAR:
    case OP_NOTPOSSTARI:
    reptype = REPTYPE_POS;
    Lmin = 0;
    Lmax = UINT32_MAX;
    Fecode++;
    goto REPEATNOTCHAR;
    case OP_NOTPOSPLUS:
    case OP_NOTPOSPLUSI:
    reptype = REPTYPE_POS;
    Lmin = 1;
    Lmax = UINT32_MAX;
    Fecode++;
    goto REPEATNOTCHAR;
    case OP_NOTPOSQUERY:
    case OP_NOTPOSQUERYI:
    reptype = REPTYPE_POS;
    Lmin = 0;
    Lmax = 1;
    Fecode++;
    goto REPEATNOTCHAR;
    case OP_NOTPOSUPTO:
    case OP_NOTPOSUPTOI:
    reptype = REPTYPE_POS;
    Lmin = 0;
    Lmax = GET2(Fecode, 1);
    Fecode += 1 + IMM2_SIZE;
    goto REPEATNOTCHAR;
    case OP_NOTSTAR:
    case OP_NOTSTARI:
    case OP_NOTMINSTAR:
    case OP_NOTMINSTARI:
    case OP_NOTPLUS:
    case OP_NOTPLUSI:
    case OP_NOTMINPLUS:
    case OP_NOTMINPLUSI:
    case OP_NOTQUERY:
    case OP_NOTQUERYI:
    case OP_NOTMINQUERY:
    case OP_NOTMINQUERYI:
    fc = *Fecode++ - ((Fop >= OP_NOTSTARI)? OP_NOTSTARI: OP_NOTSTAR);
    Lmin = rep_min[fc];
    Lmax = rep_max[fc];
    reptype = rep_typ[fc];
    REPEATNOTCHAR:
    GETCHARINCTEST(Lc, Fecode);
    if (Fop >= OP_NOTSTARI)
      {
#ifdef SUPPORT_UNICODE
      if ((utf || ucp) && Lc > 127)
        Loc = UCD_OTHERCASE(Lc);
      else
#endif
      Loc = TABLE_GET(Lc, mb->fcc, Lc);
#ifdef SUPPORT_UNICODE
      if (utf)
        {
        uint32_t d;
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          GETCHARINC(d, Feptr);
          if (Lc == d || Loc == d) RRETURN(MATCH_NOMATCH);
          }
          COST(1);
        }
      else
#endif
        {
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (Lc == *Feptr || Loc == *Feptr) RRETURN(MATCH_NOMATCH);
          Feptr++;
          COST(1);
          }
        }
      if (Lmin == Lmax) continue;
      if (reptype == REPTYPE_MIN)
        {
#ifdef SUPPORT_UNICODE
        if (utf)
          {
          uint32_t d;
          for (;;)
            {
            RMATCH(Fecode, RM204);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINC(d, Feptr);
            if (Lc == d || Loc == d) RRETURN(MATCH_NOMATCH);
            }
          }
        else
#endif
          {
          for (;;)
            {
            RMATCH(Fecode, RM29);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            if (Lc == *Feptr || Loc == *Feptr) RRETURN(MATCH_NOMATCH);
            Feptr++;
            }
          }
        PCRE2_UNREACHABLE();
        }
      else
        {
        Lstart_eptr = Feptr;
#ifdef SUPPORT_UNICODE
        if (utf)
          {
          uint32_t d;
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(d, Feptr, len);
            if (Lc == d || Loc == d) break;
            Feptr += len;
            COST_CHK(1);
            }
          if (reptype != REPTYPE_POS) for(;;)
            {
            if (Feptr <= Lstart_eptr) break;
            RMATCH(Fecode, RM205);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            Feptr--;
            BACKCHAR(Feptr);
            }
          }
        else
#endif
          {
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            if (Lc == *Feptr || Loc == *Feptr) break;
            Feptr++;
            COST(1);
            }
          if (reptype != REPTYPE_POS) for (;;)
            {
            if (Feptr == Lstart_eptr) break;
            RMATCH(Fecode, RM30);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            Feptr--;
            }
          }
        }
      }
    else
      {
#ifdef SUPPORT_UNICODE
      if (utf)
        {
        uint32_t d;
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          GETCHARINC(d, Feptr);
          if (Lc == d) RRETURN(MATCH_NOMATCH);
          }
        }
      else
#endif
        {
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (Lc == *Feptr++) RRETURN(MATCH_NOMATCH);
          }
        }
      if (Lmin == Lmax) continue;
      if (reptype == REPTYPE_MIN)
        {
#ifdef SUPPORT_UNICODE
        if (utf)
          {
          uint32_t d;
          for (;;)
            {
            RMATCH(Fecode, RM206);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINC(d, Feptr);
            if (Lc == d) RRETURN(MATCH_NOMATCH);
            }
          }
        else
#endif
          {
          for (;;)
            {
            RMATCH(Fecode, RM31);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            if (Lc == *Feptr++) RRETURN(MATCH_NOMATCH);
            }
          }
        PCRE2_UNREACHABLE();
        }
      else
        {
        Lstart_eptr = Feptr;
#ifdef SUPPORT_UNICODE
        if (utf)
          {
          uint32_t d;
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(d, Feptr, len);
            if (Lc == d) break;
            Feptr += len;
            COST_CHK(1);
            }
          if (reptype != REPTYPE_POS) for(;;)
            {
            if (Feptr <= Lstart_eptr) break;
            RMATCH(Fecode, RM207);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            Feptr--;
            BACKCHAR(Feptr);
            }
          }
        else
#endif
          {
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            if (Lc == *Feptr) break;
            Feptr++;
            COST(1);
            }
          if (reptype != REPTYPE_POS) for (;;)
            {
            if (Feptr == Lstart_eptr) break;
            RMATCH(Fecode, RM32);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            Feptr--;
            }
          }
        }
      }
    break;
#undef Lstart_eptr
#undef Lmin
#undef Lmax
#undef Lc
#undef Loc
#define Lmin               F->temp_32[0]
#define Lmax               F->temp_32[1]
#define Lstart_eptr        F->temp_sptr[0]
#define Lbyte_map_address  F->temp_sptr[1]
#define Lbyte_map          ((const unsigned char *)Lbyte_map_address)
    case OP_NCLASS:
    case OP_CLASS:
      {
      Lbyte_map_address = Fecode + 1;
      Fecode += 1 + (32 / sizeof(PCRE2_UCHAR));
      switch (*Fecode)
        {
        case OP_CRSTAR:
        case OP_CRMINSTAR:
        case OP_CRPLUS:
        case OP_CRMINPLUS:
        case OP_CRQUERY:
        case OP_CRMINQUERY:
        case OP_CRPOSSTAR:
        case OP_CRPOSPLUS:
        case OP_CRPOSQUERY:
        fc = *Fecode++ - OP_CRSTAR;
        Lmin = rep_min[fc];
        Lmax = rep_max[fc];
        reptype = rep_typ[fc];
        break;
        case OP_CRRANGE:
        case OP_CRMINRANGE:
        case OP_CRPOSRANGE:
        Lmin = GET2(Fecode, 1);
        Lmax = GET2(Fecode, 1 + IMM2_SIZE);
        if (Lmax == 0) Lmax = UINT32_MAX;
        reptype = rep_typ[*Fecode - OP_CRSTAR];
        Fecode += 1 + 2 * IMM2_SIZE;
        break;
        default:
        Lmin = Lmax = 1;
        break;
        }
#ifdef SUPPORT_UNICODE
      if (utf)
        {
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          GETCHARINC(fc, Feptr);
          if (fc > 255)
            {
            if (Fop == OP_CLASS) RRETURN(MATCH_NOMATCH);
            }
          else
            if ((Lbyte_map[fc/8] & (1u << (fc&7))) == 0) RRETURN(MATCH_NOMATCH);
          }
        }
      else
#endif
        {
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          fc = *Feptr++;
#if PCRE2_CODE_UNIT_WIDTH != 8
          if (fc > 255)
            {
            if (Fop == OP_CLASS) RRETURN(MATCH_NOMATCH);
            }
          else
#endif
          if ((Lbyte_map[fc/8] & (1u << (fc&7))) == 0) RRETURN(MATCH_NOMATCH);
          }
        }
      if (Lmin == Lmax) continue;
      if (reptype == REPTYPE_MIN)
        {
#ifdef SUPPORT_UNICODE
        if (utf)
          {
          for (;;)
            {
            RMATCH(Fecode, RM200);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINC(fc, Feptr);
            if (fc > 255)
              {
              if (Fop == OP_CLASS) RRETURN(MATCH_NOMATCH);
              }
            else
              if ((Lbyte_map[fc/8] & (1u << (fc&7))) == 0) RRETURN(MATCH_NOMATCH);
            }
          }
        else
#endif
          {
          for (;;)
            {
            RMATCH(Fecode, RM23);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            fc = *Feptr++;
#if PCRE2_CODE_UNIT_WIDTH != 8
            if (fc > 255)
              {
              if (Fop == OP_CLASS) RRETURN(MATCH_NOMATCH);
              }
            else
#endif
            if ((Lbyte_map[fc/8] & (1u << (fc&7))) == 0) RRETURN(MATCH_NOMATCH);
            }
          }
        PCRE2_UNREACHABLE();
        }
      else
        {
        Lstart_eptr = Feptr;
#ifdef SUPPORT_UNICODE
        if (utf)
          {
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(fc, Feptr, len);
            if (fc > 255)
              {
              if (Fop == OP_CLASS) break;
              }
            else
              if ((Lbyte_map[fc/8] & (1u << (fc&7))) == 0) break;
            Feptr += len;
            COST_CHK(1);
            }
          if (reptype == REPTYPE_POS) continue;
          for (;;)
            {
            RMATCH(Fecode, RM201);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Feptr-- <= Lstart_eptr) break;
            BACKCHAR(Feptr);
            }
          }
        else
#endif
          {
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            fc = *Feptr;
#if PCRE2_CODE_UNIT_WIDTH != 8
            if (fc > 255)
              {
              if (Fop == OP_CLASS) break;
              }
            else
#endif
            if ((Lbyte_map[fc/8] & (1u << (fc&7))) == 0) break;
            Feptr++;
            COST(1);
            }
          if (reptype == REPTYPE_POS) continue;
          while (Feptr >= Lstart_eptr)
            {
            RMATCH(Fecode, RM24);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            Feptr--;
            }
          }
        RRETURN(MATCH_NOMATCH);
        }
      }
    PCRE2_UNREACHABLE();
#undef Lbyte_map_address
#undef Lbyte_map
#undef Lstart_eptr
#undef Lmin
#undef Lmax
#define Lstart_eptr  F->temp_sptr[0]
#define Lxclass_data F->temp_sptr[1]
#define Lmin         F->temp_32[0]
#define Lmax         F->temp_32[1]
#ifdef SUPPORT_WIDE_CHARS
    case OP_XCLASS:
      {
      Lxclass_data = Fecode + 1 + LINK_SIZE;
      Fecode += GET(Fecode, 1);
      switch (*Fecode)
        {
        case OP_CRSTAR:
        case OP_CRMINSTAR:
        case OP_CRPLUS:
        case OP_CRMINPLUS:
        case OP_CRQUERY:
        case OP_CRMINQUERY:
        case OP_CRPOSSTAR:
        case OP_CRPOSPLUS:
        case OP_CRPOSQUERY:
        fc = *Fecode++ - OP_CRSTAR;
        Lmin = rep_min[fc];
        Lmax = rep_max[fc];
        reptype = rep_typ[fc];
        break;
        case OP_CRRANGE:
        case OP_CRMINRANGE:
        case OP_CRPOSRANGE:
        Lmin = GET2(Fecode, 1);
        Lmax = GET2(Fecode, 1 + IMM2_SIZE);
        if (Lmax == 0) Lmax = UINT32_MAX;
        reptype = rep_typ[*Fecode - OP_CRSTAR];
        Fecode += 1 + 2 * IMM2_SIZE;
        break;
        default:
        Lmin = Lmax = 1;
        break;
        }
      for (i = 1; i <= Lmin; i++)
        {
        if (Feptr >= mb->end_subject)
          {
          SCHECK_PARTIAL();
          RRETURN(MATCH_NOMATCH);
          }
        GETCHARINCTEST(fc, Feptr);
        if (!PRIV(xclass)(fc, Lxclass_data,
            (const uint8_t*)mb->start_code, utf))
          RRETURN(MATCH_NOMATCH);
        COST_CHK(1);
        }
      if (Lmin == Lmax) continue;
      if (reptype == REPTYPE_MIN)
        {
        for (;;)
          {
          RMATCH(Fecode, RM100);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          GETCHARINCTEST(fc, Feptr);
          if (!PRIV(xclass)(fc, Lxclass_data,
              (const uint8_t*)mb->start_code, utf))
            RRETURN(MATCH_NOMATCH);
          }
        PCRE2_UNREACHABLE();
        }
      else
        {
        Lstart_eptr = Feptr;
        for (i = Lmin; i < Lmax; i++)
          {
          int len = 1;
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            break;
            }
#ifdef SUPPORT_UNICODE
          GETCHARLENTEST(fc, Feptr, len);
#else
          fc = *Feptr;
#endif
          if (!PRIV(xclass)(fc, Lxclass_data,
              (const uint8_t*)mb->start_code, utf)) break;
          Feptr += len;
          COST(1);
          }
        if (reptype == REPTYPE_POS) continue;
        for(;;)
          {
          RMATCH(Fecode, RM101);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (Feptr-- <= Lstart_eptr) break;
#ifdef SUPPORT_UNICODE
          if (utf) BACKCHAR(Feptr);
#endif
          }
        RRETURN(MATCH_NOMATCH);
        }
      PCRE2_UNREACHABLE();
      }
#endif
#undef Lstart_eptr
#undef Lxclass_data
#undef Lmin
#undef Lmax
#define Lstart_eptr  F->temp_sptr[0]
#define Leclass_data F->temp_sptr[1]
#define Leclass_len  F->temp_size
#define Lmin         F->temp_32[0]
#define Lmax         F->temp_32[1]
#ifdef SUPPORT_WIDE_CHARS
    case OP_ECLASS:
      {
      Leclass_data = Fecode + 1 + LINK_SIZE;
      Fecode += GET(Fecode, 1);
      Leclass_len = (PCRE2_SIZE)(Fecode - Leclass_data);
      switch (*Fecode)
        {
        case OP_CRSTAR:
        case OP_CRMINSTAR:
        case OP_CRPLUS:
        case OP_CRMINPLUS:
        case OP_CRQUERY:
        case OP_CRMINQUERY:
        case OP_CRPOSSTAR:
        case OP_CRPOSPLUS:
        case OP_CRPOSQUERY:
        fc = *Fecode++ - OP_CRSTAR;
        Lmin = rep_min[fc];
        Lmax = rep_max[fc];
        reptype = rep_typ[fc];
        break;
        case OP_CRRANGE:
        case OP_CRMINRANGE:
        case OP_CRPOSRANGE:
        Lmin = GET2(Fecode, 1);
        Lmax = GET2(Fecode, 1 + IMM2_SIZE);
        if (Lmax == 0) Lmax = UINT32_MAX;
        reptype = rep_typ[*Fecode - OP_CRSTAR];
        Fecode += 1 + 2 * IMM2_SIZE;
        break;
        default:
        Lmin = Lmax = 1;
        break;
        }
      for (i = 1; i <= Lmin; i++)
        {
        if (Feptr >= mb->end_subject)
          {
          SCHECK_PARTIAL();
          RRETURN(MATCH_NOMATCH);
          }
        GETCHARINCTEST(fc, Feptr);
        if (!PRIV(eclass)(fc, Leclass_data, Leclass_data + Leclass_len,
                          (const uint8_t*)mb->start_code, utf))
          RRETURN(MATCH_NOMATCH);
        COST_CHK(1);
        }
      if (Lmin == Lmax) continue;
      if (reptype == REPTYPE_MIN)
        {
        for (;;)
          {
          RMATCH(Fecode, RM102);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          GETCHARINCTEST(fc, Feptr);
          if (!PRIV(eclass)(fc, Leclass_data, Leclass_data + Leclass_len,
                            (const uint8_t*)mb->start_code, utf))
            RRETURN(MATCH_NOMATCH);
          }
        PCRE2_UNREACHABLE();
        }
      else
        {
        Lstart_eptr = Feptr;
        for (i = Lmin; i < Lmax; i++)
          {
          int len = 1;
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            break;
            }
#ifdef SUPPORT_UNICODE
          GETCHARLENTEST(fc, Feptr, len);
#else
          fc = *Feptr;
#endif
          if (!PRIV(eclass)(fc, Leclass_data, Leclass_data + Leclass_len,
                            (const uint8_t*)mb->start_code, utf))
            break;
          Feptr += len;
          COST(1);
        }
        if (reptype == REPTYPE_POS) continue;
        for(;;)
          {
          RMATCH(Fecode, RM103);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (Feptr-- <= Lstart_eptr) break;
#ifdef SUPPORT_UNICODE
          if (utf) BACKCHAR(Feptr);
#endif
          }
        RRETURN(MATCH_NOMATCH);
        }
      PCRE2_UNREACHABLE();
      }
#endif
#undef Lstart_eptr
#undef Leclass_data
#undef Leclass_len
#undef Lmin
#undef Lmax
    case OP_NOT_DIGIT:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
    if (CHMAX_255(fc) && (mb->ctypes[fc] & ctype_digit) != 0)
      RRETURN(MATCH_NOMATCH);
    Fecode++;
    break;
    case OP_DIGIT:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
    if (!CHMAX_255(fc) || (mb->ctypes[fc] & ctype_digit) == 0)
      RRETURN(MATCH_NOMATCH);
    Fecode++;
    break;
    case OP_NOT_WHITESPACE:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
    if (CHMAX_255(fc) && (mb->ctypes[fc] & ctype_space) != 0)
      RRETURN(MATCH_NOMATCH);
    Fecode++;
    break;
    case OP_WHITESPACE:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
    if (!CHMAX_255(fc) || (mb->ctypes[fc] & ctype_space) == 0)
      RRETURN(MATCH_NOMATCH);
    Fecode++;
    break;
    case OP_NOT_WORDCHAR:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
    if (CHMAX_255(fc) && (mb->ctypes[fc] & ctype_word) != 0)
      RRETURN(MATCH_NOMATCH);
    Fecode++;
    break;
    case OP_WORDCHAR:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
    if (!CHMAX_255(fc) || (mb->ctypes[fc] & ctype_word) == 0)
      RRETURN(MATCH_NOMATCH);
    Fecode++;
    break;
    case OP_ANYNL:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
    switch(fc)
      {
      default: RRETURN(MATCH_NOMATCH);
      case CHAR_CR:
      if (Feptr >= mb->end_subject)
        {
        SCHECK_PARTIAL();
        }
      else if (UCHAR21TEST(Feptr) == CHAR_LF) Feptr++;
      break;
      case CHAR_LF:
      break;
      case CHAR_VT:
      case CHAR_FF:
      case CHAR_NEL:
#ifndef EBCDIC
      case 0x2028:
      case 0x2029:
#endif
      if (mb->bsr_convention == PCRE2_BSR_ANYCRLF) RRETURN(MATCH_NOMATCH);
      break;
      }
    Fecode++;
    break;
    case OP_NOT_HSPACE:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
    switch(fc)
      {
      HSPACE_CASES: RRETURN(MATCH_NOMATCH);
      default: break;
      }
    Fecode++;
    break;
    case OP_HSPACE:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
    switch(fc)
      {
      HSPACE_CASES: break;
      default: RRETURN(MATCH_NOMATCH);
      }
    Fecode++;
    break;
    case OP_NOT_VSPACE:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
    switch(fc)
      {
      VSPACE_CASES: RRETURN(MATCH_NOMATCH);
      default: break;
      }
    Fecode++;
    break;
    case OP_VSPACE:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
    switch(fc)
      {
      VSPACE_CASES: break;
      default: RRETURN(MATCH_NOMATCH);
      }
    Fecode++;
    break;
#ifdef SUPPORT_UNICODE
    case OP_PROP:
    case OP_NOTPROP:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    GETCHARINCTEST(fc, Feptr);
      {
      const uint32_t *cp;
      uint32_t chartype;
      const ucd_record *prop = GET_UCD(fc);
      notmatch = Fop == OP_NOTPROP;
      switch(Fecode[1])
        {
        case PT_LAMP:
        chartype = prop->chartype;
        if ((chartype == ucp_Lu ||
             chartype == ucp_Ll ||
             chartype == ucp_Lt) == notmatch)
          RRETURN(MATCH_NOMATCH);
        break;
        case PT_GC:
        if ((Fecode[2] == PRIV(ucp_gentype)[prop->chartype]) == notmatch)
          RRETURN(MATCH_NOMATCH);
        break;
        case PT_PC:
        if ((Fecode[2] == prop->chartype) == notmatch)
          RRETURN(MATCH_NOMATCH);
        break;
        case PT_SC:
        if ((Fecode[2] == prop->script) == notmatch)
          RRETURN(MATCH_NOMATCH);
        break;
        case PT_SCX:
          {
          BOOL ok = (Fecode[2] == prop->script ||
                     MAPBIT(PRIV(ucd_script_sets) + UCD_SCRIPTX_PROP(prop), Fecode[2]) != 0);
          if (ok == notmatch) RRETURN(MATCH_NOMATCH);
          }
        break;
        case PT_ALNUM:
        chartype = prop->chartype;
        if ((PRIV(ucp_gentype)[chartype] == ucp_L ||
             PRIV(ucp_gentype)[chartype] == ucp_N) == notmatch)
          RRETURN(MATCH_NOMATCH);
        break;
        case PT_SPACE:
        case PT_PXSPACE:
        switch(fc)
          {
          HSPACE_CASES:
          VSPACE_CASES:
          if (notmatch) RRETURN(MATCH_NOMATCH);
          break;
          default:
          if ((PRIV(ucp_gentype)[prop->chartype] == ucp_Z) == notmatch)
            RRETURN(MATCH_NOMATCH);
          break;
          }
        break;
        case PT_WORD:
        chartype = prop->chartype;
        if ((PRIV(ucp_gentype)[chartype] == ucp_L ||
             PRIV(ucp_gentype)[chartype] == ucp_N ||
             chartype == ucp_Mn ||
             chartype == ucp_Pc) == notmatch)
          RRETURN(MATCH_NOMATCH);
        break;
        case PT_CLIST:
#if PCRE2_CODE_UNIT_WIDTH == 32
            if (fc > MAX_UTF_CODE_POINT)
              {
              if (notmatch) break;;
              RRETURN(MATCH_NOMATCH);
              }
#endif
        cp = PRIV(ucd_caseless_sets) + Fecode[2];
        for (;;)
          {
          if (fc < *cp)
            { if (notmatch) break; else { RRETURN(MATCH_NOMATCH); } }
          if (fc == *cp++)
            { if (notmatch) { RRETURN(MATCH_NOMATCH); } else break; }
          COST(1);
          }
        break;
        case PT_UCNC:
        if ((fc == CHAR_DOLLAR_SIGN || fc == CHAR_COMMERCIAL_AT ||
             fc == CHAR_GRAVE_ACCENT || (fc >= 0xa0 && fc <= 0xd7ff) ||
             fc >= 0xe000) == notmatch)
          RRETURN(MATCH_NOMATCH);
        break;
        case PT_BIDICL:
        if ((UCD_BIDICLASS_PROP(prop) == Fecode[2]) == notmatch)
          RRETURN(MATCH_NOMATCH);
        break;
        case PT_BOOL:
          {
          BOOL ok = MAPBIT(PRIV(ucd_boolprop_sets) +
            UCD_BPROPS_PROP(prop), Fecode[2]) != 0;
          if (ok == notmatch) RRETURN(MATCH_NOMATCH);
          }
        break;
        default:
        PCRE2_DEBUG_UNREACHABLE();
        return PCRE2_ERROR_INTERNAL;
        }
      Fecode += 3;
      }
    break;
    case OP_EXTUNI:
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      RRETURN(MATCH_NOMATCH);
      }
    else
      {
      GETCHARINCTEST(fc, Feptr);
      Feptr = PRIV(extuni)(fc, Feptr, mb->start_subject, mb->end_subject, utf,
        NULL);
      }
    CHECK_PARTIAL();
    Fecode++;
    break;
#endif
#define Lstart_eptr  F->temp_sptr[0]
#define Lmin         F->temp_32[0]
#define Lmax         F->temp_32[1]
#define Lctype       F->temp_32[2]
#define Lpropvalue   F->temp_32[3]
    case OP_TYPEEXACT:
    Lmin = Lmax = GET2(Fecode, 1);
    Fecode += 1 + IMM2_SIZE;
    goto REPEATTYPE;
    case OP_TYPEUPTO:
    case OP_TYPEMINUPTO:
    Lmin = 0;
    Lmax = GET2(Fecode, 1);
    reptype = (*Fecode == OP_TYPEMINUPTO)? REPTYPE_MIN : REPTYPE_MAX;
    Fecode += 1 + IMM2_SIZE;
    goto REPEATTYPE;
    case OP_TYPEPOSSTAR:
    reptype = REPTYPE_POS;
    Lmin = 0;
    Lmax = UINT32_MAX;
    Fecode++;
    goto REPEATTYPE;
    case OP_TYPEPOSPLUS:
    reptype = REPTYPE_POS;
    Lmin = 1;
    Lmax = UINT32_MAX;
    Fecode++;
    goto REPEATTYPE;
    case OP_TYPEPOSQUERY:
    reptype = REPTYPE_POS;
    Lmin = 0;
    Lmax = 1;
    Fecode++;
    goto REPEATTYPE;
    case OP_TYPEPOSUPTO:
    reptype = REPTYPE_POS;
    Lmin = 0;
    Lmax = GET2(Fecode, 1);
    Fecode += 1 + IMM2_SIZE;
    goto REPEATTYPE;
    case OP_TYPESTAR:
    case OP_TYPEMINSTAR:
    case OP_TYPEPLUS:
    case OP_TYPEMINPLUS:
    case OP_TYPEQUERY:
    case OP_TYPEMINQUERY:
    fc = *Fecode++ - OP_TYPESTAR;
    Lmin = rep_min[fc];
    Lmax = rep_max[fc];
    reptype = rep_typ[fc];
    REPEATTYPE:
    Lctype = *Fecode++;
#ifdef SUPPORT_UNICODE
    if (Lctype == OP_PROP || Lctype == OP_NOTPROP)
      {
      proptype = *Fecode++;
      Lpropvalue = *Fecode++;
      }
    else proptype = -1;
#endif
    if (Lmin > 0)
      {
#ifdef SUPPORT_UNICODE
      if (proptype >= 0)
        {
        notmatch = Lctype == OP_NOTPROP;
        switch(proptype)
          {
          case PT_LAMP:
          for (i = 1; i <= Lmin; i++)
            {
            int chartype;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            chartype = UCD_CHARTYPE(fc);
            if ((chartype == ucp_Lu ||
                 chartype == ucp_Ll ||
                 chartype == ucp_Lt) == notmatch)
              RRETURN(MATCH_NOMATCH);
            COST(1);
            }
          break;
          case PT_GC:
          for (i = 1; i <= Lmin; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            if ((UCD_CATEGORY(fc) == Lpropvalue) == notmatch)
              RRETURN(MATCH_NOMATCH);
            COST(1);
            }
          break;
          case PT_PC:
          for (i = 1; i <= Lmin; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            if ((UCD_CHARTYPE(fc) == Lpropvalue) == notmatch)
              RRETURN(MATCH_NOMATCH);
            COST(1);
            }
          break;
          case PT_SC:
          for (i = 1; i <= Lmin; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            if ((UCD_SCRIPT(fc) == Lpropvalue) == notmatch)
              RRETURN(MATCH_NOMATCH);
            COST(1);
            }
          break;
          case PT_SCX:
          for (i = 1; i <= Lmin; i++)
            {
            BOOL ok;
            const ucd_record *prop;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            prop = GET_UCD(fc);
            ok = (prop->script == Lpropvalue ||
                  MAPBIT(PRIV(ucd_script_sets) + UCD_SCRIPTX_PROP(prop), Lpropvalue) != 0);
            if (ok == notmatch)
              RRETURN(MATCH_NOMATCH);
            COST(1);
            }
          break;
          case PT_ALNUM:
          for (i = 1; i <= Lmin; i++)
            {
            int category;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            category = UCD_CATEGORY(fc);
            if ((category == ucp_L || category == ucp_N) == notmatch)
              RRETURN(MATCH_NOMATCH);
            COST(1);
            }
          break;
          case PT_SPACE:
          case PT_PXSPACE:
          for (i = 1; i <= Lmin; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            switch(fc)
              {
              HSPACE_CASES:
              VSPACE_CASES:
              if (notmatch) RRETURN(MATCH_NOMATCH);
              break;
              default:
              if ((UCD_CATEGORY(fc) == ucp_Z) == notmatch)
                RRETURN(MATCH_NOMATCH);
              break;
              }
            COST(1);
            }
          break;
          case PT_WORD:
          for (i = 1; i <= Lmin; i++)
            {
            int chartype, category;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            chartype = UCD_CHARTYPE(fc);
            category = PRIV(ucp_gentype)[chartype];
            if ((category == ucp_L || category == ucp_N ||
                 chartype == ucp_Mn || chartype == ucp_Pc) == notmatch)
              RRETURN(MATCH_NOMATCH);
            COST(1);
            }
          break;
          case PT_CLIST:
          for (i = 1; i <= Lmin; i++)
            {
            const uint32_t *cp;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
#if PCRE2_CODE_UNIT_WIDTH == 32
            if (fc > MAX_UTF_CODE_POINT)
              {
              if (notmatch) continue;
              RRETURN(MATCH_NOMATCH);
              }
#endif
            cp = PRIV(ucd_caseless_sets) + Lpropvalue;
            for (;;)
              {
              if (fc < *cp)
                {
                if (notmatch) break;
                RRETURN(MATCH_NOMATCH);
                }
              if (fc == *cp++)
                {
                if (notmatch) RRETURN(MATCH_NOMATCH);
                break;
                }
              COST(1);
              }
            COST_CHK(1);
            }
          break;
          case PT_UCNC:
          for (i = 1; i <= Lmin; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            if ((fc == CHAR_DOLLAR_SIGN || fc == CHAR_COMMERCIAL_AT ||
                 fc == CHAR_GRAVE_ACCENT || (fc >= 0xa0 && fc <= 0xd7ff) ||
                 fc >= 0xe000) == notmatch)
              RRETURN(MATCH_NOMATCH);
            COST(1);
            }
          break;
          case PT_BIDICL:
          for (i = 1; i <= Lmin; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            if ((UCD_BIDICLASS(fc) == Lpropvalue) == notmatch)
              RRETURN(MATCH_NOMATCH);
            COST(1);
            }
          break;
          case PT_BOOL:
          for (i = 1; i <= Lmin; i++)
            {
            BOOL ok;
            const ucd_record *prop;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            prop = GET_UCD(fc);
            ok = MAPBIT(PRIV(ucd_boolprop_sets) +
              UCD_BPROPS_PROP(prop), Lpropvalue) != 0;
            if (ok == notmatch)
              RRETURN(MATCH_NOMATCH);
            COST(1);
            }
          break;
          default:
          PCRE2_DEBUG_UNREACHABLE();
          return PCRE2_ERROR_INTERNAL;
          }
        }
      else if (Lctype == OP_EXTUNI)
        {
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          else
            {
            GETCHARINCTEST(fc, Feptr);
            Feptr = PRIV(extuni)(fc, Feptr, mb->start_subject,
              mb->end_subject, utf, NULL);
            }
          CHECK_PARTIAL();
          COST(1);
          }
        }
      else
#endif
#ifdef SUPPORT_UNICODE
      if (utf) switch(Lctype)
        {
        case OP_ANY:
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (IS_NEWLINE(Feptr)) RRETURN(MATCH_NOMATCH);
          if (mb->partial != 0 &&
              Feptr + 1 >= mb->end_subject &&
              NLBLOCK->nltype == NLTYPE_FIXED &&
              NLBLOCK->nllen == 2 &&
              UCHAR21(Feptr) == NLBLOCK->nl[0])
            {
            mb->hitend = TRUE;
            if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
            }
          Feptr++;
          ACROSSCHAR(Feptr < mb->end_subject, Feptr, Feptr++);
          COST(1);
          }
        break;
        case OP_ALLANY:
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          Feptr++;
          ACROSSCHAR(Feptr < mb->end_subject, Feptr, Feptr++);
          COST(1);
          }
        break;
        case OP_ANYBYTE:
        if (Feptr > mb->end_subject - Lmin) RRETURN(MATCH_NOMATCH);
        Feptr += Lmin;
        break;
        case OP_ANYNL:
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          GETCHARINC(fc, Feptr);
          switch(fc)
            {
            default: RRETURN(MATCH_NOMATCH);
            case CHAR_CR:
            if (Feptr < mb->end_subject && UCHAR21(Feptr) == CHAR_LF) Feptr++;
            break;
            case CHAR_LF:
            break;
            case CHAR_VT:
            case CHAR_FF:
            case CHAR_NEL:
#ifndef EBCDIC
            case 0x2028:
            case 0x2029:
#endif
            if (mb->bsr_convention == PCRE2_BSR_ANYCRLF) RRETURN(MATCH_NOMATCH);
            break;
            }
            COST(1);
          }
        break;
        case OP_NOT_HSPACE:
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          GETCHARINC(fc, Feptr);
          switch(fc)
            {
            HSPACE_CASES: RRETURN(MATCH_NOMATCH);
            default: break;
            }
          COST(1);
          }
        break;
        case OP_HSPACE:
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          GETCHARINC(fc, Feptr);
          switch(fc)
            {
            HSPACE_CASES: break;
            default: RRETURN(MATCH_NOMATCH);
            }
          COST(1);
          }
        break;
        case OP_NOT_VSPACE:
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          GETCHARINC(fc, Feptr);
          switch(fc)
            {
            VSPACE_CASES: RRETURN(MATCH_NOMATCH);
            default: break;
            }
            COST(1);
          }
        break;
        case OP_VSPACE:
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          GETCHARINC(fc, Feptr);
          switch(fc)
            {
            VSPACE_CASES: break;
            default: RRETURN(MATCH_NOMATCH);
            }
          COST(1);
          }
        break;
        case OP_NOT_DIGIT:
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          GETCHARINC(fc, Feptr);
          if (fc < 128 && (mb->ctypes[fc] & ctype_digit) != 0)
            RRETURN(MATCH_NOMATCH);
          COST(1);
          }
        break;
        case OP_DIGIT:
        for (i = 1; i <= Lmin; i++)
          {
          uint32_t cc;
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          cc = UCHAR21(Feptr);
          if (cc >= 128 || (mb->ctypes[cc] & ctype_digit) == 0)
            RRETURN(MATCH_NOMATCH);
          Feptr++;
          COST(1);
          }
        break;
        case OP_NOT_WHITESPACE:
        for (i = 1; i <= Lmin; i++)
          {
          uint32_t cc;
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          cc = UCHAR21(Feptr);
          if (cc < 128 && (mb->ctypes[cc] & ctype_space) != 0)
            RRETURN(MATCH_NOMATCH);
          Feptr++;
          ACROSSCHAR(Feptr < mb->end_subject, Feptr, Feptr++);
          COST(1);
          }
        break;
        case OP_WHITESPACE:
        for (i = 1; i <= Lmin; i++)
          {
          uint32_t cc;
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          cc = UCHAR21(Feptr);
          if (cc >= 128 || (mb->ctypes[cc] & ctype_space) == 0)
            RRETURN(MATCH_NOMATCH);
          Feptr++;
          COST(1);
          }
        break;
        case OP_NOT_WORDCHAR:
        for (i = 1; i <= Lmin; i++)
          {
          uint32_t cc;
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          cc = UCHAR21(Feptr);
          if (cc < 128 && (mb->ctypes[cc] & ctype_word) != 0)
            RRETURN(MATCH_NOMATCH);
          Feptr++;
          ACROSSCHAR(Feptr < mb->end_subject, Feptr, Feptr++);
          COST(1);
          }
        break;
        case OP_WORDCHAR:
        for (i = 1; i <= Lmin; i++)
          {
          uint32_t cc;
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          cc = UCHAR21(Feptr);
          if (cc >= 128 || (mb->ctypes[cc] & ctype_word) == 0)
            RRETURN(MATCH_NOMATCH);
          Feptr++;
          COST(1);
          }
        break;
        default:
        PCRE2_DEBUG_UNREACHABLE();
        return PCRE2_ERROR_INTERNAL;
        }
      else
#endif
      switch(Lctype)
        {
        case OP_ANY:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (IS_NEWLINE(Feptr)) RRETURN(MATCH_NOMATCH);
          if (mb->partial != 0 &&
              Feptr + 1 >= mb->end_subject &&
              NLBLOCK->nltype == NLTYPE_FIXED &&
              NLBLOCK->nllen == 2 &&
              *Feptr == NLBLOCK->nl[0])
            {
            mb->hitend = TRUE;
            if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
            }
          Feptr++;
          }
        break;
        case OP_ALLANY:
        if (Feptr > mb->end_subject - Lmin)
          {
          SCHECK_PARTIAL();
          RRETURN(MATCH_NOMATCH);
          }
        Feptr += Lmin;
        break;
        case OP_ANYNL:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          switch(*Feptr++)
            {
            default: RRETURN(MATCH_NOMATCH);
            case CHAR_CR:
            if (Feptr < mb->end_subject && *Feptr == CHAR_LF) Feptr++;
            break;
            case CHAR_LF:
            break;
            case CHAR_VT:
            case CHAR_FF:
            case CHAR_NEL:
#if PCRE2_CODE_UNIT_WIDTH != 8
            case 0x2028:
            case 0x2029:
#endif
            if (mb->bsr_convention == PCRE2_BSR_ANYCRLF) RRETURN(MATCH_NOMATCH);
            break;
            }
          }
        break;
        case OP_NOT_HSPACE:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          switch(*Feptr++)
            {
            default: break;
            HSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
            HSPACE_MULTIBYTE_CASES:
#endif
            RRETURN(MATCH_NOMATCH);
            }
          }
        break;
        case OP_HSPACE:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          switch(*Feptr++)
            {
            default: RRETURN(MATCH_NOMATCH);
            HSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
            HSPACE_MULTIBYTE_CASES:
#endif
            break;
            }
          }
        break;
        case OP_NOT_VSPACE:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          switch(*Feptr++)
            {
            VSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
            VSPACE_MULTIBYTE_CASES:
#endif
            RRETURN(MATCH_NOMATCH);
            default: break;
            }
          }
        break;
        case OP_VSPACE:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          switch(*Feptr++)
            {
            default: RRETURN(MATCH_NOMATCH);
            VSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
            VSPACE_MULTIBYTE_CASES:
#endif
            break;
            }
          }
        break;
        case OP_NOT_DIGIT:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (MAX_255(*Feptr) && (mb->ctypes[*Feptr] & ctype_digit) != 0)
            RRETURN(MATCH_NOMATCH);
          Feptr++;
          }
        break;
        case OP_DIGIT:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (!MAX_255(*Feptr) || (mb->ctypes[*Feptr] & ctype_digit) == 0)
            RRETURN(MATCH_NOMATCH);
          Feptr++;
          }
        break;
        case OP_NOT_WHITESPACE:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (MAX_255(*Feptr) && (mb->ctypes[*Feptr] & ctype_space) != 0)
            RRETURN(MATCH_NOMATCH);
          Feptr++;
          }
        break;
        case OP_WHITESPACE:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (!MAX_255(*Feptr) || (mb->ctypes[*Feptr] & ctype_space) == 0)
            RRETURN(MATCH_NOMATCH);
          Feptr++;
          }
        break;
        case OP_NOT_WORDCHAR:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (MAX_255(*Feptr) && (mb->ctypes[*Feptr] & ctype_word) != 0)
            RRETURN(MATCH_NOMATCH);
          Feptr++;
          }
        break;
        case OP_WORDCHAR:
        COST(Lmin);
        for (i = 1; i <= Lmin; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (!MAX_255(*Feptr) || (mb->ctypes[*Feptr] & ctype_word) == 0)
            RRETURN(MATCH_NOMATCH);
          Feptr++;
          }
        break;
        default:
        PCRE2_DEBUG_UNREACHABLE();
        return PCRE2_ERROR_INTERNAL;
        }
      }
    if (Lmin == Lmax) continue;
    if (reptype == REPTYPE_MIN)
      {
#ifdef SUPPORT_UNICODE
      if (proptype >= 0)
        {
        switch(proptype)
          {
          case PT_LAMP:
          for (;;)
            {
            int chartype;
            RMATCH(Fecode, RM208);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            chartype = UCD_CHARTYPE(fc);
            if ((chartype == ucp_Lu ||
                 chartype == ucp_Ll ||
                 chartype == ucp_Lt) == (Lctype == OP_NOTPROP))
              RRETURN(MATCH_NOMATCH);
            }
          PCRE2_UNREACHABLE();
          case PT_GC:
          for (;;)
            {
            RMATCH(Fecode, RM209);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            if ((UCD_CATEGORY(fc) == Lpropvalue) == (Lctype == OP_NOTPROP))
              RRETURN(MATCH_NOMATCH);
            }
          PCRE2_UNREACHABLE();
          case PT_PC:
          for (;;)
            {
            RMATCH(Fecode, RM210);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            if ((UCD_CHARTYPE(fc) == Lpropvalue) == (Lctype == OP_NOTPROP))
              RRETURN(MATCH_NOMATCH);
            }
          PCRE2_UNREACHABLE();
          case PT_SC:
          for (;;)
            {
            RMATCH(Fecode, RM211);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            if ((UCD_SCRIPT(fc) == Lpropvalue) == (Lctype == OP_NOTPROP))
              RRETURN(MATCH_NOMATCH);
            }
          PCRE2_UNREACHABLE();
          case PT_SCX:
          for (;;)
            {
            BOOL ok;
            const ucd_record *prop;
            RMATCH(Fecode, RM224);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            prop = GET_UCD(fc);
            ok = (prop->script == Lpropvalue
                  || MAPBIT(PRIV(ucd_script_sets) + UCD_SCRIPTX_PROP(prop), Lpropvalue) != 0);
            if (ok == (Lctype == OP_NOTPROP))
              RRETURN(MATCH_NOMATCH);
            }
          PCRE2_UNREACHABLE();
          case PT_ALNUM:
          for (;;)
            {
            int category;
            RMATCH(Fecode, RM212);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            category = UCD_CATEGORY(fc);
            if ((category == ucp_L || category == ucp_N) == (Lctype == OP_NOTPROP))
              RRETURN(MATCH_NOMATCH);
            }
          PCRE2_UNREACHABLE();
          case PT_SPACE:
          case PT_PXSPACE:
          for (;;)
            {
            RMATCH(Fecode, RM213);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            switch(fc)
              {
              HSPACE_CASES:
              VSPACE_CASES:
              if (Lctype == OP_NOTPROP) RRETURN(MATCH_NOMATCH);
              break;
              default:
              if ((UCD_CATEGORY(fc) == ucp_Z) == (Lctype == OP_NOTPROP))
                RRETURN(MATCH_NOMATCH);
              break;
              }
            }
          PCRE2_UNREACHABLE();
          case PT_WORD:
          for (;;)
            {
            int chartype, category;
            RMATCH(Fecode, RM214);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            chartype = UCD_CHARTYPE(fc);
            category = PRIV(ucp_gentype)[chartype];
            if ((category == ucp_L ||
                 category == ucp_N ||
                 chartype == ucp_Mn ||
                 chartype == ucp_Pc) == (Lctype == OP_NOTPROP))
              RRETURN(MATCH_NOMATCH);
            }
          PCRE2_UNREACHABLE();
          case PT_CLIST:
          for (;;)
            {
            const uint32_t *cp;
            RMATCH(Fecode, RM215);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
#if PCRE2_CODE_UNIT_WIDTH == 32
            if (fc > MAX_UTF_CODE_POINT)
              {
              if (Lctype == OP_NOTPROP) continue;
              RRETURN(MATCH_NOMATCH);
              }
#endif
            cp = PRIV(ucd_caseless_sets) + Lpropvalue;
            for (;;)
              {
              if (fc < *cp)
                {
                if (Lctype == OP_NOTPROP) break;
                RRETURN(MATCH_NOMATCH);
                }
              if (fc == *cp++)
                {
                if (Lctype == OP_NOTPROP) RRETURN(MATCH_NOMATCH);
                break;
                }
              COST(1);
              }
            }
          PCRE2_UNREACHABLE();
          case PT_UCNC:
          for (;;)
            {
            RMATCH(Fecode, RM216);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            if ((fc == CHAR_DOLLAR_SIGN || fc == CHAR_COMMERCIAL_AT ||
                 fc == CHAR_GRAVE_ACCENT || (fc >= 0xa0 && fc <= 0xd7ff) ||
                 fc >= 0xe000) == (Lctype == OP_NOTPROP))
              RRETURN(MATCH_NOMATCH);
            }
          PCRE2_UNREACHABLE();
          case PT_BIDICL:
          for (;;)
            {
            RMATCH(Fecode, RM223);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            if ((UCD_BIDICLASS(fc) == Lpropvalue) == (Lctype == OP_NOTPROP))
              RRETURN(MATCH_NOMATCH);
            }
          PCRE2_UNREACHABLE();
          case PT_BOOL:
          for (;;)
            {
            BOOL ok;
            const ucd_record *prop;
            RMATCH(Fecode, RM222);
            if (rrc != MATCH_NOMATCH) RRETURN(rrc);
            if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              RRETURN(MATCH_NOMATCH);
              }
            GETCHARINCTEST(fc, Feptr);
            prop = GET_UCD(fc);
            ok = MAPBIT(PRIV(ucd_boolprop_sets) +
              UCD_BPROPS_PROP(prop), Lpropvalue) != 0;
            if (ok == (Lctype == OP_NOTPROP))
              RRETURN(MATCH_NOMATCH);
            }
          PCRE2_UNREACHABLE();
          default:
          PCRE2_DEBUG_UNREACHABLE();
          return PCRE2_ERROR_INTERNAL;
          }
        }
      else if (Lctype == OP_EXTUNI)
        {
        for (;;)
          {
          RMATCH(Fecode, RM217);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          else
            {
            GETCHARINCTEST(fc, Feptr);
            Feptr = PRIV(extuni)(fc, Feptr, mb->start_subject, mb->end_subject,
              utf, NULL);
            }
          CHECK_PARTIAL();
          }
        }
      else
#endif
#ifdef SUPPORT_UNICODE
      if (utf)
        {
        for (;;)
          {
          RMATCH(Fecode, RM218);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (Lctype == OP_ANY && IS_NEWLINE(Feptr)) RRETURN(MATCH_NOMATCH);
          GETCHARINC(fc, Feptr);
          switch(Lctype)
            {
            case OP_ANY:
            if (mb->partial != 0 &&
                Feptr >= mb->end_subject &&
                NLBLOCK->nltype == NLTYPE_FIXED &&
                NLBLOCK->nllen == 2 &&
                fc == NLBLOCK->nl[0])
              {
              mb->hitend = TRUE;
              if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
              }
            break;
            case OP_ALLANY:
            case OP_ANYBYTE:
            break;
            case OP_ANYNL:
            switch(fc)
              {
              default: RRETURN(MATCH_NOMATCH);
              case CHAR_CR:
              if (Feptr < mb->end_subject && UCHAR21(Feptr) == CHAR_LF) Feptr++;
              break;
              case CHAR_LF:
              break;
              case CHAR_VT:
              case CHAR_FF:
              case CHAR_NEL:
#ifndef EBCDIC
              case 0x2028:
              case 0x2029:
#endif
              if (mb->bsr_convention == PCRE2_BSR_ANYCRLF)
                RRETURN(MATCH_NOMATCH);
              break;
              }
            break;
            case OP_NOT_HSPACE:
            switch(fc)
              {
              HSPACE_CASES: RRETURN(MATCH_NOMATCH);
              default: break;
              }
            break;
            case OP_HSPACE:
            switch(fc)
              {
              HSPACE_CASES: break;
              default: RRETURN(MATCH_NOMATCH);
              }
            break;
            case OP_NOT_VSPACE:
            switch(fc)
              {
              VSPACE_CASES: RRETURN(MATCH_NOMATCH);
              default: break;
              }
            break;
            case OP_VSPACE:
            switch(fc)
              {
              VSPACE_CASES: break;
              default: RRETURN(MATCH_NOMATCH);
              }
            break;
            case OP_NOT_DIGIT:
            if (fc < 256 && (mb->ctypes[fc] & ctype_digit) != 0)
              RRETURN(MATCH_NOMATCH);
            break;
            case OP_DIGIT:
            if (fc >= 256 || (mb->ctypes[fc] & ctype_digit) == 0)
              RRETURN(MATCH_NOMATCH);
            break;
            case OP_NOT_WHITESPACE:
            if (fc < 256 && (mb->ctypes[fc] & ctype_space) != 0)
              RRETURN(MATCH_NOMATCH);
            break;
            case OP_WHITESPACE:
            if (fc >= 256 || (mb->ctypes[fc] & ctype_space) == 0)
              RRETURN(MATCH_NOMATCH);
            break;
            case OP_NOT_WORDCHAR:
            if (fc < 256 && (mb->ctypes[fc] & ctype_word) != 0)
              RRETURN(MATCH_NOMATCH);
            break;
            case OP_WORDCHAR:
            if (fc >= 256 || (mb->ctypes[fc] & ctype_word) == 0)
              RRETURN(MATCH_NOMATCH);
            break;
            default:
            PCRE2_DEBUG_UNREACHABLE();
            return PCRE2_ERROR_INTERNAL;
            }
          }
        }
      else
#endif
        {
        for (;;)
          {
          RMATCH(Fecode, RM33);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            RRETURN(MATCH_NOMATCH);
            }
          if (Lctype == OP_ANY && IS_NEWLINE(Feptr))
            RRETURN(MATCH_NOMATCH);
          fc = *Feptr++;
          switch(Lctype)
            {
            case OP_ANY:
            if (mb->partial != 0 &&
                Feptr >= mb->end_subject &&
                NLBLOCK->nltype == NLTYPE_FIXED &&
                NLBLOCK->nllen == 2 &&
                fc == NLBLOCK->nl[0])
              {
              mb->hitend = TRUE;
              if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
              }
            break;
            case OP_ALLANY:
            case OP_ANYBYTE:
            break;
            case OP_ANYNL:
            switch(fc)
              {
              default: RRETURN(MATCH_NOMATCH);
              case CHAR_CR:
              if (Feptr < mb->end_subject && *Feptr == CHAR_LF) Feptr++;
              break;
              case CHAR_LF:
              break;
              case CHAR_VT:
              case CHAR_FF:
              case CHAR_NEL:
#if PCRE2_CODE_UNIT_WIDTH != 8
              case 0x2028:
              case 0x2029:
#endif
              if (mb->bsr_convention == PCRE2_BSR_ANYCRLF)
                RRETURN(MATCH_NOMATCH);
              break;
              }
            break;
            case OP_NOT_HSPACE:
            switch(fc)
              {
              default: break;
              HSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
              HSPACE_MULTIBYTE_CASES:
#endif
              RRETURN(MATCH_NOMATCH);
              }
            break;
            case OP_HSPACE:
            switch(fc)
              {
              default: RRETURN(MATCH_NOMATCH);
              HSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
              HSPACE_MULTIBYTE_CASES:
#endif
              break;
              }
            break;
            case OP_NOT_VSPACE:
            switch(fc)
              {
              default: break;
              VSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
              VSPACE_MULTIBYTE_CASES:
#endif
              RRETURN(MATCH_NOMATCH);
              }
            break;
            case OP_VSPACE:
            switch(fc)
              {
              default: RRETURN(MATCH_NOMATCH);
              VSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
              VSPACE_MULTIBYTE_CASES:
#endif
              break;
              }
            break;
            case OP_NOT_DIGIT:
            if (MAX_255(fc) && (mb->ctypes[fc] & ctype_digit) != 0)
              RRETURN(MATCH_NOMATCH);
            break;
            case OP_DIGIT:
            if (!MAX_255(fc) || (mb->ctypes[fc] & ctype_digit) == 0)
              RRETURN(MATCH_NOMATCH);
            break;
            case OP_NOT_WHITESPACE:
            if (MAX_255(fc) && (mb->ctypes[fc] & ctype_space) != 0)
              RRETURN(MATCH_NOMATCH);
            break;
            case OP_WHITESPACE:
            if (!MAX_255(fc) || (mb->ctypes[fc] & ctype_space) == 0)
              RRETURN(MATCH_NOMATCH);
            break;
            case OP_NOT_WORDCHAR:
            if (MAX_255(fc) && (mb->ctypes[fc] & ctype_word) != 0)
              RRETURN(MATCH_NOMATCH);
            break;
            case OP_WORDCHAR:
            if (!MAX_255(fc) || (mb->ctypes[fc] & ctype_word) == 0)
              RRETURN(MATCH_NOMATCH);
            break;
            default:
            PCRE2_DEBUG_UNREACHABLE();
            return PCRE2_ERROR_INTERNAL;
            }
          }
        }
      PCRE2_DEBUG_UNREACHABLE();
      }
    else
      {
      Lstart_eptr = Feptr;
#ifdef SUPPORT_UNICODE
      if (proptype >= 0)
        {
        notmatch = Lctype == OP_NOTPROP;
        switch(proptype)
          {
          case PT_LAMP:
          for (i = Lmin; i < Lmax; i++)
            {
            int chartype;
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
            chartype = UCD_CHARTYPE(fc);
            if ((chartype == ucp_Lu ||
                 chartype == ucp_Ll ||
                 chartype == ucp_Lt) == notmatch)
              break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case PT_GC:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
            if ((UCD_CATEGORY(fc) == Lpropvalue) == notmatch) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case PT_PC:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
            if ((UCD_CHARTYPE(fc) == Lpropvalue) == notmatch) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case PT_SC:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
            if ((UCD_SCRIPT(fc) == Lpropvalue) == notmatch) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case PT_SCX:
          for (i = Lmin; i < Lmax; i++)
            {
            BOOL ok;
            const ucd_record *prop;
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
            prop = GET_UCD(fc);
            ok = (prop->script == Lpropvalue ||
                  MAPBIT(PRIV(ucd_script_sets) + UCD_SCRIPTX_PROP(prop), Lpropvalue) != 0);
            if (ok == notmatch) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case PT_ALNUM:
          for (i = Lmin; i < Lmax; i++)
            {
            int category;
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
            category = UCD_CATEGORY(fc);
            if ((category == ucp_L || category == ucp_N) == notmatch)
              break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case PT_SPACE:
          case PT_PXSPACE:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
            switch(fc)
              {
              HSPACE_CASES:
              VSPACE_CASES:
              if (notmatch) goto ENDLOOP99;
              break;
              default:
              if ((UCD_CATEGORY(fc) == ucp_Z) == notmatch)
                goto ENDLOOP99;
              break;
              }
            Feptr+= len;
            COST_CHK(1);
            }
          ENDLOOP99:
          break;
          case PT_WORD:
          for (i = Lmin; i < Lmax; i++)
            {
            int chartype, category;
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
            chartype = UCD_CHARTYPE(fc);
            category = PRIV(ucp_gentype)[chartype];
            if ((category == ucp_L ||
                 category == ucp_N ||
                 chartype == ucp_Mn ||
                 chartype == ucp_Pc) == notmatch)
              break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case PT_CLIST:
          for (i = Lmin; i < Lmax; i++)
            {
            const uint32_t *cp;
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
#if PCRE2_CODE_UNIT_WIDTH == 32
            if (fc > MAX_UTF_CODE_POINT)
              {
              if (!notmatch) goto GOT_MAX;
              }
            else
#endif
              {
              cp = PRIV(ucd_caseless_sets) + Lpropvalue;
              for (;;)
                {
                if (fc < *cp)
                  { if (notmatch) break; else goto GOT_MAX; }
                if (fc == *cp++)
                  { if (notmatch) goto GOT_MAX; else break; }
                }
                COST(1);
              }
            Feptr += len;
            COST_CHK(1);
            }
          GOT_MAX:
          break;
          case PT_UCNC:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
            if ((fc == CHAR_DOLLAR_SIGN || fc == CHAR_COMMERCIAL_AT ||
                 fc == CHAR_GRAVE_ACCENT || (fc >= 0xa0 && fc <= 0xd7ff) ||
                 fc >= 0xe000) == notmatch)
              break;
            Feptr += len;
            COST_CHK(1);
            }
          break;
          case PT_BIDICL:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
            if ((UCD_BIDICLASS(fc) == Lpropvalue) == notmatch) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case PT_BOOL:
          for (i = Lmin; i < Lmax; i++)
            {
            BOOL ok;
            const ucd_record *prop;
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLENTEST(fc, Feptr, len);
            prop = GET_UCD(fc);
            ok = MAPBIT(PRIV(ucd_boolprop_sets) +
              UCD_BPROPS_PROP(prop), Lpropvalue) != 0;
            if (ok == notmatch) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          default:
          PCRE2_DEBUG_UNREACHABLE();
          return PCRE2_ERROR_INTERNAL;
          }
        if (reptype == REPTYPE_POS) continue;
        for(;;)
          {
          if (Feptr <= Lstart_eptr) break;
          RMATCH(Fecode, RM221);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          Feptr--;
          if (utf) BACKCHAR(Feptr);
          }
        }
      else if (Lctype == OP_EXTUNI)
        {
        for (i = Lmin; i < Lmax; i++)
          {
          if (Feptr >= mb->end_subject)
            {
            SCHECK_PARTIAL();
            break;
            }
          else
            {
            GETCHARINCTEST(fc, Feptr);
            Feptr = PRIV(extuni)(fc, Feptr, mb->start_subject, mb->end_subject,
              utf, NULL);
            }
          CHECK_PARTIAL();
          COST_CHK(1);
          }
        if (reptype == REPTYPE_POS) continue;
        for(;;)
          {
#ifndef ERLANG_INTEGRATION
          int lgb, rgb;
#endif
          PCRE2_SPTR fptr;
          if (Feptr <= Lstart_eptr) break;
          RMATCH(Fecode, RM219);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          Feptr--;
          if (!utf) fc = *Feptr; else
            {
            BACKCHAR(Feptr);
            GETCHAR(fc, Feptr);
            }
          rgb = UCD_GRAPHBREAK(fc);
          for (;;)
            {
            if (Feptr <= Lstart_eptr) break;
            fptr = Feptr - 1;
            if (!utf) fc = *fptr; else
              {
              BACKCHAR(fptr);
              GETCHAR(fc, fptr);
              }
            lgb = UCD_GRAPHBREAK(fc);
            if ((PRIV(ucp_gbtable)[lgb] & (1u << rgb)) == 0) break;
            Feptr = fptr;
            rgb = lgb;
            COST_CHK(1);
            }
          }
        }
      else
#endif
#ifdef SUPPORT_UNICODE
      if (utf)
        {
        switch(Lctype)
          {
          case OP_ANY:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            if (IS_NEWLINE(Feptr)) break;
            if (mb->partial != 0 &&
                Feptr + 1 >= mb->end_subject &&
                NLBLOCK->nltype == NLTYPE_FIXED &&
                NLBLOCK->nllen == 2 &&
                UCHAR21(Feptr) == NLBLOCK->nl[0])
              {
              mb->hitend = TRUE;
              if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
              }
            Feptr++;
            ACROSSCHAR(Feptr < mb->end_subject, Feptr, Feptr++);
            COST_CHK(1);
            }
          break;
          case OP_ALLANY:
          if (Lmax < UINT32_MAX)
            {
            for (i = Lmin; i < Lmax; i++)
              {
              if (Feptr >= mb->end_subject)
                {
                SCHECK_PARTIAL();
                break;
                }
              Feptr++;
              ACROSSCHAR(Feptr < mb->end_subject, Feptr, Feptr++);
              COST_CHK(1);
              }
            }
          else
            {
            Feptr = mb->end_subject;
            SCHECK_PARTIAL();
            }
          break;
          case OP_ANYBYTE:
          fc = Lmax - Lmin;
          if (fc > (uint32_t)(mb->end_subject - Feptr))
            {
            Feptr = mb->end_subject;
            SCHECK_PARTIAL();
            }
          else Feptr += fc;
          break;
          case OP_ANYNL:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(fc, Feptr, len);
            if (fc == CHAR_CR)
              {
              if (++Feptr >= mb->end_subject) break;
              if (UCHAR21(Feptr) == CHAR_LF) Feptr++;
              }
            else
              {
              if (fc != CHAR_LF &&
                  (mb->bsr_convention == PCRE2_BSR_ANYCRLF ||
                   (fc != CHAR_VT && fc != CHAR_FF && fc != CHAR_NEL
#ifndef EBCDIC
                    && fc != 0x2028 && fc != 0x2029
#endif
                    )))
                break;
              Feptr += len;
              }
              COST_CHK(1);
            }
          break;
          case OP_NOT_HSPACE:
          case OP_HSPACE:
          for (i = Lmin; i < Lmax; i++)
            {
            BOOL gotspace;
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(fc, Feptr, len);
            switch(fc)
              {
              HSPACE_CASES: gotspace = TRUE; break;
              default: gotspace = FALSE; break;
              }
            if (gotspace == (Lctype == OP_NOT_HSPACE)) break;
            Feptr += len;
            COST_CHK(1);
            }
          break;
          case OP_NOT_VSPACE:
          case OP_VSPACE:
          for (i = Lmin; i < Lmax; i++)
            {
            BOOL gotspace;
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(fc, Feptr, len);
            switch(fc)
              {
              VSPACE_CASES: gotspace = TRUE; break;
              default: gotspace = FALSE; break;
              }
            if (gotspace == (Lctype == OP_NOT_VSPACE)) break;
            Feptr += len;
            COST_CHK(1);
            }
          break;
          case OP_NOT_DIGIT:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(fc, Feptr, len);
            if (fc < 256 && (mb->ctypes[fc] & ctype_digit) != 0) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case OP_DIGIT:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(fc, Feptr, len);
            if (fc >= 256 ||(mb->ctypes[fc] & ctype_digit) == 0) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case OP_NOT_WHITESPACE:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(fc, Feptr, len);
            if (fc < 256 && (mb->ctypes[fc] & ctype_space) != 0) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case OP_WHITESPACE:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(fc, Feptr, len);
            if (fc >= 256 ||(mb->ctypes[fc] & ctype_space) == 0) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case OP_NOT_WORDCHAR:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(fc, Feptr, len);
            if (fc < 256 && (mb->ctypes[fc] & ctype_word) != 0) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          case OP_WORDCHAR:
          for (i = Lmin; i < Lmax; i++)
            {
            int len = 1;
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            GETCHARLEN(fc, Feptr, len);
            if (fc >= 256 || (mb->ctypes[fc] & ctype_word) == 0) break;
            Feptr+= len;
            COST_CHK(1);
            }
          break;
          default:
          PCRE2_DEBUG_UNREACHABLE();
          return PCRE2_ERROR_INTERNAL;
          }
        if (reptype == REPTYPE_POS) continue;
        for(;;)
          {
          if (Feptr <= Lstart_eptr) break;
          RMATCH(Fecode, RM220);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          Feptr--;
          BACKCHAR(Feptr);
          if (Lctype == OP_ANYNL && Feptr > Lstart_eptr &&
              UCHAR21(Feptr) == CHAR_NL && UCHAR21(Feptr - 1) == CHAR_CR)
            Feptr--;
          }
        }
      else
#endif
        {
        switch(Lctype)
          {
          case OP_ANY:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            if (IS_NEWLINE(Feptr)) break;
            if (mb->partial != 0 &&
                Feptr + 1 >= mb->end_subject &&
                NLBLOCK->nltype == NLTYPE_FIXED &&
                NLBLOCK->nllen == 2 &&
                *Feptr == NLBLOCK->nl[0])
              {
              mb->hitend = TRUE;
              if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
              }
            Feptr++;
            COST_CHK(1);
            }
          break;
          case OP_ALLANY:
          case OP_ANYBYTE:
          fc = Lmax - Lmin;
          if (fc > (uint32_t)(mb->end_subject - Feptr))
            {
            Feptr = mb->end_subject;
            SCHECK_PARTIAL();
            }
          else Feptr += fc;
          break;
          case OP_ANYNL:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            fc = *Feptr;
            if (fc == CHAR_CR)
              {
              if (++Feptr >= mb->end_subject) break;
              if (*Feptr == CHAR_LF) Feptr++;
              }
            else
              {
              if (fc != CHAR_LF && (mb->bsr_convention == PCRE2_BSR_ANYCRLF ||
                 (fc != CHAR_VT && fc != CHAR_FF && fc != CHAR_NEL
#if PCRE2_CODE_UNIT_WIDTH != 8
                 && fc != 0x2028 && fc != 0x2029
#endif
                 ))) break;
              Feptr++;
              }
            COST_CHK(1);
            }
          break;
          case OP_NOT_HSPACE:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            switch(*Feptr)
              {
              default: Feptr++; break;
              HSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
              HSPACE_MULTIBYTE_CASES:
#endif
              goto ENDLOOP00;
              }
              COST_CHK(1);
            }
          ENDLOOP00:
          break;
          case OP_HSPACE:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            switch(*Feptr)
              {
              default: goto ENDLOOP01;
              HSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
              HSPACE_MULTIBYTE_CASES:
#endif
              Feptr++; break;
              }
              COST_CHK(1);
            }
          ENDLOOP01:
          break;
          case OP_NOT_VSPACE:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            switch(*Feptr)
              {
              default: Feptr++; break;
              VSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
              VSPACE_MULTIBYTE_CASES:
#endif
              goto ENDLOOP02;
              }
              COST_CHK(1);
            }
          ENDLOOP02:
          break;
          case OP_VSPACE:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            switch(*Feptr)
              {
              default: goto ENDLOOP03;
              VSPACE_BYTE_CASES:
#if PCRE2_CODE_UNIT_WIDTH != 8
              VSPACE_MULTIBYTE_CASES:
#endif
              Feptr++; break;
              }
              COST_CHK(1);
            }
          ENDLOOP03:
          break;
          case OP_NOT_DIGIT:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            if (MAX_255(*Feptr) && (mb->ctypes[*Feptr] & ctype_digit) != 0)
              break;
            Feptr++;
            COST_CHK(1);
            }
          break;
          case OP_DIGIT:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            if (!MAX_255(*Feptr) || (mb->ctypes[*Feptr] & ctype_digit) == 0)
              break;
            Feptr++;
            COST_CHK(1);
            }
          break;
          case OP_NOT_WHITESPACE:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            if (MAX_255(*Feptr) && (mb->ctypes[*Feptr] & ctype_space) != 0)
              break;
            Feptr++;
            COST_CHK(1);
            }
          break;
          case OP_WHITESPACE:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            if (!MAX_255(*Feptr) || (mb->ctypes[*Feptr] & ctype_space) == 0)
              break;
            Feptr++;
            COST_CHK(1);
            }
          break;
          case OP_NOT_WORDCHAR:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            if (MAX_255(*Feptr) && (mb->ctypes[*Feptr] & ctype_word) != 0)
              break;
            Feptr++;
            COST_CHK(1);
            }
          break;
          case OP_WORDCHAR:
          for (i = Lmin; i < Lmax; i++)
            {
            if (Feptr >= mb->end_subject)
              {
              SCHECK_PARTIAL();
              break;
              }
            if (!MAX_255(*Feptr) || (mb->ctypes[*Feptr] & ctype_word) == 0)
              break;
            Feptr++;
            COST_CHK(1);
            }
          break;
          default:
          PCRE2_DEBUG_UNREACHABLE();
          return PCRE2_ERROR_INTERNAL;
          }
        if (reptype == REPTYPE_POS) continue;
        for (;;)
          {
          if (Feptr == Lstart_eptr) break;
          RMATCH(Fecode, RM34);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          Feptr--;
          if (Lctype == OP_ANYNL && Feptr > Lstart_eptr && *Feptr == CHAR_LF &&
              Feptr[-1] == CHAR_CR) Feptr--;
          }
        }
      }
    break;
#undef Lstart_eptr
#undef Lmin
#undef Lmax
#undef Lctype
#undef Lpropvalue
#define Lmin      F->temp_32[0]
#define Lmax      F->temp_32[1]
#define Lcaseless F->temp_32[2]
#define Lcaseopts F->temp_32[3]
#define Lstart    F->temp_sptr[0]
#define Loffset   F->temp_size
    case OP_DNREF:
    case OP_DNREFI:
    Lcaseless = (Fop == OP_DNREFI);
    Lcaseopts = (Fop == OP_DNREFI)? Fecode[1 + 2*IMM2_SIZE] : 0;
      {
      int count = GET2(Fecode, 1+IMM2_SIZE);
      PCRE2_SPTR slot = mb->name_table + GET2(Fecode, 1) * mb->name_entry_size;
      Fecode += 1 + 2*IMM2_SIZE + (Fop == OP_DNREFI? 1 : 0);
      while (count-- > 0)
        {
        Loffset = (GET2(slot, 0) << 1) - 2;
        if (Loffset < Foffset_top && Fovector[Loffset] != PCRE2_UNSET) break;
        slot += mb->name_entry_size;
        COST(1);
        }
      }
    goto REF_REPEAT;
    case OP_REF:
    case OP_REFI:
    Lcaseless = (Fop == OP_REFI);
    Lcaseopts = (Fop == OP_REFI)? Fecode[1 + IMM2_SIZE] : 0;
    Loffset = (GET2(Fecode, 1) << 1) - 2;
    Fecode += 1 + IMM2_SIZE + (Fop == OP_REFI? 1 : 0);
    REF_REPEAT:
    switch (*Fecode)
      {
      case OP_CRSTAR:
      case OP_CRMINSTAR:
      case OP_CRPLUS:
      case OP_CRMINPLUS:
      case OP_CRQUERY:
      case OP_CRMINQUERY:
      fc = *Fecode++ - OP_CRSTAR;
      Lmin = rep_min[fc];
      Lmax = rep_max[fc];
      reptype = rep_typ[fc];
      break;
      case OP_CRRANGE:
      case OP_CRMINRANGE:
      Lmin = GET2(Fecode, 1);
      Lmax = GET2(Fecode, 1 + IMM2_SIZE);
      reptype = rep_typ[*Fecode - OP_CRSTAR];
      if (Lmax == 0) Lmax = UINT32_MAX;
      Fecode += 1 + 2 * IMM2_SIZE;
      break;
      default:
        {
        rrc = match_ref(Loffset, Lcaseless, Lcaseopts, F, mb, &length);
        if (rrc != 0)
          {
          if (rrc > 0) Feptr = mb->end_subject;
          CHECK_PARTIAL();
          RRETURN(MATCH_NOMATCH);
          }
        }
      Feptr += length;
      continue;
      }
    if (Loffset < Foffset_top && Fovector[Loffset] != PCRE2_UNSET)
      {
      if (Fovector[Loffset] == Fovector[Loffset + 1]) continue;
      }
    else
      {
      if (Lmin == 0 || (mb->poptions & PCRE2_MATCH_UNSET_BACKREF) != 0)
        continue;
      }
    COST(Lmin);
    for (i = 1; i <= Lmin; i++)
      {
      PCRE2_SIZE slength;
      rrc = match_ref(Loffset, Lcaseless, Lcaseopts, F, mb, &slength);
      if (rrc != 0)
        {
        if (rrc > 0) Feptr = mb->end_subject;
        CHECK_PARTIAL();
        RRETURN(MATCH_NOMATCH);
        }
      Feptr += slength;
      }
    if (Lmin == Lmax) continue;
    if (reptype == REPTYPE_MIN)
      {
      for (;;)
        {
        PCRE2_SIZE slength;
        RMATCH(Fecode, RM20);
        if (rrc != MATCH_NOMATCH) RRETURN(rrc);
        if (Lmin++ >= Lmax) RRETURN(MATCH_NOMATCH);
        rrc = match_ref(Loffset, Lcaseless, Lcaseopts, F, mb, &slength);
        if (rrc != 0)
          {
          if (rrc > 0) Feptr = mb->end_subject;
          CHECK_PARTIAL();
          RRETURN(MATCH_NOMATCH);
          }
        Feptr += slength;
        }
      PCRE2_UNREACHABLE();
      }
    else
      {
#ifndef ERLANG_INTEGRATION
      BOOL samelengths = TRUE;
#else
      samelengths = TRUE;
#endif
      Lstart = Feptr;
      Flength = Fovector[Loffset+1] - Fovector[Loffset];
      for (i = Lmin; i < Lmax; i++)
        {
        PCRE2_SIZE slength;
        rrc = match_ref(Loffset, Lcaseless, Lcaseopts, F, mb, &slength);
        if (rrc != 0)
          {
          if (rrc > 0 && mb->partial != 0 &&
              mb->end_subject > mb->start_used_ptr)
            {
            mb->hitend = TRUE;
            if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
            }
          break;
          }
        if (slength != Flength) samelengths = FALSE;
        Feptr += slength;
        COST_CHK(1);
        }
      if (samelengths)
        {
        while (Feptr >= Lstart)
          {
          RMATCH(Fecode, RM21);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          Feptr -= Flength;
          }
        }
      else
        {
        Lmax = i;
        for (;;)
          {
          RMATCH(Fecode, RM22);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (Feptr == Lstart) break;
          Feptr = Lstart;
          Lmax--;
          for (i = Lmin; i < Lmax; i++)
            {
            PCRE2_SIZE slength;
            (void)match_ref(Loffset, Lcaseless, Lcaseopts, F, mb, &slength);
            Feptr += slength;
            COST_CHK(1);
            }
          }
        }
      RRETURN(MATCH_NOMATCH);
      }
    PCRE2_DEBUG_UNREACHABLE();
#undef Lcaseless
#undef Lmin
#undef Lmax
#undef Lstart
#undef Loffset
#define Lnext_ecode F->temp_sptr[0]
    case OP_BRAZERO:
    Lnext_ecode = Fecode + 1;
    RMATCH(Lnext_ecode, RM9);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    do Lnext_ecode += GET(Lnext_ecode, 1); while (*Lnext_ecode == OP_ALT);
    Fecode = Lnext_ecode + 1 + LINK_SIZE;
    break;
    case OP_BRAMINZERO:
    Lnext_ecode = Fecode + 1;
    do Lnext_ecode += GET(Lnext_ecode, 1); while (*Lnext_ecode == OP_ALT);
    RMATCH(Lnext_ecode + 1 + LINK_SIZE, RM10);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    Fecode++;
    break;
#undef Lnext_ecode
    case OP_SKIPZERO:
    Fecode++;
    do Fecode += GET(Fecode,1); while (*Fecode == OP_ALT);
    Fecode += 1 + LINK_SIZE;
    break;
#define Lframe_type    F->temp_32[0]
#define Lmatched_once  F->temp_32[1]
#define Lzero_allowed  F->temp_32[2]
#define Lstart_eptr    F->temp_sptr[0]
#define Lstart_group   F->temp_sptr[1]
    case OP_BRAPOSZERO:
    Lzero_allowed = TRUE;
    Fecode += 1;
    if (*Fecode == OP_CBRAPOS || *Fecode == OP_SCBRAPOS)
      goto POSSESSIVE_CAPTURE;
    goto POSSESSIVE_NON_CAPTURE;
    case OP_BRAPOS:
    case OP_SBRAPOS:
    Lzero_allowed = FALSE;
    POSSESSIVE_NON_CAPTURE:
    Lframe_type = GF_NOCAPTURE;
    goto POSSESSIVE_GROUP;
    case OP_CBRAPOS:
    case OP_SCBRAPOS:
    Lzero_allowed = FALSE;
    POSSESSIVE_CAPTURE:
    number = GET2(Fecode, 1+LINK_SIZE);
    Lframe_type = GF_CAPTURE | number;
    POSSESSIVE_GROUP:
    Lmatched_once = FALSE;
    Lstart_group = Fecode;
    for (;;)
      {
      Lstart_eptr = Feptr;
      group_frame_type = Lframe_type;
      RMATCH(Fecode + PRIV(OP_lengths)[*Fecode], RM8);
      if (rrc == MATCH_KETRPOS)
        {
        Lmatched_once = TRUE;
        if (Feptr == Lstart_eptr)
          {
          do Fecode += GET(Fecode, 1); while (*Fecode == OP_ALT);
          break;
          }
        Fecode = Lstart_group;
        continue;
        }
      if (rrc == MATCH_THEN)
        {
        PCRE2_SPTR next_ecode = Fecode + GET(Fecode,1);
        if (mb->verb_ecode_ptr < next_ecode &&
            (*Fecode == OP_ALT || *next_ecode == OP_ALT))
          rrc = MATCH_NOMATCH;
        }
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      Fecode += GET(Fecode, 1);
      if (*Fecode != OP_ALT) break;
      }
    if (Lmatched_once || Lzero_allowed)
      {
      Fecode += 1 + LINK_SIZE;
      break;
      }
    RRETURN(MATCH_NOMATCH);
#undef Lmatched_once
#undef Lzero_allowed
#undef Lframe_type
#undef Lstart_eptr
#undef Lstart_group
#define Lframe_type F->temp_32[0]
#define Lnext_branch F->temp_sptr[0]
    case OP_BRA:
    if (mb->hasthen || Frdepth == 0)
      {
      Lframe_type = 0;
      goto GROUPLOOP;
      }
    for (;;)
      {
      Lnext_branch = Fecode + GET(Fecode, 1);
      if (*Lnext_branch != OP_ALT) break;
      RMATCH(Fecode + PRIV(OP_lengths)[*Fecode], RM1);
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      Fecode = Lnext_branch;
      }
    Fecode += PRIV(OP_lengths)[*Fecode];
    break;
#undef Lnext_branch
    case OP_CBRA:
    case OP_SCBRA:
    Lframe_type = GF_CAPTURE | GET2(Fecode, 1+LINK_SIZE);
    goto GROUPLOOP;
    case OP_ONCE:
    case OP_SCRIPT_RUN:
    case OP_SBRA:
    Lframe_type = GF_NOCAPTURE | Fop;
    GROUPLOOP:
    for (;;)
      {
      group_frame_type = Lframe_type;
      RMATCH(Fecode + PRIV(OP_lengths)[*Fecode], RM2);
      if (rrc == MATCH_THEN)
        {
        PCRE2_SPTR next_ecode = Fecode + GET(Fecode,1);
        if (mb->verb_ecode_ptr < next_ecode &&
            (*Fecode == OP_ALT || *next_ecode == OP_ALT))
          rrc = MATCH_NOMATCH;
        }
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      Fecode += GET(Fecode, 1);
      if (*Fecode != OP_ALT) RRETURN(MATCH_NOMATCH);
      }
    PCRE2_UNREACHABLE();
#undef Lframe_type
#define Lframe_type F->temp_32[0]
#define Lstart_branch F->temp_sptr[0]
    case OP_RECURSE:
    bracode = mb->start_code + GET(Fecode, 1);
    number = (bracode == mb->start_code)? 0 : GET2(bracode, 1 + LINK_SIZE);
    if (Fcurrent_recurse != RECURSE_UNSET)
      {
      offset = Flast_group_offset;
      while (offset != PCRE2_UNSET)
        {
        N = (heapframe *)((char *)match_data->heapframes + offset);
        P = (heapframe *)((char *)N - frame_size);
        if (N->group_frame_type == (GF_RECURSE | number))
          {
          if (Feptr == P->eptr && mb->last_used_ptr == P->recurse_last_used &&
               (mb->moptions & PCRE2_DISABLE_RECURSELOOP_CHECK) == 0)
            return PCRE2_ERROR_RECURSELOOP;
          break;
          }
        offset = P->last_group_offset;
        COST(1);
        }
      }
    F->recurse_last_used = mb->last_used_ptr;
    Lstart_branch = bracode;
    Lframe_type = GF_RECURSE | number;
    for (;;)
      {
      PCRE2_SPTR next_ecode;
      group_frame_type = Lframe_type;
      RMATCH(Lstart_branch + PRIV(OP_lengths)[*Lstart_branch], RM11);
      next_ecode = Lstart_branch + GET(Lstart_branch,1);
      if (rrc >= MATCH_BACKTRACK_MIN && rrc <= MATCH_BACKTRACK_MAX &&
          mb->verb_current_recurse == (Lframe_type ^ GF_RECURSE))
        {
        if (rrc == MATCH_THEN && mb->verb_ecode_ptr < next_ecode &&
            (*Lstart_branch == OP_ALT || *next_ecode == OP_ALT))
          rrc = MATCH_NOMATCH;
        else RRETURN(MATCH_NOMATCH);
        }
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      Lstart_branch = next_ecode;
      if (*Lstart_branch != OP_ALT) RRETURN(MATCH_NOMATCH);
      }
    PCRE2_UNREACHABLE();
#undef Lframe_type
#undef Lstart_branch
#define Lframe_type  F->temp_32[0]
    case OP_ASSERT:
    case OP_ASSERTBACK:
    case OP_ASSERT_NA:
    case OP_ASSERTBACK_NA:
    Lframe_type = GF_NOCAPTURE | Fop;
    for (;;)
      {
      group_frame_type = Lframe_type;
      RMATCH(Fecode + PRIV(OP_lengths)[*Fecode], RM3);
      if (rrc == MATCH_ACCEPT)
        {
        memcpy(Fovector,
              (char *)assert_accept_frame + offsetof(heapframe, ovector),
              assert_accept_frame->offset_top * sizeof(PCRE2_SIZE));
        Foffset_top = assert_accept_frame->offset_top;
        Fmark = assert_accept_frame->mark;
        break;
        }
      if (rrc != MATCH_NOMATCH && rrc != MATCH_THEN) RRETURN(rrc);
      Fecode += GET(Fecode, 1);
      if (*Fecode != OP_ALT) RRETURN(MATCH_NOMATCH);
      }
    do Fecode += GET(Fecode, 1); while (*Fecode == OP_ALT);
    Fecode += 1 + LINK_SIZE;
    break;
#undef Lframe_type
#define Lframe_type  F->temp_32[0]
    case OP_ASSERT_NOT:
    case OP_ASSERTBACK_NOT:
    Lframe_type  = GF_NOCAPTURE | Fop;
    for (;;)
      {
      group_frame_type = Lframe_type;
      RMATCH(Fecode + PRIV(OP_lengths)[*Fecode], RM4);
      switch(rrc)
        {
        case MATCH_ACCEPT:
        case MATCH_MATCH:
        RRETURN (MATCH_NOMATCH);
        case MATCH_NOMATCH:
        case MATCH_THEN:
        Fecode += GET(Fecode, 1);
        if (*Fecode != OP_ALT) goto ASSERT_NOT_FAILED;
        break;
        case MATCH_COMMIT:
        case MATCH_SKIP:
        case MATCH_PRUNE:
        do Fecode += GET(Fecode, 1); while (*Fecode == OP_ALT);
        goto ASSERT_NOT_FAILED;
        default:
        RRETURN(rrc);
        }
      }
    ASSERT_NOT_FAILED:
    Fecode += 1 + LINK_SIZE;
    break;
#undef Lframe_type
#define Lframe_type          F->temp_32[0]
#define Lextra_size          F->temp_32[1]
#define Lsaved_moptions      F->temp_32[2]
#define Lsaved_end_subject   F->temp_sptr[0]
#define Lsaved_eptr          F->temp_sptr[1]
#define Ltrue_end_extra      F->temp_size
    case OP_ASSERT_SCS:
      {
      PCRE2_SPTR ecode = Fecode + 1 + LINK_SIZE;
      uint32_t extra_size = 0;
      int count;
      PCRE2_SPTR slot;
      offset = 0;
      (void)offset;
      for (;;)
        {
        if (*ecode == OP_CREF)
          {
          extra_size += 1+IMM2_SIZE;
          offset = (GET2(ecode, 1) << 1) - 2;
          ecode += 1+IMM2_SIZE;
          if (offset < Foffset_top && Fovector[offset] != PCRE2_UNSET)
            goto SCS_OFFSET_FOUND;
          continue;
          }
        if (*ecode != OP_DNCREF) RRETURN(MATCH_NOMATCH);
        count = GET2(ecode, 1 + IMM2_SIZE);
        slot = mb->name_table + GET2(ecode, 1) * mb->name_entry_size;
        extra_size += 1+2*IMM2_SIZE;
        ecode += 1+2*IMM2_SIZE;
        while (count > 0)
          {
          offset = (GET2(slot, 0) << 1) - 2;
          if (offset < Foffset_top && Fovector[offset] != PCRE2_UNSET)
            goto SCS_OFFSET_FOUND;
          slot += mb->name_entry_size;
          count--;
          COST(1);
          }
        COST(1);
        }
      SCS_OFFSET_FOUND:
      for (;;)
        {
        if (*ecode == OP_CREF)
          {
          extra_size += 1+IMM2_SIZE;
          ecode += 1+IMM2_SIZE;
          }
        else if (*ecode == OP_DNCREF)
          {
          extra_size += 1+2*IMM2_SIZE;
          ecode += 1+2*IMM2_SIZE;
          }
        else break;
        }
      Lextra_size = extra_size;
      }
    Lsaved_end_subject = mb->end_subject;
    Ltrue_end_extra = mb->true_end_subject - mb->end_subject;
    Lsaved_eptr = Feptr;
    Lsaved_moptions = mb->moptions;
    Feptr = mb->start_subject + Fovector[offset];
    mb->true_end_subject = mb->end_subject =
      mb->start_subject + Fovector[offset + 1];
    mb->moptions &= ~PCRE2_NOTEOL;
    Lframe_type = GF_NOCAPTURE | Fop;
    for (;;)
      {
      group_frame_type = Lframe_type;
      RMATCH(Fecode + 1 + LINK_SIZE + Lextra_size, RM38);
      if (rrc == MATCH_ACCEPT)
        {
        memcpy(Fovector,
              (char *)assert_accept_frame + offsetof(heapframe, ovector),
              assert_accept_frame->offset_top * sizeof(PCRE2_SIZE));
        Foffset_top = assert_accept_frame->offset_top;
        Fmark = assert_accept_frame->mark;
        mb->end_subject = Lsaved_end_subject;
        mb->true_end_subject = mb->end_subject + Ltrue_end_extra;
        mb->moptions = Lsaved_moptions;
        break;
        }
      if (rrc != MATCH_NOMATCH && rrc != MATCH_THEN)
        {
        mb->end_subject = Lsaved_end_subject;
        mb->true_end_subject = mb->end_subject + Ltrue_end_extra;
        mb->moptions = Lsaved_moptions;
        RRETURN(rrc);
        }
      Fecode += GET(Fecode, 1);
      if (*Fecode != OP_ALT)
        {
        mb->end_subject = Lsaved_end_subject;
        mb->true_end_subject = mb->end_subject + Ltrue_end_extra;
        mb->moptions = Lsaved_moptions;
        RRETURN(MATCH_NOMATCH);
        }
      Lextra_size = 0;
      }
    do Fecode += GET(Fecode, 1); while (*Fecode == OP_ALT);
    Fecode += 1 + LINK_SIZE;
    Feptr = Lsaved_eptr;
    break;
#undef Lframe_type
#undef Lextra_size
#undef Lsaved_end_subject
#undef Lsaved_eptr
#undef Ltrue_end_extra
#undef Lsave_moptions
    case OP_CALLOUT:
    case OP_CALLOUT_STR:
    rrc = do_callout(F, mb, &length);
    if (rrc > 0) RRETURN(MATCH_NOMATCH);
    if (rrc < 0) RRETURN(rrc);
    Fecode += length;
    break;
    case OP_COND:
    case OP_SCOND:
    Flength = GET(Fecode, 1);
    if (Fecode[Flength] != OP_ALT) Flength -= 1 + LINK_SIZE;
    Fecode += 1 + LINK_SIZE;
    if (*Fecode == OP_CALLOUT || *Fecode == OP_CALLOUT_STR)
      {
      rrc = do_callout(F, mb, &length);
      if (rrc > 0) RRETURN(MATCH_NOMATCH);
      if (rrc < 0) RRETURN(rrc);
      Fecode += length;
      Flength -= length;
      }
    condition = FALSE;
    switch(*Fecode)
      {
      case OP_RREF:
      if (Fcurrent_recurse != RECURSE_UNSET)
        {
        number = GET2(Fecode, 1);
        condition = (number == RREF_ANY || number == Fcurrent_recurse);
        }
      break;
      case OP_DNRREF:
      if (Fcurrent_recurse != RECURSE_UNSET)
        {
        int count = GET2(Fecode, 1 + IMM2_SIZE);
        PCRE2_SPTR slot = mb->name_table + GET2(Fecode, 1) * mb->name_entry_size;
        while (count-- > 0)
          {
          number = GET2(slot, 0);
          condition = number == Fcurrent_recurse;
          if (condition) break;
          slot += mb->name_entry_size;
          COST(1);
          }
        }
      break;
      case OP_CREF:
      offset = (GET2(Fecode, 1) << 1) - 2;
      condition = offset < Foffset_top && Fovector[offset] != PCRE2_UNSET;
      break;
      case OP_DNCREF:
        {
        int count = GET2(Fecode, 1 + IMM2_SIZE);
        PCRE2_SPTR slot = mb->name_table + GET2(Fecode, 1) * mb->name_entry_size;
        while (count-- > 0)
          {
          offset = (GET2(slot, 0) << 1) - 2;
          condition = offset < Foffset_top && Fovector[offset] != PCRE2_UNSET;
          if (condition) break;
          slot += mb->name_entry_size;
          COST(1);
          }
        }
      break;
      case OP_FALSE:
      case OP_FAIL:
      break;
      case OP_TRUE:
      condition = TRUE;
      break;
#define Lpositive      F->temp_32[0]
#define Lstart_branch  F->temp_sptr[0]
      default:
      Lpositive = (*Fecode == OP_ASSERT || *Fecode == OP_ASSERTBACK);
      Lstart_branch = Fecode;
      for (;;)
        {
        group_frame_type = GF_CONDASSERT | *Fecode;
        RMATCH(Lstart_branch + PRIV(OP_lengths)[*Lstart_branch], RM5);
        switch(rrc)
          {
          case MATCH_ACCEPT:
          memcpy(Fovector,
                (char *)assert_accept_frame + offsetof(heapframe, ovector),
                assert_accept_frame->offset_top * sizeof(PCRE2_SIZE));
          Foffset_top = assert_accept_frame->offset_top;
          PCRE2_FALLTHROUGH
          case MATCH_MATCH:
          condition = Lpositive;
          break;
          case MATCH_NOMATCH:
          case MATCH_THEN:
          Lstart_branch += GET(Lstart_branch, 1);
          if (*Lstart_branch == OP_ALT) continue;
          condition = !Lpositive;
          break;
          case MATCH_COMMIT:
          case MATCH_SKIP:
          case MATCH_PRUNE:
          condition = !Lpositive;
          break;
          default:
          RRETURN(rrc);
          }
        break;
        }
      if (condition)
        {
        do Fecode += GET(Fecode, 1); while (*Fecode == OP_ALT);
        }
      break;
      }
#undef Lpositive
#undef Lstart_branch
    Fecode += condition? PRIV(OP_lengths)[*Fecode] : Flength;
    if (Fop == OP_SCOND)
      {
      group_frame_type  = GF_NOCAPTURE | Fop;
      RMATCH(Fecode, RM35);
      RRETURN(rrc);
      }
    break;
    case OP_REVERSE:
    number = GET2(Fecode, 1);
#ifdef SUPPORT_UNICODE
    if (utf)
      {
      while (number > 0)
        {
        --number;
        if (Feptr <= mb->check_subject) RRETURN(MATCH_NOMATCH);
        Feptr--;
        BACKCHAR(Feptr);
        COST(1);
        }
      }
    else
#endif
      {
      if ((ptrdiff_t)number > Feptr - mb->start_subject) RRETURN(MATCH_NOMATCH);
      Feptr -= number;
      }
    if (Feptr < mb->start_used_ptr) mb->start_used_ptr = Feptr;
    Fecode += 1 + IMM2_SIZE;
    break;
#define Lmin F->temp_32[0]
#define Lmax F->temp_32[1]
#define Leptr F->temp_sptr[0]
    case OP_VREVERSE:
    Lmin = GET2(Fecode, 1);
    Lmax = GET2(Fecode, 1 + IMM2_SIZE);
    Leptr = Feptr;
#ifdef SUPPORT_UNICODE
    if (utf)
      {
      for (i = 0; i < Lmax; i++)
        {
        if (Feptr == mb->start_subject)
          {
          if (i < Lmin) RRETURN(MATCH_NOMATCH);
          Lmax = i;
          break;
          }
        Feptr--;
        BACKCHAR(Feptr);
        COST_CHK(1);
        }
      }
    else
#endif
      {
      ptrdiff_t diff = Feptr - mb->start_subject;
      uint32_t available = (diff > 65535)? 65535 : ((diff > 0)? (int)diff : 0);
      if (Lmin > available) RRETURN(MATCH_NOMATCH);
      if (Lmax > available) Lmax = available;
      Feptr -= Lmax;
      }
    for (;;)
      {
      RMATCH(Fecode + 1 + 2 * IMM2_SIZE, RM37);
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      if (Lmax-- <= Lmin) RRETURN(MATCH_NOMATCH);
      Feptr++;
#ifdef SUPPORT_UNICODE
      if (utf) { FORWARDCHARTEST(Feptr, mb->end_subject); }
#endif
      }
    PCRE2_UNREACHABLE();
#undef Lmin
#undef Lmax
#undef Leptr
    case OP_ALT:
    branch_end = Fecode;
    do Fecode += GET(Fecode,1); while (*Fecode == OP_ALT);
    break;
    case OP_KET:
    case OP_KETRMIN:
    case OP_KETRMAX:
    case OP_KETRPOS:
    bracode = Fecode - GET(Fecode, 1);
    if (branch_end == NULL) branch_end = Fecode;
    branch_start = bracode;
    while (branch_start + GET(branch_start, 1) != branch_end)
      branch_start += GET(branch_start, 1);
    branch_end = NULL;
    if (*bracode != OP_BRA && *bracode != OP_COND)
      {
      N = (heapframe *)((char *)match_data->heapframes + Flast_group_offset);
      P = (heapframe *)((char *)N - frame_size);
      Flast_group_offset = P->last_group_offset;
#ifdef DEBUG_SHOW_RMATCH
      fprintf(stderr, "++ KET for frame=%d type=%x prev char offset=%lu\n",
        N->rdepth, N->group_frame_type,
        (char *)P->eptr - (char *)mb->start_subject);
#endif
      if (GF_IDMASK(N->group_frame_type) == GF_CONDASSERT)
        {
        if ((*bracode == OP_ASSERTBACK || *bracode == OP_ASSERTBACK_NOT) &&
            branch_start[1 + LINK_SIZE] == OP_VREVERSE && Feptr != P->eptr)
          RRETURN(MATCH_NOMATCH);
        memcpy((char *)P + offsetof(heapframe, ovector), Fovector,
          Foffset_top * sizeof(PCRE2_SIZE));
        P->offset_top = Foffset_top;
        P->mark = Fmark;
        Fback_frame = (char *)F - (char *)P;
        RRETURN(MATCH_MATCH);
        }
      }
    else P = NULL;
    switch (*bracode)
      {
      case OP_BRA:
      if (Fcurrent_recurse != 0 || Fecode[1+LINK_SIZE] != OP_END) break;
      offset = Flast_group_offset;
      PCRE2_ASSERT(offset != PCRE2_UNSET);
      if (offset == PCRE2_UNSET) return PCRE2_ERROR_INTERNAL;
      N = (heapframe *)((char *)match_data->heapframes + offset);
      P = (heapframe *)((char *)N - frame_size);
      Flast_group_offset = P->last_group_offset;
      Fecode = P->ecode + 1 + LINK_SIZE;
      if (*Fecode != OP_CREF)
        {
        memcpy(F->ovector, P->ovector, Foffset_top * sizeof(PCRE2_SIZE));
        Foffset_top = P->offset_top;
        }
      else
        recurse_update_offsets(F, P);
      Fcapture_last = P->capture_last;
      Fcurrent_recurse = P->current_recurse;
      continue;
      case OP_COND:
      case OP_SCOND:
      break;
      case OP_ASSERTBACK_NA:
      if (branch_start[1 + LINK_SIZE] == OP_VREVERSE && Feptr != P->eptr)
        RRETURN(MATCH_NOMATCH);
      PCRE2_FALLTHROUGH
      case OP_ASSERT_NA:
      if (Feptr > mb->last_used_ptr) mb->last_used_ptr = Feptr;
      Feptr = P->eptr;
      break;
      case OP_ASSERTBACK:
      if (branch_start[1 + LINK_SIZE] == OP_VREVERSE && Feptr != P->eptr)
        RRETURN(MATCH_NOMATCH);
      PCRE2_FALLTHROUGH
      case OP_ASSERT:
      if (Feptr > mb->last_used_ptr) mb->last_used_ptr = Feptr;
      Feptr = P->eptr;
      PCRE2_FALLTHROUGH
      case OP_ONCE:
      Fback_frame = ((char *)F - (char *)P);
      for (;;)
        {
        uint32_t y = GET(P->ecode,1);
        if ((P->ecode)[y] != OP_ALT) break;
        P->ecode += y;
        }
      break;
      case OP_ASSERTBACK_NOT:
      if (branch_start[1 + LINK_SIZE] == OP_VREVERSE && Feptr != P->eptr)
        RRETURN(MATCH_NOMATCH);
      PCRE2_FALLTHROUGH
      case OP_ASSERT_NOT:
      RRETURN(MATCH_MATCH);
      case OP_ASSERT_SCS:
      F->temp_sptr[0] = mb->end_subject;
      mb->end_subject = P->temp_sptr[0];
      mb->true_end_subject = mb->end_subject + P->temp_size;
      Feptr = P->temp_sptr[1];
      RMATCH(Fecode + 1 + LINK_SIZE, RM39);
      mb->end_subject = F->temp_sptr[0];
      mb->true_end_subject = mb->end_subject;
      RRETURN(rrc);
      break;
      case OP_SCRIPT_RUN:
      if (!PRIV(script_run)(P->eptr, Feptr, utf)) RRETURN(MATCH_NOMATCH);
      break;
      case OP_CBRA:
      case OP_CBRAPOS:
      case OP_SCBRA:
      case OP_SCBRAPOS:
      number = GET2(bracode, 1+LINK_SIZE);
      if (Fcurrent_recurse == number)
        {
        P = (heapframe *)((char *)N - frame_size);
        Fecode = P->ecode + 1 + LINK_SIZE;
        if (*Fecode != OP_CREF)
          {
          memcpy(F->ovector, P->ovector, Foffset_top * sizeof(PCRE2_SIZE));
          Foffset_top = P->offset_top;
          }
        else
          recurse_update_offsets(F, P);
        Fcapture_last = P->capture_last;
        Fcurrent_recurse = P->current_recurse;
        continue;
        }
      offset = (number << 1) - 2;
      Fcapture_last = number;
      Fovector[offset] = P->eptr - mb->start_subject;
      Fovector[offset+1] = Feptr - mb->start_subject;
      if (offset >= Foffset_top) Foffset_top = offset + 2;
      break;
      }
    if (*Fecode == OP_KETRPOS)
      {
      memcpy((char *)P + offsetof(heapframe, eptr),
             (char *)F + offsetof(heapframe, eptr),
             frame_copy_size);
      RRETURN(MATCH_KETRPOS);
      }
    if (Fop != OP_KET && (P == NULL || Feptr != P->eptr))
      {
      if (Fop == OP_KETRMIN)
        {
        RMATCH(Fecode + 1 + LINK_SIZE, RM6);
        if (rrc != MATCH_NOMATCH) RRETURN(rrc);
        Fecode -= GET(Fecode, 1);
        break;
        }
      RMATCH(bracode, RM7);
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      }
    Fecode += 1 + LINK_SIZE;
    break;
    case OP_CIRC:
    if (Feptr != mb->start_subject || (mb->moptions & PCRE2_NOTBOL) != 0)
      RRETURN(MATCH_NOMATCH);
    Fecode++;
    break;
    case OP_SOD:
    if (Feptr != mb->start_subject) RRETURN(MATCH_NOMATCH);
    Fecode++;
    break;
    case OP_DOLL:
    if ((mb->moptions & PCRE2_NOTEOL) != 0) RRETURN(MATCH_NOMATCH);
    if ((mb->poptions & PCRE2_DOLLAR_ENDONLY) == 0) goto ASSERT_NL_OR_EOS;
    PCRE2_FALLTHROUGH
    case OP_EOD:
    if (Feptr < mb->true_end_subject) RRETURN(MATCH_NOMATCH);
    if (mb->partial != 0)
      {
      mb->hitend = TRUE;
      if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
      }
    Fecode++;
    break;
    case OP_EODN:
    ASSERT_NL_OR_EOS:
    if (Feptr < mb->true_end_subject &&
        (!IS_NEWLINE(Feptr) || Feptr != mb->true_end_subject - mb->nllen))
      {
      if (mb->partial != 0 &&
          Feptr + 1 >= mb->end_subject &&
          NLBLOCK->nltype == NLTYPE_FIXED &&
          NLBLOCK->nllen == 2 &&
          UCHAR21TEST(Feptr) == NLBLOCK->nl[0])
        {
        mb->hitend = TRUE;
        if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
        }
      RRETURN(MATCH_NOMATCH);
      }
    if (mb->partial != 0)
      {
      mb->hitend = TRUE;
      if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
      }
    Fecode++;
    break;
    case OP_CIRCM:
    if ((mb->moptions & PCRE2_NOTBOL) != 0 && Feptr == mb->start_subject)
      RRETURN(MATCH_NOMATCH);
    if (Feptr != mb->start_subject &&
        ((Feptr == mb->end_subject &&
           (mb->poptions & PCRE2_ALT_CIRCUMFLEX) == 0) ||
         !WAS_NEWLINE(Feptr)))
      RRETURN(MATCH_NOMATCH);
    Fecode++;
    break;
    case OP_DOLLM:
    if (Feptr < mb->end_subject)
      {
      if (!IS_NEWLINE(Feptr))
        {
        if (mb->partial != 0 &&
            Feptr + 1 >= mb->end_subject &&
            NLBLOCK->nltype == NLTYPE_FIXED &&
            NLBLOCK->nllen == 2 &&
            UCHAR21TEST(Feptr) == NLBLOCK->nl[0])
          {
          mb->hitend = TRUE;
          if (mb->partial > 1) return PCRE2_ERROR_PARTIAL;
          }
        RRETURN(MATCH_NOMATCH);
        }
      }
    else
      {
      if ((mb->moptions & PCRE2_NOTEOL) != 0) RRETURN(MATCH_NOMATCH);
      SCHECK_PARTIAL();
      }
    Fecode++;
    break;
    case OP_SOM:
    if (Feptr != mb->start_subject + mb->start_offset) RRETURN(MATCH_NOMATCH);
    Fecode++;
    break;
    case OP_SET_SOM:
    Fstart_match = Feptr;
    Fecode++;
    break;
    case OP_NOT_WORD_BOUNDARY:
    case OP_WORD_BOUNDARY:
    case OP_NOT_UCP_WORD_BOUNDARY:
    case OP_UCP_WORD_BOUNDARY:
    if (Feptr == mb->check_subject) prev_is_word = FALSE; else
      {
      PCRE2_SPTR lastptr = Feptr - 1;
#ifdef SUPPORT_UNICODE
      if (utf)
        {
        BACKCHAR(lastptr);
        GETCHAR(fc, lastptr);
        }
      else
#endif
      fc = *lastptr;
      if (lastptr < mb->start_used_ptr) mb->start_used_ptr = lastptr;
#ifdef SUPPORT_UNICODE
      if (Fop == OP_UCP_WORD_BOUNDARY || Fop == OP_NOT_UCP_WORD_BOUNDARY)
        {
        int chartype = UCD_CHARTYPE(fc);
        int category = PRIV(ucp_gentype)[chartype];
        prev_is_word = (category == ucp_L || category == ucp_N ||
          chartype == ucp_Mn || chartype == ucp_Pc);
        }
      else
#endif
      prev_is_word = CHMAX_255(fc) && (mb->ctypes[fc] & ctype_word) != 0;
      }
    if (Feptr >= mb->end_subject)
      {
      SCHECK_PARTIAL();
      cur_is_word = FALSE;
      }
    else
      {
      PCRE2_SPTR nextptr = Feptr + 1;
#ifdef SUPPORT_UNICODE
      if (utf)
        {
        FORWARDCHARTEST(nextptr, mb->end_subject);
        GETCHAR(fc, Feptr);
        }
      else
#endif
      fc = *Feptr;
      if (nextptr > mb->last_used_ptr) mb->last_used_ptr = nextptr;
#ifdef SUPPORT_UNICODE
      if (Fop == OP_UCP_WORD_BOUNDARY || Fop == OP_NOT_UCP_WORD_BOUNDARY)
        {
        int chartype = UCD_CHARTYPE(fc);
        int category = PRIV(ucp_gentype)[chartype];
        cur_is_word = (category == ucp_L || category == ucp_N ||
          chartype == ucp_Mn || chartype == ucp_Pc);
        }
      else
#endif
      cur_is_word = CHMAX_255(fc) && (mb->ctypes[fc] & ctype_word) != 0;
      }
    if ((*Fecode++ == OP_WORD_BOUNDARY || Fop == OP_UCP_WORD_BOUNDARY)?
         cur_is_word == prev_is_word : cur_is_word != prev_is_word)
      RRETURN(MATCH_NOMATCH);
    break;
    case OP_MARK:
    Fmark = mb->nomatch_mark = Fecode + 2;
    RMATCH(Fecode + PRIV(OP_lengths)[*Fecode] + Fecode[1], RM12);
    if (rrc == MATCH_SKIP_ARG &&
             PRIV(strcmp)(Fecode + 2, mb->verb_skip_ptr) == 0)
      {
      mb->verb_skip_ptr = Feptr;
      RRETURN(MATCH_SKIP);
      }
    RRETURN(rrc);
    case OP_FAIL:
    RRETURN(MATCH_NOMATCH);
    case OP_COMMIT:
    RMATCH(Fecode + PRIV(OP_lengths)[*Fecode], RM13);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    mb->verb_current_recurse = Fcurrent_recurse;
    RRETURN(MATCH_COMMIT);
    case OP_COMMIT_ARG:
    Fmark = mb->nomatch_mark = Fecode + 2;
    RMATCH(Fecode + PRIV(OP_lengths)[*Fecode] + Fecode[1], RM36);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    mb->verb_current_recurse = Fcurrent_recurse;
    RRETURN(MATCH_COMMIT);
    case OP_PRUNE:
    RMATCH(Fecode + PRIV(OP_lengths)[*Fecode], RM14);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    mb->verb_current_recurse = Fcurrent_recurse;
    RRETURN(MATCH_PRUNE);
    case OP_PRUNE_ARG:
    Fmark = mb->nomatch_mark = Fecode + 2;
    RMATCH(Fecode + PRIV(OP_lengths)[*Fecode] + Fecode[1], RM15);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    mb->verb_current_recurse = Fcurrent_recurse;
    RRETURN(MATCH_PRUNE);
    case OP_SKIP:
    RMATCH(Fecode + PRIV(OP_lengths)[*Fecode], RM16);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    mb->verb_skip_ptr = Feptr;
    mb->verb_current_recurse = Fcurrent_recurse;
    RRETURN(MATCH_SKIP);
    case OP_SKIP_ARG:
    mb->skip_arg_count++;
    if (mb->skip_arg_count <= mb->ignore_skip_arg)
      {
      Fecode += PRIV(OP_lengths)[*Fecode] + Fecode[1];
      break;
      }
    RMATCH(Fecode + PRIV(OP_lengths)[*Fecode] + Fecode[1], RM17);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    mb->verb_skip_ptr = Fecode + 2;
    mb->verb_current_recurse = Fcurrent_recurse;
    RRETURN(MATCH_SKIP_ARG);
    case OP_THEN:
    RMATCH(Fecode + PRIV(OP_lengths)[*Fecode], RM18);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    mb->verb_ecode_ptr = Fecode;
    mb->verb_current_recurse = Fcurrent_recurse;
    RRETURN(MATCH_THEN);
    case OP_THEN_ARG:
    Fmark = mb->nomatch_mark = Fecode + 2;
    RMATCH(Fecode + PRIV(OP_lengths)[*Fecode] + Fecode[1], RM19);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    mb->verb_ecode_ptr = Fecode;
    mb->verb_current_recurse = Fcurrent_recurse;
    RRETURN(MATCH_THEN);
    default:
    PCRE2_DEBUG_UNREACHABLE();
    return PCRE2_ERROR_INTERNAL;
    }
  }
PCRE2_DEBUG_UNREACHABLE();
#define LBL(val) case val: goto L_RM##val;
RETURN_SWITCH:
if (Feptr > mb->last_used_ptr) mb->last_used_ptr = Feptr;
if (Frdepth == 0) return rrc;
F = (heapframe *)((char *)F - Fback_frame);
mb->cb->callout_flags |= PCRE2_CALLOUT_BACKTRACK;
#ifdef DEBUG_SHOW_RMATCH
fprintf(stderr, "++ RETURN %d to RM%d\n", rrc, Freturn_id);
#endif
switch (Freturn_id)
  {
  LBL( 1) LBL( 2) LBL( 3) LBL( 4) LBL( 5) LBL( 6) LBL( 7) LBL( 8)
  LBL( 9) LBL(10) LBL(11) LBL(12) LBL(13) LBL(14) LBL(15) LBL(16)
  LBL(17) LBL(18) LBL(19) LBL(20) LBL(21) LBL(22) LBL(23) LBL(24)
  LBL(25) LBL(26) LBL(27) LBL(28) LBL(29) LBL(30) LBL(31) LBL(32)
  LBL(33) LBL(34) LBL(35) LBL(36) LBL(37) LBL(38) LBL(39)
#ifdef SUPPORT_WIDE_CHARS
  LBL(100) LBL(101) LBL(102) LBL(103)
#endif
#ifdef SUPPORT_UNICODE
  LBL(200) LBL(201) LBL(202) LBL(203) LBL(204) LBL(205) LBL(206)
  LBL(207) LBL(208) LBL(209) LBL(210) LBL(211) LBL(212) LBL(213)
  LBL(214) LBL(215) LBL(216) LBL(217) LBL(218) LBL(219) LBL(220)
  LBL(221) LBL(222) LBL(223) LBL(224)
#endif
  default:
  PCRE2_DEBUG_UNREACHABLE();
  return PCRE2_ERROR_INTERNAL;
  }
#undef LBL
#ifdef ERLANG_INTEGRATION
LOOP_COUNT_RETURN:
{
  match_local_variable_store *store = (match_local_variable_store *)F;
  F = (heapframe*)((char*)store - frame_size);
  start_ecode = store->start_ecode;
  top_bracket = store->top_bracket;
  frames_top = store->frames_top;
  assert_accept_frame = store->assert_accept_frame;
  frame_copy_size = store->frame_copy_size;
  branch_end = store->branch_end;
  branch_start = store->branch_start;
  bracode = store->bracode;
  offset = store->offset;
  length = store->length;
  rrc = store->rrc;
  i = store->i;
  fc = store->fc;
  notmatch = store->notmatch;
  samelengths = store->samelengths;
  number = store->number;
  reptype = store->reptype;
  group_frame_type = store->group_frame_type;
  condition = store->condition;
  cur_is_word = store->cur_is_word;
  prev_is_word = store->prev_is_word;
  utf = store->utf;
  ucp = store->ucp;
  proptype = store->proptype;
  rgb = store->rgb;
  EDEBUGF(("LOOP_COUNT_RETURN: %d",F->return_id));
  switch (F->return_id)
    {
#include "pcre2_match_loop_break_cases.gen.h"
     default:
       EDEBUGF(("jump error in pcre match: label %d non-existent\n", F->return_id));
       abort();
     }
}
LOOP_COUNT_BREAK:
  {
    heapframe *newframe = (heapframe*)((char*)F + frame_size);
    match_local_variable_store *store = (match_local_variable_store *) newframe;
    store->start_ecode = start_ecode;
    store->top_bracket = top_bracket;
    store->frames_top = frames_top;
    store->assert_accept_frame = assert_accept_frame;
    store->frame_copy_size = frame_copy_size;
    store->branch_end = branch_end;
    store->branch_start = branch_start;
    store->bracode = bracode;
    store->offset = offset;
    store->length = length;
    store->rrc = rrc;
    store->i = i;
    store->fc = fc;
    store->notmatch = notmatch;
    store->samelengths = samelengths;
    store->number = number;
    store->reptype = reptype;
    store->group_frame_type = group_frame_type;
    store->condition = condition;
    store->cur_is_word = cur_is_word;
    store->prev_is_word = prev_is_word;
    store->utf = utf;
    store->ucp = ucp;
    store->proptype = proptype;
    store->rgb = rgb;
    mb->state_save = newframe;
    EDEBUGF(("Break loop!"));
    return PCRE2_ERROR_LOOP_LIMIT;
  }
#endif
}
#ifdef ERLANG_INTEGRATION
typedef struct {
    int Xrc;
    const uint8_t *Xstart_bits;
    const pcre2_real_code *Xre;
    uint32_t Xoriginal_options;
    BOOL Xanchored;
    BOOL Xfirstline;
    BOOL Xhas_first_cu;
    BOOL Xhas_req_cu;
    BOOL Xstartline;
    PCRE2_SPTR Xmemchr_found_first_cu;
    PCRE2_SPTR Xmemchr_found_first_cu2;
    PCRE2_UCHAR Xfirst_cu;
    PCRE2_UCHAR Xfirst_cu2;
    PCRE2_UCHAR Xreq_cu;
    PCRE2_UCHAR Xreq_cu2;
    PCRE2_SPTR Xoriginal_subject;
    PCRE2_SPTR Xbumpalong_limit;
    PCRE2_SPTR Xend_subject;
    PCRE2_SPTR Xtrue_end_subject;
    PCRE2_SPTR Xstart_match;
    PCRE2_SPTR Xreq_cu_ptr;
    PCRE2_SPTR Xstart_partial;
    PCRE2_SPTR Xmatch_partial;
    BOOL Xutf;
    BOOL Xucp;
    BOOL Xallow_invalid;
    uint32_t Xfragment_options;
    PCRE2_SIZE Xframe_size;
    PCRE2_SIZE Xheapframes_size;
    pcre2_callout_block Xcb;
    match_block Xactual_match_block;
    match_block *Xmb;
    PCRE2_SPTR Xp;
    PCRE2_SPTR Xpp;
    PCRE2_SPTR Xpp1;
    PCRE2_SPTR Xpp2;
    PCRE2_SIZE Xsearchlength;
    struct PRIV(valid_utf_ystate) valid_utf_ystate;
    const char* memchr_yield_ptr;
    uint32_t outer_yield_line;
#define ERLANG_PCRE2_MATCH_YIELD_MATCH 0
#define ERLANG_PCRE2_MATCH_YIELD_VALID_UTF 1
    PCRE2_SPTR Xsubject;
    PCRE2_SIZE Xlength;
    PCRE2_SIZE Xstart_offset;
    uint32_t Xoptions;
    pcre2_match_data *Xmatch_data;
    pcre2_match_context *Xmcontext;
} PcreExecContext;
#define BYTES_TO_LOOPS(B) ((B) >> 3)
#define LOOPS_TO_BYTES(L) ((L) << 3)
static void *memchr_erlang(const void *start, int c, size_t n,
                           const char** restrict yield_pos_p,
                           int32_t* restrict loops_left_p)
{
    const char* from;
    char* found;
    size_t left;
    if (*loops_left_p <= 0) {
        *yield_pos_p = start;
        return NULL;
    }
    if (*yield_pos_p) {
        from = *yield_pos_p;
        *yield_pos_p = NULL;
        left = ((const char*)start + n) - from;
    }
    else {
        from = start;
        left = n;
    }
    if ((size_t)*loops_left_p > BYTES_TO_LOOPS(left)) {
        found = memchr(from, c, left);
        *loops_left_p -= BYTES_TO_LOOPS(found ? (found - from) : left);
        return found;
    }
    else {
        const size_t max_bytes = LOOPS_TO_BYTES(*loops_left_p);
        found = memchr(from, c, max_bytes);
        if (found) {
            *loops_left_p -= BYTES_TO_LOOPS(found - from);
            return found;
        }
        *yield_pos_p = from + max_bytes;
        *loops_left_p = 0;
        return NULL;
    }
}
#define MEMCHR_ERLANG(RET, START, VAL, LEN)  \
do { \
  if (LEN == 0) { \
    RET = NULL; \
  } \
  else { \
    void* return_val_; \
    NAME_CAT(ERLANG_PCRE2_MATCH_YIELD_LINE_,__LINE__): \
    DBG_COST_CHK_VISIT(); \
    return_val_ = memchr_erlang(START, VAL, LEN, \
                                &exec_context->memchr_yield_ptr, \
                                &match_data->loops_left); \
    if (exec_context->memchr_yield_ptr) { \
        DBG_COST_CHK_YIELD(); \
        exec_context->outer_yield_line = __LINE__; \
        goto erlang_swapout; \
    } \
    RET = return_val_; \
  } \
} while (0)
#else
# define MEMCHR_ERLANG(RET, START, VAL, LEN)  RET = memchr(START, VAL, LEN)
#endif
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_match(const pcre2_code *code, PCRE2_SPTR subject, PCRE2_SIZE length,
  PCRE2_SIZE start_offset, uint32_t options, pcre2_match_data *match_data,
  pcre2_match_context *mcontext)
{
#ifndef ERLANG_INTEGRATION
int rc;
const uint8_t *start_bits = NULL;
const pcre2_real_code *re = (const pcre2_real_code *)code;
uint32_t original_options = options;
BOOL anchored;
BOOL firstline;
BOOL has_first_cu = FALSE;
BOOL has_req_cu = FALSE;
BOOL startline;
#if PCRE2_CODE_UNIT_WIDTH == 8
PCRE2_SPTR memchr_found_first_cu;
PCRE2_SPTR memchr_found_first_cu2;
#endif
PCRE2_UCHAR first_cu = 0;
PCRE2_UCHAR first_cu2 = 0;
PCRE2_UCHAR req_cu = 0;
PCRE2_UCHAR req_cu2 = 0;
PCRE2_UCHAR null_str[1] = { 0xcd };
PCRE2_SPTR original_subject = subject;
PCRE2_SPTR bumpalong_limit;
PCRE2_SPTR end_subject;
PCRE2_SPTR true_end_subject;
PCRE2_SPTR start_match;
PCRE2_SPTR req_cu_ptr;
PCRE2_SPTR start_partial;
PCRE2_SPTR match_partial;
#ifdef SUPPORT_JIT
BOOL use_jit;
#endif
BOOL utf = FALSE;
#ifdef SUPPORT_UNICODE
BOOL ucp = FALSE;
BOOL allow_invalid;
uint32_t fragment_options = 0;
#ifdef SUPPORT_JIT
BOOL jit_checked_utf = FALSE;
#endif
#endif
PCRE2_SIZE frame_size;
PCRE2_SIZE heapframes_size;
pcre2_callout_block cb;
match_block actual_match_block;
match_block *mb = &actual_match_block;
#else
#define SWAPIN() do {				                    \
  rc = exec_context->Xrc;			                    \
  start_bits = exec_context->Xstart_bits;	      \
  re = exec_context->Xre;                        \
  original_options = exec_context->Xoriginal_options;	\
  anchored = exec_context->Xanchored;             \
  firstline = exec_context->Xfirstline;           \
  has_first_cu = exec_context->Xhas_first_cu;     \
  has_req_cu = exec_context->Xhas_req_cu;         \
  startline = exec_context->Xstartline;           \
  memchr_found_first_cu = exec_context->Xmemchr_found_first_cu;	\
  memchr_found_first_cu2 = exec_context->Xmemchr_found_first_cu2;	\
  first_cu = exec_context->Xfirst_cu;   	      \
  first_cu2 = exec_context->Xfirst_cu2;   	      \
  req_cu = exec_context->Xreq_cu;               \
  req_cu2 = exec_context->Xreq_cu2;               \
  original_subject = exec_context->Xoriginal_subject; \
  bumpalong_limit = exec_context->Xbumpalong_limit;	\
  end_subject = exec_context->Xend_subject;	    \
  true_end_subject = exec_context->Xtrue_end_subject;	\
  start_match = exec_context->Xstart_match;	    \
  req_cu_ptr = exec_context->Xreq_cu_ptr;	      \
  start_partial = exec_context->Xstart_partial;	\
  match_partial = exec_context->Xmatch_partial;	\
  utf = exec_context->Xutf;		                  \
  ucp = exec_context->Xucp;			                \
  allow_invalid = exec_context->Xallow_invalid;	\
  fragment_options = exec_context->Xfragment_options;	\
  frame_size = exec_context->Xframe_size;	\
  heapframes_size = exec_context->Xheapframes_size;	\
  cb = exec_context->Xcb;			                  \
  mb = exec_context->Xmb;			                  \
  p = exec_context->Xp;			                          \
  pp = exec_context->Xpp;			                  \
  pp1 = exec_context->Xpp1;                                       \
  pp2 = exec_context->Xpp2;                                       \
  searchlength = exec_context->Xsearchlength;                     \
                                \
  subject = exec_context->Xsubject;             \
  length = exec_context->Xlength;               \
  start_offset = exec_context->Xstart_offset;   \
  options = exec_context->Xoptions;             \
  match_data = exec_context->Xmatch_data;       \
  mcontext = exec_context->Xmcontext;           \
                           \
  mb->cb = &cb;                                 \
} while (0)
PcreExecContext *exec_context;
PcreExecContext internal_context;
int rc;
const uint8_t *start_bits;
const pcre2_real_code *re;
uint32_t original_options;
BOOL anchored;
BOOL firstline;
BOOL has_first_cu;
BOOL has_req_cu;
BOOL startline;
PCRE2_SPTR memchr_found_first_cu;
PCRE2_SPTR memchr_found_first_cu2;
PCRE2_UCHAR first_cu;
PCRE2_UCHAR first_cu2;
PCRE2_UCHAR req_cu;
PCRE2_UCHAR req_cu2;
PCRE2_UCHAR null_str[1] = { 0xcd };
PCRE2_SPTR original_subject;
PCRE2_SPTR bumpalong_limit;
PCRE2_SPTR end_subject;
PCRE2_SPTR true_end_subject;
PCRE2_SPTR start_match;
PCRE2_SPTR req_cu_ptr;
PCRE2_SPTR start_partial;
PCRE2_SPTR match_partial;
BOOL utf;
BOOL ucp;
BOOL allow_invalid;
uint32_t fragment_options;
PCRE2_SIZE frame_size;
PCRE2_SIZE heapframes_size;
pcre2_callout_block cb;
match_block *mb;
PCRE2_SPTR p;
PCRE2_SPTR pp;
PCRE2_SPTR pp1;
PCRE2_SPTR pp2;
PCRE2_SIZE searchlength;
if (*(match_data->restart_data) != NULL) {
   exec_context = (PcreExecContext *) *(match_data->restart_data);
   SWAPIN();
   switch (exec_context->outer_yield_line) {
   case ERLANG_PCRE2_MATCH_YIELD_MATCH:
       goto RESTART_INTERRUPTED;
   case ERLANG_PCRE2_MATCH_YIELD_VALID_UTF:
       goto restart_valid_utf;
#include "pcre2_match_memchr_break_cases.gen.h"
   default:
       EDEBUGF(("jump error in pcre2_match: label %u non-existent\n", exec_context->outer_yield_line));
       abort();
   }
 } else {
   exec_context = &internal_context;
   *(match_data->restart_data) = NULL;
   exec_context->memchr_yield_ptr = NULL;
   start_bits = NULL;
   re = (const pcre2_real_code *)code;
   original_options = options;
   has_first_cu = FALSE;
   has_req_cu = FALSE;
   first_cu = 0;
   first_cu2 = 0;
   req_cu = 0;
   req_cu2 = 0;
   original_subject = subject;
   utf = FALSE;
   ucp = FALSE;
   fragment_options = 0;
   mb = &(exec_context->Xactual_match_block);
   start_partial = NULL;
   match_partial = NULL;
   mb->state_save = NULL;
}
#endif
if (subject == NULL && length == 0) subject = null_str;
if (match_data == NULL) return PCRE2_ERROR_NULL;
if (code == NULL || subject == NULL)
  return match_data->rc = PCRE2_ERROR_NULL;
if ((options & ~PUBLIC_MATCH_OPTIONS) != 0)
  return match_data->rc = PCRE2_ERROR_BADOPTION;
start_match = subject + start_offset;
req_cu_ptr = start_match - 1;
if (length == PCRE2_ZERO_TERMINATED)
  {
  length = PRIV(strlen)(subject);
  }
true_end_subject = end_subject = subject + length;
if (start_offset > length) return match_data->rc = PCRE2_ERROR_BADOFFSET;
if (re->magic_number != MAGIC_NUMBER)
  return match_data->rc = PCRE2_ERROR_BADMAGIC;
if ((re->flags & PCRE2_MODE_MASK) != PCRE2_CODE_UNIT_WIDTH/8)
  return match_data->rc = PCRE2_ERROR_BADMODE;
#define FF (PCRE2_NOTEMPTY_SET|PCRE2_NE_ATST_SET)
#define OO (PCRE2_NOTEMPTY|PCRE2_NOTEMPTY_ATSTART)
options |= (re->flags & FF) / ((FF & (~FF+1)) / (OO & (~OO+1)));
#undef FF
#undef OO
#ifdef SUPPORT_JIT
use_jit = (re->executable_jit != NULL &&
          (options & ~PUBLIC_JIT_MATCH_OPTIONS) == 0);
#endif
#ifdef SUPPORT_UNICODE
utf = (re->overall_options & PCRE2_UTF) != 0;
allow_invalid = (re->overall_options & PCRE2_MATCH_INVALID_UTF) != 0;
ucp = (re->overall_options & PCRE2_UCP) != 0;
#endif
mb->partial = ((options & PCRE2_PARTIAL_HARD) != 0)? 2 :
              ((options & PCRE2_PARTIAL_SOFT) != 0)? 1 : 0;
if (mb->partial != 0 &&
   ((re->overall_options | options) & PCRE2_ENDANCHORED) != 0)
  return match_data->rc = PCRE2_ERROR_BADOPTION;
if (mcontext != NULL && mcontext->offset_limit != PCRE2_UNSET &&
     (re->overall_options & PCRE2_USE_OFFSET_LIMIT) == 0)
  return match_data->rc = PCRE2_ERROR_BADOFFSETLIMIT;
if ((match_data->flags & PCRE2_MD_COPIED_SUBJECT) != 0)
  {
  match_data->memctl.free((void *)match_data->subject,
    match_data->memctl.memory_data);
  match_data->flags &= ~PCRE2_MD_COPIED_SUBJECT;
  }
match_data->subject = NULL;
match_data->startchar = 0;
#ifdef SUPPORT_JIT
if (use_jit)
  {
#ifdef SUPPORT_UNICODE
  if (utf && (options & PCRE2_NO_UTF_CHECK) == 0 && !allow_invalid)
    {
#if PCRE2_CODE_UNIT_WIDTH != 32
    if (start_match < end_subject && NOT_FIRSTCU(*start_match))
      {
      if (start_offset > 0) return match_data->rc = PCRE2_ERROR_BADUTFOFFSET;
#if PCRE2_CODE_UNIT_WIDTH == 8
      return match_data->rc = PCRE2_ERROR_UTF8_ERR20;
#else
      return match_data->rc = PCRE2_ERROR_UTF16_ERR3;
#endif
      }
#endif
#if PCRE2_CODE_UNIT_WIDTH != 32
    for (unsigned int i = re->max_lookbehind; i > 0 && start_match > subject; i--)
      {
      start_match--;
      while (start_match > subject &&
#if PCRE2_CODE_UNIT_WIDTH == 8
      (*start_match & 0xc0) == 0x80)
#else
      (*start_match & 0xfc00) == 0xdc00)
#endif
        start_match--;
      }
#else
    if (start_offset >= re->max_lookbehind)
      start_match -= re->max_lookbehind;
    else
      start_match = subject;
#endif
    rc = PRIV(valid_utf)(start_match,
      length - (start_match - subject), &(match_data->startchar));
    if (rc != 0)
      {
      match_data->startchar += start_match - subject;
      return match_data->rc = rc;
      }
    jit_checked_utf = TRUE;
    }
#endif
  rc = pcre2_jit_match(code, subject, length, start_offset, options,
    match_data, mcontext);
  if (rc != PCRE2_ERROR_JIT_BADOPTION)
    {
    match_data->options = original_options;
    if (rc >= 0 && (options & PCRE2_COPY_MATCHED_SUBJECT) != 0)
      {
      if (length != 0)
        {
        match_data->subject = match_data->memctl.malloc(CU2BYTES(length),
          match_data->memctl.memory_data);
        if (match_data->subject == NULL)
          return match_data->rc = PCRE2_ERROR_NOMEMORY;
        memcpy((void *)match_data->subject, subject, CU2BYTES(length));
        }
      else
        match_data->subject = NULL;
      match_data->flags |= PCRE2_MD_COPIED_SUBJECT;
      }
    else
      {
      if (match_data->subject != NULL) match_data->subject = original_subject;
      }
    return rc;
    }
  }
#endif
mb->check_subject = subject;
#ifdef SUPPORT_UNICODE
if (utf &&
#ifdef SUPPORT_JIT
    !jit_checked_utf &&
#endif
    ((options & PCRE2_NO_UTF_CHECK) == 0 || allow_invalid))
  {
#if PCRE2_CODE_UNIT_WIDTH != 32
  BOOL skipped_bad_start = FALSE;
#endif
#if PCRE2_CODE_UNIT_WIDTH != 32
  if (allow_invalid)
    {
    while (start_match < end_subject && NOT_FIRSTCU(*start_match))
      {
      start_match++;
      skipped_bad_start = TRUE;
      }
    }
  else if (start_match < end_subject && NOT_FIRSTCU(*start_match))
    {
    if (start_offset > 0) return match_data->rc = PCRE2_ERROR_BADUTFOFFSET;
#if PCRE2_CODE_UNIT_WIDTH == 8
    return match_data->rc = PCRE2_ERROR_UTF8_ERR20;
#else
    return match_data->rc = PCRE2_ERROR_UTF16_ERR3;
#endif
    }
#endif
  mb->check_subject = start_match;
#if PCRE2_CODE_UNIT_WIDTH != 32
  if (!skipped_bad_start)
    {
    unsigned int i;
    for (i = re->max_lookbehind; i > 0 && mb->check_subject > subject; i--)
      {
      mb->check_subject--;
      while (mb->check_subject > subject &&
#if PCRE2_CODE_UNIT_WIDTH == 8
      (*mb->check_subject & 0xc0) == 0x80)
#else
      (*mb->check_subject & 0xfc00) == 0xdc00)
#endif
        mb->check_subject--;
      }
    }
#else
  if (start_offset >= re->max_lookbehind)
    mb->check_subject -= re->max_lookbehind;
  else
    mb->check_subject = subject;
#endif
  for (;;)
    {
#ifndef ERLANG_INTEGRATION
    rc = PRIV(valid_utf)(mb->check_subject,
      length - (mb->check_subject - subject), &(match_data->startchar));
#else
    exec_context->valid_utf_ystate.yielded = 0;
restart_valid_utf:
    exec_context->valid_utf_ystate.loops_left = match_data->loops_left;
    rc = PRIV(yielding_valid_utf)(mb->check_subject,
                                  length - (mb->check_subject - subject),
                                  &(match_data->startchar),
                                  &(exec_context->valid_utf_ystate));
    match_data->loops_left = exec_context->valid_utf_ystate.loops_left;
#endif
    if (rc == 0) break;
#if defined(ERLANG_INTEGRATION)
    if (rc == PCRE2_ERROR_UTF8_YIELD) {
        DBG_FAKE_COST_CHK();
        exec_context->outer_yield_line = ERLANG_PCRE2_MATCH_YIELD_VALID_UTF;
        goto erlang_swapout;
    }
#endif
    match_data->startchar += mb->check_subject - subject;
    if (!allow_invalid || rc > 0) return match_data->rc = rc;
    end_subject = subject + match_data->startchar;
    if (end_subject < start_match)
      {
      mb->check_subject = end_subject + 1;
#if PCRE2_CODE_UNIT_WIDTH != 32
      while (mb->check_subject < start_match && NOT_FIRSTCU(*mb->check_subject))
        mb->check_subject++;
#endif
      end_subject = true_end_subject;
      }
    else
      {
      fragment_options = PCRE2_NOTEOL;
      break;
      }
    }
  }
#endif
if (mcontext == NULL)
  {
  mcontext = (pcre2_match_context *)(&PRIV(default_match_context));
  mb->memctl = re->memctl;
  }
else mb->memctl = mcontext->memctl;
anchored = ((re->overall_options | options) & PCRE2_ANCHORED) != 0;
firstline = !anchored && (re->overall_options & PCRE2_FIRSTLINE) != 0;
startline = (re->flags & PCRE2_STARTLINE) != 0;
bumpalong_limit = (mcontext->offset_limit == PCRE2_UNSET)?
  true_end_subject : subject + mcontext->offset_limit;
mb->cb = &cb;
cb.version = 2;
cb.subject = subject;
cb.subject_length = (PCRE2_SIZE)(end_subject - subject);
cb.callout_flags = 0;
mb->callout = mcontext->callout;
mb->callout_data = mcontext->callout_data;
mb->start_subject = subject;
mb->start_offset = start_offset;
mb->end_subject = end_subject;
mb->true_end_subject = true_end_subject;
mb->hasthen = (re->flags & PCRE2_HASTHEN) != 0;
mb->hasbsk = (re->flags & PCRE2_HASBSK) != 0;
mb->allowemptypartial = (re->max_lookbehind > 0) ||
    (re->flags & PCRE2_MATCH_EMPTY) != 0;
mb->allowlookaroundbsk =
  (re->extra_options & PCRE2_EXTRA_ALLOW_LOOKAROUND_BSK) != 0;
mb->poptions = re->overall_options;
mb->ignore_skip_arg = 0;
mb->mark = mb->nomatch_mark = NULL;
mb->name_table = (PCRE2_SPTR)((const uint8_t *)re + sizeof(pcre2_real_code));
mb->name_count = re->name_count;
mb->name_entry_size = re->name_entry_size;
mb->start_code = (PCRE2_SPTR)((const uint8_t *)re + re->code_start);
mb->bsr_convention = re->bsr_convention;
mb->nltype = NLTYPE_FIXED;
switch(re->newline_convention)
  {
  case PCRE2_NEWLINE_CR:
  mb->nllen = 1;
  mb->nl[0] = CHAR_CR;
  break;
  case PCRE2_NEWLINE_LF:
  mb->nllen = 1;
  mb->nl[0] = CHAR_NL;
  break;
  case PCRE2_NEWLINE_NUL:
  mb->nllen = 1;
  mb->nl[0] = CHAR_NUL;
  break;
  case PCRE2_NEWLINE_CRLF:
  mb->nllen = 2;
  mb->nl[0] = CHAR_CR;
  mb->nl[1] = CHAR_NL;
  break;
  case PCRE2_NEWLINE_ANY:
  mb->nltype = NLTYPE_ANY;
  break;
  case PCRE2_NEWLINE_ANYCRLF:
  mb->nltype = NLTYPE_ANYCRLF;
  break;
  default:
  PCRE2_DEBUG_UNREACHABLE();
  return match_data->rc = PCRE2_ERROR_INTERNAL;
  }
frame_size = (offsetof(heapframe, ovector) +
  re->top_bracket * 2 * sizeof(PCRE2_SIZE) + HEAPFRAME_ALIGNMENT - 1) &
  ~(HEAPFRAME_ALIGNMENT - 1);
mb->heap_limit = ((mcontext->heap_limit < re->limit_heap)?
  mcontext->heap_limit : re->limit_heap);
mb->match_limit = (mcontext->match_limit < re->limit_match)?
  mcontext->match_limit : re->limit_match;
mb->match_limit_depth = (mcontext->depth_limit < re->limit_depth)?
  mcontext->depth_limit : re->limit_depth;
heapframes_size = frame_size * 10;
if (heapframes_size < START_FRAMES_SIZE) heapframes_size = START_FRAMES_SIZE;
if (heapframes_size / 1024 > mb->heap_limit)
  {
  PCRE2_SIZE max_size = 1024 * mb->heap_limit;
  if (max_size < frame_size) return match_data->rc = PCRE2_ERROR_HEAPLIMIT;
  heapframes_size = max_size;
  }
if (match_data->heapframes_size < heapframes_size)
  {
  match_data->memctl.free(match_data->heapframes,
    match_data->memctl.memory_data);
  match_data->heapframes = match_data->memctl.malloc(heapframes_size + frame_size,
    match_data->memctl.memory_data);
  if (match_data->heapframes == NULL)
    {
    match_data->heapframes_size = 0;
    return match_data->rc = PCRE2_ERROR_NOMEMORY;
    }
  match_data->heapframes_size = heapframes_size;
  }
memset((char *)(match_data->heapframes) + offsetof(heapframe, ovector), 0xff,
  frame_size - offsetof(heapframe, ovector));
mb->lcc = re->tables + lcc_offset;
mb->fcc = re->tables + fcc_offset;
mb->ctypes = re->tables + ctypes_offset;
if ((re->flags & PCRE2_FIRSTSET) != 0)
  {
  has_first_cu = TRUE;
  first_cu = first_cu2 = (PCRE2_UCHAR)(re->first_codeunit);
  if ((re->flags & PCRE2_FIRSTCASELESS) != 0)
    {
    first_cu2 = TABLE_GET(first_cu, mb->fcc, first_cu);
#ifdef SUPPORT_UNICODE
#if PCRE2_CODE_UNIT_WIDTH == 8
    if (first_cu > 127 && ucp && !utf) first_cu2 = UCD_OTHERCASE(first_cu);
#else
    if (first_cu > 127 && (utf || ucp)) first_cu2 = UCD_OTHERCASE(first_cu);
#endif
#endif
    }
  }
else
  if (!startline && (re->flags & PCRE2_FIRSTMAPSET) != 0)
    start_bits = re->start_bitmap;
if ((re->flags & PCRE2_LASTSET) != 0)
  {
  has_req_cu = TRUE;
  req_cu = req_cu2 = (PCRE2_UCHAR)(re->last_codeunit);
  if ((re->flags & PCRE2_LASTCASELESS) != 0)
    {
    req_cu2 = TABLE_GET(req_cu, mb->fcc, req_cu);
#ifdef SUPPORT_UNICODE
#if PCRE2_CODE_UNIT_WIDTH == 8
    if (req_cu > 127 && ucp && !utf) req_cu2 = UCD_OTHERCASE(req_cu);
#else
    if (req_cu > 127 && (utf || ucp)) req_cu2 = UCD_OTHERCASE(req_cu);
#endif
#endif
    }
  }
#ifdef SUPPORT_UNICODE
FRAGMENT_RESTART:
#endif
start_partial = match_partial = NULL;
mb->hitend = FALSE;
#if PCRE2_CODE_UNIT_WIDTH == 8
memchr_found_first_cu = NULL;
memchr_found_first_cu2 = NULL;
#endif
for(;;)
  {
  PCRE2_SPTR new_start_match;
  if ((re->optimization_flags & PCRE2_OPTIM_START_OPTIMIZE) != 0)
    {
    if (firstline)
      {
      PCRE2_SPTR t = start_match;
#ifdef SUPPORT_UNICODE
      if (utf)
        {
        while (t < end_subject && !IS_NEWLINE(t))
          {
          t++;
          ACROSSCHAR(t < end_subject, t, t++);
          }
        }
      else
#endif
      while (t < end_subject && !IS_NEWLINE(t)) t++;
      end_subject = t;
      }
    if (anchored)
      {
      if (has_first_cu || start_bits != NULL)
        {
        BOOL ok = start_match < end_subject;
        if (ok)
          {
          PCRE2_UCHAR c = UCHAR21TEST(start_match);
          ok = has_first_cu && (c == first_cu || c == first_cu2);
          if (!ok && start_bits != NULL)
            {
#if PCRE2_CODE_UNIT_WIDTH != 8
            if (c > 255) c = 255;
#endif
            ok = (start_bits[c/8] & (1u << (c&7))) != 0;
            }
          }
        if (!ok)
          {
          rc = MATCH_NOMATCH;
          break;
          }
        }
      }
    else
      {
      if (has_first_cu)
        {
        if (first_cu != first_cu2)
          {
#if PCRE2_CODE_UNIT_WIDTH != 8
          PCRE2_UCHAR smc;
          while (start_match < end_subject &&
                (smc = UCHAR21TEST(start_match)) != first_cu &&
                 smc != first_cu2)
            start_match++;
#else
#ifdef ERLANG_INTEGRATION
          pp1 = NULL;
          pp2 = NULL;
          searchlength = end_subject - start_match;
#else
          PCRE2_SPTR pp1 = NULL;
          PCRE2_SPTR pp2 = NULL;
          PCRE2_SIZE searchlength = end_subject - start_match;
#endif
          if (memchr_found_first_cu == NULL ||
              start_match > memchr_found_first_cu)
            {
            MEMCHR_ERLANG(pp1, start_match, first_cu, searchlength);
            memchr_found_first_cu = (pp1 == NULL)? end_subject : pp1;
            }
          else pp1 = (memchr_found_first_cu == end_subject)? NULL :
            memchr_found_first_cu;
          if (memchr_found_first_cu2 == NULL ||
              start_match > memchr_found_first_cu2)
            {
            MEMCHR_ERLANG(pp2, start_match, first_cu2, searchlength);
            memchr_found_first_cu2 = (pp2 == NULL)? end_subject : pp2;
            }
          else pp2 = (memchr_found_first_cu2 == end_subject)? NULL :
            memchr_found_first_cu2;
          if (pp1 == NULL)
            start_match = (pp2 == NULL)? end_subject : pp2;
          else
            start_match = (pp2 == NULL || pp1 < pp2)? pp1 : pp2;
#endif
          }
        else
          {
#if PCRE2_CODE_UNIT_WIDTH != 8
          while (start_match < end_subject && UCHAR21TEST(start_match) !=
                 first_cu)
            start_match++;
#else
          MEMCHR_ERLANG(start_match, start_match, first_cu, end_subject - start_match);
          if (start_match == NULL) start_match = end_subject;
#endif
          }
        if (mb->partial == 0 && start_match >= mb->end_subject)
          {
          rc = MATCH_NOMATCH;
          break;
          }
        }
      else if (startline)
        {
        if (start_match > mb->start_subject + start_offset)
          {
#ifdef SUPPORT_UNICODE
          if (utf)
            {
            while (start_match < end_subject && !WAS_NEWLINE(start_match))
              {
              start_match++;
              ACROSSCHAR(start_match < end_subject, start_match, start_match++);
              }
            }
          else
#endif
          while (start_match < end_subject && !WAS_NEWLINE(start_match))
            start_match++;
          if (start_match[-1] == CHAR_CR &&
               (mb->nltype == NLTYPE_ANY || mb->nltype == NLTYPE_ANYCRLF) &&
               start_match < end_subject &&
               UCHAR21TEST(start_match) == CHAR_NL)
            start_match++;
          }
        }
      else if (start_bits != NULL)
        {
        while (start_match < end_subject)
          {
          uint32_t c = UCHAR21TEST(start_match);
#if PCRE2_CODE_UNIT_WIDTH != 8
          if (c > 255) c = 255;
#endif
          if ((start_bits[c/8] & (1u << (c&7))) != 0) break;
          start_match++;
          }
        if (mb->partial == 0 && start_match >= mb->end_subject)
          {
          rc = MATCH_NOMATCH;
          break;
          }
        }
      }
    end_subject = mb->end_subject;
    if (mb->partial == 0)
      {
#ifndef ERLANG_INTEGRATION
      PCRE2_SPTR p;
#endif
      if (end_subject - start_match < re->minlength)
        {
        rc = MATCH_NOMATCH;
        break;
        }
      p = start_match + (has_first_cu? 1:0);
      if (has_req_cu && p > req_cu_ptr)
        {
        PCRE2_SIZE check_length = end_subject - start_match;
        if (check_length < REQ_CU_MAX ||
              (!anchored && check_length < REQ_CU_MAX * 1000))
          {
          if (req_cu != req_cu2)
            {
#if PCRE2_CODE_UNIT_WIDTH != 8
            while (p < end_subject)
              {
              uint32_t pp = UCHAR21INCTEST(p);
              if (pp == req_cu || pp == req_cu2) { p--; break; }
              }
#else
# ifdef ERLANG_INTEGRATION
            pp = p;
# else
            PCRE2_SPTR pp = p;
# endif
            MEMCHR_ERLANG(p, pp, req_cu, end_subject - pp);
            if (p == NULL)
              {
              MEMCHR_ERLANG(p, pp, req_cu2, end_subject - pp);
              if (p == NULL) p = end_subject;
              }
#endif
            }
          else
            {
#if PCRE2_CODE_UNIT_WIDTH != 8
            while (p < end_subject)
              {
              if (UCHAR21INCTEST(p) == req_cu) { p--; break; }
              }
#else
            MEMCHR_ERLANG(p, p, req_cu, end_subject - p);
            if (p == NULL) p = end_subject;
#endif
            }
          if (p >= end_subject)
            {
            rc = MATCH_NOMATCH;
            break;
            }
          req_cu_ptr = p;
          }
        }
      }
    }
  if (start_match > bumpalong_limit)
    {
    rc = MATCH_NOMATCH;
    break;
    }
  cb.start_match = (PCRE2_SIZE)(start_match - subject);
  cb.callout_flags |= PCRE2_CALLOUT_STARTMATCH;
  mb->start_used_ptr = start_match;
  mb->last_used_ptr = start_match;
#ifdef SUPPORT_UNICODE
  mb->moptions = options | fragment_options;
#else
  mb->moptions = options;
#endif
  mb->match_call_count = 0;
  mb->end_offset_top = 0;
  mb->skip_arg_count = 0;
#ifdef DEBUG_SHOW_OPS
  fprintf(stderr, "++ Calling match()\n");
#endif
  rc = match(start_match, mb->start_code, re->top_bracket, frame_size,
    match_data, mb);
#ifdef DEBUG_SHOW_OPS
  fprintf(stderr, "++ match() returned %d\n\n", rc);
#endif
#ifdef ERLANG_INTEGRATION
  while(rc == PCRE2_ERROR_LOOP_LIMIT)
    {
    EDEBUGF(("Loop limit break detected"));
    exec_context->outer_yield_line = ERLANG_PCRE2_MATCH_YIELD_MATCH;
  erlang_swapout:
    if (exec_context == &internal_context)
      {
      exec_context = (PcreExecContext *)
        (match_data->memctl.malloc(sizeof(PcreExecContext),
                                   match_data->memctl.memory_data));
      *(match_data->restart_data) = (void *) exec_context;
      *exec_context = internal_context;
      }
    #if ERTS_AT_LEAST_GCC_VSN__(4, 7, 2)
    #  pragma GCC diagnostic push
    #  pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    #endif
    exec_context->Xrc = rc;
    exec_context->Xstart_bits = start_bits;
    exec_context->Xre = re;
    exec_context->Xoriginal_options = original_options;
    exec_context->Xanchored = anchored;
    exec_context->Xfirstline = firstline;
    exec_context->Xhas_first_cu = has_first_cu;
    exec_context->Xhas_req_cu = has_req_cu;
    exec_context->Xstartline = startline;
    exec_context->Xmemchr_found_first_cu = memchr_found_first_cu;
    exec_context->Xmemchr_found_first_cu2 = memchr_found_first_cu2;
    exec_context->Xfirst_cu = first_cu;
    exec_context->Xfirst_cu2 = first_cu2;
    exec_context->Xreq_cu = req_cu;
    exec_context->Xreq_cu2 = req_cu2;
    exec_context->Xoriginal_subject = original_subject;
    exec_context->Xbumpalong_limit = bumpalong_limit;
    exec_context->Xend_subject = end_subject;
    exec_context->Xtrue_end_subject = true_end_subject;
    exec_context->Xstart_match = start_match;
    exec_context->Xreq_cu_ptr = req_cu_ptr;
    exec_context->Xstart_partial = start_partial;
    exec_context->Xmatch_partial = match_partial;
    exec_context->Xutf = utf;
    exec_context->Xucp = ucp;
    exec_context->Xallow_invalid = allow_invalid;
    exec_context->Xfragment_options = fragment_options;
    exec_context->Xframe_size = frame_size;
    exec_context->Xheapframes_size = heapframes_size;
    exec_context->Xcb = cb;
    exec_context->Xp = p;
    exec_context->Xpp = pp;
    exec_context->Xpp1 = pp1;
    exec_context->Xpp2 = pp2;
    exec_context->Xsearchlength = searchlength;
    #if ERTS_AT_LEAST_GCC_VSN__(4, 7, 2)
    #  pragma GCC diagnostic pop
    #endif
    exec_context->Xsubject = subject;
    exec_context->Xlength = length;
    exec_context->Xstart_offset = start_offset;
    exec_context->Xoptions = options;
    exec_context->Xmatch_data = match_data;
    exec_context->Xmcontext = mcontext;
    exec_context->Xmb = &(exec_context->Xactual_match_block);
    exec_context->Xmb->cb = &(exec_context->Xcb);
    return PCRE2_ERROR_LOOP_LIMIT;
  RESTART_INTERRUPTED:
      rc = match(NULL,NULL,0,frame_size,match_data,mb);
    }
  mb->state_save = NULL;
#endif
  if (mb->hitend && start_partial == NULL)
    {
    start_partial = mb->start_used_ptr;
    match_partial = start_match;
    }
  switch(rc)
    {
    case MATCH_SKIP_ARG:
    new_start_match = start_match;
    mb->ignore_skip_arg = mb->skip_arg_count;
    break;
    case MATCH_SKIP:
    if (mb->verb_skip_ptr > start_match)
      {
      new_start_match = mb->verb_skip_ptr;
      break;
      }
    PCRE2_FALLTHROUGH
    case MATCH_NOMATCH:
    case MATCH_PRUNE:
    case MATCH_THEN:
    mb->ignore_skip_arg = 0;
    new_start_match = start_match + 1;
#ifdef SUPPORT_UNICODE
    if (utf)
      ACROSSCHAR(new_start_match < end_subject, new_start_match,
        new_start_match++);
#endif
    break;
    case MATCH_COMMIT:
    rc = MATCH_NOMATCH;
    goto ENDLOOP;
    default:
    goto ENDLOOP;
    }
  rc = MATCH_NOMATCH;
  if (firstline && IS_NEWLINE(start_match)) break;
  start_match = new_start_match;
  if (anchored || start_match > end_subject) break;
  if (start_match > subject + start_offset &&
      start_match[-1] == CHAR_CR &&
      start_match < end_subject &&
      *start_match == CHAR_NL &&
      (re->flags & PCRE2_HASCRORLF) == 0 &&
        (mb->nltype == NLTYPE_ANY ||
         mb->nltype == NLTYPE_ANYCRLF ||
         mb->nllen == 2))
    start_match++;
  mb->mark = NULL;
  }
ENDLOOP:
#ifdef SUPPORT_UNICODE
if (utf && end_subject != true_end_subject &&
    (rc == MATCH_NOMATCH || rc == PCRE2_ERROR_PARTIAL))
  {
  for (;;)
    {
    start_match = end_subject + 1;
#if PCRE2_CODE_UNIT_WIDTH != 32
    while (start_match < true_end_subject && NOT_FIRSTCU(*start_match))
      start_match++;
#endif
    if (start_match >= true_end_subject)
      {
      rc = MATCH_NOMATCH;
      match_partial = NULL;
      break;
      }
    mb->check_subject = start_match;
    rc = PRIV(valid_utf)(start_match, length - (start_match - subject),
      &(match_data->startchar));
    if (rc == 0)
      {
      mb->end_subject = end_subject = true_end_subject;
      fragment_options = PCRE2_NOTBOL;
      goto FRAGMENT_RESTART;
      }
    else if (rc < 0)
      {
      mb->end_subject = end_subject = start_match + match_data->startchar;
      if (end_subject > start_match)
        {
        fragment_options = PCRE2_NOTBOL|PCRE2_NOTEOL;
        goto FRAGMENT_RESTART;
        }
      }
    }
  }
#endif
match_data->code = re;
match_data->mark = mb->mark;
match_data->matchedby = PCRE2_MATCHEDBY_INTERPRETER;
match_data->options = original_options;
if (rc == MATCH_MATCH)
  {
  match_data->rc = ((int)mb->end_offset_top >= 2 * match_data->oveccount)?
    0 : (int)mb->end_offset_top/2 + 1;
  match_data->subject_length = length;
  match_data->start_offset = start_offset;
  match_data->startchar = start_match - subject;
  match_data->leftchar = mb->start_used_ptr - subject;
  match_data->rightchar = ((mb->last_used_ptr > mb->end_match_ptr)?
    mb->last_used_ptr : mb->end_match_ptr) - subject;
  if ((options & PCRE2_COPY_MATCHED_SUBJECT) != 0)
    {
    if (length != 0)
      {
      match_data->subject = match_data->memctl.malloc(CU2BYTES(length),
        match_data->memctl.memory_data);
      if (match_data->subject == NULL)
        return match_data->rc = PCRE2_ERROR_NOMEMORY;
      memcpy((void *)match_data->subject, subject, CU2BYTES(length));
      }
    else
      match_data->subject = NULL;
    match_data->flags |= PCRE2_MD_COPIED_SUBJECT;
    }
  else match_data->subject = original_subject;
  return match_data->rc;
  }
match_data->mark = mb->nomatch_mark;
if (rc != MATCH_NOMATCH && rc != PCRE2_ERROR_PARTIAL) match_data->rc = rc;
else if (match_partial != NULL)
  {
  match_data->subject = original_subject;
  match_data->subject_length = length;
  match_data->start_offset = start_offset;
  match_data->ovector[0] = match_partial - subject;
  match_data->ovector[1] = end_subject - subject;
  match_data->startchar = match_partial - subject;
  match_data->leftchar = start_partial - subject;
  match_data->rightchar = end_subject - subject;
  match_data->rc = PCRE2_ERROR_PARTIAL;
  }
else
  {
  match_data->subject = original_subject;
  match_data->subject_length = length;
  match_data->start_offset = start_offset;
  match_data->rc = PCRE2_ERROR_NOMATCH;
  }
return match_data->rc;
}
#if defined(ERLANG_INTEGRATION)
#undef anchored
#undef startline
#undef firstline
#undef has_first_cu
#undef has_req_cu
#undef first_cu2
#undef req_cu
#undef req_cu2
#undef match_block
#undef mb
#undef start_match
#undef start_partial
#undef match_partial
#undef study
#undef re
#undef frame_zero
PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_free_restart_data(pcre2_match_data *mdata) {
  PcreExecContext * top = (PcreExecContext *) *mdata->restart_data;
  if (top != NULL) {
      mdata->memctl.free(top, mdata->memctl.memory_data);
      *mdata->restart_data = NULL;
  }
}
#endif
#undef NLBLOCK
#undef PSSTART
#undef PSEND
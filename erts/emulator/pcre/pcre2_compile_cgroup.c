#include "pcre2_compile.h"
uint16_t
PRIV(compile_get_hash_from_name)(PCRE2_SPTR name, uint32_t length)
{
uint16_t hash;
PCRE2_ASSERT(length > 0);
hash = (uint16_t)((name[0] & 0x7f) | ((name[length - 1] & 0xff) << 7));
PCRE2_ASSERT(hash <= NAMED_GROUP_HASH_MASK);
return hash;
}
named_group *
PRIV(compile_find_named_group)(PCRE2_SPTR name,
  uint32_t length, compile_block *cb)
{
uint16_t hash = PRIV(compile_get_hash_from_name)(name, length);
named_group *ng;
named_group *end = cb->named_groups + cb->names_found;
for (ng = cb->named_groups; ng < end; ng++)
  if (length == ng->length && hash == NAMED_GROUP_GET_HASH(ng) &&
      PRIV(strncmp)(name, ng->name, length) == 0) return ng;
return NULL;
}
uint32_t
PRIV(compile_add_name_to_table)(compile_block *cb,
  named_group *ng, uint32_t tablecount)
{
uint32_t i;
PCRE2_SPTR name = ng->name;
int length = ng->length;
uint32_t duplicate_count = 1;
PCRE2_UCHAR *slot = cb->name_table;
PCRE2_ASSERT(length > 0);
if ((ng->hash_dup & NAMED_GROUP_IS_DUPNAME) != 0)
  {
  named_group *ng_it;
  named_group *end = cb->named_groups + cb->names_found;
  for (ng_it = ng + 1; ng_it < end; ng_it++)
    if (ng_it->name == name) duplicate_count++;
  }
for (i = 0; i < tablecount; i++)
  {
  int crc = memcmp(name, slot + IMM2_SIZE, CU2BYTES(length));
  if (crc == 0 && slot[IMM2_SIZE + length] != 0)
    crc = -1;
  if (crc < 0)
    {
    (void)memmove(slot + cb->name_entry_size * duplicate_count, slot,
      CU2BYTES((tablecount - i) * cb->name_entry_size));
    break;
    }
  slot += cb->name_entry_size;
  }
tablecount += duplicate_count;
while (TRUE)
  {
  PUT2(slot, 0, ng->number);
  memcpy(slot + IMM2_SIZE, name, CU2BYTES(length));
  memset(slot + IMM2_SIZE + length, 0,
    CU2BYTES(cb->name_entry_size - length - IMM2_SIZE));
  if (--duplicate_count == 0) break;
  while (TRUE)
    {
    ++ng;
    if (ng->name == name) break;
    }
  slot += cb->name_entry_size;
  }
return tablecount;
}
BOOL
PRIV(compile_find_dupname_details)(PCRE2_SPTR name, uint32_t length,
  int *indexptr, int *countptr, int *errorcodeptr, compile_block *cb)
{
uint32_t i, groupnumber;
int count;
PCRE2_UCHAR *slot = cb->name_table;
for (i = 0; i < cb->names_found; i++)
  {
  if (PRIV(strncmp)(name, slot + IMM2_SIZE, length) == 0 &&
      slot[IMM2_SIZE + length] == 0) break;
  slot += cb->name_entry_size;
  }
if (i >= cb->names_found)
  {
  PCRE2_DEBUG_UNREACHABLE();
  *errorcodeptr = ERR53;
  cb->erroroffset = name - cb->start_pattern;
  return FALSE;
  }
*indexptr = i;
count = 0;
for (;;)
  {
  count++;
  groupnumber = GET2(slot, 0);
  cb->backref_map |= (groupnumber < 32)? (1u << groupnumber) : 1;
  if (groupnumber > cb->top_backref) cb->top_backref = groupnumber;
  if (++i >= cb->names_found) break;
  slot += cb->name_entry_size;
  if (PRIV(strncmp)(name, slot + IMM2_SIZE, length) != 0 ||
    (slot + IMM2_SIZE)[length] != 0) break;
  }
*countptr = count;
return TRUE;
}
static size_t
PRIV(compile_process_capture_list)(uint32_t *pptr, PCRE2_SIZE offset,
  int *errorcodeptr, compile_block *cb)
{
size_t i, size = 0;
named_group *ng;
PCRE2_SPTR name;
uint32_t length;
named_group *end = cb->named_groups + cb->names_found;
while (TRUE)
  {
  ++pptr;
  switch (META_CODE(*pptr))
    {
    case META_OFFSET:
    GETPLUSOFFSET(offset, pptr);
    continue;
    case META_CAPTURE_NAME:
    offset += META_DATA(*pptr);
    length = *(++pptr);
    name = cb->start_pattern + offset;
    ng = PRIV(compile_find_named_group)(name, length, cb);
    if (ng == NULL)
      {
      *errorcodeptr = ERR15;
      cb->erroroffset = offset;
      return 0;
      }
    if ((ng->hash_dup & NAMED_GROUP_IS_DUPNAME) == 0)
      {
      pptr[-1] = META_CAPTURE_NUMBER;
      pptr[0] = ng->number;
      size++;
      continue;
      }
    pptr[-1] = META_CAPTURE_NAME;
    pptr[0] = (uint32_t)(ng - cb->named_groups);
    size++;
    name = ng->name;
    while (++ng < end)
      if (ng->name == name) size++;
    continue;
    case META_CAPTURE_NUMBER:
    offset += META_DATA(*pptr);
    i = *(++pptr);
    if (i > cb->bracount)
      {
      *errorcodeptr = ERR15;
      cb->erroroffset = offset;
      return 0;
      }
    if (i > cb->top_backref) cb->top_backref = (uint16_t)i;
    size++;
    continue;
    default:
    break;
    }
  PCRE2_ASSERT(size > 0);
  return size;
  }
}
uint32_t *
PRIV(compile_parse_scan_substr_args)(uint32_t *pptr,
  int *errorcodeptr, compile_block *cb, PCRE2_SIZE *lengthptr)
{
uint8_t *captures;
uint8_t *capture_ptr;
uint8_t bit;
PCRE2_SPTR name;
named_group *ng;
named_group *end = cb->named_groups + cb->names_found;
BOOL all_found;
size_t size;
PCRE2_ASSERT(*pptr == META_OFFSET);
if (PRIV(compile_process_capture_list)(pptr - 1, 0, errorcodeptr, cb) == 0)
  return NULL;
size = (cb->bracount + 1 + 7) >> 3;
captures = (uint8_t*)cb->cx->memctl.malloc(size, cb->cx->memctl.memory_data);
if (captures == NULL)
  {
  *errorcodeptr = ERR21;
  READPLUSOFFSET(cb->erroroffset, pptr);
  return NULL;
  }
memset(captures, 0, size);
while (TRUE)
  {
  switch (META_CODE(*pptr))
    {
    case META_OFFSET:
    pptr++;
    SKIPOFFSET(pptr);
    continue;
    case META_CAPTURE_NAME:
    ng = cb->named_groups + pptr[1];
    PCRE2_ASSERT((ng->hash_dup & NAMED_GROUP_IS_DUPNAME) != 0);
    pptr += 2;
    name = ng->name;
    all_found = TRUE;
    do
      {
      if (ng->name != name) continue;
      capture_ptr = captures + (ng->number >> 3);
      PCRE2_ASSERT(capture_ptr < captures + size);
      bit = (uint8_t)(1 << (ng->number & 0x7));
      if ((*capture_ptr & bit) == 0)
        {
        *capture_ptr |= bit;
        all_found = FALSE;
        }
      }
    while (++ng < end);
    if (!all_found)
      {
      *lengthptr += 1 + 2 * IMM2_SIZE;
      continue;
      }
    pptr[-2] = META_CAPTURE_NUMBER;
    pptr[-1] = 0;
    continue;
    case META_CAPTURE_NUMBER:
    pptr += 2;
    capture_ptr = captures + (pptr[-1] >> 3);
    PCRE2_ASSERT(capture_ptr < captures + size);
    bit = (uint8_t)(1 << (pptr[-1] & 0x7));
    if ((*capture_ptr & bit) != 0)
      {
      pptr[-1] = 0;
      continue;
      }
    *capture_ptr |= bit;
    *lengthptr += 1 + IMM2_SIZE;
    continue;
    default:
    break;
    }
  break;
  }
cb->cx->memctl.free(captures, cb->cx->memctl.memory_data);
return pptr - 1;
}
static void do_heapify_u16(uint16_t *captures, size_t size, size_t i)
{
size_t max;
size_t left;
size_t right;
uint16_t tmp;
while (TRUE)
  {
  max = i;
  left = (i << 1) + 1;
  right = left + 1;
  if (left < size && captures[left] > captures[max]) max = left;
  if (right < size && captures[right] > captures[max]) max = right;
  if (i == max) return;
  tmp = captures[i];
  captures[i] = captures[max];
  captures[max] = tmp;
  i = max;
  }
}
BOOL
PRIV(compile_parse_recurse_args)(uint32_t *pptr_start,
  PCRE2_SIZE offset, int *errorcodeptr, compile_block *cb)
{
uint32_t *pptr = pptr_start;
size_t i, size;
PCRE2_SPTR name;
named_group *ng;
named_group *end = cb->named_groups + cb->names_found;
recurse_arguments *args;
uint16_t *captures;
uint16_t *current;
uint16_t *captures_end;
uint16_t tmp;
size = PRIV(compile_process_capture_list)(pptr, offset, errorcodeptr, cb);
if (size == 0) return FALSE;
args = cb->cx->memctl.malloc(
  sizeof(recurse_arguments) + size * sizeof(uint16_t), cb->cx->memctl.memory_data);
if (args == NULL)
  {
  *errorcodeptr = ERR21;
  cb->erroroffset = offset;
  return FALSE;
  }
args->header.next = NULL;
#ifdef PCRE2_DEBUG
args->header.type = CDATA_RECURSE_ARGS;
#endif
args->size = size;
if (cb->last_data != NULL)
  cb->last_data->next = &args->header;
else
  cb->first_data = &args->header;
cb->last_data = &args->header;
captures = (uint16_t*)(args + 1);
while (TRUE)
  {
  ++pptr;
  switch (META_CODE(*pptr))
    {
    case META_OFFSET:
    SKIPOFFSET(pptr);
    continue;
    case META_CAPTURE_NAME:
    ng = cb->named_groups + *(++pptr);
    PCRE2_ASSERT((ng->hash_dup & NAMED_GROUP_IS_DUPNAME) != 0);
    *captures++ = (uint16_t)(ng->number);
    name = ng->name;
    while (++ng < end)
      if (ng->name == name) *captures++ = (uint16_t)(ng->number);
    continue;
    case META_CAPTURE_NUMBER:
    *captures++ = *(++pptr);
    continue;
    default:
    break;
    }
  break;
  }
PCRE2_ASSERT(size == (size_t)(captures - (uint16_t*)(args + 1)));
args->skip_size = (size_t)(pptr - pptr_start) - 1;
if (size == 1) return TRUE;
captures = (uint16_t*)(args + 1);
i = (size >> 1) - 1;
while (TRUE)
  {
  do_heapify_u16(captures, size, i);
  if (i == 0) break;
  i--;
  }
for (i = size - 1; i > 0; i--)
  {
  tmp = captures[0];
  captures[0] = captures[i];
  captures[i] = tmp;
  do_heapify_u16(captures, i, 0);
  }
captures_end = captures + size;
tmp = *captures++;
current = captures;
while (current < captures_end)
  {
  if (*current != tmp)
    {
    tmp = *current;
    *captures++ = tmp;
    }
  current++;
  }
args->size = (size_t)(captures - (uint16_t*)(args + 1));
return TRUE;
}
#include "pcre2_internal.h"
#define SERIALIZED_DATA_MAGIC 0x50523253u
#define SERIALIZED_DATA_VERSION \
  ((PCRE2_MAJOR) | ((PCRE2_MINOR) << 16))
#define SERIALIZED_DATA_CONFIG \
  (sizeof(PCRE2_UCHAR) | ((sizeof(void*)) << 8) | ((sizeof(PCRE2_SIZE)) << 16))
PCRE2_EXP_DEFN int32_t PCRE2_CALL_CONVENTION
pcre2_serialize_encode(const pcre2_code **codes, int32_t number_of_codes,
   uint8_t **serialized_bytes, PCRE2_SIZE *serialized_size,
   pcre2_general_context *gcontext)
{
uint8_t *bytes;
uint8_t *dst_bytes;
int32_t i;
PCRE2_SIZE total_size;
const pcre2_real_code *re;
const uint8_t *tables;
pcre2_serialized_data *data;
const pcre2_memctl *memctl = (gcontext != NULL) ?
  &gcontext->memctl : &PRIV(default_compile_context).memctl;
if (codes == NULL || serialized_bytes == NULL || serialized_size == NULL)
  return PCRE2_ERROR_NULL;
if (number_of_codes <= 0) return PCRE2_ERROR_BADDATA;
total_size = sizeof(pcre2_serialized_data) + TABLES_LENGTH;
tables = NULL;
for (i = 0; i < number_of_codes; i++)
  {
  if (codes[i] == NULL) return PCRE2_ERROR_NULL;
  re = (const pcre2_real_code *)(codes[i]);
  if (re->magic_number != MAGIC_NUMBER) return PCRE2_ERROR_BADMAGIC;
  if (tables == NULL)
    tables = re->tables;
  else if (tables != re->tables)
    return PCRE2_ERROR_MIXEDTABLES;
  total_size += re->blocksize;
  }
bytes = memctl->malloc(total_size + sizeof(pcre2_memctl), memctl->memory_data);
if (bytes == NULL) return PCRE2_ERROR_NOMEMORY;
memcpy(bytes, memctl, sizeof(pcre2_memctl));
bytes += sizeof(pcre2_memctl);
data = (pcre2_serialized_data *)bytes;
data->magic = SERIALIZED_DATA_MAGIC;
data->version = SERIALIZED_DATA_VERSION;
data->config = SERIALIZED_DATA_CONFIG;
data->number_of_codes = number_of_codes;
dst_bytes = bytes + sizeof(pcre2_serialized_data);
memcpy(dst_bytes, tables, TABLES_LENGTH);
dst_bytes += TABLES_LENGTH;
for (i = 0; i < number_of_codes; i++)
  {
  re = (const pcre2_real_code *)(codes[i]);
  (void)memcpy(dst_bytes, (const char *)re, re->blocksize);
  (void)memset(dst_bytes + offsetof(pcre2_real_code, memctl), 0,
    sizeof(pcre2_memctl));
  (void)memset(dst_bytes + offsetof(pcre2_real_code, tables), 0,
    sizeof(void *));
  (void)memset(dst_bytes + offsetof(pcre2_real_code, executable_jit), 0,
    sizeof(void *));
  dst_bytes += re->blocksize;
  }
*serialized_bytes = bytes;
*serialized_size = total_size;
return number_of_codes;
}
PCRE2_EXP_DEFN int32_t PCRE2_CALL_CONVENTION
pcre2_serialize_decode(pcre2_code **codes, int32_t number_of_codes,
   const uint8_t *bytes, pcre2_general_context *gcontext)
{
const pcre2_serialized_data *data = (const pcre2_serialized_data *)bytes;
const pcre2_memctl *memctl = (gcontext != NULL) ?
  &gcontext->memctl : &PRIV(default_compile_context).memctl;
const uint8_t *src_bytes;
pcre2_real_code *dst_re;
uint8_t *tables;
int32_t i, j;
if (data == NULL || codes == NULL) return PCRE2_ERROR_NULL;
if (number_of_codes <= 0) return PCRE2_ERROR_BADDATA;
if (data->number_of_codes <= 0) return PCRE2_ERROR_BADSERIALIZEDDATA;
if (data->magic != SERIALIZED_DATA_MAGIC) return PCRE2_ERROR_BADMAGIC;
if (data->version != SERIALIZED_DATA_VERSION) return PCRE2_ERROR_BADMODE;
if (data->config != SERIALIZED_DATA_CONFIG) return PCRE2_ERROR_BADMODE;
if (number_of_codes > data->number_of_codes)
  number_of_codes = data->number_of_codes;
src_bytes = bytes + sizeof(pcre2_serialized_data);
tables = memctl->malloc(TABLES_LENGTH + sizeof(PCRE2_SIZE), memctl->memory_data);
if (tables == NULL) return PCRE2_ERROR_NOMEMORY;
memcpy(tables, src_bytes, TABLES_LENGTH);
*(PCRE2_SIZE *)(tables + TABLES_LENGTH) = number_of_codes;
src_bytes += TABLES_LENGTH;
for (i = 0; i < number_of_codes; i++)
  {
  CODE_BLOCKSIZE_TYPE blocksize;
  memcpy(&blocksize, src_bytes + offsetof(pcre2_real_code, blocksize),
    sizeof(CODE_BLOCKSIZE_TYPE));
  if (blocksize <= sizeof(pcre2_real_code))
    return PCRE2_ERROR_BADSERIALIZEDDATA;
  dst_re = (pcre2_real_code *)PRIV(memctl_malloc)(blocksize,
    (pcre2_memctl *)gcontext);
  if (dst_re == NULL)
    {
    memctl->free(tables, memctl->memory_data);
    for (j = 0; j < i; j++)
      {
      memctl->free(codes[j], memctl->memory_data);
      codes[j] = NULL;
      }
    return PCRE2_ERROR_NOMEMORY;
    }
  memcpy(((uint8_t *)dst_re) + sizeof(pcre2_memctl),
    src_bytes + sizeof(pcre2_memctl), blocksize - sizeof(pcre2_memctl));
  if (dst_re->magic_number != MAGIC_NUMBER ||
      dst_re->name_entry_size > MAX_NAME_SIZE + IMM2_SIZE + 1 ||
      dst_re->name_count > MAX_NAME_COUNT)
    {
    memctl->free(dst_re, memctl->memory_data);
    return PCRE2_ERROR_BADSERIALIZEDDATA;
    }
  dst_re->tables = tables;
  dst_re->executable_jit = NULL;
  dst_re->flags |= PCRE2_DEREF_TABLES;
  codes[i] = dst_re;
  src_bytes += blocksize;
  }
return number_of_codes;
}
PCRE2_EXP_DEFN int32_t PCRE2_CALL_CONVENTION
pcre2_serialize_get_number_of_codes(const uint8_t *bytes)
{
const pcre2_serialized_data *data = (const pcre2_serialized_data *)bytes;
if (data == NULL) return PCRE2_ERROR_NULL;
if (data->magic != SERIALIZED_DATA_MAGIC) return PCRE2_ERROR_BADMAGIC;
if (data->version != SERIALIZED_DATA_VERSION) return PCRE2_ERROR_BADMODE;
if (data->config != SERIALIZED_DATA_CONFIG) return PCRE2_ERROR_BADMODE;
return data->number_of_codes;
}
PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_serialize_free(uint8_t *bytes)
{
if (bytes != NULL)
  {
  pcre2_memctl *memctl = (pcre2_memctl *)(bytes - sizeof(pcre2_memctl));
  memctl->free(memctl, memctl->memory_data);
  }
}
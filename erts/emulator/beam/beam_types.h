#ifndef _BEAM_TYPES_H
#define _BEAM_TYPES_H
#include "sys.h"
#define BEAM_TYPES_VERSION 4
#define BEAM_TYPE_NONE               (0)
#define BEAM_TYPE_ATOM               (1 << 0)
#define BEAM_TYPE_BITSTRING          (1 << 1)
#define BEAM_TYPE_CONS               (1 << 2)
#define BEAM_TYPE_FLOAT              (1 << 3)
#define BEAM_TYPE_FUN                (1 << 4)
#define BEAM_TYPE_INTEGER            (1 << 5)
#define BEAM_TYPE_MAP                (1 << 6)
#define BEAM_TYPE_NIL                (1 << 7)
#define BEAM_TYPE_PID                (1 << 8)
#define BEAM_TYPE_PORT               (1 << 9)
#define BEAM_TYPE_REFERENCE          (1 << 10)
#define BEAM_TYPE_TUPLE              (1 << 11)
#define BEAM_TYPE_RECORD             (1 << 12)
#define BEAM_TYPE_ANY                ((1 << 13) - 1)
#define BEAM_TYPE_HAS_LOWER_BOUND    (1 << 13)
#define BEAM_TYPE_HAS_UPPER_BOUND    (1 << 14)
#define BEAM_TYPE_HAS_UNIT           (1 << 15)
#define BEAM_TYPE_METADATA_MASK      (BEAM_TYPE_HAS_LOWER_BOUND | \
                                      BEAM_TYPE_HAS_UPPER_BOUND | \
                                      BEAM_TYPE_HAS_UNIT)
typedef struct {
    int type_union;
    int metadata_flags;
    Sint64 min;
    Sint64 max;
    byte size_unit;
} BeamType;
int beam_types_decode_type(const byte *data, BeamType *out);
void beam_types_decode_extra(const byte *data, BeamType *out);
#endif
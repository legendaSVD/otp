#ifndef __BEAM_CATCHES_H
#define __BEAM_CATCHES_H
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "code_ix.h"
#include "beam_code.h"
#define BEAM_CATCHES_NIL	(-1)
void beam_catches_init(void);
void beam_catches_start_staging(void);
void beam_catches_end_staging(int commit);
unsigned beam_catches_cons(ErtsCodePtr cp, unsigned cdr, ErtsCodePtr **);
ErtsCodePtr beam_catches_car(unsigned i);
void beam_catches_delmod(unsigned head,
                         const BeamCodeHeader *hdr,
                         unsigned code_bytes,
                         ErtsCodeIndex code_ix);
ErtsCodePtr beam_catches_car_staging(unsigned i);
#define catch_pc(x)	beam_catches_car(catch_val((x)))
#endif
#ifndef __ATOM_H__
#define __ATOM_H__
#include "index.h"
#include "erl_atom_table.h"
#define MAX_ATOM_CHARACTERS 255
#define MAX_ATOM_SZ_FROM_LATIN1 (2*MAX_ATOM_CHARACTERS)
#define MAX_ATOM_SZ_LIMIT (4*MAX_ATOM_CHARACTERS)
#define ATOM_LIMIT (1024*1024)
#define MIN_ATOM_TABLE_SIZE 8192
#define ATOM_BAD_ENCODING_ERROR -1
#define ATOM_MAX_CHARS_ERROR -2
#ifndef ARCH_32
#define MAX_ATOM_TABLE_SIZE ((MAX_ATOM_INDEX + 1 < (UWORD_CONSTANT(1) << 32)) ? MAX_ATOM_INDEX + 1 : ((UWORD_CONSTANT(1) << 31) - 1))
#else
#define MAX_ATOM_TABLE_SIZE (MAX_ATOM_INDEX + 1)
#endif
typedef struct atom {
    IndexSlot slot;
    Sint16 len;
    Sint16 latin1_chars;
    int ord0;
    union{
        byte* name;
        Eterm bin;
    } u;
} Atom;
extern IndexTable erts_atom_table;
ERTS_GLB_INLINE Atom* atom_tab(Uint i);
ERTS_GLB_INLINE int erts_is_atom_utf8_bytes(byte *text, size_t len, Eterm term);
ERTS_GLB_INLINE int erts_is_atom_str(const char *str, Eterm term, int is_latin1);
ERTS_GLB_INLINE int erts_is_atom_index_ok(Uint ix);
const byte *erts_atom_get_name(const Atom *atom);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE Atom*
atom_tab(Uint i)
{
    return (Atom *) erts_index_lookup(&erts_atom_table, i);
}
ERTS_GLB_INLINE int erts_is_atom_utf8_bytes(byte *text, size_t len, Eterm term)
{
    Atom *a;
    if (!is_atom(term))
	return 0;
    a = atom_tab(atom_val(term));
    return (len == (size_t) a->len
	    && sys_memcmp((void *) erts_atom_get_name(a), (void *) text, len) == 0);
}
ERTS_GLB_INLINE int erts_is_atom_str(const char *str, Eterm term, int is_latin1)
{
    Atom *a;
    int i, len;
    const byte* aname;
    const byte* s = (const byte*) str;
    if (!is_atom(term))
	return 0;
    a = atom_tab(atom_val(term));
    len = a->len;
    aname = erts_atom_get_name(a);
    if (is_latin1) {
	for (i = 0; i < len; s++) {
	    if (aname[i] < 0x80) {
		if (aname[i] != *s || *s == '\0')
		    return 0;
		i++;
	    }
	    else {
		if (aname[i]   != (0xC0 | (*s >> 6)) ||
		    aname[i+1] != (0x80 | (*s & 0x3F))) {
		    return 0;
		}
		i += 2;
	    }
	}
    }
    else {
	for (i = 0; i < len; i++, s++)
	    if (aname[i] != *s || *s == '\0')
		return 0;
    }
    return *s == '\0';
}
ERTS_GLB_INLINE int erts_is_atom_index_ok(Uint ix)
{
    return ix < (Uint)erts_atom_table.entries;
}
#endif
typedef enum {
    ERTS_ATOM_ENC_7BIT_ASCII,
    ERTS_ATOM_ENC_LATIN1,
    ERTS_ATOM_ENC_UTF8
} ErtsAtomEncoding;
#define ERTS_IS_ATOM_STR(LSTR, TERM) \
  (erts_is_atom_utf8_bytes((byte *) LSTR, sizeof(LSTR) - 1, (TERM)))
#define ERTS_DECL_AM(S) Eterm AM_ ## S = am_atom_put(#S, sizeof(#S) - 1)
#define ERTS_INIT_AM(S) AM_ ## S = am_atom_put(#S, sizeof(#S) - 1)
#define ERTS_MAKE_AM(Str) am_atom_put(Str, sizeof(Str) - 1)
int atom_table_size(void);
int atom_table_sz(void);
Eterm am_atom_put(const char*, Sint);
Eterm erts_atom_put(const byte *name, Sint len, ErtsAtomEncoding enc, int trunc);
int erts_atom_put_index(const byte *name, Sint len, ErtsAtomEncoding enc, int trunc);
void init_atom_table(void);
void atom_info(fmtfn_t, void *);
void dump_atoms(fmtfn_t, void *);
Uint erts_get_atom_limit(void);
int erts_atom_get(const char* name, Uint len, Eterm* ap, ErtsAtomEncoding enc);
void erts_atom_get_text_space_sizes(Uint *reserved, Uint *used);
#endif
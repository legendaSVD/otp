#ifndef _ERL_UNICODE_H
#define _ERL_UNICODE_H
Uint erts_atom_to_string_length(Eterm atom);
Eterm erts_atom_to_string(Eterm **hpp, Eterm atom, Eterm tail);
#endif
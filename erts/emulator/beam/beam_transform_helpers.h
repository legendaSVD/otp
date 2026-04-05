#ifndef __BEAM_TRANSFORM_HELPERS__
#define __BEAM_TRANSFORM_HELPERS__
int beam_load_safe_mul(UWord a, UWord b, UWord* resp);
int beam_load_map_key_sort(LoaderState* stp, BeamOpArg Size, BeamOpArg* Rest);
Eterm beam_load_get_term(LoaderState* stp, BeamOpArg Key);
void beam_load_sort_select_vals(BeamOpArg* base, size_t n);
#endif
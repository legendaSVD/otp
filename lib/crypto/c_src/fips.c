#include "fips.h"
#include "digest.h"
ERL_NIF_TERM info_fips(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
#ifdef FIPS_SUPPORT
    return FIPS_MODE() ? atom_enabled : atom_not_enabled;
#else
    return atom_not_supported;
#endif
}
ERL_NIF_TERM enable_fips_mode_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    return enable_fips_mode(env, argv[0]);
}
ERL_NIF_TERM enable_fips_mode(ErlNifEnv* env, ERL_NIF_TERM fips_mode_to_set)
#ifdef FIPS_SUPPORT
{
    if (fips_mode_to_set == atom_true) {
        if (FIPS_mode_set(1)) return atom_true;
                         else return atom_false;
    } else if (fips_mode_to_set == atom_false) {
        if (!FIPS_mode_set(0)) return atom_false;
                          else return atom_true;
    } else
        return enif_make_badarg(env);
}
#else
{
    if (fips_mode_to_set == atom_true) return atom_false;
    else if (fips_mode_to_set == atom_false) return atom_true;
    else return enif_make_badarg(env);
}
#endif
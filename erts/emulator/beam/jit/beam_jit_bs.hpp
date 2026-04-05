#ifndef __BEAM_JIT_BS_HPP__
#define __BEAM_JIT_BS_HPP__
#include "beam_jit_common.hpp"
struct BscSegment {
    BscSegment()
            : type(am_false), unit(1), flags(0), src(ArgNil()), size(ArgNil()),
              error_info(0), offsetInAccumulator(0), effectiveSize(-1),
              action(action::DIRECT) {
    }
    Eterm type;
    Uint unit;
    Uint flags;
    ArgVal src;
    ArgVal size;
    Uint error_info;
    Uint offsetInAccumulator;
    Sint effectiveSize;
    enum class action { DIRECT, ACCUMULATE_FIRST, ACCUMULATE, STORE } action;
};
std::vector<BscSegment> beam_jit_bsc_init(const Span<const ArgVal> &args);
std::vector<BscSegment> beam_jit_bsc_combine_segments(
        const std::vector<BscSegment> segments);
struct BsmSegment {
    BsmSegment()
            : action(action::TEST_HEAP), live(ArgNil()), size(0), unit(1),
              flags(0), dst(ArgXRegister(0)){};
    enum class action {
        TEST_HEAP,
        ENSURE_AT_LEAST,
        ENSURE_EXACTLY,
        READ,
        EXTRACT_BITSTRING,
        EXTRACT_INTEGER,
        READ_INTEGER,
        GET_INTEGER,
        GET_BITSTRING,
        SKIP,
        DROP,
        GET_TAIL,
        EQ
    } action;
    ArgVal live;
    Uint size;
    Uint unit;
    Uint flags;
    ArgRegister dst;
};
std::vector<BsmSegment> beam_jit_bsm_init(const BeamFile *beam,
                                          const Span<const ArgVal> &List);
std::vector<BsmSegment> beam_jit_opt_bsm_segments(
        const std::vector<BsmSegment> segments,
        const ArgWord &Need,
        const ArgWord &Live);
#endif
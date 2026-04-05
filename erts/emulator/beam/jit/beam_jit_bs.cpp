#include "beam_asm.hpp"
#include "beam_jit_common.hpp"
#include "beam_jit_bs.hpp"
#include <iterator>
#include <numeric>
extern "C"
{
#include "beam_file.h"
};
std::vector<BscSegment> beam_jit_bsc_init(const Span<const ArgVal> &args) {
    std::size_t n = args.size();
    std::vector<BscSegment> segments;
    for (std::size_t i = 0; i < n; i += 6) {
        BscSegment seg;
        JitBSCOp bsc_op;
        Uint bsc_segment;
        seg.type = args[i].as<ArgImmed>().get();
        bsc_segment = args[i + 1].as<ArgWord>().get();
        seg.unit = args[i + 2].as<ArgWord>().get();
        seg.flags = args[i + 3].as<ArgWord>().get();
        seg.src = args[i + 4];
        seg.size = args[i + 5];
        switch (seg.type) {
        case am_float:
            bsc_op = BSC_OP_FLOAT;
            break;
        case am_integer:
            bsc_op = BSC_OP_INTEGER;
            break;
        case am_utf8:
            bsc_op = BSC_OP_UTF8;
            break;
        case am_utf16:
            bsc_op = BSC_OP_UTF16;
            break;
        case am_utf32:
            bsc_op = BSC_OP_UTF32;
            break;
        default:
            bsc_op = BSC_OP_BITSTRING;
            break;
        }
        seg.error_info = beam_jit_set_bsc_segment_op(bsc_segment, bsc_op);
        segments.insert(segments.end(), seg);
    }
    return segments;
}
template<typename It>
static auto fold_group(std::vector<BscSegment> &segs, It first, It last) {
    auto &back = segs.emplace_back(*first);
    back.action = BscSegment::action::ACCUMULATE_FIRST;
    return std::accumulate(std::next(first),
                           last,
                           back.effectiveSize,
                           [&segs](Sint acc, const BscSegment &seg) {
                               auto &back = segs.emplace_back(seg);
                               back.action = BscSegment::action::ACCUMULATE;
                               return acc + back.effectiveSize;
                           });
}
static void push_group(std::vector<BscSegment> &segs,
                       std::vector<BscSegment>::const_iterator start,
                       std::vector<BscSegment>::const_iterator end) {
    if (start < end) {
        auto groupSize = ((start->flags & BSF_LITTLE) != 0)
                                 ? fold_group(segs,
                                              std::make_reverse_iterator(end),
                                              std::make_reverse_iterator(start))
                                 : fold_group(segs, start, end);
        auto &seg = segs.emplace_back();
        seg.type = am_integer;
        seg.action = BscSegment::action::STORE;
        seg.effectiveSize = groupSize;
        seg.flags = start->flags;
    }
}
std::vector<BscSegment> beam_jit_bsc_combine_segments(
        const std::vector<BscSegment> segments) {
    std::vector<BscSegment> segs;
    auto group = segments.cend();
    Sint combinedSize = 0;
    for (auto it = segments.cbegin(); it != segments.cend(); it++) {
        auto &seg = *it;
        switch (seg.type) {
        case am_integer: {
            if (!(0 < seg.effectiveSize && seg.effectiveSize <= 64)) {
                push_group(segs, group, it);
                group = segments.cend();
                segs.push_back(seg);
                continue;
            }
            if (group == segments.cend()) {
                group = it;
                combinedSize = seg.effectiveSize;
                continue;
            }
            bool sameEndian =
                    (seg.flags & BSF_LITTLE) == (group->flags & BSF_LITTLE);
            bool suitableSizes =
                    ((seg.flags & BSF_LITTLE) == 0 || combinedSize % 8 == 0);
            if (sameEndian && combinedSize + seg.effectiveSize <= 64 &&
                suitableSizes) {
                combinedSize += seg.effectiveSize;
                continue;
            }
            push_group(segs, group, it);
            group = it;
            combinedSize = seg.effectiveSize;
            break;
        }
        default:
            push_group(segs, group, it);
            group = segments.cend();
            segs.push_back(seg);
            break;
        }
    }
    push_group(segs, group, segments.cend());
    Uint offset = 0;
    for (int i = segs.size() - 1; i >= 0; i--) {
        switch (segs[i].action) {
        case BscSegment::action::STORE:
            offset = 64 - segs[i].effectiveSize;
            break;
        case BscSegment::action::ACCUMULATE_FIRST:
        case BscSegment::action::ACCUMULATE:
            segs[i].offsetInAccumulator = offset;
            offset += segs[i].effectiveSize;
            break;
        default:
            break;
        }
    }
    return segs;
}
static UWord bs_get_flags(const BeamFile *beam, const ArgVal &val) {
    if (val.isNil()) {
        return 0;
    } else if (val.isLiteral()) {
        Eterm term = beamfile_get_literal(beam, val.as<ArgLiteral>().get());
        UWord flags = 0;
        while (is_list(term)) {
            Eterm *consp = list_val(term);
            Eterm elem = CAR(consp);
            switch (elem) {
            case am_little:
            case am_native:
                flags |= BSF_LITTLE;
                break;
            case am_signed:
                flags |= BSF_SIGNED;
                break;
            }
            term = CDR(consp);
        }
        ASSERT(is_nil(term));
        return flags;
    } else if (val.isWord()) {
        return val.as<ArgWord>().get();
    } else {
        ASSERT(0);
        return 0;
    }
}
std::vector<BsmSegment> beam_jit_bsm_init(const BeamFile *beam,
                                          const Span<const ArgVal> &List) {
    std::vector<BsmSegment> segments;
    auto current = List.begin();
    auto end = List.begin() + List.size();
    while (current < end) {
        auto cmd = current++->as<ArgImmed>().get();
        BsmSegment seg;
        switch (cmd) {
        case am_ensure_at_least: {
            seg.action = BsmSegment::action::ENSURE_AT_LEAST;
            seg.size = current[0].as<ArgWord>().get();
            seg.unit = current[1].as<ArgWord>().get();
            current += 2;
            break;
        }
        case am_ensure_exactly: {
            seg.action = BsmSegment::action::ENSURE_EXACTLY;
            seg.size = current[0].as<ArgWord>().get();
            current += 1;
            break;
        }
        case am_binary:
        case am_integer: {
            auto size = current[2].as<ArgWord>().get();
            auto unit = current[3].as<ArgWord>().get();
            switch (cmd) {
            case am_integer:
                seg.action = BsmSegment::action::GET_INTEGER;
                break;
            case am_binary:
                seg.action = BsmSegment::action::GET_BITSTRING;
                break;
            }
            seg.live = current[0];
            seg.size = size * unit;
            seg.unit = unit;
            seg.flags = bs_get_flags(beam, current[1]);
            seg.dst = current[4].as<ArgRegister>();
            current += 5;
            break;
        }
        case am_get_tail: {
            seg.action = BsmSegment::action::GET_TAIL;
            seg.live = current[0].as<ArgWord>();
            seg.dst = current[2].as<ArgRegister>();
            current += 3;
            break;
        }
        case am_skip: {
            seg.action = BsmSegment::action::SKIP;
            seg.size = current[0].as<ArgWord>().get();
            seg.flags = 0;
            current += 1;
            break;
        }
        case am_Eq: {
            seg.action = BsmSegment::action::EQ;
            seg.live = current[0];
            seg.size = current[1].as<ArgWord>().get();
            seg.unit = current[2].as<ArgWord>().get();
            current += 3;
            break;
        }
        default:
            abort();
            break;
        }
        segments.push_back(seg);
    }
    return segments;
}
std::vector<BsmSegment> beam_jit_opt_bsm_segments(
        const std::vector<BsmSegment> segments,
        const ArgWord &Need,
        const ArgWord &Live) {
    std::vector<BsmSegment> segs;
    Uint heap_need = Need.get();
    for (auto seg : segments) {
        switch (seg.action) {
        case BsmSegment::action::GET_INTEGER:
            if (seg.size >= SMALL_BITS) {
                heap_need += BIG_NEED_FOR_BITS(seg.size);
            }
            break;
        case BsmSegment::action::GET_BITSTRING:
            heap_need += erts_extracted_bitstring_size(seg.size);
            break;
        case BsmSegment::action::GET_TAIL:
            heap_need += BUILD_SUB_BITSTRING_HEAP_NEED;
            break;
        default:
            break;
        }
    }
    int read_action_pos = -1;
    int seg_index = 0;
    int count = segments.size();
    for (int i = 0; i < count; i++) {
        auto seg = segments[i];
        if (heap_need != 0 && seg.live.isWord()) {
            BsmSegment s = seg;
            read_action_pos = -1;
            s.action = BsmSegment::action::TEST_HEAP;
            s.size = heap_need;
            segs.push_back(s);
            heap_need = 0;
            seg_index++;
        }
        switch (seg.action) {
        case BsmSegment::action::GET_INTEGER:
        case BsmSegment::action::GET_BITSTRING: {
            bool is_common_size;
            switch (seg.size) {
            case 8:
            case 16:
            case 32:
                is_common_size = true;
                break;
            default:
                is_common_size = false;
                break;
            }
            if (seg.size > 64) {
                read_action_pos = -1;
            } else if (read_action_pos < 0 &&
                       seg.action == BsmSegment::action::GET_INTEGER &&
                       is_common_size && i + 1 == count) {
                seg.action = BsmSegment::action::READ_INTEGER;
                read_action_pos = -1;
            } else {
                if (read_action_pos < 0 ||
                    seg.size + segs.at(read_action_pos).size > 64) {
                    BsmSegment s;
                    read_action_pos = seg_index;
                    s.action = BsmSegment::action::READ;
                    s.size = seg.size;
                    segs.push_back(s);
                    seg_index++;
                } else {
                    segs.at(read_action_pos).size += seg.size;
                }
                switch (seg.action) {
                case BsmSegment::action::GET_INTEGER:
                    seg.action = BsmSegment::action::EXTRACT_INTEGER;
                    break;
                case BsmSegment::action::GET_BITSTRING:
                    seg.action = BsmSegment::action::EXTRACT_BITSTRING;
                    break;
                default:
                    break;
                }
            }
            segs.push_back(seg);
            break;
        }
        case BsmSegment::action::EQ: {
            if (read_action_pos < 0 ||
                seg.size + segs.at(read_action_pos).size > 64) {
                BsmSegment s;
                read_action_pos = seg_index;
                s.action = BsmSegment::action::READ;
                s.size = seg.size;
                segs.push_back(s);
                seg_index++;
            } else {
                segs.at(read_action_pos).size += seg.size;
            }
            auto &prev = segs.back();
            if (prev.action == BsmSegment::action::EQ &&
                prev.size + seg.size <= 64) {
                prev.size += seg.size;
                prev.unit = prev.unit << seg.size | seg.unit;
                seg_index--;
            } else {
                segs.push_back(seg);
            }
            break;
        }
        case BsmSegment::action::SKIP:
            if (read_action_pos >= 0 &&
                seg.size + segs.at(read_action_pos).size <= 64) {
                segs.at(read_action_pos).size += seg.size;
                seg.action = BsmSegment::action::DROP;
            } else {
                read_action_pos = -1;
            }
            segs.push_back(seg);
            break;
        default:
            read_action_pos = -1;
            segs.push_back(seg);
            break;
        }
        seg_index++;
    }
    if (heap_need) {
        BsmSegment seg;
        seg.action = BsmSegment::action::TEST_HEAP;
        seg.size = heap_need;
        seg.live = Live;
        segs.push_back(seg);
    }
    return segs;
}
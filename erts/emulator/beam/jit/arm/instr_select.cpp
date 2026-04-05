#include <algorithm>
#include <numeric>
#include "beam_asm.hpp"
using namespace asmjit;
template<typename T>
static constexpr bool isInt13(T value) {
    typedef typename std::make_unsigned<T>::type U;
    typedef typename std::make_signed<T>::type S;
    return Support::is_uint_n<12>(U(value)) ||
           Support::is_uint_n<12>(-S(value));
}
static std::pair<UWord, int> plan_untag(const Span<const ArgVal> &args) {
    auto left = args.begin(), right = args.begin();
    auto best_left = left, best_right = right;
    int count, shift;
    count = args.size() / 2;
    ASSERT(left->isImmed() && (args.begin() + count)->isLabel());
    ASSERT(left->isSmall() || right->isAtom());
    shift = left->isSmall() ? _TAG_IMMED1_SIZE : _TAG_IMMED2_SIZE;
    while (right < (args.begin() + count)) {
        auto distance = std::distance(left, right);
        UWord left_value, mid_value, right_value;
        left_value = left->as<ArgImmed>().get() >> shift;
        mid_value = (left + distance / 2)->as<ArgImmed>().get() >> shift;
        right_value = right->as<ArgImmed>().get() >> shift;
        if (isInt13(left_value - mid_value) &&
            isInt13(right_value - mid_value)) {
            if (distance > std::distance(best_left, best_right)) {
                best_right = right;
                best_left = left;
            }
            right++;
        } else {
            left++;
        }
    }
    auto distance = std::distance(best_left, best_right);
    if (distance <= 6) {
        return std::make_pair(0, 0);
    }
    if (isInt13(best_left->as<ArgImmed>().get()) &&
        isInt13(best_right->as<ArgImmed>().get())) {
        return std::make_pair(0, 0);
    }
    if (isInt13(best_left->as<ArgImmed>().get() >> shift) &&
        isInt13(best_right->as<ArgImmed>().get() >> shift)) {
        return std::make_pair(0, shift);
    }
    auto mid_value = (best_left + distance / 2)->as<ArgImmed>().get();
    return std::make_pair(mid_value, shift);
}
const std::vector<ArgVal> BeamModuleAssembler::emit_select_untag(
        const ArgSource &Src,
        const Span<const ArgVal> &args,
        a64::Gp comparand,
        Label fail,
        UWord base,
        int shift) {
    ASSERT(base != 0 || shift > 0);
    comment("(comparing untagged+rebased values)");
    if ((args.first().isSmall() && always_small(Src)) ||
        (args.first().isAtom() && exact_type<BeamTypeId::Atom>(Src))) {
        comment("(skipped type test)");
    } else {
        if (args.first().isSmall()) {
            a.and_(TMP1, comparand, imm(_TAG_IMMED1_MASK));
            a.cmp(TMP1, imm(_TAG_IMMED1_SMALL));
        } else {
            ASSERT(args.first().isAtom());
            a.and_(TMP1, comparand, imm(_TAG_IMMED2_MASK));
            a.cmp(TMP1, imm(_TAG_IMMED2_ATOM));
        }
        a.b_ne(resolve_label(fail, disp1MB));
    }
    if (shift != 0) {
        a.lsr(ARG1, comparand, imm(shift));
        base >>= shift;
        comparand = ARG1;
    }
    std::vector<ArgVal> result(args.begin(), args.end());
    int count = args.size() / 2;
    if (base != 0) {
        sub(ARG1, comparand, base);
        std::vector<int> sorted_indexes(count);
        std::iota(sorted_indexes.begin(), sorted_indexes.end(), 0);
        std::sort(sorted_indexes.begin(),
                  sorted_indexes.end(),
                  [&](int lhs, int rhs) {
                      auto lhs_value =
                              (args[lhs].as<ArgImmed>().get() >> shift) - base;
                      auto rhs_value =
                              (args[rhs].as<ArgImmed>().get() >> shift) - base;
                      return lhs_value < rhs_value;
                  });
        for (auto i = 0; i < count; i++) {
            const auto &src_value = args[sorted_indexes[i]];
            const auto &src_label = args[sorted_indexes[i] + count];
            auto &dst_value = result[i];
            auto &dst_label = result[i + count];
            dst_value =
                    ArgWord((src_value.as<ArgImmed>().get() >> shift) - base);
            dst_label = src_label;
        }
    } else {
        for (auto i = 0; i < count; i++) {
            auto &dst_value = result[i];
            auto &dst_label = result[i + count];
            dst_value = ArgWord(args[i].as<ArgImmed>().get() >> shift);
            dst_label = args[i + count];
        }
    }
    ASSERT(std::is_sorted(result.begin(),
                          result.begin() + count,
                          [](const ArgWord &lhs, const ArgWord &rhs) {
                              return lhs.get() < rhs.get();
                          }));
    return result;
}
void BeamModuleAssembler::emit_linear_search(a64::Gp comparand,
                                             Label fail,
                                             const Span<const ArgVal> &args) {
    int count = args.size() / 2;
    ASSERT(count < 128);
    check_pending_stubs();
    for (int i = 0; i < count; i++) {
        const ArgVal &value = args[i];
        const ArgVal &label = args[i + count];
        int j;
        int n = 1;
        for (j = i + 1; j < count && args[j + count] == label; j++) {
            n++;
        }
        if (n < 2) {
            cmp_arg(comparand, value);
            a.b_eq(resolve_beam_label(label, disp1MB));
        } else {
            int in_range = 1;
            for (j = i + 1; j < n; j++) {
                if (!(value.isWord() && value.as<ArgWord>().get() + in_range ==
                                                args[j].as<ArgWord>().get())) {
                    break;
                }
                in_range++;
            }
            if (in_range > 2) {
                uint64_t first = value.as<ArgWord>().get();
                if (first == 0) {
                    a.cmp(comparand, imm(in_range));
                } else {
                    sub(TMP6, comparand, first);
                    a.cmp(TMP6, imm(in_range));
                }
                a.b_lo(resolve_beam_label(label, disp1MB));
                i += in_range - 1;
            } else {
                emit_optimized_two_way_select(comparand,
                                              value,
                                              args[i + 1],
                                              label);
                i++;
            }
        }
    }
    if (fail.is_valid()) {
        a.b(resolve_label(fail, disp128MB));
        mark_unreachable_check_pending_stubs();
    }
}
void BeamModuleAssembler::emit_i_select_tuple_arity(
        const ArgRegister &Src,
        const ArgLabel &Fail,
        const ArgWord &Size,
        const Span<const ArgVal> &args) {
    auto src = load_source(Src);
    emit_is_boxed(resolve_beam_label(Fail, dispUnknown), Src, src.reg);
    a64::Gp boxed_ptr = emit_ptr_val(TMP1, src.reg);
    a.ldur(TMP1, emit_boxed_val(boxed_ptr, 0));
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Tuple) {
        comment("simplified tuple test since the source is always a tuple "
                "when boxed");
    } else {
        ERTS_CT_ASSERT(_TAG_HEADER_ARITYVAL == 0);
        a.tst(TMP1, imm(_TAG_HEADER_MASK));
        a.b_ne(resolve_beam_label(Fail, disp1MB));
    }
    Label fail = rawLabels[Fail.get()];
    emit_binsearch_nodes(TMP1, 0, args.size() / 2 - 1, fail, args);
}
void BeamModuleAssembler::emit_i_select_val_lins(
        const ArgSource &Src,
        const ArgVal &Fail,
        const ArgWord &Size,
        const Span<const ArgVal> &args) {
    ASSERT(Size.get() == args.size());
    Label fail, next;
    if (Fail.isLabel()) {
        next = fail = rawLabels[Fail.as<ArgLabel>().get()];
    } else {
        ASSERT(Fail.isNil());
        next = a.new_label();
    }
    auto src = load_source(Src, ARG1);
    auto plan = plan_untag(args);
    auto base = plan.first;
    auto shift = plan.second;
    if (base == 0 && shift == 0) {
        emit_linear_search(src.reg, fail, args);
    } else {
        auto untagged =
                emit_select_untag(Src, args, src.reg, next, base, shift);
        emit_linear_search(ARG1, fail, Span(untagged.data(), untagged.size()));
    }
    if (!Fail.isLabel()) {
        bind_veneer_target(next);
    }
}
void BeamModuleAssembler::emit_i_select_val_bins(
        const ArgSource &Src,
        const ArgVal &Fail,
        const ArgWord &Size,
        const Span<const ArgVal> &args) {
    ASSERT(Size.get() == args.size());
    int count = args.size() / 2;
    Label fail;
    if (Fail.isLabel()) {
        fail = rawLabels[Fail.as<ArgLabel>().get()];
    } else {
        ASSERT(Fail.isNil());
        fail = a.new_label();
    }
    comment("Binary search in table of %lu elements", count);
    auto src = load_source(Src, ARG1);
    auto plan = plan_untag(args);
    auto base = plan.first;
    auto shift = plan.second;
    if (base == 0 && shift == 0) {
        emit_binsearch_nodes(src.reg, 0, count - 1, fail, args);
    } else {
        auto untagged =
                emit_select_untag(Src, args, src.reg, fail, base, shift);
        emit_binsearch_nodes(ARG1,
                             0,
                             count - 1,
                             fail,
                             Span(untagged.data(), untagged.size()));
    }
    if (!Fail.isLabel()) {
        bind_veneer_target(fail);
    }
}
void BeamModuleAssembler::emit_binsearch_nodes(a64::Gp reg,
                                               size_t Left,
                                               size_t Right,
                                               Label fail,
                                               const Span<const ArgVal> &args) {
    ASSERT(Left <= Right);
    ASSERT(Right < args.size() / 2);
    const size_t remaining = (Right - Left + 1);
    const size_t mid = (Left + Right) >> 1;
    const size_t count = args.size() / 2;
    if (remaining <= 10) {
        std::vector<ArgVal> shrunk;
        comment("Linear search in [%lu..%lu], %lu elements",
                Left,
                Right,
                remaining);
        shrunk.reserve(remaining * 2);
        shrunk.insert(shrunk.end(),
                      args.begin() + Left,
                      args.begin() + Left + remaining);
        shrunk.insert(shrunk.end(),
                      args.begin() + Left + count,
                      args.begin() + count + Left + remaining);
        emit_linear_search(reg, fail, Span(shrunk.data(), shrunk.size()));
        return;
    }
    comment("Subtree [%lu..%lu], pivot %lu", Left, Right, mid);
    check_pending_stubs();
    cmp_arg(reg, args[mid]);
    auto &lbl = args[mid + count];
    ASSERT(Left != Right);
    ASSERT(Left != mid);
    a.b_eq(resolve_beam_label(lbl, disp1MB));
    Label right_tree = a.new_label();
    a.b_hs(resolve_label(right_tree, disp1MB));
    emit_binsearch_nodes(reg, Left, mid - 1, fail, args);
    bind_veneer_target(right_tree);
    emit_binsearch_nodes(reg, mid + 1, Right, fail, args);
}
void BeamModuleAssembler::emit_i_jump_on_val(const ArgSource &Src,
                                             const ArgVal &Fail,
                                             const ArgWord &Base,
                                             const ArgWord &Size,
                                             const Span<const ArgVal> &args) {
    Label fail;
    Label data = a.new_label();
    auto src = load_source(Src, TMP1);
    ASSERT(Size.get() == args.size());
    if (always_small(Src)) {
        comment("(skipped type test)");
    } else {
        a.and_(TMP3, src.reg, imm(_TAG_IMMED1_MASK));
        a.cmp(TMP3, imm(_TAG_IMMED1_SMALL));
        if (Fail.isLabel()) {
            a.b_ne(resolve_beam_label(Fail, disp1MB));
        } else {
            ASSERT(Fail.isNil());
            fail = a.new_label();
            a.b_ne(fail);
        }
    }
    a.asr(TMP1, src.reg, imm(_TAG_IMMED1_SIZE));
    if (Base.get() != 0) {
        if (Support::is_uint_n<12>((Sint)Base.get())) {
            a.sub(TMP1, TMP1, imm(Base.get()));
        } else {
            mov_imm(TMP3, Base.get());
            a.sub(TMP1, TMP1, TMP3);
        }
    }
    cmp(TMP1, args.size());
    if (Fail.isLabel()) {
        a.b_hs(resolve_beam_label(Fail, disp1MB));
    } else {
        a.b_hs(fail);
    }
    bool embedInText = args.size() <= 6;
    if (embedInText) {
        a.adr(TMP2, data);
    } else {
        embed_vararg_rodata(args, TMP2);
    }
    a.ldr(TMP3, a64::Mem(TMP2, TMP1, a64::lsl(3)));
    a.br(TMP3);
    mark_unreachable_check_pending_stubs();
    a.bind(data);
    if (embedInText) {
        for (const ArgVal &arg : args) {
            ASSERT(arg.getType() == ArgVal::Type::Label);
            a.embed_label(rawLabels[arg.as<ArgLabel>().get()]);
        }
    }
    if (Fail.getType() == ArgVal::Type::Immediate) {
        a.bind(fail);
    }
}
void BeamModuleAssembler::emit_optimized_two_way_select(a64::Gp reg,
                                                        const ArgVal &value1,
                                                        const ArgVal &value2,
                                                        const ArgVal &label) {
    uint64_t x = value1.isImmed() ? value1.as<ArgImmed>().get()
                                  : value1.as<ArgWord>().get();
    uint64_t y = value2.isImmed() ? value2.as<ArgImmed>().get()
                                  : value2.as<ArgWord>().get();
    uint64_t diff = x ^ y;
    a64::Gp tmp = TMP6;
    if (x + 1 == y) {
        comment("(Src == %ld || Src == %ld) <=> (Src - %ld) < 2", x, y, x);
        if (x == 0) {
            a.cmp(reg, imm(2));
        } else {
            sub(tmp, reg, x);
            a.cmp(tmp, imm(2));
        }
        a.b_lo(resolve_beam_label(label, disp1MB));
    } else if ((diff & (diff - 1)) == 0) {
        uint64_t combined = x | y;
        ArgWord val(combined);
        comment("(Src == 0x%x || Src == 0x%x) <=> (Src | 0x%x) == 0x%x",
                x,
                y,
                diff,
                combined);
        a.orr(tmp, reg, imm(diff));
        cmp_arg(tmp, val);
        a.b_eq(resolve_beam_label(label, disp1MB));
    } else {
        if (x < 32) {
            cmp(reg, y);
            a.ccmp(reg, imm(x), imm(NZCV::kEqual), imm(arm::CondCode::kNE));
        } else if (-y < 32) {
            cmp(reg, x);
            a.ccmn(reg, imm(-y), imm(NZCV::kEqual), imm(arm::CondCode::kNE));
        } else {
            cmp(reg, x);
            a.mov(tmp, y);
            a.ccmp(reg, tmp, imm(NZCV::kEqual), imm(arm::CondCode::kNE));
        }
        a.b_eq(resolve_beam_label(label, disp1MB));
    }
}
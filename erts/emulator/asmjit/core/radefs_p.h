#ifndef ASMJIT_CORE_RADEFS_P_H_INCLUDED
#define ASMJIT_CORE_RADEFS_P_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER
#include <asmjit/core/archtraits.h>
#include <asmjit/core/builder.h>
#include <asmjit/core/compilerdefs.h>
#include <asmjit/core/logger.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenabitset_p.h>
#include <asmjit/support/arenavector.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
#ifndef ASMJIT_NO_LOGGING
# define ASMJIT_RA_LOG_FORMAT(...)  \
  do {                              \
    if (ASMJIT_UNLIKELY(logger))    \
      logger->logf(__VA_ARGS__);    \
  } while (0)
# define ASMJIT_RA_LOG_COMPLEX(...) \
  do {                              \
    if (ASMJIT_UNLIKELY(logger)) {  \
      __VA_ARGS__                   \
    }                               \
  } while (0)
#else
# define ASMJIT_RA_LOG_FORMAT(...) ((void)0)
# define ASMJIT_RA_LOG_COMPLEX(...) ((void)0)
#endif
struct RATiedReg;
class BaseRAPass;
class RABlock;
class BaseNode;
struct RAStackSlot;
using RAWorkRegVector = ArenaVector<RAWorkReg*>;
enum class RAWorkId : uint32_t {};
enum class RABlockId : uint32_t {};
static constexpr RAWorkId kBadWorkId = RAWorkId(Globals::kInvalidId);
static constexpr RABlockId kBadBlockId = RABlockId(Globals::kInvalidId);
static constexpr uint32_t kMaxConsecutiveRegs = 4;
enum class RAStrategyType : uint8_t {
  kSimple  = 0,
  kComplex = 1
};
ASMJIT_DEFINE_ENUM_COMPARE(RAStrategyType)
enum class RAStrategyFlags : uint8_t {
  kNone = 0
};
ASMJIT_DEFINE_ENUM_FLAGS(RAStrategyFlags)
struct RAStrategy {
  RAStrategyType _type = RAStrategyType::kSimple;
  RAStrategyFlags _flags = RAStrategyFlags::kNone;
  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _type = RAStrategyType::kSimple;
    _flags = RAStrategyFlags::kNone;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAStrategyType type() const noexcept { return _type; }
  ASMJIT_INLINE_NODEBUG void set_type(RAStrategyType type) noexcept { _type = type; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_simple() const noexcept { return _type == RAStrategyType::kSimple; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_complex() const noexcept { return _type >= RAStrategyType::kComplex; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAStrategyFlags flags() const noexcept { return _flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(RAStrategyFlags flag) const noexcept { return Support::test(_flags, flag); }
  ASMJIT_INLINE_NODEBUG void add_flags(RAStrategyFlags flags) noexcept { _flags |= flags; }
};
struct RARegCount {
  uint32_t _counters;
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _counters = 0; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const RARegCount& other) const noexcept { return _counters == other._counters; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const RARegCount& other) const noexcept { return _counters != other._counters; }
  [[nodiscard]]
  ASMJIT_INLINE uint32_t get(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    uint32_t shift = uint32_t(group) * 8u;
    return (_counters >> shift) & 0xFFu;
  }
  ASMJIT_INLINE void set(RegGroup group, uint32_t n) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    ASMJIT_ASSERT(n <= 0xFFu);
    uint32_t shift = uint32_t(group) * 8u;
    _counters = (_counters & ~uint32_t(0xFFu << shift)) + (n << shift);
  }
  ASMJIT_INLINE void add(RegGroup group, uint32_t n = 1) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    ASMJIT_ASSERT(get(group) + n <= 0xFFu);
    uint32_t shift = uint32_t(group) * 8u;
    _counters += n << shift;
  }
};
struct RARegIndex : public RARegCount {
  ASMJIT_INLINE void build_indexes(const RARegCount& count) noexcept {
    ASMJIT_ASSERT(count.get(RegGroup(0)) + count.get(RegGroup(1)) <= 0xFFu);
    ASMJIT_ASSERT(count.get(RegGroup(0)) + count.get(RegGroup(1)) + count.get(RegGroup(2)) <= 0xFFu);
    uint32_t i = count._counters;
    _counters = (i + (i << 8u) + (i << 16)) << 8u;
  }
};
struct RARegMask {
  using RegMasks = Support::Array<RegMask, Globals::kNumVirtGroups>;
  RegMasks _masks;
  ASMJIT_INLINE_NODEBUG void init(const RARegMask& other) noexcept { _masks = other._masks; }
  ASMJIT_INLINE_NODEBUG void init(const RegMasks& masks) noexcept { _masks = masks; }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _masks.fill(0); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const RARegMask& other) const noexcept { return _masks == other._masks; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const RARegMask& other) const noexcept { return _masks != other._masks; }
  template<typename Index>
  [[nodiscard]]
  inline uint32_t& operator[](const Index& index) noexcept { return _masks[index]; }
  template<typename Index>
  [[nodiscard]]
  inline const uint32_t& operator[](const Index& index) const noexcept { return _masks[index]; }
  [[nodiscard]]
  inline bool is_empty() const noexcept {
    return _masks.aggregate<Support::Or>() == 0;
  }
  [[nodiscard]]
  inline bool has(RegGroup group, RegMask mask = 0xFFFFFFFFu) const noexcept {
    return (_masks[group] & mask) != 0;
  }
  template<class Operator>
  inline void op(const RARegMask& other) noexcept {
    _masks.combine<Operator>(other._masks);
  }
  template<class Operator>
  inline void op(RegGroup group, RegMask mask) noexcept {
    _masks[group] = Operator::op(_masks[group], mask);
  }
  inline void clear(RegGroup group, RegMask mask) noexcept {
    _masks[group] = _masks[group] & ~mask;
  }
  inline void clear(const RegMasks& masks) noexcept {
    _masks.combine<Support::AndNot>(masks);
  }
};
class RARegsStats {
public:
  enum Index : uint32_t {
    kIndexUsed       = 0,
    kIndexFixed      = 8,
    kIndexClobbered  = 16
  };
  enum Mask : uint32_t {
    kMaskUsed        = 0xFFu << kIndexUsed,
    kMaskFixed       = 0xFFu << kIndexFixed,
    kMaskClobbered   = 0xFFu << kIndexClobbered
  };
  uint32_t _packed = 0;
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _packed = 0; }
  ASMJIT_INLINE_NODEBUG void combine_with(const RARegsStats& other) noexcept { _packed |= other._packed; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_used() const noexcept { return (_packed & kMaskUsed) != 0u; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_used(RegGroup group) const noexcept { return Support::bit_test(_packed, kIndexUsed + uint32_t(group)); }
  ASMJIT_INLINE_NODEBUG void make_used(RegGroup group) noexcept { _packed |= Support::bit_mask<uint32_t>(kIndexUsed + uint32_t(group)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_fixed() const noexcept { return (_packed & kMaskFixed) != 0u; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_fixed(RegGroup group) const noexcept { return Support::bit_test(_packed, kIndexFixed + uint32_t(group)); }
  ASMJIT_INLINE_NODEBUG void make_fixed(RegGroup group) noexcept { _packed |= Support::bit_mask<uint32_t>(kIndexFixed + uint32_t(group)); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_clobbered() const noexcept { return (_packed & kMaskClobbered) != 0u; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_clobbered(RegGroup group) const noexcept { return Support::bit_test(_packed, kIndexClobbered + uint32_t(group)); }
  ASMJIT_INLINE_NODEBUG void make_clobbered(RegGroup group) noexcept { _packed |= Support::bit_mask<uint32_t>(kIndexClobbered + uint32_t(group)); }
};
class RALiveCount {
public:
  Support::Array<uint32_t, Globals::kNumVirtGroups> n {};
  ASMJIT_INLINE_NODEBUG RALiveCount() noexcept = default;
  ASMJIT_INLINE_NODEBUG RALiveCount(const RALiveCount& other) noexcept = default;
  ASMJIT_INLINE_NODEBUG void init(const RALiveCount& other) noexcept { n = other.n; }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { n.fill(0); }
  ASMJIT_INLINE_NODEBUG RALiveCount& operator=(const RALiveCount& other) noexcept = default;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t& operator[](RegGroup group) noexcept { return n[group]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint32_t& operator[](RegGroup group) const noexcept { return n[group]; }
  template<class Operator>
  inline void op(const RALiveCount& other) noexcept { n.combine<Operator>(other.n); }
};
struct RALiveSpan {
  static inline constexpr NodePosition kNaN = NodePosition(0);
  static inline constexpr NodePosition kInf = NodePosition(0xFFFFFFFFu);
  NodePosition a {};
  NodePosition b {};
  ASMJIT_INLINE_NODEBUG RALiveSpan() noexcept = default;
  ASMJIT_INLINE_NODEBUG RALiveSpan(const RALiveSpan& other) noexcept = default;
  ASMJIT_INLINE_NODEBUG RALiveSpan(NodePosition a, NodePosition b) noexcept : a(a), b(b) {}
  ASMJIT_INLINE_NODEBUG void init(NodePosition first, NodePosition last) noexcept {
    a = first;
    b = last;
  }
  ASMJIT_INLINE_NODEBUG void init(const RALiveSpan& other) noexcept { init(other.a, other.b); }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { init(NodePosition(0), NodePosition(0)); }
  ASMJIT_INLINE_NODEBUG RALiveSpan& operator=(const RALiveSpan& other) = default;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_valid() const noexcept { return a < b; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t width() const noexcept { return uint32_t(b) - uint32_t(a); }
};
class RALiveSpans {
public:
  ASMJIT_NONCOPYABLE(RALiveSpans)
  ArenaVector<RALiveSpan> _data {};
  ASMJIT_INLINE_NODEBUG RALiveSpans() noexcept = default;
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _data.reset(); }
  ASMJIT_INLINE_NODEBUG void release(Arena& arena) noexcept { _data.release(arena); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _data.is_empty(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _data.size(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RALiveSpan* data() noexcept { return _data.data(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RALiveSpan* data() const noexcept { return _data.data(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_open() const noexcept {
    size_t size = _data.size();
    return size && _data[size - 1].b == RALiveSpan::kInf;
  }
  ASMJIT_INLINE_NODEBUG void swap(RALiveSpans& other) noexcept { _data.swap(other._data); }
  ASMJIT_INLINE Error open_at(Arena& arena, NodePosition start, NodePosition end) noexcept {
    bool was_open;
    return open_at(arena, start, end, was_open);
  }
  ASMJIT_INLINE Error open_at(Arena& arena, NodePosition start, NodePosition end, bool& was_open) noexcept {
    size_t size = _data.size();
    was_open = false;
    if (size > 0) {
      RALiveSpan& last = _data[size - 1];
      if (last.b >= start) {
        was_open = last.b > start;
        last.b = end;
        return Error::kOk;
      }
    }
    return _data.append(arena, RALiveSpan(start, end));
  }
  ASMJIT_INLINE void close_at(NodePosition end) noexcept {
    ASMJIT_ASSERT(!is_empty());
    size_t size = _data.size();
    _data[size - 1u].b = end;
  }
  inline uint32_t width() const noexcept {
    uint32_t width = 0;
    for (const RALiveSpan& span : _data)
      width += span.width();
    return width;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RALiveSpan& operator[](uint32_t index) noexcept { return _data[index]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RALiveSpan& operator[](uint32_t index) const noexcept { return _data[index]; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool intersects(const RALiveSpans& other) const noexcept {
    return intersects(*this, other);
  }
  [[nodiscard]]
  ASMJIT_INLINE Error non_overlapping_union_of(Arena& arena, const RALiveSpans& x, const RALiveSpans& y) noexcept {
    size_t final_size = x.size() + y.size();
    ASMJIT_PROPAGATE(_data.reserve_grow(arena, final_size));
    RALiveSpan* dst_ptr = _data.data();
    const RALiveSpan* x_span = x.data();
    const RALiveSpan* y_span = y.data();
    const RALiveSpan* x_end = x_span + x.size();
    const RALiveSpan* y_end = y_span + y.size();
    if (x_span != x_end && y_span != y_end) {
      NodePosition xa, ya;
      xa = x_span->a;
      for (;;) {
        while (y_span->b <= xa) {
          dst_ptr->init(*y_span);
          dst_ptr++;
          if (++y_span == y_end) {
            goto Done;
          }
        }
        ya = y_span->a;
        while (x_span->b <= ya) {
          *dst_ptr++ = *x_span;
          if (++x_span == x_end) {
            goto Done;
          }
        }
        xa = x_span->a;
        if (y_span->b > xa) {
          return Error::kByPass;
        }
      }
    }
  Done:
    while (x_span != x_end) {
      *dst_ptr++ = *x_span++;
    }
    while (y_span != y_end) {
      dst_ptr->init(*y_span);
      dst_ptr++;
      y_span++;
    }
    _data._set_end(dst_ptr);
    return Error::kOk;
  }
  [[nodiscard]]
  static ASMJIT_INLINE bool intersects(const RALiveSpans& x, const RALiveSpans& y) noexcept {
    const RALiveSpan* x_span = x.data();
    const RALiveSpan* y_span = y.data();
    const RALiveSpan* x_end = x_span + x.size();
    const RALiveSpan* y_end = y_span + y.size();
    if (x_span == x_end || y_span == y_end) {
      return false;
    }
    NodePosition xa, ya;
    xa = x_span->a;
    for (;;) {
      while (y_span->b <= xa) {
        if (++y_span == y_end) {
          return false;
        }
      }
      ya = y_span->a;
      while (x_span->b <= ya) {
        if (++x_span == x_end) {
          return false;
        }
      }
      xa = x_span->a;
      if (y_span->b > xa) {
        return true;
      }
    }
  }
};
class RALiveStats {
public:
  uint32_t _width = 0;
  float _freq = 0.0f;
  float _priority = 0.0f;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t width() const noexcept { return _width; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG float freq() const noexcept { return _freq; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG float priority() const noexcept { return _priority; }
};
enum class RATiedFlags : uint32_t {
  kNone = 0,
  kRead = uint32_t(OpRWFlags::kRead),
  kWrite = uint32_t(OpRWFlags::kWrite),
  kRW = uint32_t(OpRWFlags::kRW),
  kUse = 0x00000004u,
  kOut = 0x00000008u,
  kUseRM = 0x00000010u,
  kOutRM = 0x00000020u,
  kUseFixed = 0x00000040u,
  kOutFixed = 0x00000080u,
  kUseDone = 0x00000100u,
  kOutDone = 0x00000200u,
  kUseConsecutive = 0x00000400u,
  kOutConsecutive = 0x00000800u,
  kLeadConsecutive = 0x00001000u,
  kConsecutiveData = 0x00006000u,
  kUnique = 0x00008000u,
  kDuplicate = 0x00010000u,
  kFirst = 0x00020000u,
  kLast = 0x00040000u,
  kKill = 0x00080000u,
  kX86_Gpb = 0x01000000u,
  kInst_RegToMemPatched = 0x40000000u,
  kInst_IsTransformable = 0x80000000u
};
ASMJIT_DEFINE_ENUM_FLAGS(RATiedFlags)
static_assert(uint32_t(RATiedFlags::kRead ) == 0x1, "RATiedFlags::kRead must be 0x1");
static_assert(uint32_t(RATiedFlags::kWrite) == 0x2, "RATiedFlags::kWrite must be 0x2");
static_assert(uint32_t(RATiedFlags::kRW   ) == 0x3, "RATiedFlags::kRW must be 0x3");
struct RATiedReg {
  RAWorkReg* _work_reg;
  RAWorkReg* _consecutive_parent;
  RATiedFlags _flags;
  union {
    struct {
      uint8_t _ref_count;
      uint8_t _rm_size;
      uint8_t _use_id;
      uint8_t _out_id;
    };
    uint32_t _packed;
  };
  RegMask _use_reg_mask;
  RegMask _out_reg_mask;
  uint32_t _use_rewrite_mask;
  uint32_t _out_rewrite_mask;
  static inline RATiedFlags consecutive_data_to_flags(uint32_t offset) noexcept {
    ASMJIT_ASSERT(offset < 4);
    constexpr uint32_t kOffsetShift = Support::ctz_const<RATiedFlags::kConsecutiveData>;
    return (RATiedFlags)(offset << kOffsetShift);
  }
  static inline uint32_t consecutive_data_from_flags(RATiedFlags flags) noexcept {
    constexpr uint32_t kOffsetShift = Support::ctz_const<RATiedFlags::kConsecutiveData>;
    return uint32_t(flags & RATiedFlags::kConsecutiveData) >> kOffsetShift;
  }
  inline void init(RAWorkReg* work_reg, RATiedFlags flags, RegMask use_reg_mask, uint32_t use_id, uint32_t use_rewrite_mask, RegMask out_reg_mask, uint32_t out_id, uint32_t out_rewrite_mask, uint32_t rm_size = 0, RAWorkReg* consecutive_parent = nullptr) noexcept {
    _work_reg = work_reg;
    _consecutive_parent = consecutive_parent;
    _flags = flags;
    _ref_count = 1;
    _rm_size = uint8_t(rm_size);
    _use_id = uint8_t(use_id);
    _out_id = uint8_t(out_id);
    _use_reg_mask = use_reg_mask;
    _out_reg_mask = out_reg_mask;
    _use_rewrite_mask = use_rewrite_mask;
    _out_rewrite_mask = out_rewrite_mask;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* work_reg() const noexcept { return _work_reg; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_consecutive_parent() const noexcept { return _consecutive_parent != nullptr; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* consecutive_parent() const noexcept { return _consecutive_parent; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t consecutive_data() const noexcept { return consecutive_data_from_flags(_flags); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedFlags flags() const noexcept { return _flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(RATiedFlags flag) const noexcept { return Support::test(_flags, flag); }
  ASMJIT_INLINE_NODEBUG void add_flags(RATiedFlags flags) noexcept { _flags |= flags; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read() const noexcept { return has_flag(RATiedFlags::kRead); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_write() const noexcept { return has_flag(RATiedFlags::kWrite); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read_only() const noexcept { return (_flags & RATiedFlags::kRW) == RATiedFlags::kRead; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_write_only() const noexcept { return (_flags & RATiedFlags::kRW) == RATiedFlags::kWrite; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read_write() const noexcept { return (_flags & RATiedFlags::kRW) == RATiedFlags::kRW; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_use() const noexcept { return has_flag(RATiedFlags::kUse); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_out() const noexcept { return has_flag(RATiedFlags::kOut); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_lead_consecutive() const noexcept { return has_flag(RATiedFlags::kLeadConsecutive); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_use_consecutive() const noexcept { return has_flag(RATiedFlags::kUseConsecutive); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_out_consecutive() const noexcept { return has_flag(RATiedFlags::kOutConsecutive); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_unique() const noexcept { return has_flag(RATiedFlags::kUnique); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_any_consecutive_flag() const noexcept { return has_flag(RATiedFlags::kLeadConsecutive | RATiedFlags::kUseConsecutive | RATiedFlags::kOutConsecutive); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_use_rm() const noexcept { return has_flag(RATiedFlags::kUseRM); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_out_rm() const noexcept { return has_flag(RATiedFlags::kOutRM); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t rm_size() const noexcept { return _rm_size; }
  inline void make_read_only() noexcept {
    _flags = (_flags & ~(RATiedFlags::kOut | RATiedFlags::kWrite)) | RATiedFlags::kUse;
    _use_rewrite_mask |= _out_rewrite_mask;
    _out_rewrite_mask = 0;
  }
  inline void make_write_only() noexcept {
    _flags = (_flags & ~(RATiedFlags::kUse | RATiedFlags::kRead)) | RATiedFlags::kOut;
    _out_rewrite_mask |= _use_rewrite_mask;
    _use_rewrite_mask = 0;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_duplicate() const noexcept { return has_flag(RATiedFlags::kDuplicate); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_first() const noexcept { return has_flag(RATiedFlags::kFirst); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_last() const noexcept { return has_flag(RATiedFlags::kLast); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_kill() const noexcept { return has_flag(RATiedFlags::kKill); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_out_or_kill() const noexcept { return has_flag(RATiedFlags::kOut | RATiedFlags::kKill); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask use_reg_mask() const noexcept { return _use_reg_mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask out_reg_mask() const noexcept { return _out_reg_mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t ref_count() const noexcept { return _ref_count; }
  ASMJIT_INLINE_NODEBUG void add_ref_count(uint32_t n = 1) noexcept { _ref_count = uint8_t(_ref_count + n); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_use_id() const noexcept { return _use_id != Reg::kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_out_id() const noexcept { return _out_id != Reg::kIdBad; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t use_id() const noexcept { return _use_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t out_id() const noexcept { return _out_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t use_rewrite_mask() const noexcept { return _use_rewrite_mask; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t out_rewrite_mask() const noexcept { return _out_rewrite_mask; }
  ASMJIT_INLINE_NODEBUG void set_use_id(uint32_t index) noexcept { _use_id = uint8_t(index); }
  ASMJIT_INLINE_NODEBUG void set_out_id(uint32_t index) noexcept { _out_id = uint8_t(index); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_use_done() const noexcept { return has_flag(RATiedFlags::kUseDone); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_out_done() const noexcept { return has_flag(RATiedFlags::kOutDone); }
  ASMJIT_INLINE_NODEBUG void mark_use_done() noexcept { add_flags(RATiedFlags::kUseDone); }
  ASMJIT_INLINE_NODEBUG void mark_out_done() noexcept { add_flags(RATiedFlags::kOutDone); }
};
ASMJIT_END_NAMESPACE
#endif
#endif
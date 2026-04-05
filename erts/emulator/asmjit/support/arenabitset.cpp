#include <asmjit/core/api-build_p.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenabitset_p.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
Error ArenaBitSet::copy_from(Arena& arena, const ArenaBitSet& other) noexcept {
  BitWord* data = _data;
  size_t new_size = other.size();
  if (!new_size) {
    _size = 0;
    return Error::kOk;
  }
  if (new_size > _capacity) {
    size_t minimum_capacity_in_bits = Support::align_up<size_t>(new_size, Support::bit_size_of<BitWord>);
    if (ASMJIT_UNLIKELY(minimum_capacity_in_bits < new_size)) {
      return make_error(Error::kOutOfMemory);
    }
    size_t minimum_capacity = minimum_capacity_in_bits / 8u;
    size_t allocated_capacity;
    BitWord* new_data = static_cast<BitWord*>(arena.alloc_reusable(minimum_capacity, Out(allocated_capacity)));
    if (ASMJIT_UNLIKELY(!new_data)) {
      return make_error(Error::kOutOfMemory);
    }
    size_t allocated_capacity_in_bits = allocated_capacity * 8;
    if (ASMJIT_UNLIKELY(allocated_capacity_in_bits < allocated_capacity)) {
      allocated_capacity_in_bits = minimum_capacity_in_bits;
    }
    if (data) {
      arena.free_reusable(data, _capacity / 8);
    }
    data = new_data;
    _data = data;
    _capacity = uint32_t(allocated_capacity_in_bits);
  }
  _size = uint32_t(new_size);
  _copy_bits(data, other.data(), _words_per_bits(uint32_t(new_size)));
  return Error::kOk;
}
Error ArenaBitSet::_resize(Arena& arena, size_t new_size, size_t ideal_capacity, bool new_bits_value) noexcept {
  ASMJIT_ASSERT(ideal_capacity >= new_size);
  if (new_size <= _size) {
    size_t idx = new_size / Support::bit_size_of<BitWord>;
    size_t bit = new_size % Support::bit_size_of<BitWord>;
    if (bit) {
      _data[idx] &= (BitWord(1) << bit) - 1u;
    }
    _size = uint32_t(new_size);
    return Error::kOk;
  }
  size_t old_size = _size;
  BitWord* data = _data;
  if (new_size > _capacity) {
    size_t minimum_capacity_in_bits = Support::align_up(ideal_capacity, Support::bit_size_of<BitWord>);
    if (ASMJIT_UNLIKELY(minimum_capacity_in_bits < new_size)) {
      return make_error(Error::kOutOfMemory);
    }
    size_t minimum_capacity = minimum_capacity_in_bits / 8u;
    size_t allocated_capacity;
    BitWord* new_data = static_cast<BitWord*>(arena.alloc_reusable(minimum_capacity, Out(allocated_capacity)));
    if (ASMJIT_UNLIKELY(!new_data)) {
      return make_error(Error::kOutOfMemory);
    }
    size_t allocated_capacity_in_bits = allocated_capacity * 8u;
    if (ASMJIT_UNLIKELY(allocated_capacity_in_bits < allocated_capacity)) {
      allocated_capacity_in_bits = minimum_capacity_in_bits;
    }
    _copy_bits(new_data, data, _words_per_bits(old_size));
    if (data) {
      arena.free_reusable(data, _capacity / 8);
    }
    data = new_data;
    _data = data;
    _capacity = uint32_t(allocated_capacity_in_bits);
  }
  size_t idx = old_size / Support::bit_size_of<BitWord>;
  size_t start_bit = old_size % Support::bit_size_of<BitWord>;
  size_t end_bit = new_size % Support::bit_size_of<BitWord>;
  BitWord pattern = Support::bool_as_mask<BitWord>(new_bits_value);
  if (start_bit) {
    size_t num_bits = 0;
    if (idx == (new_size / Support::bit_size_of<BitWord>)) {
      ASMJIT_ASSERT(start_bit < end_bit);
      num_bits = end_bit - start_bit;
    }
    else {
      num_bits = Support::bit_size_of<BitWord> - start_bit;
    }
    data[idx++] |= pattern << num_bits;
  }
  size_t end_index = _words_per_bits(new_size);
  while (idx < end_index) {
    data[idx++] = pattern;
  }
  if (end_bit) {
    data[end_index - 1] = pattern & ((BitWord(1) << end_bit) - 1);
  }
  _size = uint32_t(new_size);
  return Error::kOk;
}
Error ArenaBitSet::_append(Arena& arena, bool value) noexcept {
  constexpr uint32_t kThreshold = Globals::kGrowThreshold * 8u;
  uint32_t new_size = _size + 1;
  uint32_t ideal_capacity = _capacity;
  if (ideal_capacity < 128) {
    ideal_capacity = 128;
  }
  else if (ideal_capacity <= kThreshold) {
    ideal_capacity *= 2;
  }
  else {
    ideal_capacity += kThreshold;
  }
  if (ASMJIT_UNLIKELY(ideal_capacity < _capacity)) {
    if (ASMJIT_UNLIKELY(_size == std::numeric_limits<uint32_t>::max())) {
      return make_error(Error::kOutOfMemory);
    }
    ideal_capacity = new_size;
  }
  return _resize(arena, new_size, ideal_capacity, value);
}
#if defined(ASMJIT_TEST)
static void test_arena_bitvector(Arena& arena) {
  uint32_t i, count;
  uint32_t kMaxCount = 100;
  ArenaBitSet vec;
  EXPECT_TRUE(vec.is_empty());
  EXPECT_EQ(vec.size(), 0u);
  INFO("ArenaBitSet::resize()");
  for (count = 1; count < kMaxCount; count++) {
    vec.clear();
    EXPECT_EQ(vec.resize(arena, count, false), Error::kOk);
    EXPECT_EQ(vec.size(), count);
    for (i = 0; i < count; i++) {
      EXPECT_FALSE(vec.bit_at(i));
    }
    vec.clear();
    EXPECT_EQ(vec.resize(arena, count, true), Error::kOk);
    EXPECT_EQ(vec.size(), count);
    for (i = 0; i < count; i++) {
      EXPECT_TRUE(vec.bit_at(i));
    }
  }
  INFO("ArenaBitSet::fill_bits() / clear_bits()");
  for (count = 1; count < kMaxCount; count += 2) {
    vec.clear();
    EXPECT_EQ(vec.resize(arena, count), Error::kOk);
    EXPECT_EQ(vec.size(), count);
    for (i = 0; i < (count + 1) / 2; i++) {
      bool value = bool(i & 1);
      if (value) {
        vec.fill_bits(i, count - i * 2);
      }
      else {
        vec.clear_bits(i, count - i * 2);
      }
    }
    for (i = 0; i < count; i++) {
      EXPECT_EQ(vec.bit_at(i), bool(i & 1));
    }
  }
}
UNIT(arena_bitvector, -1) {
  Arena arena(8192);
  test_arena_bitvector(arena);
}
#endif
ASMJIT_END_NAMESPACE
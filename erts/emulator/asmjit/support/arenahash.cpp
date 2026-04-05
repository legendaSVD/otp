#include <asmjit/core/api-build_p.h>
#include <asmjit/support/support.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenahash.h>
ASMJIT_BEGIN_NAMESPACE
#define ASMJIT_POPULATE_PRIMES(ENTRY) \
  ENTRY(2         , 0x80000000, 32),  \
  ENTRY(11        , 0xBA2E8BA3, 35),  \
  ENTRY(29        , 0x8D3DCB09, 36),  \
  ENTRY(41        , 0xC7CE0C7D, 37),  \
  ENTRY(59        , 0x8AD8F2FC, 37),  \
  ENTRY(83        , 0xC565C87C, 38),  \
  ENTRY(131       , 0xFA232CF3, 39),  \
  ENTRY(191       , 0xAB8F69E3, 39),  \
  ENTRY(269       , 0xF3A0D52D, 40),  \
  ENTRY(383       , 0xAB1CBDD4, 40),  \
  ENTRY(541       , 0xF246FACC, 41),  \
  ENTRY(757       , 0xAD2589A4, 41),  \
  ENTRY(1061      , 0xF7129426, 42),  \
  ENTRY(1499      , 0xAEE116B7, 42),  \
  ENTRY(2099      , 0xF9C7A737, 43),  \
  ENTRY(2939      , 0xB263D25C, 43),  \
  ENTRY(4111      , 0xFF10E02E, 44),  \
  ENTRY(5779      , 0xB5722823, 44),  \
  ENTRY(8087      , 0x81A97405, 44),  \
  ENTRY(11321     , 0xB93E91DB, 45),  \
  ENTRY(15859     , 0x843CC26B, 45),  \
  ENTRY(22189     , 0xBD06B9EA, 46),  \
  ENTRY(31051     , 0x8713F186, 46),  \
  ENTRY(43451     , 0xC10F1CB9, 47),  \
  ENTRY(60869     , 0x89D06A86, 47),  \
  ENTRY(85159     , 0xC502AF3B, 48),  \
  ENTRY(102107    , 0xA44F65AE, 48),  \
  ENTRY(122449    , 0x89038F77, 48),  \
  ENTRY(146819    , 0xE48AF7E9, 49),  \
  ENTRY(176041    , 0xBE9B145B, 49),  \
  ENTRY(211073    , 0x9EF882BA, 49),  \
  ENTRY(253081    , 0x849571AB, 49),  \
  ENTRY(303469    , 0xDD239C97, 50),  \
  ENTRY(363887    , 0xB86C196D, 50),  \
  ENTRY(436307    , 0x99CFA4E9, 50),  \
  ENTRY(523177    , 0x804595C0, 50),  \
  ENTRY(627293    , 0xD5F69FCF, 51),  \
  ENTRY(752177    , 0xB27063BA, 51),  \
  ENTRY(901891    , 0x94D170AC, 51),  \
  ENTRY(1081369   , 0xF83C9767, 52),  \
  ENTRY(1296563   , 0xCF09435D, 52),  \
  ENTRY(1554583   , 0xACAC7198, 52),  \
  ENTRY(1863971   , 0x90033EE3, 52),  \
  ENTRY(2234923   , 0xF0380EBD, 53),  \
  ENTRY(2679673   , 0xC859731E, 53),  \
  ENTRY(3212927   , 0xA718DE27, 53),  \
  ENTRY(3852301   , 0x8B5D1B4B, 53),  \
  ENTRY(4618921   , 0xE8774804, 54),  \
  ENTRY(5076199   , 0xD386574E, 54),  \
  ENTRY(5578757   , 0xC0783FE1, 54),  \
  ENTRY(6131057   , 0xAF21B08F, 54),  \
  ENTRY(6738031   , 0x9F5AFD6E, 54),  \
  ENTRY(7405163   , 0x90FFC3B9, 54),  \
  ENTRY(8138279   , 0x83EFECFC, 54),  \
  ENTRY(8943971   , 0xF01AA2EF, 55),  \
  ENTRY(9829447   , 0xDA7979B2, 55),  \
  ENTRY(10802581  , 0xC6CB2771, 55),  \
  ENTRY(11872037  , 0xB4E2C7DD, 55),  \
  ENTRY(13047407  , 0xA4974124, 55),  \
  ENTRY(14339107  , 0x95C39CF1, 55),  \
  ENTRY(15758737  , 0x8845C763, 55),  \
  ENTRY(17318867  , 0xF7FE593F, 56),  \
  ENTRY(19033439  , 0xE1A75D93, 56),  \
  ENTRY(20917763  , 0xCD5389B3, 56),  \
  ENTRY(22988621  , 0xBAD4841A, 56),  \
  ENTRY(25264543  , 0xA9FFF2FF, 56),  \
  ENTRY(27765763  , 0x9AAF8BF3, 56),  \
  ENTRY(30514607  , 0x8CC04E18, 56),  \
  ENTRY(33535561  , 0x80127068, 56),  \
  ENTRY(36855587  , 0xE911F0BB, 57),  \
  ENTRY(38661533  , 0xDE2ED7BE, 57),  \
  ENTRY(40555961  , 0xD3CDF2FD, 57),  \
  ENTRY(42543269  , 0xC9E9196C, 57),  \
  ENTRY(44627909  , 0xC07A9EB6, 57),  \
  ENTRY(46814687  , 0xB77CEF65, 57),  \
  ENTRY(49108607  , 0xAEEAC65C, 57),  \
  ENTRY(51514987  , 0xA6BF0EF0, 57),  \
  ENTRY(54039263  , 0x9EF510B5, 57),  \
  ENTRY(56687207  , 0x97883B42, 57),  \
  ENTRY(59464897  , 0x907430ED, 57),  \
  ENTRY(62378699  , 0x89B4CA91, 57),  \
  ENTRY(65435273  , 0x83461568, 57),  \
  ENTRY(68641607  , 0xFA489AA8, 58),  \
  ENTRY(72005051  , 0xEE97B1C5, 58),  \
  ENTRY(75533323  , 0xE3729293, 58),  \
  ENTRY(79234469  , 0xD8D2BBA3, 58),  \
  ENTRY(83116967  , 0xCEB1F196, 58),  \
  ENTRY(87189709  , 0xC50A4426, 58),  \
  ENTRY(91462061  , 0xBBD6052B, 58),  \
  ENTRY(95943737  , 0xB30FD999, 58),  \
  ENTRY(100644991 , 0xAAB29CED, 58),  \
  ENTRY(105576619 , 0xA2B96421, 58),  \
  ENTRY(110749901 , 0x9B1F8434, 58),  \
  ENTRY(116176651 , 0x93E08B4A, 58),  \
  ENTRY(121869317 , 0x8CF837E0, 58),  \
  ENTRY(127840913 , 0x86627F01, 58),  \
  ENTRY(134105159 , 0x801B8178, 58),  \
  ENTRY(140676353 , 0xF43F294F, 59),  \
  ENTRY(147569509 , 0xE8D67089, 59),  \
  ENTRY(154800449 , 0xDDF6243C, 59),  \
  ENTRY(162385709 , 0xD397E6AE, 59),  \
  ENTRY(170342629 , 0xC9B5A65A, 59),  \
  ENTRY(178689419 , 0xC0499865, 59),  \
  ENTRY(187445201 , 0xB74E35FA, 59),  \
  ENTRY(196630033 , 0xAEBE3AC1, 59),  \
  ENTRY(206264921 , 0xA694A37F, 59),  \
  ENTRY(216371963 , 0x9ECCA59F, 59),  \
  ENTRY(226974197 , 0x9761B6AE, 59),  \
  ENTRY(238095983 , 0x904F79A1, 59),  \
  ENTRY(249762697 , 0x8991CD1F, 59),  \
  ENTRY(262001071 , 0x8324BCA5, 59),  \
  ENTRY(274839137 , 0xFA090732, 60),  \
  ENTRY(288306269 , 0xEE5B16ED, 60),  \
  ENTRY(302433337 , 0xE338CE49, 60),  \
  ENTRY(317252587 , 0xD89BABC0, 60),  \
  ENTRY(374358107 , 0xB790EF43, 60),  \
  ENTRY(441742621 , 0x9B908414, 60),  \
  ENTRY(521256293 , 0x83D596FA, 60),  \
  ENTRY(615082441 , 0xDF72B16E, 61),  \
  ENTRY(725797313 , 0xBD5CDB3B, 61),  \
  ENTRY(856440829 , 0xA07A14E9, 61),  \
  ENTRY(1010600209, 0x87FF5289, 61),  \
  ENTRY(1192508257, 0xE6810540, 62),  \
  ENTRY(1407159797, 0xC357A480, 62),  \
  ENTRY(1660448617, 0xA58B5B4F, 62),  \
  ENTRY(1959329399, 0x8C4AB55F, 62),  \
  ENTRY(2312008693, 0xEDC86320, 63),  \
  ENTRY(2728170257, 0xC982C4D2, 63),  \
  ENTRY(3219240923, 0xAAC599B6, 63)
struct HashPrime {
  uint32_t prime;
  uint32_t rcp;
};
static const HashPrime ArenaHash_prime_array[] = {
  #define E(PRIME, RCP, SHIFT) { PRIME, RCP }
  ASMJIT_POPULATE_PRIMES(E)
  #undef E
};
static const uint8_t ArenaHash_prime_shift[] = {
  #define E(PRIME, RCP, SHIFT) uint8_t(SHIFT)
  ASMJIT_POPULATE_PRIMES(E)
  #undef E
};
void ArenaHashBase::_rehash(Arena& arena, uint32_t prime_index) noexcept {
  ASMJIT_ASSERT(prime_index < ASMJIT_ARRAY_SIZE(ArenaHash_prime_array));
  uint32_t new_count = ArenaHash_prime_array[prime_index].prime;
  ArenaHashNode** old_data = _data;
  ArenaHashNode** new_data = reinterpret_cast<ArenaHashNode**>(arena.alloc_reusable_zeroed(size_t(new_count) * sizeof(ArenaHashNode*)));
  if (ASMJIT_UNLIKELY(new_data == nullptr)) {
    return;
  }
  uint32_t i;
  uint32_t old_count = _buckets_count;
  _data = new_data;
  _buckets_count = new_count;
  _buckets_grow = uint32_t(new_count * 0.9);
  _rcp_value = ArenaHash_prime_array[prime_index].rcp;
  _rcp_shift = ArenaHash_prime_shift[prime_index];
  _prime_index = uint8_t(prime_index);
  for (i = 0; i < old_count; i++) {
    ArenaHashNode* node = old_data[i];
    while (node) {
      ArenaHashNode* next = node->_hash_next;
      uint32_t hash_mod = _calc_mod(node->_hash_code);
      node->_hash_next = new_data[hash_mod];
      new_data[hash_mod] = node;
      node = next;
    }
  }
  if (old_data != _embedded) {
    arena.free_reusable(old_data, old_count * sizeof(ArenaHashNode*));
  }
}
ArenaHashNode* ArenaHashBase::_insert(Arena& arena, ArenaHashNode* node) noexcept {
  uint32_t hash_mod = _calc_mod(node->_hash_code);
  ArenaHashNode* next = _data[hash_mod];
  node->_hash_next = next;
  _data[hash_mod] = node;
  if (++_size > _buckets_grow) {
    uint32_t prime_index = Support::min<uint32_t>(_prime_index + 2, ASMJIT_ARRAY_SIZE(ArenaHash_prime_array) - 1);
    if (prime_index > _prime_index) {
      _rehash(arena, prime_index);
    }
  }
  return node;
}
ArenaHashNode* ArenaHashBase::_remove(Arena& arena, ArenaHashNode* node) noexcept {
  Support::maybe_unused(arena);
  uint32_t hash_mod = _calc_mod(node->_hash_code);
  ArenaHashNode** prev_ptr = &_data[hash_mod];
  ArenaHashNode* p = *prev_ptr;
  while (p) {
    if (p == node) {
      *prev_ptr = p->_hash_next;
      _size--;
      return node;
    }
    prev_ptr = &p->_hash_next;
    p = *prev_ptr;
  }
  return nullptr;
}
#if defined(ASMJIT_TEST)
struct MyHashNode : public ArenaHashNode {
  inline MyHashNode(uint32_t key) noexcept
    : ArenaHashNode(key),
      _key(key) {}
  uint32_t _key;
};
struct MyKeyMatcher {
  inline MyKeyMatcher(uint32_t key) noexcept
    : _key(key) {}
  inline uint32_t hash_code() const noexcept { return _key; }
  inline bool matches(const MyHashNode* node) const noexcept { return node->_key == _key; }
  uint32_t _key;
};
UNIT(arena_hash) {
  uint32_t kCount = BrokenAPI::has_arg("--quick") ? 1000 : 10000;
  Arena arena(4096);
  ArenaHash<MyHashNode> hash_table;
  uint32_t key;
  INFO("Inserting %u elements to HashTable", unsigned(kCount));
  for (key = 0; key < kCount; key++) {
    hash_table.insert(arena, arena.new_oneshot<MyHashNode>(key));
  }
  uint32_t count = kCount;
  INFO("Removing %u elements from HashTable and validating each operation", unsigned(kCount));
  do {
    MyHashNode* node;
    for (key = 0; key < count; key++) {
      node = hash_table.get(MyKeyMatcher(key));
      EXPECT_NOT_NULL(node);
      EXPECT_EQ(node->_key, key);
    }
    {
      count--;
      node = hash_table.get(MyKeyMatcher(count));
      hash_table.remove(arena, node);
      node = hash_table.get(MyKeyMatcher(count));
      EXPECT_NULL(node);
    }
  } while (count);
  EXPECT_TRUE(hash_table.is_empty());
}
#endif
ASMJIT_END_NAMESPACE
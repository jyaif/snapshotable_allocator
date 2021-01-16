#include "application/memory/fls.h"

#include <cassert>

#include "third_party/nex_test/src/test.h"

NEX_TEST(Fls, FindLastBitSet) {
  assert(FindLastBitSet(0) == 0);
  assert(FindLastBitSet(1) == 1);
  assert(FindLastBitSet(2) == 2);
  assert(FindLastBitSet(3) == 2);
  assert(FindLastBitSet(4) == 3);
  assert(FindLastBitSet(127) == 7);
  assert(FindLastBitSet(128) == 8);
  assert(FindLastBitSet(1023) == 10);
  assert(FindLastBitSet(1024) == 11);
}

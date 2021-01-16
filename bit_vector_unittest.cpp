#include "application/memory/bit_vector.h"

#include <cassert>

#include "third_party/nex_test/src/test.h"

NEX_TEST(Memory, BitVector) {
  {
    BitVector bv(10);
    int first = bv.GetFirstBitSetAndClear();
    assert(first == 0);
    first = bv.GetFirstBitSetAndClear();
    assert(first == 1);
    first = bv.GetFirstBitSetAndClear();
    assert(first == 2);
    first = bv.GetFirstBitSetAndClear();
    assert(first == 3);
    bv.SetBit(1);
    first = bv.GetFirstBitSetAndClear();
    assert(first == 1);
    first = bv.GetFirstBitSetAndClear();
    assert(first == 4);
  }

  {
    BitVector bv(100);
    for (int i = 0; i < 100; i++) {
      int first = bv.GetFirstBitSetAndClear();
      assert(first == i);
    }
    bv.SetBit(60);
    bv.SetBit(40);
    bv.SetBit(50);
    assert(bv.GetFirstBitSetAndClear() == 40);
    assert(bv.GetFirstBitSetAndClear() == 50);
    assert(bv.GetFirstBitSetAndClear() == 60);
  }
}

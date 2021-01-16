#include "application/memory/memory_snapshot_impl.h"

#include <array>
#include <cassert>
#include <cstring>

#include "third_party/nex_test/src/test.h"

NEX_TEST(Memory, Snapshot) {
  std::array<uint8_t, 10000> array;
  memset(array.data(), 'x', 10000);

  MemorySnapshotImpl msi;
  msi.AppendData(array.data() + 1, 1998);
  msi.AppendData(array.data() + 3000, 5000);

  memset(array.data(), 0, 10000);
  msi.RestoreEverything();
  assert(array[0] == 0);
  assert(array[1] == 'x');
  assert(array[1998] == 'x');
  assert(array[1999] == 0);

  assert(array[2999] == 0);
  assert(array[3000] == 'x');
  assert(array[7999] == 'x');
  assert(array[8000] == 0);
}

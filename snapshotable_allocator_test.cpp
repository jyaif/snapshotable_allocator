#include "application/memory/snapshotable_allocator.h"
#include "application/memory/snapshotable_allocator_impl.h"
#include "third_party/nex_test/src/test.h"

#include <cassert>
#include <cstdio>

template <class T>
void SnapshotTest() {
  T sm;

  auto snapshot1 = sm.NewEmptySnapshot();
  sm.SaveMemoryIntoSnapshot(*snapshot1);

  const char special_char_1 = 'k';
  const char special_char_2 = 'p';

  void* ptr = sm.Alloc(100);
  memset(ptr, special_char_1, 100);

  auto snapshot2 = sm.NewEmptySnapshot();
  sm.SaveMemoryIntoSnapshot(*snapshot2);

  sm.LoadMemoryFromSnapshot(*snapshot1);
  void* ptr2 = sm.Alloc(100);
  assert(ptr == ptr2);
  memset(ptr2, special_char_2, 100);

  sm.LoadMemoryFromSnapshot(*snapshot2);
  for (int i = 0; i < 100; i++) {
    assert(static_cast<char*>(ptr)[i] == special_char_1);
  }
}

template <class T>
void AllocAndFree() {
  T sm;

  // Check that you can free nullptr without crashing.
  sm.Free(nullptr);

  // Check that you can write to allocated memory.
  void* ptr = sm.Alloc(1000);
  memset(ptr, 0, 1000);

  // Check that the same allocation after a |free| returns the same pointer.
  sm.Free(ptr);
  void* ptr2 = sm.Alloc(1000);
  assert(ptr == ptr2);
}

template <class T>
void Realloc() {
  T sm;

  // Check that re-allocating the same size returns the same pointer and same
  // data.
  uint8_t* ptr = static_cast<uint8_t*>(sm.Alloc(100));
  for (int i = 0; i < 100; i++) {
    ptr[i] = i * 2;
  }
  uint8_t* ptr2 = static_cast<uint8_t*>(sm.Realloc(ptr, 100));
  assert(ptr == ptr2);
  for (int i = 0; i < 100; i++) {
    assert(ptr2[i] == i * 2);
  }

  // Check that re-allocating to a much larger size results in a new pointer
  // with memory copied.
  const char special_char = 'x';
  memset(ptr, special_char, 100);
  void* ptr3 = sm.Realloc(ptr2, 1000);
  assert(ptr3 != ptr);
  for (int i = 0; i < 100; i++) {
    assert(static_cast<char*>(ptr3)[i] == special_char);
  }
}

template <class T>
void ManyAllocations() {
  T sm;
  for (int i = 0; i < 1000; i++) {
    sm.Alloc(100);
  }

  auto snapshot = sm.NewEmptySnapshot();
  sm.SaveMemoryIntoSnapshot(*snapshot);
  sm.LoadMemoryFromSnapshot(*snapshot);
}

NEX_TEST(Memory, AllocAndFree) {
  AllocAndFree<SnapshotableAllocatorImpl>();
}

NEX_TEST(Memory, Realloc) {
  Realloc<SnapshotableAllocatorImpl>();
}

NEX_TEST(Memory, SnapshotTest) {
  SnapshotTest<SnapshotableAllocatorImpl>();
}

NEX_TEST(Memory, ManyAllocations) {
  ManyAllocations<SnapshotableAllocatorImpl>();
}

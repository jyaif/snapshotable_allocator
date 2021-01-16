#include "application/memory/scoped_allocator_usage.h"
#include "application/memory/snapshotable_allocator.h"
#include "application/memory/snapshotable_allocator_impl.h"
#include "third_party/nex_test/src/test.h"

#include <cassert>
#include <cstdio>

NEX_TEST(Memory, ScopedAllocatorUsage) {
  SnapshotableAllocatorImpl allocator(8);
  std::unique_ptr<std::string> s;

  // Initialize the string with "hello"
  {
    ScopedAllocatorUsage sau(&allocator);
    s = std::make_unique<std::string>();
    *s = "hello";
  }

  // Take a snapshot
  auto snapshot = allocator.TakeSnapshot();

  // Change the value of the string to "world"
  {
    ScopedAllocatorUsage sau(&allocator);
    *s = "world ";
  }

  // Verify the value of the string before and after the rollback.
  assert(*s == "world ");
  allocator.LoadMemoryFromSnapshot(*snapshot);
  assert(*s == "hello");

  // Free the memory
  {
    ScopedAllocatorUsage sau(&allocator);
    s.reset();
  }
}

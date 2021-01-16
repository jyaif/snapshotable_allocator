#ifndef APPLICATION_MEMORY_SNAPSHOTABLE_ALLOCATOR_FAKE_H_
#define APPLICATION_MEMORY_SNAPSHOTABLE_ALLOCATOR_FAKE_H_

#include <cassert>
#include <memory>

#include "application/memory/snapshotable_allocator.h"

class SnapshotableAllocatorFake : public SnapshotableAllocator {
 public:
  void* Realloc(void* ptr, size_t size) override { return realloc(ptr, size); }
  void* Alloc(size_t size) override { return malloc(size); }
  void Free(void* ptr) override { free(ptr); }

  void LoadMemoryFromSnapshot(MemorySnapshot const& snapshot) override {
    assert(false);
  }
  void SaveMemoryIntoSnapshot(MemorySnapshot& snapshot) override {}
  std::unique_ptr<MemorySnapshot> NewEmptySnapshot() override {
    return std::make_unique<MemorySnapshot>();
  }
};

#endif  // APPLICATION_MEMORY_SNAPSHOTABLE_ALLOCATOR_FAKE_H_

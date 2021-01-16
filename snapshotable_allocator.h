#ifndef APPLICATION_MEMORY_SNAPSHOTABLE_ALLOCATOR_H_
#define APPLICATION_MEMORY_SNAPSHOTABLE_ALLOCATOR_H_

#include <cstdio>
#include <cstdlib>
#include <memory>

#include "application/memory/allocator.h"

class MemorySnapshot {
 public:
  virtual ~MemorySnapshot(){};
};

class SnapshotableAllocator : public Allocator {
 public:
  virtual ~SnapshotableAllocator(){};

  // Returns a new Snapshot.
  // From a performance point of view, it's better to use
  // |SaveMemoryIntoSnapshot| directly with pre-created snapshots.
  std::unique_ptr<MemorySnapshot> TakeSnapshot() {
    auto snapshot = NewEmptySnapshot();
    SaveMemoryIntoSnapshot(*snapshot);
    return snapshot;
  }

  virtual void LoadMemoryFromSnapshot(MemorySnapshot const& snapshot) = 0;
  virtual void SaveMemoryIntoSnapshot(MemorySnapshot& snapshot) = 0;
  virtual std::unique_ptr<MemorySnapshot> NewEmptySnapshot() = 0;
};

#endif  // APPLICATION_MEMORY_SNAPSHOTABLE_ALLOCATOR_H_

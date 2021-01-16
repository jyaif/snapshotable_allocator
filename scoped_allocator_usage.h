#ifndef APPLICATION_MEMORY_SCOPED_ALLOCATOR_USAGE_H_
#define APPLICATION_MEMORY_SCOPED_ALLOCATOR_USAGE_H_

#include <thread>

#include "application/memory/snapshotable_allocator.h"

// For debugging.
Allocator* AllocatorInUse();

class ScopedAllocatorUsage {
 public:
  ScopedAllocatorUsage(Allocator* allocator = nullptr);
  ~ScopedAllocatorUsage();

 private:
  Allocator* previous_allocator_;
};

#endif  // APPLICATION_MEMORY_SCOPED_ALLOCATOR_USAGE_H_

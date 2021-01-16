#ifndef APPLICATION_MEMORY_MEMORY_SNAPSHOT_IMPL_H_
#define APPLICATION_MEMORY_MEMORY_SNAPSHOT_IMPL_H_

#include "application/memory/snapshotable_allocator.h"

class MemorySnapshotImpl : public MemorySnapshot {
 public:
  MemorySnapshotImpl();
  virtual ~MemorySnapshotImpl();
  void Clear();
  void RestoreEverything() const;
  void AppendData(void const* ptr, size_t length);
  uint64_t const* GetData(size_t& size);

 private:
  // The content in |data_| is a sequence of:
  // {length in bytes (8 bytes), ptr (8 bytes), the data (variable amount of
  // bytes)} aligned on 8 bytes.
  uint64_t* data_ = nullptr;
  size_t int64_allocated_ = 4000;
  size_t int64_used_ = 0;
};

#endif  // APPLICATION_MEMORY_MEMORY_SNAPSHOT_IMPL_H_

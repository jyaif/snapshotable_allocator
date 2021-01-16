#include "application/memory/memory_snapshot_impl.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "application/memory/fls.h"

MemorySnapshotImpl::MemorySnapshotImpl() {
  static_assert(sizeof(uint64_t) >= sizeof(void*));
  data_ = static_cast<uint64_t*>(malloc(int64_allocated_ * sizeof(uint64_t)));
  data_[0] = 0;
}

MemorySnapshotImpl::~MemorySnapshotImpl() {
  free(data_);
}

void MemorySnapshotImpl::Clear() {
  int64_used_ = 0;
}

void MemorySnapshotImpl::RestoreEverything() const {
  size_t index = 0;
  while (index < int64_used_) {
    uint64_t length = data_[index];
    void* dest = reinterpret_cast<void*>(data_[index + 1]);
    memcpy(dest, &data_[index + 2], length);
    index += ((length + sizeof(uint64_t) - 1) / sizeof(uint64_t)) + 2;
  }
}

void MemorySnapshotImpl::AppendData(void const* ptr, size_t length) {
  size_t int64_necessary =
      2 + ((length + sizeof(uint64_t) - 1) / sizeof(uint64_t));
  if (int64_used_ + int64_necessary > int64_allocated_) {
    int64_allocated_ *= 2;
    data_ = static_cast<uint64_t*>(
        realloc(data_, int64_allocated_ * sizeof(uint64_t)));
    AppendData(ptr, length);
    return;
  }
  data_[int64_used_] = length;
  uintptr_t ptr_converted = reinterpret_cast<uintptr_t>(ptr);
  data_[int64_used_ + 1] = ptr_converted;
  memcpy(&data_[int64_used_ + 2], ptr, length);
  int64_used_ += int64_necessary;
}

uint64_t const* MemorySnapshotImpl::GetData(size_t& size) {
  size = int64_used_;
  return data_;
}

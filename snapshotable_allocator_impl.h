#ifndef APPLICATION_MEMORY_SNAPSHOTABLE_ALLOCATOR_IMPL_H_
#define APPLICATION_MEMORY_SNAPSHOTABLE_ALLOCATOR_IMPL_H_

#include <vector>

#include "application/memory/bit_vector.h"
#include "application/memory/snapshotable_allocator.h"

class ItemBucket {
 public:
  ItemBucket(size_t item_size, size_t item_count);
  ~ItemBucket();
  void* Allocate();
  bool HasSpaceLeft();
  bool IsUnused();
  void Free(void* ptr);
  ItemBucket* next_bucket_ = nullptr;
  uint32_t* allocated_memory_;
  uint8_t* memory_;
  BitVector free_map_;
  size_t GetItemSize() { return item_size_; };
  size_t GetItemCount() { return item_count_; };

 private:
  uint32_t* free_slots_ptr_;
  const size_t item_size_;
  const size_t item_count_;
};

class AllocatorPositionInMemory {
 public:
  void* memory_start_;
  ItemBucket* bucket_;
  bool operator<(const AllocatorPositionInMemory& other) const {
    return memory_start_ <= other.memory_start_;
  }
};

class SnapshotableAllocatorImpl;

class FixedSizeAllocator2 {
 public:
  FixedSizeAllocator2(size_t item_size,
                      size_t item_per_bucket,
                      SnapshotableAllocatorImpl& memory);
  ~FixedSizeAllocator2();
  void* Allocate();

 private:
  SnapshotableAllocatorImpl& memory_;
  ItemBucket* first_bucket_ = nullptr;
  size_t item_size_;
  size_t item_per_bucket_;
};

class SnapshotableAllocatorImpl : public SnapshotableAllocator {
 public:
  SnapshotableAllocatorImpl(size_t min_size_for_individual_copy = 1024);

  void* Alloc(size_t size) override;
  void* Realloc(void* ptr, size_t size) override;
  void Free(void* ptr) override;

  void LoadMemoryFromSnapshot(MemorySnapshot const& snapshot) override;
  void SaveMemoryIntoSnapshot(MemorySnapshot& snapshot) override;
  std::unique_ptr<MemorySnapshot> NewEmptySnapshot() override;

  void NewBucketWasCreated(ItemBucket& bucket);

  enum MemoryInitialization { NONE, ZERO, RANDOM };
  void SetMemoryInitializationStyle(MemoryInitialization initialization_style);

 private:
  ItemBucket* BucketForPointer(void* ptr);
  void* InternalAlloc(size_t size);
  void InitializeMemory(void* ptr);

  std::vector<FixedSizeAllocator2> allocators_;
  std::vector<AllocatorPositionInMemory> sorted_buckets_;
  MemoryInitialization initialization_style_ = NONE;
  size_t min_size_for_individual_copy_;
};

#endif  // APPLICATION_MEMORY_SNAPSHOTABLE_ALLOCATOR_IMPL_H_

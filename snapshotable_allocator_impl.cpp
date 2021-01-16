#include "application/memory/snapshotable_allocator_impl.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>

#include "application/memory/aligned_alloc.h"
#include "application/memory/bit_vector.h"
#include "application/memory/fls.h"
#include "application/memory/memory_snapshot_impl.h"
#include "application/memory/scoped_allocator_usage.h"
#include "application/memory/snapshotable_allocator_impl.h"

namespace {
constexpr int extra_bytes_ = 16;
}

ItemBucket::ItemBucket(size_t item_size, size_t item_count)
    : free_map_(item_count), item_size_(item_size), item_count_(item_count) {
  // +16 to also store |free_slots_ptr_|.
  // +4 would be enough, but then the alignment would be bad (we need 16 byte
  // alignment).
  // 16 bytes alignment is necessary because `movaps` can be generated, and
  // crashes if the alignment is not 16 bytes.
  //|free_slots_ptr_| is stored in the same contiguous bag of bytes
  // to allow fast snapshoting/restoring of the free slot counter.
  allocated_memory_ = static_cast<uint32_t*>(
      AlignedAlloc(16, item_size * item_count + extra_bytes_));
  free_slots_ptr_ = allocated_memory_;
  memory_ = reinterpret_cast<uint8_t*>(allocated_memory_) + extra_bytes_;

  *free_slots_ptr_ = static_cast<uint32_t>(item_count);
}

ItemBucket::~ItemBucket() {
  AlignedFree(allocated_memory_);
}

void* ItemBucket::Allocate() {
  if (!HasSpaceLeft()) {
    assert(false);
    return nullptr;
  }
  int allocated_index = free_map_.GetFirstBitSetAndClear();
  (*free_slots_ptr_)--;
  return memory_ + (allocated_index * item_size_);
}

bool ItemBucket::HasSpaceLeft() {
  return (*free_slots_ptr_) > 0;
}

bool ItemBucket::IsUnused() {
  return (*free_slots_ptr_) == item_count_;
}

void ItemBucket::Free(void* ptr) {
  size_t offset_from_base = static_cast<uint8_t*>(ptr) - memory_;
  offset_from_base /= item_size_;
  assert(offset_from_base >= 0 && offset_from_base < item_count_);
  free_map_.SetBit(offset_from_base);
  (*free_slots_ptr_)++;
}

FixedSizeAllocator2::FixedSizeAllocator2(size_t item_size,
                                         size_t item_per_bucket,
                                         SnapshotableAllocatorImpl& memory)
    : memory_(memory),
      item_size_(item_size),
      item_per_bucket_(item_per_bucket) {
  ScopedAllocatorUsage sau;
  first_bucket_ = new ItemBucket(item_size_, item_per_bucket_);
  memory_.NewBucketWasCreated(*first_bucket_);
}

FixedSizeAllocator2::~FixedSizeAllocator2() {
  ScopedAllocatorUsage sau;
  delete first_bucket_;
}

void* FixedSizeAllocator2::Allocate() {
  ItemBucket* bucket = first_bucket_;
  while (!bucket->HasSpaceLeft()) {
    ItemBucket* next = bucket->next_bucket_;
    if (next == nullptr) {
      ScopedAllocatorUsage sau;
      next = new ItemBucket(item_size_, item_per_bucket_);
      memory_.NewBucketWasCreated(*next);
      bucket->next_bucket_ = next;
    }
    bucket = next;
  }
  return bucket->Allocate();
}

SnapshotableAllocatorImpl::SnapshotableAllocatorImpl(
    size_t min_size_for_individual_copy)
    : min_size_for_individual_copy_(min_size_for_individual_copy) {
  ScopedAllocatorUsage sau;
  std::array<std::pair<size_t, size_t>, 20> m = {{
      {1, 32},    {2, 32},    {4, 32},     {8, 128},    {16, 256},
      {32, 512},  {64, 1024}, {128, 512},  {256, 256},  {512, 128},
      {1024, 32}, {2048, 16}, {4096, 4},   {8192, 4},   {16384, 4},
      {32768, 4}, {65536, 4}, {131072, 4}, {262144, 2}, {524288, 2},
  }};
  // |reserve| is necessary to avoid copying the elements of |allocators_|
  // when the vector is resized:
  // Copying an allocator is very expensive.
  allocators_.reserve(m.size());
  for (auto const& p : m) {
    allocators_.emplace_back(p.first, p.second, *this);
  }
}

void* SnapshotableAllocatorImpl::Alloc(size_t size) {
  auto ptr = InternalAlloc(size);
  InitializeMemory(ptr);
  return ptr;
}

void* SnapshotableAllocatorImpl::Realloc(void* ptr, size_t size) {
  if (ptr == nullptr) {
    return Alloc(size);
  }
  void* old_ptr = ptr;

  ItemBucket* bucket = BucketForPointer(ptr);
  size_t old_size = bucket->GetItemSize();
  bucket->Free(ptr);

  void* new_ptr = InternalAlloc(size);
  if (new_ptr != old_ptr) {
    if (old_size < size) {
      memcpy(new_ptr, old_ptr, old_size);
    }
    if (old_size >= size) {
      memcpy(new_ptr, old_ptr, size);
    }
  }
  return new_ptr;
}

void SnapshotableAllocatorImpl::Free(void* ptr) {
  if (!ptr) {
    return;
  }
  BucketForPointer(ptr)->Free(ptr);
}

void SnapshotableAllocatorImpl::LoadMemoryFromSnapshot(
    MemorySnapshot const& snapshot) {
  MemorySnapshotImpl const& ms =
      static_cast<MemorySnapshotImpl const&>(snapshot);
  ms.RestoreEverything();
}

void SnapshotableAllocatorImpl::SaveMemoryIntoSnapshot(
    MemorySnapshot& snapshot) {
  MemorySnapshotImpl& ms = static_cast<MemorySnapshotImpl&>(snapshot);
  ms.Clear();
  int skipped_memory_ = 0;
  for (AllocatorPositionInMemory& allocator_position : sorted_buckets_) {
    auto bucket = allocator_position.bucket_;
    ms.AppendData(bucket->free_map_.GetDataLocation(),
                  bucket->free_map_.GetDataLength());
    if (bucket->IsUnused()) {
      ms.AppendData(bucket->allocated_memory_, 4);
    } else {
      // Memory optimization where only the slots used in the buckets are
      // copied, as opposed to the entire bucket.
      // The lower the threshold, the more memory is saved.
      // In practice, saves 50% memory with a size of 128.
      if (bucket->GetItemSize() >= min_size_for_individual_copy_) {
        const size_t count = bucket->GetItemCount();
        ms.AppendData(bucket->allocated_memory_, extra_bytes_);

        uint8_t* bucket_start_ =
            reinterpret_cast<uint8_t*>(bucket->allocated_memory_);
        bucket_start_ += extra_bytes_;
        for (uint32_t index = 0; index < count; index++) {
          if (!bucket->free_map_.GetBit(index)) {
            ms.AppendData(bucket_start_, bucket->GetItemSize());
          } else {
            skipped_memory_ += bucket->GetItemSize();
          }
          bucket_start_ += bucket->GetItemSize();
        }
      } else {
        ms.AppendData(
            bucket->allocated_memory_,
            (bucket->GetItemSize() * bucket->GetItemCount()) + extra_bytes_);
      }
    }
  }
}

std::unique_ptr<MemorySnapshot> SnapshotableAllocatorImpl::NewEmptySnapshot() {
  return std::make_unique<MemorySnapshotImpl>();
}

void SnapshotableAllocatorImpl::NewBucketWasCreated(ItemBucket& bucket) {
  AllocatorPositionInMemory apim = {
      bucket.memory_ + bucket.GetItemSize() * bucket.GetItemCount(), &bucket};
  auto it =
      std::upper_bound(sorted_buckets_.begin(), sorted_buckets_.end(), apim);
  sorted_buckets_.insert(it, apim);
  assert(std::is_sorted(sorted_buckets_.begin(), sorted_buckets_.end()));
}

void SnapshotableAllocatorImpl::SetMemoryInitializationStyle(
    MemoryInitialization initialization_style) {
  initialization_style_ = initialization_style;
}

ItemBucket* SnapshotableAllocatorImpl::BucketForPointer(void* ptr) {
  AllocatorPositionInMemory apim = {ptr, nullptr};
  auto it =
      std::lower_bound(sorted_buckets_.begin(), sorted_buckets_.end(), apim);
  if (it == sorted_buckets_.end()) {
    assert(false);
  }
  return it->bucket_;
}

void* SnapshotableAllocatorImpl::InternalAlloc(size_t size) {
  int index = FindLastBitSet(static_cast<int32_t>(size));
  assert(index < allocators_.size());
  return allocators_[index].Allocate();
}

void SnapshotableAllocatorImpl::InitializeMemory(void* ptr) {
  switch (initialization_style_) {
    case NONE:
      break;
    case ZERO:
      memset(ptr, 0, BucketForPointer(ptr)->GetItemSize());
      break;
    case RANDOM: {
      uint8_t* p = (uint8_t*)ptr;
      size_t size = BucketForPointer(ptr)->GetItemSize();
      for (size_t i = 0; i < size; i++) {
        p[i] = rand() % 255;
      }
      break;
    }
  }
}

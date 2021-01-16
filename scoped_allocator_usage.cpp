#include "application/memory/scoped_allocator_usage.h"

#include <cassert>
#include <cstdlib>
#include <map>
#include <new>

namespace {

thread_local Allocator* thread_allocator = nullptr;

}  // namespace

Allocator* AllocatorInUse() {
  return thread_allocator;
}

void* Alloc(std::size_t count) {
  void* ptr;
  if (thread_allocator != nullptr) {
    ptr = thread_allocator->Alloc(count);
  } else {
    ptr = malloc(count);
  }
  return ptr;
}

void Free(void* ptr) {
  if (thread_allocator) {
    thread_allocator->Free(ptr);
  } else {
    free(ptr);
  }
}

void* Realloc(void* ptr, size_t new_size) {
  void* ptr2;
  if (thread_allocator) {
    ptr2 = thread_allocator->Realloc(ptr, new_size);
  } else {
    ptr2 = realloc(ptr, new_size);
  }
  return ptr2;
}

// New
void* operator new(std::size_t count) {
  return Alloc(count);
}
void* operator new[](std::size_t count) {
  return Alloc(count);
}
void* operator new(std::size_t count, const std::nothrow_t& tag) noexcept {
  return Alloc(count);
}
void* operator new[](std::size_t count, const std::nothrow_t& tag) noexcept {
  return Alloc(count);
}
// New C++17
void* operator new(std::size_t count, std::align_val_t al) {
  return Alloc(count);
}
void* operator new[](std::size_t count, std::align_val_t al) {
  return Alloc(count);
}
void* operator new(std::size_t count,
                   std::align_val_t al,
                   const std::nothrow_t&) noexcept {
  return Alloc(count);
}
void* operator new[](std::size_t count,
                     std::align_val_t al,
                     const std::nothrow_t&) noexcept {
  return Alloc(count);
}
void* operator new(std::size_t count, std::align_val_t al, ...) noexcept {
  return Alloc(count);
}
void* operator new[](std::size_t count, std::align_val_t al, ...) noexcept {
  return Alloc(count);
}

// Delete
void operator delete(void* ptr) noexcept {
  Free(ptr);
}
void operator delete[](void* ptr) noexcept {
  Free(ptr);
}
void operator delete(void* ptr, std::size_t sz) noexcept {
  Free(ptr);
}
void operator delete[](void* ptr, std::size_t sz) noexcept {
  Free(ptr);
}
void operator delete(void* ptr, const std::nothrow_t& tag) noexcept {
  Free(ptr);
}
void operator delete[](void* ptr, const std::nothrow_t& tag) noexcept {
  Free(ptr);
}
// Delete C++17
void operator delete(void* ptr, std::align_val_t al) noexcept {
  Free(ptr);
}
void operator delete[](void* ptr, std::align_val_t al) noexcept {
  Free(ptr);
}
void operator delete(void* ptr, std::size_t sz, std::align_val_t al) noexcept {
  Free(ptr);
}
void operator delete[](void* ptr,
                       std::size_t sz,
                       std::align_val_t al) noexcept {
  Free(ptr);
}
void operator delete(void* ptr,
                     std::align_val_t al,
                     const std::nothrow_t& tag) noexcept {
  Free(ptr);
}
void operator delete[](void* ptr,
                       std::align_val_t al,
                       const std::nothrow_t& tag) noexcept {
  Free(ptr);
}

#if __cplusplus > 201703L

#error New memory management functions need to be implemented.

#endif

ScopedAllocatorUsage::ScopedAllocatorUsage(Allocator* allocator) {
  previous_allocator_ = thread_allocator;
  thread_allocator = allocator;
}

ScopedAllocatorUsage::~ScopedAllocatorUsage() {
  thread_allocator = previous_allocator_;
}

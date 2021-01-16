#include "application/memory/aligned_alloc.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <type_traits>

void* AlignedAlloc(std::size_t alignment, std::size_t size) {
  void* ptr = nullptr;
  bool error = posix_memalign(&ptr, alignment, size);
  if (error) {
    assert(false);
  }
  return ptr;
}

void AlignedFree(void* ptr) {
  free(ptr);
}

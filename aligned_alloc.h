#ifndef APPLICATION_MEMORY_ALIGNED_ALLOC_H_
#define APPLICATION_MEMORY_ALIGNED_ALLOC_H_

#include <cstddef>

// implementation of https://en.cppreference.com/w/cpp/memory/c/aligned_alloc
void* AlignedAlloc(std::size_t alignment, std::size_t size);
void AlignedFree(void* ptr);

#endif  // APPLICATION_MEMORY_ALIGNED_ALLOC_H_

#ifndef APPLICATION_MEMORY_ALLOCATOR_H_
#define APPLICATION_MEMORY_ALLOCATOR_H_

class Allocator {
 public:
  virtual ~Allocator(){};
  virtual void* Realloc(void* ptr, size_t size) = 0;
  virtual void* Alloc(size_t size) = 0;
  virtual void Free(void* ptr) = 0;
};

#endif  // APPLICATION_MEMORY_ALLOCATOR_H_

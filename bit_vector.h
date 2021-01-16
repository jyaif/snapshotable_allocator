#ifndef APPLICATION_MEMORY_BIT_VECTOR_H_
#define APPLICATION_MEMORY_BIT_VECTOR_H_

#include <stdint.h>
#include <cstring>

class BitVector {
 public:
  BitVector(size_t bit_count);
  ~BitVector();
  // Must only be called if there is a bit that has been set.
  int GetFirstBitSetAndClear();
  void SetBit(size_t index);

  // Returns whether the bit at `index` is set or not.
  bool GetBit(uint32_t index);

  // For serialization:
  void* GetDataLocation();
  size_t GetDataLength();

 private:
  uint32_t* data_;
  size_t byte_count_;
};

#endif  // APPLICATION_MEMORY_BIT_VECTOR_H_

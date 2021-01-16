#include "application/memory/bit_vector.h"

#include <cassert>
#include <cstdlib>

BitVector::BitVector(size_t bit_counts) {
  size_t count = (bit_counts + 31) / 32;
  byte_count_ = count * sizeof(uint32_t);
  data_ = static_cast<uint32_t*>(malloc(byte_count_));
  memset(data_, 0xff, byte_count_);
}

BitVector::~BitVector() {
  free(data_);
}

int BitVector::GetFirstBitSetAndClear() {
  uint32_t* ptr = data_;
  while (!*ptr) {
    ptr++;
  }
  int offset = ffs(*ptr) - 1;
  *ptr ^= 1 << offset;
  offset += (ptr - data_) * sizeof(uint32_t) * 8;
  return offset;
}

void BitVector::SetBit(size_t index) {
  int shift = index % 32;
  int i = index / 32;
  data_[i] |= (1 << shift);
}

// Returns whether the bit at `index` is set or not.
bool BitVector::GetBit(uint32_t index) {
  uint32_t shift = index % 32;
  uint32_t i = index / 32;
  return data_[i] & (1 << shift);
}

void* BitVector::GetDataLocation() {
  return data_;
}

size_t BitVector::GetDataLength() {
  return byte_count_;
}

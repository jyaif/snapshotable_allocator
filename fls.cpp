#include "application/memory/fls.h"

#if __has_builtin(__builtin_clz)

int FindLastBitSet(int32_t value) {
  if (value == 0) {
    return 0;
  }
  return (sizeof(int32_t) * 8) - __builtin_clz(value);
}

#else

int FindLastBitSet(int32_t value) {
  if (value == 0) {
    return 0;
  }
  int bit;
  for (bit = 1; value != 1; bit++)
    value = value >> 1;
  return bit;
}

#endif

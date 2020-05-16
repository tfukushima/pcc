#include "pcc.h"

/**
 * Aligns an integer value to the specific number.
 *
 * @param n     the integer to be aligned
 * @param align the number to which the integer value is aligned
 * @return the integer value aligned to the given number.
 */
int align_to(int n, int align) {
  return (n + align - 1) & ~(align - 1);
}
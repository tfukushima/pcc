#include "pcc.h"

Type *int_type = &(Type){ TY_INT };

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

/**
 * Examines if the given type is integer.
 *
 * @param type the type to examine
 * @return true if the given type is integer, otherwise false
 */
bool is_integer(const Type *type) {
  return type->kind == TY_INT;
}

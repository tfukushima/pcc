#include "pcc.h"

Type *int_type = &(Type){ TY_INT };
Type *ptr_type = &(Type){ TY_PTR };

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

/**
 * Examines if the given type is pointer.
 *
 * @param type the type to examine
 * @return true if the given type is pointer, otherwise false
 */
bool is_ptr(const Type *type) {
  return type->kind == TY_PTR;
}

/**
 * Makes a pointer type to the given type.
 *
 * @param type the type to point to
 * @return the pointer type that points to the given type
 */
Type *make_ptr_to(const Type *type) {
  Type *ptr = calloc(1, sizeof(Type));
  ptr->kind = TY_PTR;
  ptr->ptr_to = type;
  return ptr;
}

// Functions used in the tests. They should be compiled and linked to the
// assembly code compiled by pcc with the existing compilers and linkders
// sucn as gcc/clang and ld.

int foo() {
  return 42;
}

int bar(int a, int b) {
  return a + b;
}

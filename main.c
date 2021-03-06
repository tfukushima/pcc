#include "pcc.h"

// The current token
Token *token;

// The whole input
char *user_input;

int main(int argc,  char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid number of arguments\n");
    return 1;
  }

  user_input = argv[1];

  // Tokenize the argument.
  token = tokenize(argv[1]);
  // Parse the tokenized input.
  Function *prog = program();
  LVar *lv = prog->locals;
  // Generate the assembly code from the parsed AST.
  codegen(prog);

  return 0;
}

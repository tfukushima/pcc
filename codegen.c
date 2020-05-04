#include "pcc.h"

/*
 * Generate a series of assembly code that pushes the left value to the stack
 * and output it to stdout.
 */
static void gen_lval(const Node *node) {
  if (node->kind != ND_LVAR) {
    error_at(token->str, "The left hand side of the assiment is not left value.");
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

/*
 * Generate a series of assembly code that emulates stack machine from the AST
 *
 * @param node the node from which the assembly code is generated
 */
static void gen(const Node *node) {
  // Handle terminal and assignment nodes.
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");

      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");

      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      // Intel's idiv operation concatenates RDX and RAX, regards them as a
      // 128bit intege, devide it by the given operand, set its quotient
      // to RAX and set its remainder to RDX.
      // cqo operation expand the 64bit RAX value to 128bit and set it to
      // RDX and RAX.
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      // sete sets the result of cmp to the register given as its operand.
      // If the operands of cmp are equql it sets 1 to the operand, otherwise
      // it sets to 0 to the register. AL is an alias for the lower 8bit of
      // RAX and the upper 58bit is preserved in sete. movzb clears the upper
      // 58bit up with zeros.
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}

/*
 * Generate prologue of the code and output it to stdout.
 */
static void gen_prologue() {
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");  // allocate 26 (chars) * 8 bytes for variables.
}

/*
 * Generate epilogue of the code and output it to stdout.
 */
static void gen_epilogue() {
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

/**
 * Generate a complete assembly code that emulates stack machine from the AST
 * and output it to stdout.
 *
 * @param node the node from which the assembly code is generated
 */
void codegen(const Node *node) {
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen_prologue();

  for (int i = 0; code[i]; i++) {
    // Generate a seris of assembly code descending the AST nodes.
    gen(code[i]);

    // Pop the top of the stack and load it to RAX.
    printf("  pop rax\n");
  }

  gen_epilogue();
}

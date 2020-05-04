#include "pcc.h"

/*
 * Generate a series of assembly code that emulates stack machine from the AST
 *
 * @param node the node from which the assembly code is generated
 */
static void gen(const Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
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

  // Generate a seris of assembly code descending the AST nodes.
  gen(node);

  // Pop the top of the stack and load it to RAX.
  printf("  pop rax\n");
  printf("  ret\n");
}

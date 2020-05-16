#include "pcc.h"

static int label_seq = 0;

static void gen(const Node *node);

/*
 * Generate a series of assembly code that pushes the left value to the stack
 * and output it to stdout.
 */
static void gen_lval(const Node *node) {
  if (node->kind != ND_LVAR) {
    error_at(token->str, "The left hand side of the assiment is not left value.");
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->lvar->offset);
  printf("  push rax\n");
}

/*
 * Generates a series of assembly code for the if statement.
 */
static void gen_if(const Node *node) {
  if (node->kind != ND_IF) {
    error_at(token->str, "Not an if statement.");
  }

  // Generate the condition code.
  gen(node->lhs);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .L.else.%d\n", label_seq);
  const Node *bodies = node->rhs;
  // Generate the body code.
  gen(bodies->lhs);
  printf("  jmp .L.end.%d\n", label_seq);
  printf(".L.else.%d:\n", label_seq);
  // Generate the else body code if any.
  if (bodies->rhs) {
    gen(bodies->rhs);
  }
  printf(".L.end.%d:\n", label_seq);
  label_seq++;
}

/*
 * Generates a series of assembly code for the while statement.
 */
static void gen_while(const Node *node) {
  if (node->kind != ND_WHILE) {
    error_at(token->str, "Not a while statement.");
  }

  printf(".L.begin.%d:\n", label_seq);
  // Generate the condition code;
  gen(node->lhs);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .L.end.%d\n", label_seq);
  gen(node->rhs);
  printf("  jmp .L.begin.%d\n", label_seq);
  printf(".L.end.%d:\n", label_seq);
  label_seq++;
}

/*
 * Generates a series of assembly code for the for statement.
 */
static void gen_for(const Node *node) {
  if (node->kind != ND_FOR) {
    error_at(token->str, "Not a for statement.");
  }

  // Generate the code for the declaration clause.
  const Node * const decl = node->lhs;
  if (decl) {
    gen(decl);
  }
  const Node *rest = node->rhs;
  printf(".L.begin.%d:\n", label_seq);
  // Generate the code for the condition clause.
  const Node * const cond = rest->lhs;
  if (cond) {
    gen(cond);
  }
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .L.end.%d\n", label_seq);
  rest = rest->rhs;
  const Node * const post = rest->lhs;
  const Node * const body = rest->rhs;
  // Generate the code fo the body of the for statement.
  gen(body);
  if (post) {
    // Generate the code for the post processing clause.
    gen(post);
  }
  printf("  jmp .L.begin.%d\n", label_seq);
  printf(".L.end.%d:\n", label_seq);
  label_seq++;
}

/*
 * Generates a series of assembly code for the block.
 */
static void gen_block(const Node *node) {
  if (node->kind != ND_BLOCK) {
    error_at(token->str, "Not a block.");
  }

  const Node *cur = node->next;
  while (cur) {
    gen(cur);
    if (cur->kind != ND_RETURN) {
      printf("  pop rax\n");
    }
    cur = cur->next;
  }
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
    case ND_IF:
      gen_if(node);
      return;
    case ND_WHILE:
      gen_while(node);
      return;
    case ND_FOR:
      gen_for(node);
      return;
    case ND_BLOCK:
      gen_block(node);
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
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
 * Generate prologue of the function and output it to stdout.
 */
static void gen_prologue(const Function *program) {
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", program->stack_size);
}

/*
 * Generate epilogue of the function and output it to stdout.
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
 * @param program the function from which the assembly code is generated
 */
void codegen(const Function *program) {
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen_prologue(program);

  Node *cur = program->node;
  while (cur) {
    // Generate a seris of assembly code descending the AST nodes.
    gen(cur);

    // Pop the top of the stack and load it to RAX.
    printf("  pop rax\n");
    cur = cur->next;
  }

  gen_epilogue();
}

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tokenizer

/**
 * The kind of tokens
 */
typedef enum {
  TK_RESERVED,  // Operator token
  TK_NUM,       // Number token
  TK_EOF,       // End of file, which is the end of the input
} TokenKind;

typedef struct Token Token;

/**
 * Token type
 */
struct Token {
  TokenKind kind;  // The kind of the token
  Token *next;     // The next input token
  int val;         // The value of the number if the kind of the token is TK_NUM
  char *str;       // The token string
  int len;         // The length of the token
};

// The current token
Token *token;

// The whole input
char *user_input;

/**
 * Report an error.
 *
 * This function takes the same arguments as printf.
 *
 * @param loc the error location in the input
 * @param fmt the format string
 * @param ... the parameters to be used for the formatting
 */
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

/**
 * Consume a token
 *
 * If the next token is the expected operator, scan a token and return true.
 * Otherwise return false.
 *
 * @param op the pointer to the operator string
 * @return true if the next token is the expected operator, otherwise false
 */
bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next;

  return true;
}

/**
 * Expects a valid token
 *
 * If the next token is the expected operator, scan a token. Otherwise report the
 * error.
 *
 * @param op the pointer to the operator string
 */
void expect(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    error_at(token->str, "expected \"%c\"", op);
  }
  token = token->next;
}

/**
 * Expects a number token
 *
 * If the token is a number, scan a token and return its value. Otherwise report
 * the error.
 *
 * @return the value of the number token if the next token is a number
 */
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "Not a number");
  }
  int val = token->val;
  token = token->next;

  return val;
}

/**
 * Examine if it is EOF
 *
 * @return true if it is EOF, otherwise false
 */
bool at_eof() {
  return token->kind == TK_EOF;
}

/**
 * Create a new token and chain it to the curernt token
 *
 * @param kind the kind of the token to create
 * @param cur  the pointer to the current token
 * @param str  the token string
 * @param len  the length of the token string
 * @return the pointer to the created token
 */
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;

  return tok;
}

/**
 * Tokenize the input string
 *
 * @param p the pointer to the input string
 * @return the token tokenized from the input string
 */
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // Skip the white spaces.
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strchr("+-*/()<>=!", *p)) {
      if ((*p == '=' || *p == '!' || *p == '<' || *p == '>') && *(p+1) == '=') {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
      } else {
        cur = new_token(TK_RESERVED, cur, p++, 1);
      }
      continue;
    }

    if (isdigit(*p)) {
      const char *start = p;
      int val = strtol(p, &p, 10);
      cur = new_token(TK_NUM, cur, p, p - start);
      cur->val = val;
      continue;
    }

    error_at(cur->str, "Cannot tokenize");
  }

  new_token(TK_EOF, cur, p, 0);

  return head.next;
}


// Parser

/**
 * The kind of abstract syntax tree (AST) nodes
 */
typedef enum {
  ND_ADD,   // +
  ND_SUB,   // -
  ND_MUL,   // *
  ND_DIV,   // /
  ND_NUM,   // Integer
  ND_EQ,    // ==
  ND_NE,    // !=
  ND_LT,    // <
  ND_LE,    // <=
} NodeKind;

typedef struct Node Node;

/**
 * The AST node type
 */
struct Node {
  NodeKind kind; // The kind of the node
  const Node *lhs;     // Left hand side
  const Node *rhs;     // Right hand side
  int val;       // The value of the integer if the kind is ND_NUM
};

/**
 * Create a new AST node
 *
 * @param kind the kind of the AST node to create
 * @param lhs  the lhs of the AST node to create
 * @param rhs  the rhs of the AST node to create
 * @return the pointer to the created AST node
 */
Node *new_node(NodeKind kind, const Node *lhs, const Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;

  return node;
}

/**
 * Create a new AST node for a number
 *
 * @param val the value of the AST number node to create
 * @return the pointer to the created number node
 */
Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;

  return node;
}

// Production rules:
//   expr       = equality
//   equality   = relational ("==" relational | "!=" relational)*
//   relational = add ("<" add | "<=" add | ">" add | ">=" add)*
//   add        = mul ("+" mul | "-" mul)*
//   mul        = unary ("*" unary | "/" unary)*
//   unary      = ("+"  | "-")? primary
//   primary    = num | "(" expr ")"
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

/**
 * Parse tokens with the "expr" production rule
 *
 *   expr       = equality
 *
 * @return the constructed AST node
 */
Node *expr() {
  return equality();
}

/**
 * Parse tokens with the "equality" production rule
 *
 *   equality   = relational ("==" relational | "!=" relational)*
 *
 *  @return the constructed AST node
 */
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_node(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

/**
 * Parse tokens with the "relational" production rule
 *
 *   relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 *
 *  @return the constructed AST node
 */
Node *relational() {
  Node *node = add();

  for (;;) {
    // flip the operator and the operand positions to canonicalize ">" to "<"
    // and ">=" to "<=".
    if (consume(">")) {
      node = new_node(ND_LT, add(), node);
    } else if (consume(">=")) {
      node = new_node(ND_LE, add(), node);
    } else if (consume("<")) {
      node = new_node(ND_LT, node, add());
    } else if (consume("<=")) {
      node = new_node(ND_LE, node, add());
    } else {
      return node;
    }
  }
}

/**
 * Parse tokens with the "add" production rule
 *
 *   add        = mul ("+" mul | "-" mul)*
 *
 * @return the constructed AST node
 */
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

/**
 * Parse tokens with the "mul" production rule
 *
 *   mul        = unary ("*" unary | "/" unary)*
 *
 * @return the constructed AST node
 */
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

/**
 * Parse tokes with the "unary" production rule
 *
 *   unary   = ("+"  | "-")? primary
 *
 * @return the constructed AST node
 */
Node *unary() {
  if (consume("+")) {
    return primary();
  }

  if (consume("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());
  }

  return primary();
}

/**
 * Parse tokens with the "primary" production rule
 *
 *   primary = num | "(" expr ")"
 *
 * @return the constructed AST node
 */
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    consume(")");
    return node;
  }

  return new_node_num(expect_number());
}


// Assembly code generator

/**
 * Generate a series of assembly code that emulates stack machine from the AST
 *
 * @param node the node from which the assembly code is generated
 */
void gen(const Node *node) {
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


int main(int argc,  char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid number of arguments\n");
    return 1;
  }

  user_input = argv[1];

  // Tokenize the argument.
  token = tokenize(argv[1]);
  // Parse the tokenized input.
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Generate a seris of assembly code descending the AST nodes.
  gen(node);

  // Pop the top of the stack and load it to RAX.
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

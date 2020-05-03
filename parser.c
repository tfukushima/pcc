#include "pcc.h"

/*
 * Create a new AST node
 *
 * @param kind the kind of the AST node to create
 * @param lhs  the lhs of the AST node to create
 * @param rhs  the rhs of the AST node to create
 * @return the pointer to the created AST node
 */
static Node *new_node(NodeKind kind, const Node *lhs, const Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;

  return node;
}

/*
 * Create a new AST node for a number
 *
 * @param val the value of the AST number node to create
 * @return the pointer to the created number node
 */
static Node *new_node_num(int val) {
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
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

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

/*
 * Parse tokens with the "equality" production rule
 *
 *   equality   = relational ("==" relational | "!=" relational)*
 *
 *  @return the constructed AST node
 */
static Node *equality() {
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

/*
 * Parse tokens with the "relational" production rule
 *
 *   relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 *
 *  @return the constructed AST node
 */
static Node *relational() {
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

/*
 * Parse tokens with the "add" production rule
 *
 *   add        = mul ("+" mul | "-" mul)*
 *
 * @return the constructed AST node
 */
static Node *add() {
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

/*
 * Parse tokens with the "mul" production rule
 *
 *   mul        = unary ("*" unary | "/" unary)*
 *
 * @return the constructed AST node
 */
static Node *mul() {
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

/*
 * Parse tokes with the "unary" production rule
 *
 *   unary   = ("+"  | "-")? primary
 *
 * @return the constructed AST node
 */
static Node *unary() {
  if (consume("+")) {
    return primary();
  }

  if (consume("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());
  }

  return primary();
}

/*
 * Parse tokens with the "primary" production rule
 *
 *   primary = num | "(" expr ")"
 *
 * @return the constructed AST node
 */
static Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    consume(")");
    return node;
  }

  return new_node_num(expect_number());
}

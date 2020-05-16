#include "pcc.h"

// The entire parsed code in AST.
Node *code[100];

// The list of local variables. All local variables created during the parsing
// are accumulated to this list. Newer variables are prepended to the list and
// the head of the list is the latest defined vvariable. It is initialized with
// NULL that represents the end of the list.
static LVar* locals = NULL;

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

/*
 * Finds a local variable by its name.
 *
 * @return the found local variable if any, otherwise the null pointer
 */
static LVar *find_lvar(const Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (!strncmp(tok->str, var->name, tok->len)) {
      return var;
    }
  }

  return NULL;
}


static LVar *new_lvar(const char *name) {
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = name;
  lvar->offset = (locals ? locals->offset : 0) + 8;
  locals = lvar;

  return lvar;
}

static Node *new_lvar_node(LVar *lvar) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  node->lvar = lvar;

  return node;
}

static Node *new_funcall_node(const char *name) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNCALL;
  node->name = name;
}

// Production rules:
//   program    = function*
//   function   = ident ( "(" ident? ("," ident)* ")" ) "{" stmt* "}"
//   stmt       = expr ";"
//              | "{" stmt* "}"
//              | "if" "(" expr ")" stmt ("else" stmt)?
//              | "while" "(" expr ")" stmt
//              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//              | "return" expr ";"
//   expr       = assign
//   assign     = equality ("=" assign)?
//   equality   = relational ("==" relational | "!=" relational)*
//   relational = add ("<" add | "<=" add | ">" add | ">=" add)*
//   add        = mul ("+" mul | "-" mul)*
//   mul        = unary ("*" unary | "/" unary)*
//   unary      = ("+"  | "-")? primary
//   primary    = num
//              | ident ( "(" expr? ("," expr)* ")" )?
//              | "(" expr ")"
static Function *function();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

/**
 * Parse tokens with the "program" production rule
 *
 *   program    = function*
 *
 * The parsed result is store in the global variable "code".
 */
Function *program() {
  Function *cur_func = NULL;
  Function *head;
  while (!at_eof()) {
    Function *func = function();
    if (!cur_func) {
      cur_func = func;
      head = cur_func;
    } else {
      cur_func->next = func;
      cur_func = cur_func->next;
    }
  }

  return head;
}


/*
 * Parse tokens with the "stmt" production rule
 *
 *   function   = ident ( "(" expr? ("," expr)* ")" ) "{" stmt* "}"
 *
 * @return the constructed AST node
 */
static Function *function() {
  const Token *tok = consume_ident();
  if (!tok) {
    error_at(tok->str, "not a function.");
  }

  const char *func_name = strndup(tok->str, tok->len);

  Function *func = calloc(1, sizeof(Function));
  func->name = func_name;

  Node *cur_node = NULL;
  // Parse the function arguments.
  expect("(");
  int argn = 0;
  while (!consume(")")) {
    if (argn > 0) {
      expect(",");
    }
    tok = consume_ident();
    LVar *lvar = find_lvar(tok);
    if (!lvar) {
      lvar = new_lvar(strndup(tok->str, tok->len));
    }
    Node *lvar_node = new_lvar_node(lvar);
    if (!cur_node) {
      func->node = lvar_node;
      cur_node = func->node;
    } else {
      cur_node->next = lvar_node;
      cur_node = cur_node->next;
    }
    argn++;
  }

  func->argn = argn;

  // Parse the function body.
  expect("{");
  while (!consume("}")) {
    Node *node = stmt();
    if (!func->node) {
      func->node = node;
      cur_node = func->node;
    } else {
      cur_node->next = node;
      cur_node = cur_node->next;
    }
  }
  func->locals = locals;
  func->stack_size = locals ? align_to(locals->offset, 16) : 0;
  locals = NULL;

  return func;
}

/*
 * Parse tokens with the "stmt" production rule
 *
 *   stmt       = expr ";"
 *              | "{" stmt* "}"
 *              | "if" "(" expr ")" stmt ("else" stmt)?
 *              | "while" "(" expr ")" stmt
 *              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *              | "return" expr ";"
 *
 * @return the constructed AST node
 */
static Node *stmt() {
  Node *node;

  if (consume("{")) {
    node = new_node(ND_BLOCK, NULL, NULL);
    Node *head = node;
    while (!consume("}")) {
        node->next = stmt();
        node = node->next;
    }
    return head;
  } else if (consume("if")) {
    expect("(");
    Node *cond = expr();
    expect(")");
    Node *body = stmt();
    Node *ebody = NULL;
    if (consume("else")) {
      ebody = stmt();
    }
    node = new_node(ND_IF, cond, new_node(ND_IF, body, ebody));
    return node;
  } else if (consume("while")) {
    expect("(");
    Node *cond = expr();
    expect(")");
    node = new_node(ND_WHILE, cond, stmt());
    return node;
  } else if (consume("for")) {
    expect("(");
    Node *decl = NULL;
    if (!consume(";")) {
      decl = expr();
      expect(";");
    }
    Node *cond = NULL;
    if (!consume(";")) {
      cond = expr();
      expect(";");
    }
    Node *post = NULL;
    if (!consume(")")) {
      post = expr();
      expect(")");
    }
    const Node *body = stmt();
    return new_node(ND_FOR, decl, new_node(ND_FOR, cond, new_node(ND_FOR, post, body)));
  } else if (consume("return")) {
    node = new_node(ND_RETURN, expr(), NULL);
  } else {
    node = expr();
  }
  expect(";");

  return node;
}

/*
 * Parse tokens with the "expr" production rule
 *
 *   expr       = assign
 *
 * @return the constructed AST node
 */
static Node *expr() {
  return assign();
}

/*
 * Parse tokens with the "assign" production rule
 *
 *   assign     = equality ("=" assign)?
 *
 * @return the constructed AST node
 */
static Node *assign() {
  Node *node = equality();

  if (consume("=")) {
    node = new_node(ND_ASSIGN, node, assign());
  }

  return node;
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
 *   primary    = num
 *              | ident ( "(" expr? ("," expr)* ")" )?
 *              | "(" expr ")"
 *
 * @return the constructed AST node
 */
static Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    consume(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    const char *func = strndup(tok->str, tok->len);
    if (consume("(")) {
      Node *args = NULL;
      const Node *head = NULL;
      int argn = 0;
      while (!consume(")")) {
        if (args == NULL) {
          args = new_node(ND_FUNCALL, expr(), NULL);
          head = args;
        } else {
          expect(",");
          args->rhs = new_node(ND_FUNCALL, expr(), NULL);
          args = args->rhs;
        }
        argn++;
      }
      Node *funcall = new_funcall_node(func);
      funcall->lhs = head;
      funcall->val = argn;
      return funcall;
    }
    LVar *lvar = find_lvar(tok);
    if (!lvar) {
      lvar = new_lvar(strndup(tok->str, tok->len));
    }

    return new_lvar_node(lvar);
  }

  return new_node_num(expect_number());
}

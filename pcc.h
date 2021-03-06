#define _POSIX_C_SOURCE 200809L  // For strndup
#ifndef PCC_H_
#define PCC_H_

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
  TK_IDENT,     // Identifier
  TK_NUM,       // Number token
  TK_RETURN,    // "return" token
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

/**
 * The current token
 */
extern Token *token;

/**
 * The whole input
 */
extern char *user_input;

/**
 * Report an error.
 *
 * This function takes the same arguments as printf.
 *
 * @param loc the error location in the input
 * @param fmt the format string
 * @param ... the parameters to be used for the formatting
 */
void error_at(char *loc, char *fmt, ...);

/**
 * Consume a token
 *
 * If the next token is the expected operator, scan a token and return true.
 * Otherwise return false.
 *
 * @param op the pointer to the operator string
 * @return true if the next token is the expected operator, otherwise false
 */
bool consume(char *op);

/**
 * Consumes an identifier token.
 *
 * If the next token is an identifier, scan a token and return the node
 * constructed for the identifier.
 *
 * @return the current identifier token
 */
Token *consume_ident();

/**
 * Expects a valid token
 *
 * If the next token is the expected operator, scan a token. Otherwise report the
 * error.
 *
 * @param op the pointer to the operator string
 */
void expect(char *op);

/**
 * Expects a number token
 *
 * If the token is a number, scan a token and return its value. Otherwise report
 * the error.
 *
 * @return the value of the number token if the next token is a number
 */
int expect_number();

/**
 * Examine if it is EOF
 *
 * @return true if it is EOF, otherwise false
 */
bool at_eof();

/**
 * Tokenize the input string
 *
 * @param p the pointer to the input string
 * @return the token tokenized from the input string
 */
Token *tokenize(char *p);


// Parser

/**
 * The kind of abstract syntax tree (AST) nodes
 */
typedef enum {
  ND_ADD,      // +
  ND_SUB,      // -
  ND_MUL,      // *
  ND_DIV,      // /
  ND_NUM,      // Integer
  ND_EQ,       // ==
  ND_NE,       // !=
  ND_LT,       // <
  ND_LE,       // <=
  ND_ASSIGN,   // Variable assignment
  ND_LVAR,     // Local variable
  ND_IF,       // if statement
  ND_WHILE,    // while statement
  ND_FOR,      // for statement
  ND_BLOCK,    // blocks
  ND_FUNCALL,  // function call
  ND_RETURN,   // Return statement
} NodeKind;

typedef struct LVar LVar;

/**
 * Local variable type
 */
struct LVar {
  LVar *next;        // The next local variable or NULL.
  const char *name;  // The name of the local variable.
  int offset;        // The offset from the base register, RBP.
};

typedef struct Node Node;

/**
 * The AST node type
 */
struct Node {
  NodeKind kind;     // The kind of the node
  Node *next;        // The next AST node that contains another statement
  const Node *lhs;   // Left hand side
  const Node *rhs;   // Right hand side
  int val;           // The value of the integer if the kind is ND_NUM
  LVar *lvar;        // The local variable only if the kind is ND_LVAR
  const char *name;  // The name of the funciton only if kind is ND_FUNCALL
};

/**
 * The entire parsed code in AST.
 */
extern Node *code[100];

typedef struct Function Function;

/**
 * The function implementation consists of its parsed code, local variables
 * and required stack size.
 */
struct Function {
  Node *node;
  LVar *locals;
  int stack_size;
};


/**
 * Parse tokens with the "program" production rule
 *
 *   stmt       = expr ";"
 *
 * The parsed result is store in the global variable "code".
 *
 * @return the parsed code as a function
 */
Function *program();


// Assembly code generator

/**
 * Generate a series of assembly code that emulates stack machine from the AST
 *
 * @param program the function from which the assembly code is generated
 */
void codegen(const Function *program);

#endif  // PCC_H_

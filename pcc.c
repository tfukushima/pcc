#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  TokenKind kind;  //
  Token *next;     // The next input token
  int val;         // The value of the number if the kind of the token is TK_NUM
  char *str;       // The token string
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
 * @param op the operator character
 * @return true if the next token is the expected operator, otherwise false
 */
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
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
 * @param op the operator character
 */
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error_at(token->str, "The operaor is not '%c'", op);
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
 * @return the pointer to the created token
 */
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
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

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(cur->str, "Cannot tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc,  char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid number of arguments\n");
    return 1;
  }

  user_input = argv[1];

  // Tokenize the argument.
  token = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // The first token must be a number.
  printf(" mov rax, %d\n", expect_number());

  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}

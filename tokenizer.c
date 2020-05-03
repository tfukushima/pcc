#include "pcc.h"

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

/*
 * Create a new token and chain it to the curernt token
 *
 * @param kind the kind of the token to create
 * @param cur  the pointer to the current token
 * @param str  the token string
 * @param len  the length of the token string
 * @return the pointer to the created token
 */
static Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
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
#include "pcc.h"

/*
 * Examines if the character is an alphabet or '_'.
 */
static inline bool isalphau(char c) {
  return isalpha(c) || c == '_';
}

/*
 * Examines if the character is an alphabet, a number or '_'.
 */
static inline bool isalnumu(char c) {
  return isalnum(c) || c == '_';
}

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
  if ((token->kind != TK_RESERVED && token->kind != TK_RETURN) ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next;

  return true;
}

/**
 * Consumes an identifier token.
 *
 * If the next token is an identifier, scan a token and return the node
 * constructed for the identifier.
 *
 * @return the current identifier token
 */
Token *consume_ident() {
  if (token->kind != TK_IDENT ||
      token->len < 1) {
    return NULL;
  }

  Token *cur = token;
  token = token->next;

  return cur;
}

/**
 * Expects a valid token
 *
 * If the next token is the expected string, scan a token. Otherwise report the
 * error.
 *
 * @param op the pointer to the expected string
 */
void expect(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    error_at(token->str, "expected \"%s\"", op);
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

    if (!strncmp(p, "if", 2) && !isalnumu(p[2])) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (!strncmp(p, "else", 4) && !isalnumu(p[4])) {
      cur = new_token(TK_RESERVED, cur, p, 4);
      p += 4;
      continue;
    }

    if (!strncmp(p, "while", 5) && !isalnumu(p[5])) {
      cur = new_token(TK_RESERVED, cur, p, 5);
      p += 5;
      continue;
    }

    if (!strncmp(p, "return", 6) && !isalnumu(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (isalphau(*p)) {
      int vlen = 1;
      while (isalnumu(*(p+vlen))) {
        vlen++;
      }
      cur = new_token(TK_IDENT, cur, p, vlen);
      p += vlen;
      continue;
    }

    if (strchr("+-*/()<>=!;", *p)) {
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

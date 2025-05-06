#include "lex.h"

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *token_type_names[] = {[TOKEN_NONE] = "no token",
                            [TOKEN_AT] = "@",
                            [TOKEN_COMMA] = ",",
                            [TOKEN_COLON] = ":",
                            [TOKEN_SEMICOLON] = ";",
                            [TOKEN_PAREN_LEFT] = "(",
                            [TOKEN_PAREN_RIGHT] = ")",
                            [TOKEN_BRACE_LEFT] = "{",
                            [TOKEN_BRACE_RIGHT] = "}",
                            [TOKEN_OP_ADD] = "+",
                            [TOKEN_OP_SUB] = "-",
                            [TOKEN_OP_MUL] = "*",
                            [TOKEN_OP_DIV] = "/",
                            [TOKEN_OP_EQU] = "=",
                            [TOKEN_IDENTIFIER] = "identifier",
                            [TOKEN_INT] = "int",
                            [TOKEN_KW_RET] = "ret",
                            [TOKEN_KW_DEC] = "dec",
                            [TOKEN_KW_IF] = "if",
                            [TOKEN_TYPE_INT] = "int_type",
                            [TOKEN_EOF] = "EOF"};

FILE *src_fd = NULL;

bool is_valid_identifier(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_valid_number(char c) { return (c >= '0' && c <= '9'); }

void print_token(const token_t *token) {
  printf("('%s'", token_type_names[token->type]);
  switch (token->type) {
  case TOKEN_INT:
    printf(": %ld", token->value.int64);
    break;
  case TOKEN_IDENTIFIER:
    printf(": \"%s\"", token->value.str);
    break;
  default:
    break;
  }
  printf(")");
}

long lex_get_pos() { return ftell(src_fd); }
int lex_set_pos(long pos) { return fseek(src_fd, pos, SEEK_SET); }
void set_source_file(FILE *fd) { src_fd = fd; }

#define MATCHES(TYPE, LITERAL)                                                 \
  case TYPE: {                                                                 \
    char content[sizeof(LITERAL)] = {0};                                       \
    fread(content, sizeof(char), sizeof(LITERAL) - 1, src_fd);                 \
    if (strncmp(content, LITERAL, sizeof(LITERAL) - 1) != 0) {                 \
      fseek(src_fd, -((long)sizeof(LITERAL) - 1), SEEK_CUR);                   \
      return false;                                                            \
    }                                                                          \
    break;                                                                     \
  }

// Similar to MATCHES, but fails if following char is identifier char
// We can safely leave the preceding char unchecked because identifiers will
// 'consume' the keyword and keywords will have already failed
#define MATCHES_KW(TYPE, LITERAL)                                              \
  case TYPE: {                                                                 \
    char content[sizeof(LITERAL) + 1] = {0};                                   \
    fread(content, sizeof(char), sizeof(LITERAL), src_fd);                     \
    if (strncmp(content, LITERAL, sizeof(LITERAL) - 2) != 0 ||                 \
        is_valid_ident_char(content[sizeof(LITERAL) - 1])) {                   \
      fseek(src_fd, -((long)sizeof(LITERAL)), SEEK_CUR);                       \
      return false;                                                            \
    }                                                                          \
    fseek(src_fd, -1, SEEK_CUR);                                               \
    break;                                                                     \
  }

#define MATCHES_CHR(TYPE, CHR)                                                 \
  case TYPE: {                                                                 \
    int content = fgetc(src_fd);                                               \
    if (content != CHR) {                                                      \
      fseek(src_fd, -1, SEEK_CUR);                                             \
      return false;                                                            \
    }                                                                          \
    break;                                                                     \
  }

bool is_valid_ident_char(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

void skip_whitespace() {
  int c;
  do {
    c = fgetc(src_fd);
  } while (isspace(c) && c != EOF);
  ungetc(c, src_fd);
}

token_value_t *try_parse_token_value(token_type_t type) {
  skip_whitespace();
  token_value_t *value = malloc(sizeof(token_value_t));
  switch (type) {
  case TOKEN_INT: {
    char content[16] = {0};
    char *endptr;

    // Read 15 chars
    fread(content, sizeof(char), sizeof(content) - 1, src_fd);

    // Parse int
    errno = 0;
    long i = strtol(content, &endptr, 10);
    if (i == 0 && (errno != 0 || endptr - content == 0))
      return NULL;
    value->int64 = i;

    // Seek back to right after num
    fseek(src_fd, (endptr - content) - (long)strlen(content), SEEK_CUR);
    break;
  }
  case TOKEN_IDENTIFIER: {
    char content[32] = {0};
    unsigned int len;

    // Read 15 chars
    fread(content, sizeof(char), sizeof(content) - 1, src_fd);

    // Find end of string
    for (len = 0; len < sizeof(content); ++len) {
      if (!is_valid_ident_char(content[len])) {
        break;
      }
    }
    // If identifier parsed, set the value
    if (len != 0) {
      char *ident = calloc(len, sizeof(char));
      strncpy(ident, content, len);
      ident[len] = '\0';
      value->str = ident;
    }

    // Seek back
    fseek(src_fd, len - (long)strlen(content), SEEK_CUR);

    // Return NULL if no identifier could be parsed
    if (len == 0)
      return NULL;
    break;
  }
  default:
    printf("error: unknown constant token type");
    return NULL;
  }
  return value;
}

bool try_parse_token(token_type_t type) {
  skip_whitespace();
  switch (type) {
    MATCHES_CHR(TOKEN_EOF, EOF);
    MATCHES_CHR(TOKEN_AT, '@');
    MATCHES_CHR(TOKEN_COMMA, ',');
    MATCHES_CHR(TOKEN_COLON, ':');
    MATCHES_CHR(TOKEN_SEMICOLON, ';');
    MATCHES_CHR(TOKEN_PAREN_LEFT, '(');
    MATCHES_CHR(TOKEN_PAREN_RIGHT, ')');
    MATCHES_CHR(TOKEN_BRACE_LEFT, '{');
    MATCHES_CHR(TOKEN_BRACE_RIGHT, '}');
    MATCHES_CHR(TOKEN_OP_ADD, '+');
    MATCHES_CHR(TOKEN_OP_SUB, '-');
    MATCHES_CHR(TOKEN_OP_MUL, '*');
    MATCHES_CHR(TOKEN_OP_DIV, '/');
    MATCHES_CHR(TOKEN_OP_EQU, '=');
    MATCHES_KW(TOKEN_KW_RET, "ret");
    MATCHES_KW(TOKEN_KW_DEC, "dec");
    MATCHES_KW(TOKEN_KW_IF, "if");
    MATCHES_KW(TOKEN_TYPE_INT, "int");
  default:
    printf("error: unknown constant token type");
    return false;
  }
  return true;
}

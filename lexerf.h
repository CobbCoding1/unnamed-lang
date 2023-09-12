#ifndef LEXER_H_
#define LEXER_H

typedef enum {
  INT,
  KEYWORD,
  SEPARATOR,
  END_OF_TOKENS,
} TokenType;

typedef struct {
  TokenType type;
  char *value;
} Token;


void print_token(Token token);
Token *generate_number(char *current, int *current_index);
Token *generate_keyword(char *current, int *current_index);
Token *generate_separator(char *current, int *current_index);
Token *lexer(FILE *file);

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
  BEGINNING,
  INT,
  KEYWORD,
  SEPARATOR,
  OPERATOR,
  IDENTIFIER,
  STRING,
  COMP,
  END_OF_TOKENS,
} TokenType;

typedef struct {
  TokenType type;
  char *value;
  size_t line_num;
} Token;

size_t line_number = 0;

void print_token(Token token){
  printf("TOKEN VALUE: ");
  printf("'");
  for(int i = 0; token.value[i] != '\0'; i++){
    printf("%c", token.value[i]);
  }
  printf("'");
  printf("\nline number: %zu", token.line_num);

  switch(token.type){
    case INT:
      printf(" TOKEN TYPE: INT\n");
      break;
    case KEYWORD:
      printf(" TOKEN TYPE: KEYWORD\n");
      break;
    case SEPARATOR:
      printf(" TOKEN TYPE: SEPARATOR\n");
      break;
    case OPERATOR:
      printf(" TOKEN TYPE: OPERATOR\n");
      break;
    case IDENTIFIER:
      printf(" TOKEN TYPE: IDENTIFIER\n");
      break;
    case STRING:
      printf(" TOKEN TYPE: STRING\n");
      break;
    case COMP:
      printf(" TOKEN TYPE: COMPARATOR\n");
      break;
    case END_OF_TOKENS:
      printf(" END OF TOKENS\n");
      break;
    case BEGINNING:
      printf("BEGINNING\n");
      break;
  }
}

Token *generate_number(char *current, int *current_index){
  Token *token = malloc(sizeof(Token));
  token->line_num = malloc(sizeof(size_t));
  token->line_num = line_number;
  token->type = INT;
  char *value = malloc(sizeof(char) * 8);
  int value_index = 0;
  while(isdigit(current[*current_index]) && current[*current_index] != '\0'){
    if(!isdigit(current[*current_index])){
      break;
    }
    value[value_index] = current[*current_index];
    value_index++;
    *current_index += 1;
  }
  value[value_index] = '\0';
  token->value = value;
  return token;
}

Token *generate_keyword_or_identifier(char *current, int *current_index){
  Token *token = malloc(sizeof(Token));
  token->line_num = malloc(sizeof(size_t));
  token->line_num = line_number;
  char *keyword = malloc(sizeof(char) * 8);
  int keyword_index = 0;
  while(isalpha(current[*current_index]) && current[*current_index] != '\0'){
    keyword[keyword_index] = current[*current_index];
    keyword_index++;
    *current_index += 1;
  }
  keyword[keyword_index] = '\0';
  if(strcmp(keyword, "exit") == 0){
    token->type = KEYWORD;
    token->value = "EXIT";
  } else if(strcmp(keyword, "int") == 0){
    token->type = KEYWORD;
    token->value = "INT";
  } else if(strcmp(keyword, "if") == 0){
    token->type = KEYWORD;
    token->value = "IF";
  } else if(strcmp(keyword, "while") == 0){
    token->type = KEYWORD;
    token->value = "WHILE";
  } else if(strcmp(keyword, "write") == 0){
    token->type = KEYWORD;
    token->value = "WRITE";
  } else if(strcmp(keyword, "eq") == 0){
    token->type = COMP;
    token->value = "EQ";
  } else if(strcmp(keyword, "neq") == 0){
    token->type = COMP;
    token->value = "NEQ";
  } else if(strcmp(keyword, "less") == 0){
    token->type = COMP;
    token->value = "LESS";
  } else if(strcmp(keyword, "greater") == 0){
    token->type = COMP;
    token->value = "GREATER";
  } else {
    token->type = IDENTIFIER;
    token->value = keyword;
  }
  return token;
}

Token *generate_string_token(char *current, int *current_index){
  Token *token = malloc(sizeof(Token));
  token->line_num = malloc(sizeof(size_t));
  token->line_num = line_number;
  char *value = malloc(sizeof(char) * 64);
  int value_index = 0;
  *current_index += 1;
  while(current[*current_index] != '"'){
    value[value_index] = current[*current_index];
    value_index++;
    *current_index += 1;
  }
  value[value_index] = '\0';
  token->type = STRING;
  token->value = value;
  return token;
}

Token *generate_separator_or_operator(char *current, int *current_index, TokenType type){
  Token *token = malloc(sizeof(Token));
  token->value = malloc(sizeof(char) * 2);
  token->value[0] = current[*current_index];
  token->value[1] = '\0';
  token->line_num = malloc(sizeof(size_t));
  token->line_num = line_number;
  token->type = type;
  return token;
}

size_t tokens_index;

Token *lexer(FILE *file){
  int length;
  char *current = 0;

  fseek(file, 0, SEEK_END);
  length = ftell(file);
  fseek(file, 0, SEEK_SET);

  current = malloc(sizeof(char) * length);
  fread(current, 1, length, file);

  fclose(file);

  current[length] = '\0';
  int current_index = 0;

  int number_of_tokens = 12;
  int tokens_size = 0;
  Token *tokens = malloc(sizeof(Token) * number_of_tokens);
  tokens_index = 0;

  while(current[current_index] != '\0'){
    Token *token = malloc(sizeof(Token));
    tokens_size++;
    if(tokens_size > number_of_tokens){
      number_of_tokens *= 1.5;
      tokens = realloc(tokens, sizeof(Token) * number_of_tokens);
    }
    if(current[current_index] == ';'){
      token = generate_separator_or_operator(current, &current_index, SEPARATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == ','){
      token = generate_separator_or_operator(current, &current_index, SEPARATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == '('){
      token = generate_separator_or_operator(current, &current_index, SEPARATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == ')'){
      token = generate_separator_or_operator(current, &current_index, SEPARATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == '{'){
      token = generate_separator_or_operator(current, &current_index, SEPARATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == '}'){
      token = generate_separator_or_operator(current, &current_index, SEPARATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == '='){
      token = generate_separator_or_operator(current, &current_index, OPERATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == '+'){
      token = generate_separator_or_operator(current, &current_index, OPERATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == '-'){
      token = generate_separator_or_operator(current, &current_index, OPERATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == '*'){
      token = generate_separator_or_operator(current, &current_index, OPERATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == '/'){
      token = generate_separator_or_operator(current, &current_index, OPERATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == '%'){
      token = generate_separator_or_operator(current, &current_index, OPERATOR);
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == '"'){
      token = generate_string_token(current, &current_index);
      tokens[tokens_index] = *token;
      tokens_index++;
    }else if(isdigit(current[current_index])){
      token = generate_number(current, &current_index); 
      tokens[tokens_index] = *token;
      tokens_index++;
      current_index--;
    } else if(isalpha(current[current_index])){
      token = generate_keyword_or_identifier(current, &current_index);
      tokens[tokens_index] = *token;
      tokens_index++;
      current_index--;
    } else if(current[current_index] == '\n'){
      line_number += 1;
    } 
    free(token);
    current_index++;
  }
  tokens[tokens_index].value = '\0';
  tokens[tokens_index].type = END_OF_TOKENS;
  return tokens;
}



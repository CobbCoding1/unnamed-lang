#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

void print_token(Token token){
  printf("TOKEN VALUE: ");
  for(int i = 0; token.value[i] != '\0'; i++){
    printf("%c", token.value[i]);
  }
  if(token.type == INT){
    printf(" TOKEN TYPE: INT\n");
  }
  if(token.type == KEYWORD){
    printf(" TOKEN TYPE: KEYWORD\n");
  }
  if(token.type == SEPARATOR){
    printf(" TOKEN TYPE: SEPARATOR\n");
  }
}

Token *generate_number(char *current, int *current_index){
  Token *token = malloc(sizeof(Token));
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
  int dig = isdigit(current[*current_index]);
  printf("is digit: %d | is end of line: %d\n", dig, current[*current_index] != '\0');
  printf("VALUE INDEX: %d\n", value_index);
  token->value = value;
  return token;
}

Token *generate_keyword(char *current, int *current_index){
  Token *token = malloc(sizeof(Token));
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
  }
  return token;
}

size_t tokens_index;

Token *lexer(FILE *file){
  int length;
  char *buffer = 0;
  fseek(file, 0, SEEK_END);
  length = ftell(file);
  fseek(file, 0, SEEK_SET);
  buffer = malloc(sizeof(char) * length);
  fread(buffer, 1, length, file);
  fclose(file);
  buffer[length + 1] = '\0';
  char *current = malloc(sizeof(char) * length + 1);
  current = buffer;
  int current_index = 0;

  Token *tokens = malloc(sizeof(Token) * 1024);
  tokens_index = 0;

  while(current[current_index] != '\0'){
    Token *token = malloc(sizeof(Token));
    printf("current index %d\n", current_index);
    if(current[current_index] == ';'){
      token = malloc(sizeof(Token));
      token->value[0] = current[current_index];
      token->value[1] = '\0';
      token->type = SEPARATOR;
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == '('){
      token = malloc(sizeof(Token));
      token->value[0] = current[current_index];
      token->value[1] = '\0';
      token->type = SEPARATOR;
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(current[current_index] == ')'){
      token = malloc(sizeof(Token));
      token->value[0] = current[current_index];
      token->value[1] = '\0';
      token->type = SEPARATOR;
      tokens[tokens_index] = *token;
      tokens_index++;
    } else if(isdigit(current[current_index])){
      token = generate_number(current, &current_index); 
      printf("TOKEN VALUE: %s\n", token->value);
      tokens[tokens_index] = *token;
      tokens_index++;
      current_index--;
    } else if(isalpha(current[current_index])){
      token = generate_keyword(current, &current_index);
      print_token(*token);
      tokens[tokens_index] = *token;
      tokens_index++;
      current_index--;
    }

    free(token);
    current_index++;
  }
  tokens[tokens_index].value = '\0';
  tokens[tokens_index].type = END_OF_TOKENS;
  return tokens;
}



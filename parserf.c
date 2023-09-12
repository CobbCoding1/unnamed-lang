#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexerf.h"

typedef enum {
  INT_LIT,
  STATEMENT,
  EXTRA,
  BEGINNING,
} NodeTypes;

typedef struct Node {
  char *value;
  NodeTypes type;
  struct Node *right;
  struct Node *left;
} Node;

void print_tree(Node *node, int indent, char *identifier){
  if(node == NULL){
    return;
  }
  for(int i = 0; i < indent; i++){
    printf(" ");
  }
  printf("%s -> ", identifier);
  for(size_t i = 0; node->value[i] != '\0'; i++){
    printf("%c", node->value[i]);
  }
  printf("\n");
  print_tree(node->left, indent + 1, "left");
  print_tree(node->right, indent + 1, "right");
}

Node *init_node(Node *node, char *value, NodeTypes type){
  node = malloc(sizeof(Node));
  node->value = malloc(sizeof(char) * 2);
  node->type = (int)type;
  node->value = value;
  node->left = NULL;
  node->right = NULL;
  return node;
}

Token *parser(Token *tokens){
  Token *current_token = &tokens[0];
  Node *root = malloc(sizeof(Node));
  Node *left = malloc(sizeof(Node));
  Node *right = malloc(sizeof(Node));
  root = init_node(root, "PROGRAM", BEGINNING);

  Node *current = root;

  while(current_token->type != END_OF_TOKENS){
    if(current == NULL){
      break;
    }
    if(current == root){
      //;
    }

    if(current_token->type == KEYWORD && strcmp(current_token->value, "exit")){
      Node *exit_node = malloc(sizeof(Node));
      exit_node = init_node(exit_node, current_token->value, STATEMENT);
      root->right = exit_node; 
      current = exit_node;
      current_token++;
      if(current_token->type != SEPARATOR){
        printf("ERROR\n");
        exit(1);
      }
      Node *open_paren_node = malloc(sizeof(Node));
      open_paren_node = init_node(open_paren_node, current_token->value, EXTRA);
      current->left = open_paren_node;

      current_token++;

      Node *expr_node = malloc(sizeof(Node));
      expr_node = init_node(expr_node, current_token->value, INT_LIT);
      current->left->left = expr_node;

      current_token++;

      Node *close_paren_node = malloc(sizeof(Node));
      close_paren_node = init_node(close_paren_node, current_token->value, EXTRA);
      current->left->right = close_paren_node;

      current_token++;

      Node *semi_node = malloc(sizeof(Node));
      semi_node = init_node(semi_node, current_token->value, EXTRA);
      current->right = semi_node;
      
      printf("EXIT\n");
      break;
    }
    current_token++;
  }
  print_tree(root, 0, "root");
  return current_token;
}

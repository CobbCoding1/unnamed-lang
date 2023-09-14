#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexerf.h"

typedef struct Node {
  char *value;
  TokenType type;
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

Node *init_node(Node *node, char *value, TokenType type){
  node = malloc(sizeof(Node));
  node->value = malloc(sizeof(char) * 2);
  node->type = (int)type;
  node->value = value;
  node->left = NULL;
  node->right = NULL;
  return node;
}

void print_error(char *error_type){
  printf("ERROR: %s\n", error_type);
  exit(1);
}

Token *generate_operation_nodes(Token *current_token, Node *current_node){
  Node *oper_node = malloc(sizeof(Node));
  oper_node = init_node(oper_node, current_token->value, OPERATOR);
  current_node->left->left = oper_node;
  current_node = oper_node;
  printf("CURRENT TOKEN 1: %s\n", current_token->value);
  current_token--;
  Node *expr_node = malloc(sizeof(Node));
  expr_node = init_node(expr_node, current_token->value, INT);
  current_node->left = expr_node;
  current_token++;
  current_token++;
  while(current_token->type == INT || current_token->type == OPERATOR){
    if(current_token->type == INT){
      printf("CURRENT TOKEN 3: %s\n", current_token->value);
      if(current_token->type != INT || current_token == NULL){
        printf("Syntax Error hERE\n");
        exit(1);
      }
      current_token++;
      if(current_token->type != OPERATOR){
        current_token--;
        Node *second_expr_node = malloc(sizeof(Node));
        second_expr_node = init_node(second_expr_node, current_token->value, INT);
        current_node->right = second_expr_node;
      }
    }
    if(current_token->type == OPERATOR){
      Node *next_oper_node = malloc(sizeof(Node));
      next_oper_node = init_node(next_oper_node, current_token->value, OPERATOR);
      current_node->right = next_oper_node;
      current_node = next_oper_node;
      current_token--;
      Node *second_expr_node = malloc(sizeof(Node));
      second_expr_node = init_node(second_expr_node, current_token->value, INT);
      current_node->left = second_expr_node;
      current_token++; 
    }
    current_token++;
  }
  return current_token;
}

Node *parser(Token *tokens){
  Token *current_token = &tokens[0];
  Node *root = malloc(sizeof(Node));
  root = init_node(root, "PROGRAM", BEGINNING);

  Node *current = root;

  while(current_token->type != END_OF_TOKENS){
    if(current == NULL){
      break;
    }
    if(current == root){
      //;
    }

    switch(current_token->type){
      case KEYWORD:
        printf("TOKEN VALUE: %s\n", current_token->value);
        if(strcmp(current_token->value, "EXIT") == 0){
          Node *exit_node = malloc(sizeof(Node));
          exit_node = init_node(exit_node, current_token->value, KEYWORD);
          root->right = exit_node;
          current = exit_node;
          current_token++;
          if(current_token->type == END_OF_TOKENS){
            print_error("Invalid Syntax on OPEN");
          }
          if(strcmp(current_token->value, "(") == 0 && current_token->type == SEPARATOR){
            Node *open_paren_node = malloc(sizeof(Node));
            open_paren_node = init_node(open_paren_node, current_token->value, SEPARATOR);
            current->left = open_paren_node;
            current_token++;
            if(current_token->type == END_OF_TOKENS){
              print_error("Invalid Syntax on INT");
            }
            if(current_token->type == INT){
              current_token++;
              if(current_token->type == OPERATOR && current_token != NULL){
                current_token = generate_operation_nodes(current_token, current);
                current_token--;
              } else {
                current_token--;
                Node *expr_node = malloc(sizeof(Node));
                expr_node = init_node(expr_node, current_token->value, INT);
                current->left->left = expr_node;
              }
              current_token++;
              printf("current token: %s\n", current_token->value);
              if(current_token->type == END_OF_TOKENS){
                print_error("Invalid Syntax on cLOSE");
              }
              if(strcmp(current_token->value, ")") == 0 && current_token->type == SEPARATOR && current_token->type != END_OF_TOKENS){
                Node *close_paren_node = malloc(sizeof(Node));
                close_paren_node = init_node(close_paren_node, current_token->value, SEPARATOR);
                current->left->right = close_paren_node;
                current_token++;
                if(current_token->type == END_OF_TOKENS){
                  print_error("Invalid Syntax on SEMI");
                }
                if(strcmp(current_token->value, ";") == 0 && current_token->type == SEPARATOR){
                  Node *semi_node = malloc(sizeof(Node));
                  semi_node = init_node(semi_node, current_token->value, SEPARATOR);
                  current->right = semi_node;
                  break;
                } else {
                  print_error("Invalid Syntax on SEMI");
                }
              } else {
                  print_error("Invalid Syntax on CLOSE");
              }
            } else {
              print_error("Invalid Syntax INT");
            }
        } else {
          print_error("Invalid Syntax OPEN");
        }
      }
      case SEPARATOR:
        break; 
      case OPERATOR:
        break;
      case INT:
        printf("INTEGER\n");
        break;
      case BEGINNING:
        //
      case END_OF_TOKENS:
        //
    }
    current_token++;
  }
  print_tree(root, 0, "root");
  return root;
}

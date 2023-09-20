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
  if(current_token->type == INT){
    Node *expr_node = malloc(sizeof(Node));
    expr_node = init_node(expr_node, current_token->value, INT);
    current_node->left = expr_node;
  } else if(current_token->type == IDENTIFIER){
    Node *identifier_node = malloc(sizeof(Node));
    identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
    current_node->left = identifier_node;
  } else {
    printf("ERROR: expected int or identifier\n");
    exit(1);
  }
  current_token++;
  current_token++;
  while(current_token->type == INT || current_token->type == IDENTIFIER || current_token->type == OPERATOR){
    if(current_token->type == INT || current_token->type == IDENTIFIER){
      printf("CURRENT TOKEN 3: %s\n", current_token->value);
      if((current_token->type != INT && current_token->type != IDENTIFIER) || current_token == NULL){
        printf("Syntax Error hERE\n");
        exit(1);
      }
      printf("AJSDKLJASLK current token: %s\n", current_token->value);
      current_token++;
      if(current_token->type != OPERATOR){
        current_token--;
        if(current_token->type == INT){
          Node *second_expr_node = malloc(sizeof(Node));
          second_expr_node = init_node(second_expr_node, current_token->value, INT);
          current_node->right = second_expr_node;
        } else if(current_token->type == IDENTIFIER){
          Node *second_identifier_node = malloc(sizeof(Node));
          second_identifier_node = init_node(second_identifier_node, current_token->value, IDENTIFIER);
          current_node->right = second_identifier_node;
        } else {
          printf("ERROR: Expected Integer or Identifier\n");
          exit(1);
        }
      }
    }
    if(current_token->type == OPERATOR){
      Node *next_oper_node = malloc(sizeof(Node));
      next_oper_node = init_node(next_oper_node, current_token->value, OPERATOR);
      current_node->right = next_oper_node;
      current_node = next_oper_node;
      current_token--;
      if(current_token->type == INT){
        Node *second_expr_node = malloc(sizeof(Node));
        second_expr_node = init_node(second_expr_node, current_token->value, INT);
        current_node->left = second_expr_node;
      } else if(current_token->type == IDENTIFIER){
        Node *second_identifier_node = malloc(sizeof(Node));
        second_identifier_node = init_node(second_identifier_node, current_token->value, IDENTIFIER);
        current_node->left = second_identifier_node;
      } else {
        printf("ERROR: Expected IDENTIFIER or INT\n");
        exit(1);
      }
      current_token++; 
    }
    current_token++;
  }
  return current_token;
}

void handle_exit_syscall(Node *root, Token *current_token, Node *current){
    Node *exit_node = malloc(sizeof(Node));
    exit_node = init_node(exit_node, current_token->value, KEYWORD);
    current->right = exit_node;
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
      if(current_token->type == INT || current_token->type == IDENTIFIER){
        current_token++;
        if(current_token->type == OPERATOR && current_token != NULL){
          current_token = generate_operation_nodes(current_token, current);
          current_token--;
        } else {
          current_token--;
          Node *expr_node = malloc(sizeof(Node));
          expr_node = init_node(expr_node, current_token->value, current_token->type);
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

void handle_token_errors(char *error_text, Token *current_token, TokenType type){
  if(current_token->type == END_OF_TOKENS || current_token->type != type){
    print_error(error_text);
  }
}

Node *create_variable_reusage(Token *current_token, Node *current){
  Node *main_identifier_node = malloc(sizeof(Node));
  main_identifier_node = init_node(main_identifier_node, current_token->value, IDENTIFIER);
  current->left = main_identifier_node;
  current = main_identifier_node;
  current_token++;

  handle_token_errors("Invalid syntax after idenfitier", current_token, OPERATOR);

  if(current_token->type == OPERATOR){
    if(strcmp(current_token->value, "=") != 0){
      print_error("Invalid Variable Syntax on =");
    }
    Node *equals_node = malloc(sizeof(Node));
    equals_node = init_node(equals_node, current_token->value, OPERATOR);
    current->left = equals_node;
    current = equals_node;
    current_token++;
  }
  if(current_token->type == END_OF_TOKENS || (current_token->type != INT && current_token->type != IDENTIFIER)){
    print_error("Invalid Syntax After Equals");
  }

  current_token++;
  printf("Token in variable parse: %s\n", current_token->value);
  if(current_token->type == OPERATOR){
    Node *oper_node = malloc(sizeof(Node));
    oper_node = init_node(oper_node, current_token->value, OPERATOR);
    current->left = oper_node;
    current = oper_node;
    current_token--;
    if(current_token->type == INT){
      Node *expr_node = malloc(sizeof(Node));
      expr_node = init_node(expr_node, current_token->value, INT);
      oper_node->left = expr_node;
      current_token++;
      current_token++;
    } else if(current_token->type == IDENTIFIER){
      Node *identifier_node = malloc(sizeof(Node));
      identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
      oper_node->left = identifier_node;
      current_token++;
      current_token++;
    } else {
      print_error("ERROR: Expected IDENTIFIER or INT");
    }
    current_token++;
    printf("CURRENST TOKEN: %s\n", current_token->value);

    if(current_token->type == OPERATOR){
      Node *oper_node = malloc(sizeof(Node));
      oper_node = init_node(oper_node, current_token->value, OPERATOR);
      current->right = oper_node;
      current = oper_node;
      int operation = 1;
      current_token--;
      current_token--;
      while(operation){
        current_token++;
        if(current_token->type == INT){
          Node *expr_node = malloc(sizeof(Node));
          expr_node = init_node(expr_node, current_token->value, INT);
          current->left = expr_node;
        } else if(current_token->type == IDENTIFIER){
          Node *identifier_node = malloc(sizeof(Node));
          identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
          current->left = identifier_node;
        } else {
          printf("ERROR: Unexpected Token\n");
          exit(1);
        }
        current_token++;
        if(current_token->type == OPERATOR){
          current_token++;
          current_token++;
          if(current_token->type != OPERATOR){
            current_token--;
            if(current_token->type == INT){
              printf("CURR TOKEN! %s\n", current_token->value);
              Node *expr_node = malloc(sizeof(Node));
              expr_node = init_node(expr_node, current_token->value, INT);
              current->right = expr_node;
              current_token++;
            } else if(current_token->type == IDENTIFIER){
              Node *identifier_node = malloc(sizeof(Node));
              identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
              current->right = identifier_node;
              current_token++;
            } else {
              printf("ERROR: UNRECOGNIZED TOKEN!\n");
              exit(1);
            }
            operation = 0;
          } else {
            current_token--;
            current_token--;
            Node *oper_node = malloc(sizeof(Node));
            oper_node = init_node(oper_node, current_token->value, OPERATOR);
            current->right = oper_node;
            current = oper_node;
          }
        } else {
          operation = 0;
        }
      }
    } else {
      current_token--;
      printf("CORRENT TOKEN: %s\n", current_token->value);
      if(current_token->type == INT){
        Node *expr_node = malloc(sizeof(Node));
        expr_node = init_node(expr_node, current_token->value, INT);
        oper_node->right = expr_node;
      } else if(current_token->type == IDENTIFIER){
        Node *identifier_node = malloc(sizeof(Node));
        identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
        oper_node->right = identifier_node;
      }
      current_token++;
    }
  } else {
    current_token--;
    if(current_token->type == INT){
      printf("INTAGET\n");
      Node *expr_node = malloc(sizeof(Node));
      expr_node = init_node(expr_node, current_token->value, INT);
      current->left = expr_node;
      current_token++;
    } else if(current_token->type == IDENTIFIER){
      Node *identifier_node = malloc(sizeof(Node));
      identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
      current->left = identifier_node;
      current_token++;
    }
  }
  handle_token_errors("Invalid Syntax After Expression", current_token, SEPARATOR);

  current = main_identifier_node;
  printf("CURRRR TOKEN: %s\n", current_token->value);
  if(strcmp(current_token->value, ";") == 0){
    Node *semi_node = malloc(sizeof(Node));
    semi_node = init_node(semi_node, current_token->value, SEPARATOR);
    current->right = semi_node;
    current = semi_node;
  }
  return current;
}


Node *create_variables(Token *current_token, Node *current){
  Node *var_node = malloc(sizeof(Node));
  var_node = init_node(var_node, current_token->value, KEYWORD);
  current->left = var_node;
  current = var_node;
  current_token++;
  handle_token_errors("Invalid syntax after INT", current_token, IDENTIFIER);
  if(current_token->type == IDENTIFIER){
    Node *identifier_node = malloc(sizeof(Node));
    identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
    current->left = identifier_node;
    current = identifier_node;
    current_token++;
  }
  handle_token_errors("Invalid Syntax After Identifier", current_token, OPERATOR);

  if(current_token->type == OPERATOR){
    if(strcmp(current_token->value, "=") != 0){
      print_error("Invalid Variable Syntax on =");
    }
    Node *equals_node = malloc(sizeof(Node));
    equals_node = init_node(equals_node, current_token->value, OPERATOR);
    current->left = equals_node;
    current = equals_node;
    current_token++;
  }
  if(current_token->type == END_OF_TOKENS || (current_token->type != INT && current_token->type != IDENTIFIER)){
    print_error("Invalid Syntax After Equals");
  }

  current_token++;
  printf("CURRRRRRRRRRRRR: %s\n", current_token->value);
  if(current_token->type == OPERATOR){
    Node *oper_node = malloc(sizeof(Node));
    oper_node = init_node(oper_node, current_token->value, OPERATOR);
    current->left = oper_node;
    current = oper_node;
    current_token--;
    if(current_token->type == INT){
      Node *expr_node = malloc(sizeof(Node));
      expr_node = init_node(expr_node, current_token->value, INT);
      oper_node->left = expr_node;
      current_token++;
      current_token++;
    } else if(current_token->type == IDENTIFIER){
      Node *identifier_node = malloc(sizeof(Node));
      identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
      oper_node->left = identifier_node;
      current_token++;
      current_token++;
    } else {
      print_error("ERROR: Expected IDENTIFIER or INT");
    }
    current_token++;
    printf("CURRENST TOKEN: %s\n", current_token->value);

    if(current_token->type == OPERATOR){
      Node *oper_node = malloc(sizeof(Node));
      oper_node = init_node(oper_node, current_token->value, OPERATOR);
      current->right = oper_node;
      current = oper_node;
      int operation = 1;
      current_token--;
      current_token--;
      while(operation){
        current_token++;
        if(current_token->type == INT){
          Node *expr_node = malloc(sizeof(Node));
          expr_node = init_node(expr_node, current_token->value, INT);
          current->left = expr_node;
        } else if(current_token->type == IDENTIFIER){
          Node *identifier_node = malloc(sizeof(Node));
          identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
          current->left = identifier_node;
        } else {
          printf("ERROR: Unexpected Token\n");
          exit(1);
        }
        current_token++;
        if(current_token->type == OPERATOR){
          current_token++;
          current_token++;
          if(current_token->type != OPERATOR){
            current_token--;
            if(current_token->type == INT){
              printf("CURR TOKEN! %s\n", current_token->value);
              Node *expr_node = malloc(sizeof(Node));
              expr_node = init_node(expr_node, current_token->value, INT);
              current->right = expr_node;
              current_token++;
            } else if(current_token->type == IDENTIFIER){
              Node *identifier_node = malloc(sizeof(Node));
              identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
              current->right = identifier_node;
              current_token++;
            } else {
              printf("ERROR: UNRECOGNIZED TOKEN!\n");
              exit(1);
            }
            operation = 0;
          } else {
            current_token--;
            current_token--;
            Node *oper_node = malloc(sizeof(Node));
            oper_node = init_node(oper_node, current_token->value, OPERATOR);
            current->right = oper_node;
            current = oper_node;
          }
        } else {
          operation = 0;
        }
      }
    } else {
      current_token--;
      printf("CORRENT TOKEN: %s\n", current_token->value);
      if(current_token->type == INT){
        Node *expr_node = malloc(sizeof(Node));
        expr_node = init_node(expr_node, current_token->value, INT);
        oper_node->right = expr_node;
      } else if(current_token->type == IDENTIFIER){
        Node *identifier_node = malloc(sizeof(Node));
        identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
        oper_node->right = identifier_node;
      }
      current_token++;
    }
  } else {
    current_token--;
    if(current_token->type == INT){
      Node *expr_node = malloc(sizeof(Node));
      expr_node = init_node(expr_node, current_token->value, INT);
      current->left = expr_node;
      current_token++;
    } else if(current_token->type == IDENTIFIER){
      Node *identifier_node = malloc(sizeof(Node));
      identifier_node = init_node(identifier_node, current_token->value, IDENTIFIER);
      current->left = identifier_node;
      current_token++;
    }
  }

  //if(current_token->type == OPERATOR){
  //  current_token = generate_operation_nodes(current_token, current);
  //}

  handle_token_errors("Invalid Syntax After Expression", current_token, SEPARATOR);

  current = var_node;
  printf("CURRRR TOKEN: %s\n", current_token->value);
  if(strcmp(current_token->value, ";") == 0){
    Node *semi_node = malloc(sizeof(Node));
    semi_node = init_node(semi_node, current_token->value, SEPARATOR);
    current->right = semi_node;
    current = semi_node;
  }
  return current;
}

Node *parser(Token *tokens){
  Token *current_token = &tokens[0];
  Node *root = malloc(sizeof(Node));
  root = init_node(root, "PROGRAM", BEGINNING);

  Node *current = root;

  Node *open_curly = malloc(sizeof(Node));
  Node *close_curly = malloc(sizeof(Node));

  while(current_token->type != END_OF_TOKENS){
    if(current == NULL){
      break;
    }
    switch(current_token->type){
      case KEYWORD:
        printf("TOKEN VALUE: %s\n", current_token->value);
        if(strcmp(current_token->value, "EXIT") == 0){
          handle_exit_syscall(root, current_token, current);
        }
        if(strcmp(current_token->value, "INT") == 0){
          current = create_variables(current_token, current);
        }
        break;
      case SEPARATOR:
        if(strcmp(current_token->value, "{") == 0){
          Token *temp = current_token;
          open_curly = init_node(open_curly, temp->value, SEPARATOR);
          current->left = open_curly;
          current = open_curly;
          int curly_count = 0;
          int close_curly_count = 0;
          close_curly = init_node(close_curly, "}", SEPARATOR);
          current->right = close_curly;
          temp = &tokens[0];
          //temp++;
          while(temp->type != END_OF_TOKENS){
            if(temp == NULL){
              printf("ERROR: Expected }\n");
              exit(1);
            }
            if(temp->type == SEPARATOR){
              if(strcmp(temp->value, "{") == 0){
                curly_count++;
              }
              if(strcmp(temp->value, "}") == 0){
                close_curly_count++;
              }
            }
            temp++;
          }
          if(curly_count != close_curly_count){
            printf("%d:%d\n", curly_count, close_curly_count);
            printf("ERROR: Expected curly brace\n");
            exit(1);
          }
          //
        }
        if(strcmp(current_token->value, "}") == 0){
          current = close_curly;
        }
        break; 
      case OPERATOR:
        break;
      case INT:
        printf("INTEGER\n");
        break;
      case IDENTIFIER:
        current_token--;
        if(current_token->type == SEPARATOR && strcmp(current_token->value, ";") == 0){
          current_token++;
          current = create_variable_reusage(current_token, current);
        } else {
          current_token++;
        }
        break;
      case BEGINNING:
        break;
      case END_OF_TOKENS:
        break;
    }
    current_token++;
  }
  print_tree(root, 0, "root");
  return root;
}

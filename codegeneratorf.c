#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <ctype.h>
#include <assert.h>

#include "lexerf.h"
#include "parserf.h"
#include "./hashmap/hashmapoperators.h"
//#include "./hashmap/hashmapvars.h"
#include "./hashmap/hashmap.h"

size_t stack_size = 0;
size_t current_stack_size = 0;
const unsigned initial_size = 100;
struct hashmap_s hashmap;

void push(char *reg, FILE *file){
  fprintf(file, "  push %s\n", reg);
  stack_size++;
}

void push_var(size_t stack_pos, FILE *file){
  fprintf(file, "  push QWORD [rsp + %zu]\n", (stack_size - stack_pos) * 8);
  stack_size++;
}

void pop(char *reg, FILE *file){
  fprintf(file, "  pop %s\n", reg);
  stack_size--;
}

void mov(char *reg1, char *reg2, FILE *file){
  fprintf(file, "  mov %s, %s\n", reg1, reg2);
}

Node *handle_adding(Node *tmp, FILE *file){
  pop("rax", file);
  tmp = tmp->right;
  if(tmp->left->type == INT){
    mov("rbx", tmp->left->value, file);
  } else if(tmp->left->type == IDENTIFIER){
    int *element = malloc(sizeof(int));
    element = hashmap_get(&hashmap, tmp->left->value, strlen(tmp->left->value));
    if(element == NULL){
      printf("ERROR: Not in hashmap!\n");
      exit(1);
    }
    push_var(*element, file);
    pop("rbx", file);
  }

  fprintf(file, "  %s rax, rbx\n", "add");
  push("rax", file);
  return tmp;
}

Node *handle_subbing(Node *tmp, FILE *file){
  pop("rax", file);
  tmp = tmp->right;
  if(tmp->left->type == INT){
    mov("rbx", tmp->left->value, file);
  } else if(tmp->left->type == IDENTIFIER){
    int *element = malloc(sizeof(int));
    element = hashmap_get(&hashmap, tmp->left->value, strlen(tmp->left->value));
    if(element == NULL){
      printf("ERROR: Not in hashmap!\n");
      exit(1);
    }
    push_var(*element, file);
    pop("rbx", file);
  }
  fprintf(file, "  %s rax, rbx\n", "sub");
  push("rax", file);
  return tmp;
}

Node *handle_mul(Node *tmp, FILE *file){
  pop("rax", file);
  tmp = tmp->right;
  if(tmp->left->type == INT){
    mov("rbx", tmp->left->value, file);
  } else if(tmp->left->type == IDENTIFIER){
    int *element = malloc(sizeof(int));
    element = hashmap_get(&hashmap, tmp->left->value, strlen(tmp->left->value));
    if(element == NULL){
      printf("ERROR: Not in hashmap!\n");
      exit(1);
    }
    push_var(*element, file);
    pop("rbx", file);
  }
  fprintf(file, "  mul rbx\n");
  push("rax", file);
  return tmp;
}

Node *handle_div(Node *tmp, FILE *file){
  pop("rax", file);
  tmp = tmp->right;
  if(tmp->left->type == INT){
    mov("rbx", tmp->left->value, file);
  } else if(tmp->left->type == IDENTIFIER){
    int *element = malloc(sizeof(int));
    element = hashmap_get(&hashmap, tmp->left->value, strlen(tmp->left->value));
    if(element == NULL){
      printf("ERROR: Not in hashmap!\n");
      exit(1);
    }
    push_var(*element, file);
    pop("rbx", file);
  }
  fprintf(file, "  div rbx\n");
  push("rax", file);
  return tmp;
}

Node *generate_operator_code(Node *node, FILE *file){
  Node *tmp = node;
  if(tmp->left->type == INT){
    mov("rax", tmp->left->value, file);
  } else if(tmp->left->type == IDENTIFIER){
    int *element = malloc(sizeof(int));
    element = hashmap_get(&hashmap, tmp->left->value, strlen(tmp->left->value));
    if(element == NULL){
      printf("ERROR: Not in hashmap!\n");
      exit(1);
    }
    push_var(*element, file);
    pop("rax", file);
  }
  push("rax", file);
  int did_loop = 0;
  while(tmp->right->type == OPERATOR){
    did_loop = 1;
    char oper = tmp->value[0];
    if(oper == '+'){
      tmp = handle_adding(tmp, file);
    } else if(oper == '-'){
      tmp = handle_subbing(tmp, file);
    } else if(oper == '*'){
      tmp = handle_mul(tmp, file);
    } else if(oper == '/'){
      tmp = handle_div(tmp, file);
    }
  }
  if(did_loop){
    if(tmp->value[0] == '*' || tmp->value[0] == '/'){
      pop("rax", file);
      if(tmp->right->type == INT){
        mov("rbx", tmp->right->value, file);
      } else if(tmp->right->type == IDENTIFIER){
        int *element = malloc(sizeof(int));
        element = hashmap_get(&hashmap, tmp->right->value, strlen(tmp->right->value));
        if(element == NULL){
          printf("ERROR: Not in hashmap!\n");
          exit(1);
        }
        push_var(*element, file);
        pop("rbx", file);
      }
      fprintf(file, "  %s rbx\n", search(tmp->value[0])->data);
      push("rax", file);
    } else {
      pop("rax", file);
      if(tmp->right->type == INT){
        mov("rbx", tmp->right->value, file);
      } else if(tmp->right->type == IDENTIFIER){
        int *element = malloc(sizeof(int));
        element = hashmap_get(&hashmap, tmp->right->value, strlen(tmp->right->value));
        if(element == NULL){
          printf("ERROR: Not in hashmap!\n");
          exit(1);
        }
        push_var(*element, file);
        pop("rbx", file);
      }
      fprintf(file, "  %s rax, rbx\n", search(tmp->value[0])->data);
      push("rax", file);
    }
  } else {
    if(tmp->value[0] == '-'){
      if(tmp->right->type == INT){
        mov("rbx", tmp->right->value, file);
      } else if(tmp->right->type == IDENTIFIER){
        int *element = malloc(sizeof(int));
        element = hashmap_get(&hashmap, tmp->right->value, strlen(tmp->right->value));
        if(element == NULL){
          printf("ERROR: Not in hashmap!\n");
          exit(1);
        }
        push_var(*element, file);
        pop("rbx", file);
      }
      pop("rax", file);
      fprintf(file, "  %s rax, rbx\n", "sub");
      push("rax", file);
    } else if(tmp->value[0] == '+'){
        if(tmp->right->type == INT){
          mov("rbx", tmp->right->value, file);
          //fprintf(file, "  add rax, rbx\n");
          //push("rbx", file);
        } else if(tmp->right->type == IDENTIFIER){
          int *element = malloc(sizeof(int));
          element = hashmap_get(&hashmap, tmp->right->value, strlen(tmp->right->value));
          if(element == NULL){
            printf("ERROR: Not in hashmap!\n");
            exit(1);
          }
          push_var(*element, file);
          pop("rbx", file);
        }
        fprintf(file, "  add rax, rbx\n");
        push("rax", file);
      } else if(tmp->value[0] == '*'){
        if(tmp->right->type == INT){
          mov("rbx", tmp->right->value, file);
        } else if(tmp->right->type == IDENTIFIER){
          int *element = malloc(sizeof(int));
          element = hashmap_get(&hashmap, tmp->right->value, strlen(tmp->right->value));
          if(element == NULL){
            printf("ERROR: Not in hashmap!\n");
            exit(1);
          }
          push_var(*element, file);
          pop("rbx", file);
        }
        fprintf(file, "  mul rbx\n");
        push("rax", file);
      } else if(tmp->value[0] == '/'){
          if(tmp->right->type == INT){
            mov("rbx", tmp->right->value, file);
          } else if(tmp->right->type == IDENTIFIER){
            int *element = malloc(sizeof(int));
            element = hashmap_get(&hashmap, tmp->right->value, strlen(tmp->right->value));
            if(element == NULL){
              printf("ERROR: Not in hashmap!\n");
              exit(1);
            }
            push_var(*element, file);
            pop("rbx", file);
          }
        fprintf(file, "  div rbx\n");
        push("rax", file);
    }
  }
  node->left = NULL;
  node->right = NULL;
  return node;
}

void traverse_tree(Node *node, int is_left, FILE *file, int syscall_number){
  if(node == NULL){
    return;
  }
  if(strcmp(node->value, "EXIT") == 0){
    syscall_number = 60;
  }
  if(strcmp(node->value, "INT") == 0){
    // Push
    Node *value = node->left->left->left;
    if(value->type == IDENTIFIER){
      size_t *var_value = malloc(sizeof(size_t));
      var_value = hashmap_get(&hashmap, value->value, strlen(value->value));
      if(var_value == NULL){
        printf("ERROR: Value not in hashmap\n");
        exit(1);
      }
      push_var(*var_value, file);
    } else if(value->type == INT) {
      push(value->value, file);
    } else if(value->type == OPERATOR){
      generate_operator_code(value, file);
    } else {
      printf("ERROR\n");
      exit(1);
    }
    size_t *cur_size = malloc(sizeof(size_t));
    *cur_size = stack_size;
    if(hashmap_put(&hashmap, node->left->value, strlen(node->left->value), cur_size) != 0){
      printf("ERROR: Could not insert into hash table!\n");
      exit(1);
    }
    node->left = NULL;

  }
  if(strcmp(node->value, "(") == 0){

  }
  if(node->type == OPERATOR){
    if(node->value[0] == '='){

    } else {
      generate_operator_code(node, file);
    }
  } 
  if(node->type == INT){
    fprintf(file, "  mov rax, %s\n", node->value);
    push("rax", file);
  }
  if(node->type == IDENTIFIER){
    if(syscall_number == 60){
       fprintf(file, "  mov rax, %d\n", syscall_number);
       size_t *var_value = malloc(sizeof(size_t));
       var_value = hashmap_get(&hashmap, node->value, strlen(node->value));
       if(var_value == NULL){
         printf("ERROR: Value not in hashmap\n");
         exit(1);
       }
       push_var(*var_value, file);
       //fprintf(file, "  push QWORD [rsp + %zu]\n", (stack_size - *var_value) * 8);
       pop("rdi", file);
       fprintf(file, "  syscall\n");
    } 
  }
  if(strcmp(node->value, ")") == 0){

  }

  if(strcmp(node->value, "{") == 0){
    current_stack_size = stack_size;
  }

  if(strcmp(node->value, "}") == 0){
    for(; stack_size > current_stack_size;){
      printf("CURRENT STACK SIZE\n");
      pop("rsi", file);
    }
  }

  if(strcmp(node->value, ";") == 0){
    if(syscall_number == 60){
      fprintf(file, "  mov rax, %d\n", syscall_number);
      fprintf(file, "  pop rdi\n");
      fprintf(file, "  syscall\n");
    }
  }
  if(is_left){

  } else {

  }
  for(size_t i = 0; node->value[i] != '\0'; i++){
    printf("%c", node->value[i]);
  }
  printf("\n");
  traverse_tree(node->left, 1, file, syscall_number);
  traverse_tree(node->right, 0, file, syscall_number);

}

int generate_code(Node *root){
  insert('-', "sub");
  insert('+', "add");
  insert('*', "mul");
  insert('/', "div");
  FILE *file = fopen("generated.asm", "w");
  assert(file != NULL && "FILE COULD NOT BE OPENED\n");

  assert(hashmap_create(initial_size, &hashmap) == 0 && "ERROR: Could not create hashmap\n");

  fprintf(file, "section .text\n");
  fprintf(file, "  global _start\n\n");
  fprintf(file, "_start:\n");
  //print_tree(root, 0, "root");

  traverse_tree(root, 0, file, 0);
  fclose(file);

  return 0;
}

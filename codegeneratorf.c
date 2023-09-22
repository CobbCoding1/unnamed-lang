#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <ctype.h>
#include <assert.h>

#include "lexerf.h"
#include "parserf.h"
#include "./hashmap/hashmapoperators.h"
#include "./hashmap/hashmap.h"

#define MAX_STACK_SIZE_SIZE 1024

char curly_stack[MAX_STACK_SIZE_SIZE];
size_t curly_stack_size = 0;
size_t stack_size = 0;
int current_stack_size_size = 0;
int label_number = 0;
//int if_label_number = 0;
size_t current_stack_size[MAX_STACK_SIZE_SIZE];
const unsigned initial_size = 100;
struct hashmap_s hashmap;

typedef enum{
  ADD,
  SUB,
  DIV,
  MUL,
  NOT_OPERATOR
} OperatorType;

void create_label(FILE *file){
  label_number--;
  fprintf(file, "label%d:\n", label_number);
}

void if_label(FILE *file){
  fprintf(file, "  jne label%d\n", label_number);
  label_number++;
}

void stack_push(size_t value){
  printf("current stack: %zu\n", current_stack_size[current_stack_size_size]);
  current_stack_size_size++;
  current_stack_size[current_stack_size_size] = value;
  printf("current stack: %zu\n", current_stack_size[current_stack_size_size]);
}

size_t stack_pop(){
  printf("POP\n");
  if(current_stack_size_size == 0){
    printf("ERROR: stack is already empty\n");
    exit(1);
  }
  size_t result = current_stack_size[current_stack_size_size];
  //current_stack_size_size--;
  return result;
}

void curly_stack_push(char value){
  curly_stack_size++;
  curly_stack[curly_stack_size] = value;
}

char curly_stack_pop(){
  if(curly_stack_size == 0){
    return '\0';
  } 
  char result = curly_stack[curly_stack_size];
  curly_stack_size--;
  return result;
}

char curly_stack_peek(){
  if(curly_stack_size == 0){
    return '\0';
  }
  return curly_stack[curly_stack_size];
}

static int log_and_free_out_of_scope(void* const context, struct hashmap_element_s* const e){
  (void)(context);
  printf("CURRENT STACK SIZE: %zu, e->data%zu\n", current_stack_size[current_stack_size_size], *(size_t*)e->data);
  if(*(size_t*)e->data > current_stack_size[current_stack_size_size]){
    printf("REMOVED A VAR\n\n\n\n\n");
    if(hashmap_remove(&hashmap, e->key, strlen(e->key)) != 0){
      printf("COULD NOT REMOVE ELEMENT\n");
    } 
  }
  return 0;
}

void push(char *reg, FILE *file){
  fprintf(file, "  push %s\n", reg);
  stack_size++;
}

void push_var(size_t stack_pos, FILE *file){
  fprintf(file, "  push QWORD [rsp + %zu]\n", (stack_size - stack_pos) * 8);
  stack_size++;
}

void pop(char *reg, FILE *file){
  stack_size--;
  fprintf(file, "  pop %s\n", reg);
  printf("STACK SIZE: %zu\n", stack_size);
}

void mov(char *reg1, char *reg2, FILE *file){
  fprintf(file, "  mov %s, %s\n", reg1, reg2);
}

OperatorType check_operator(Node *node){
  if(node->type != OPERATOR){
    return NOT_OPERATOR;
  }

  if(strcmp(node->value, "+") == 0){
    return ADD;
  }
  if(strcmp(node->value, "-") == 0){
      return SUB;
  }
  if(strcmp(node->value, "/") == 0){
    return DIV;
  }
  if(strcmp(node->value, "*") == 0){
      return MUL;
  }
  return NOT_OPERATOR;
}

int mov_if_var_or_not(char *reg, Node *node, FILE *file){
  if(node->type == IDENTIFIER){
    int *value = malloc(sizeof(int));
    value = hashmap_get(&hashmap, node->value, strlen(node->value));
    if(value == NULL){
      printf("ERROR: Variable %s not declared in current scope\n", node->value);
      exit(1);
    }
    push_var(*value, file);
    pop(reg, file);
    return 0;
  }
  if(node->type == INT){
    fprintf(file, "  mov %s, %s\n", reg, node->value);
    return 0;
  }
  return -1;
}

Node *generate_operator_code(Node *node, FILE *file){
  mov_if_var_or_not("rax", node->left, file);
  push("rax", file);
  Node *tmp = node;
  OperatorType oper_type = check_operator(tmp);
  while(tmp->type == OPERATOR){
    pop("rax", file);
    oper_type = check_operator(tmp);
    tmp = tmp->right;
    if(tmp->type != OPERATOR){
      break;
    }
    mov_if_var_or_not("rbx", tmp->left, file);
    switch(oper_type){
      case ADD:
        fprintf(file, "  add rax, rbx\n");
        break;
      case SUB:
        fprintf(file, "  sub rax, rbx\n");
        break;
      case DIV:
        fprintf(file, "  div rbx\n");
        break;
      case MUL:
        fprintf(file, "  mul rbx\n");
        break;
      case NOT_OPERATOR:
        printf("ERROR: Invalid Syntax\n");
        exit(1);
    }
    push("rax", file);
    oper_type = check_operator(tmp);
  }
  mov_if_var_or_not("rbx", tmp, file);
  switch(oper_type){
    case ADD:
      fprintf(file, "  add rax, rbx\n");
      break;
    case SUB:
      fprintf(file, "  sub rax, rbx\n");
      break;
    case DIV:
      fprintf(file, "  div rbx\n");
      break;
    case MUL:
      fprintf(file, "  mul rbx\n");
      break;
    case NOT_OPERATOR:
      printf("ERROR: Invalid Syntaxx\n");
      exit(1);
  }
  push("rax", file);
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
    Node *value = malloc(sizeof(Node));
    value = node->left->left->left;
    if(value->type == IDENTIFIER){
      size_t *var_value = malloc(sizeof(size_t));
      var_value = hashmap_get(&hashmap, value->value, strlen(value->value));
      printf("VAR VALUE: %zu, value: %s\n", *var_value, value->value);
      if(var_value == NULL){
        printf("ERROR: %s Not Declared In Current Context\n", value->value);
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
    if(hashmap_get(&hashmap, node->left->value, strlen(node->left->value)) != NULL){
      printf("ERROR: Variable %s is already declared in current scope\n", node->left->value);
      exit(1);
    }
    if(hashmap_put(&hashmap, node->left->value, strlen(node->left->value), cur_size) != 0){
      printf("ERROR: Could not insert into hash table!\n");
      exit(1);
    }
    node->left = NULL;

  }

  if(strcmp(node->value, "IF") == 0){
    curly_stack_push('I');
    Node *current = malloc(sizeof(Node));
    current = node->left->left;
  printf("MADE IT HERE IF\n");
    if(current->left->type == INT || current->left->type == IDENTIFIER){
      mov_if_var_or_not("rax", current->left, file);
      push("rax", file);
    } else {
      generate_operator_code(current->left, file);
    }
  printf("MADE IT HERE IF\n");
    if(current->right->type == INT || current->right->type == IDENTIFIER){
      mov_if_var_or_not("rbx", current->right, file);
      push("rbx", file);
    } else {
      generate_operator_code(current->right, file);
    }
    pop("rax", file);
    pop("rbx", file);
    fprintf(file, "  cmp rax, rbx\n");
    if_label(file);
    //fprintf(file, "  jne label%d\n", if_label_number);
    //label_number++;
    node->left->left = NULL;
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
       printf("NODE VALUE: %s, %zu, %d\n", node->value, current_stack_size[current_stack_size_size], current_stack_size_size);
       if(var_value == NULL){
         printf("ERROR: Not Declared in current scope: %s\n", node->value);
         exit(1);
       } else {
       }
       push_var(*var_value, file);
       pop("rdi", file);
       fprintf(file, "  syscall\n");
       syscall_number = 0;
    } else {
      printf("VARIABLE IS HEREHERE\n");
      if(hashmap_get(&hashmap, node->value, strlen(node->value)) == NULL){
        printf("ERROR: Variable %s is not declared in current scope\n", node->value);
        exit(1);
      }

      Node *value = node->left->left;
        printf("VAR VALUE: %s\n", value->value);
      if(value->type == IDENTIFIER){
        size_t *var_value = malloc(sizeof(size_t));
        var_value = hashmap_get(&hashmap, value->value, strlen(value->value));
        printf("IDENTIFIER VALUE :%zu\n", *var_value);
        if(var_value == NULL){
          printf("ERROR: %s Not Declared In Current Context\n", value->value);
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
      if(hashmap_put(&hashmap, node->value, strlen(node->value), cur_size) != 0){
        printf("ERROR: Could not insert into hash table!\n");
        exit(1);
      }
      node->left = NULL;
    } 
  }
  if(strcmp(node->value, ")") == 0){

  }

  if(strcmp(node->value, "{") == 0){
    stack_push(stack_size);
    curly_stack_push('{');
  }

  if(strcmp(node->value, "}") == 0){
    char current_curly = curly_stack_pop();
    char next_curly = curly_stack_peek();
    printf("CURRENT CURLY: %c, %c\n", current_curly, next_curly);
    if(next_curly == 'I' || current_curly == 'I'){
      create_label(file);
    }

    size_t stack_value = stack_pop();
    printf("STACK VALUE: %zu, %zu\n", stack_value, stack_size);
    for(; stack_size > stack_value;){
      pop("rsi", file);
    }
    printf("STACK SIZE: %zu\n", stack_value);

    void* log = malloc(sizeof(char));
    if(hashmap_iterate_pairs(&hashmap, log_and_free_out_of_scope, (void*)log) != 0){
      printf("ERROR: Could not free\n");
      exit(1);
    }

  }

  if(strcmp(node->value, ";") == 0){
    if(syscall_number == 60){
      fprintf(file, "  mov rax, %d\n", syscall_number);
      fprintf(file, "  pop rdi\n");
      fprintf(file, "  syscall\n");
      syscall_number = 0;
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

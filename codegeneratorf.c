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

char *curly_stack[MAX_STACK_SIZE_SIZE];
size_t curly_stack_size = 0;
int curly_count = 0;
int global_curly = 0;
size_t stack_size = 0;
int current_stack_size_size = 0;
int label_number = 0;
int loop_label_number = 0;
int text_label = 0;
size_t current_stack_size[MAX_STACK_SIZE_SIZE];
const unsigned initial_size = 100;
struct hashmap_s hashmap;

typedef enum{
  ADD,
  SUB,
  DIV,
  MUL,
  MOD,
  NOT_OPERATOR
} OperatorType;


void create_label(FILE *file, int num){
  label_number--;
  fprintf(file, "label%d:\n", num);
}

void create_end_loop(FILE *file){
  loop_label_number--;
  fprintf(file, " jmp loop%d\n", loop_label_number);
}

void create_loop_label(FILE *file){
  fprintf(file, "loop%d:\n", loop_label_number);
  loop_label_number++;
}

void if_label(FILE *file, char *comp, int num){
  if(strcmp(comp, "EQ") == 0){
    fprintf(file, "  jne label%d\n", num);
  } else if(strcmp(comp, "NEQ") == 0){
    fprintf(file, "  je label%d\n", num);
  } else if(strcmp(comp, "LESS") == 0){
    fprintf(file, "  jge label%d\n", num);
  } else if(strcmp(comp, "GREATER") == 0){
    fprintf(file, "  jle label%d\n", label_number);
  } else {
    printf("ERROR: Unexpected comparator\n");
    exit(1);
  }
  label_number++;
}

void stack_push(size_t value){
  current_stack_size_size++;
  current_stack_size[current_stack_size_size] = value;
}

size_t stack_pop(){
  if(current_stack_size_size == 0){
    printf("ERROR: stack is already empty\n");
    exit(1);
  }
  size_t result = current_stack_size[current_stack_size_size];
  return result;
}

void curly_stack_push(char *value){
  curly_stack_size++;
  curly_stack[curly_stack_size] = value;
}

char *curly_stack_pop(){
  if(curly_stack_size == 0){
    return NULL;
  } 
  char *result = curly_stack[curly_stack_size];
  curly_stack_size--;
  return result;
}

char *curly_stack_peek(){
  if(curly_stack_size == 0){
    return NULL;
  }
  return curly_stack[curly_stack_size];
}

static int log_and_free_out_of_scope(void* const context, struct hashmap_element_s* const e){
  (void)(context);
  if(*(size_t*)e->data > (current_stack_size[current_stack_size_size] + 1)){
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

void push_var(size_t stack_pos, char *var_name, FILE *file){
  fprintf(file, "  push QWORD [rsp + %zu]\n", (stack_size - stack_pos) * 8);
  stack_size++;
}

void modify_var(size_t stack_pos, char *new_value, char *var_name, FILE *file){
  fprintf(file, "  mov QWORD [rsp + %zu], %s\n", ((stack_size) - (stack_pos)) * 8, new_value);
  fprintf(file, "  push QWORD [rsp + %zu]\n", (stack_size - stack_pos) * 8);
}

void pop(char *reg, FILE *file){
  stack_size--;
  fprintf(file, "  pop %s\n", reg);
  if(stack_size > 1000){
    exit(1);
  }
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
  if(strcmp(node->value, "%") == 0){
    return MOD;
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
    push_var(*value, node->value, file);
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
        fprintf(file, "  xor rdx, rdx\n");
        fprintf(file, "  div rbx\n");
        break;
      case MUL:
        fprintf(file, "  mul rbx\n");
        break;
      case MOD:
        fprintf(file, "  xor rdx, rdx\n");
        fprintf(file, "  div rbx\n");
        break;
      case NOT_OPERATOR:
        printf("ERROR: Invalid Syntax\n");
        exit(1);
        break;
    }
    if(oper_type != MOD){
      push("rax", file);
    } else {
      push("rdx", file);
    }
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
      fprintf(file, "  xor rdx, rdx\n");
      fprintf(file, "  div rbx\n");
      break;
    case MUL:
      fprintf(file, "  mul rbx\n");
      break;
    case MOD:
      fprintf(file, "  xor rdx, rdx\n");
      fprintf(file, "  div rbx\n");
      break;
    case NOT_OPERATOR:
      printf("ERROR: Invalid Syntax\n");
      exit(1);
      break;
  }
  if(oper_type != MOD){
    push("rax", file);
  } else {
    push("rdx", file);
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
    Node *value = malloc(sizeof(Node));
    value = node->left->left->left;
    if(value->type == IDENTIFIER){
      size_t *var_value = malloc(sizeof(size_t));
      var_value = hashmap_get(&hashmap, value->value, strlen(value->value));
      if(var_value != 0){
        printf("ERROR\n");
        exit(1);
      }
      if(var_value == NULL){
        printf("ERROR: %s Not Declared In Current Context\n", value->value);
        exit(1);
      }
      push_var(*var_value, value->value, file);
    } else if(value->type == INT) {
      push(value->value, file);
    } else if(value->type == OPERATOR){
      generate_operator_code(value, file);
    } else {
      printf("ERROR\n");
      exit(1);
    }
    size_t *var_location = malloc(sizeof(size_t));
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

  } else if(strcmp(node->value, "IF") == 0){
    curly_stack_push("IF");
    Node *current = malloc(sizeof(Node));
    current = node->left->left;
    if(current->left->type == INT || current->left->type == IDENTIFIER){
      mov_if_var_or_not("rax", current->left, file);
      push("rax", file);
    } else {
      generate_operator_code(current->left, file);
    }
    if(current->right->type == INT || current->right->type == IDENTIFIER){
      mov_if_var_or_not("rbx", current->right, file);
      push("rbx", file);
    } else {
      generate_operator_code(current->right, file);
    }
    pop("rax", file);
    pop("rbx", file);
    fprintf(file, "  cmp rax, rbx\n");
    if_label(file, current->value, curly_count);
    node->left->left = NULL;
  } else if(strcmp(node->value, "WHILE") == 0){
    curly_stack_push("W");
    create_loop_label(file);
    Node *current = malloc(sizeof(Node));
    current = node->left->left;
    if(current->left->type == INT || current->left->type == IDENTIFIER){
      mov_if_var_or_not("rax", current->left, file);
      push("rax", file);
    } else {
      generate_operator_code(current->left, file);
    }
    if(current->right->type == INT || current->right->type == IDENTIFIER){
      mov_if_var_or_not("rbx", current->right, file);
      push("rbx", file);
    } else {
      generate_operator_code(current->right, file);
    }
    pop("rbx", file);
    pop("rax", file);
    fprintf(file, "  cmp rax, rbx\n");
    if(strcmp(current->value, "EQ") == 0){
      if_label(file, "EQ", curly_count);
    } else if(strcmp(current->value, "NEQ") == 0){
      if_label(file, "NEQ", curly_count);
    } else if(strcmp(current->value, "LESS") == 0){
      if_label(file, "LESS", curly_count);
    } else if(strcmp(current->value, "GREATER") == 0){
      if_label(file, "GREATER", curly_count);
    } else {
      printf("ERROR: Unknown Operator\n");
      exit(1);
    }
    node->left->left = NULL;
  } else if(strcmp(node->value, "WRITE") == 0){
    char *text = malloc(sizeof(char) * 8);
    char *identifier = malloc(sizeof(char)*8);
    if(node->left->type == IDENTIFIER){
      identifier = hashmap_get(&hashmap, node->left->value, strlen(node->left->value));
      if(identifier == NULL){
        printf("ERROR: Value is not defined\n");
        exit(1);
      }
      push_var(*identifier, node->right->value, file);
      mov("rdi", "printf_format", file);
      pop("rsi", file);

      fprintf(file, "  xor rax, rax\n");

      fprintf(file, "  call printf WRT ..plt\n");
      
    } else {
      identifier = node->left->value;
      sprintf(text, "text%d", text_label);
      fprintf(file, "section .data\n");
      fprintf(file, " %s db \"%s\", 10\n", text, node->left->value);
      fprintf(file, "section .text\n");
      mov("rax", "1", file);
      mov("rdx", node->right->value, file);
      mov("rdi", "1", file);
      mov("rsi", text, file);
      text_label++;
      free(text);
      fprintf(file, "  syscall\n");
    }
    Node *tmp = malloc(sizeof(Node));
    tmp = node->right->right;
    node->right = NULL;
    node = tmp;
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
       size_t *var_value = malloc(sizeof(size_t));
       var_value = hashmap_get(&hashmap, node->value, strlen(node->value));
       if(var_value == NULL){
         printf("ERROR: Not Declared in current scope: %s\n", node->value);
         exit(1);
       } else {
       }
       push_var(*var_value, node->value, file);
       pop("rdi", file);
       fprintf(file, "  mov rax, %d\n", syscall_number);
       fprintf(file, "  syscall\n");
       syscall_number = 0;
    } else {
      if(hashmap_get(&hashmap, node->value, strlen(node->value)) == NULL){
        printf("ERROR: Variable %s is not declared in current scope\n", node->value);
        exit(1);
      }

      Node *value = node->left->left;
      size_t *var_location = malloc(sizeof(size_t));
      var_location = hashmap_get(&hashmap, node->value, strlen(node->value));
      if(value->type == IDENTIFIER){
        size_t *var_value = malloc(sizeof(size_t));
        var_value = hashmap_get(&hashmap, value->value, strlen(value->value));
        if(var_value == NULL){
          printf("ERROR: %s Not Declared In Current Context\n", value->value);
          exit(1);
        }
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

      pop("rax", file);
      modify_var(*var_location+1, "rax", node->value, file);
      node->left = NULL;
    } 
  }
  if(strcmp(node->value, ")") == 0){

  }

  if(strcmp(node->value, "{") == 0){
    stack_push(stack_size);
    curly_count++;
    char *curly_count_string = malloc(sizeof(char) * 4);
    sprintf(curly_count_string, "%d", curly_count);
    curly_stack_push(curly_count_string);
  }

  if(strcmp(node->value, "}") == 0){
    char *current_curly = curly_stack_pop();
    char *next_curly = curly_stack_pop();

    if(next_curly[0] == 'I'){
      create_label(file, atoi(current_curly)-1);
      global_curly = atoi(current_curly);
    } else if(next_curly[0] == 'W'){
      create_end_loop(file);
      create_label(file, atoi(current_curly)-1);
      global_curly = atoi(current_curly);
    }


    size_t stack_value = stack_pop();
    for(; stack_size != stack_value;){
      pop("rsi", file);
    }

    void* log = malloc(sizeof(char));
    if(hashmap_iterate_pairs(&hashmap, log_and_free_out_of_scope, (void*)log) != 0){
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
  traverse_tree(node->left, 1, file, syscall_number);
  traverse_tree(node->right, 0, file, syscall_number);

}

int generate_code(Node *root, char *filename){
  insert('-', "sub");
  insert('+', "add");
  insert('*', "mul");
  insert('/', "div");
  FILE *file = fopen(filename, "w");
  assert(file != NULL && "FILE COULD NOT BE OPENED\n");
  printf("HERHE\n");

  assert(hashmap_create(initial_size, &hashmap) == 0 && "ERROR: Could not create hashmap\n");

  fprintf(file, "section .data\n");
  fprintf(file, "  printf_format: db '%s', 10, 0\n", "%d");
  fprintf(file, "extern printf\n");
  fprintf(file, "global main\n");
  fprintf(file, "section .text\n");
  fprintf(file, "main:\n");

  traverse_tree(root, 0, file, 0);
  fclose(file);


  return 0;
}

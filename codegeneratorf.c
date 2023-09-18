#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <ctype.h>
#include <assert.h>

#include "lexerf.h"
#include "parserf.h"
#include "./hashmap/hashmap.h"
#include "./hashmap/hashmapvars.h"

size_t stack_size = 0;

void push(char *reg, FILE *file){
  fprintf(file, "  push %s\n", reg);
  stack_size++;
}

void pop(char *reg, FILE *file){
  fprintf(file, "  pop %s\n", reg);
  stack_size++;
}

void mov(char *reg1, char *reg2, FILE *file){
  fprintf(file, "  mov %s, %s\n", reg1, reg2);
}

Node *handle_adding(Node *tmp, FILE *file){
  pop("rax", file);
  tmp = tmp->right;
  mov("rbx", tmp->left->value, file);
  fprintf(file, "  %s rax, rbx\n", "add");
  push("rax", file);
  return tmp;
}

Node *handle_subbing(Node *tmp, FILE *file){
  pop("rax", file);
  tmp = tmp->right;
  mov("rbx", tmp->left->value, file);
  fprintf(file, "  %s rax, rbx\n", "sub");
  push("rax", file);
  return tmp;
}

Node *handle_mul(Node *tmp, FILE *file){
  pop("rax", file);
  tmp = tmp->right;
  mov("rbx", tmp->left->value, file);
  fprintf(file, "  mul rbx\n");
  push("rax", file);
  return tmp;
}

Node *handle_div(Node *tmp, FILE *file){
  pop("rax", file);
  tmp = tmp->right;
  mov("rbx", tmp->left->value, file);
  fprintf(file, "  div rbx\n");
  push("rax", file);
  return tmp;
}

Node *generate_operator_code(Node *node, int syscall_number, FILE *file){
  Node *tmp = node;
  mov("rax", tmp->left->value, file);
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
      mov("rbx", tmp->right->value, file);
      fprintf(file, "  %s rbx\n", search(tmp->value[0])->data);
      push("rax", file);
    } else {
      pop("rax", file);
      mov("rbx", tmp->right->value, file);
      fprintf(file, "  %s rax, rbx\n", search(tmp->value[0])->data);
      push("rax", file);
    }
  } else {
    if(tmp->value[0] == '-'){
      mov("rbx", tmp->right->value, file);
      fprintf(file, "  %s rax, rbx\n", "sub");
      push("rax", file);
    } else if(tmp->value[0] == '+'){
      mov("rbx", tmp->right->value, file);
      fprintf(file, "  add rax, rbx\n");
      push("rax", file);
    } else if(tmp->value[0] == '*'){
      mov("rbx", tmp->right->value, file);
      fprintf(file, "  mul rbx\n");
      push("rax", file);
    } else if(tmp->value[0] == '/'){
      mov("rbx", tmp->right->value, file);
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
    printf("ASHDJK\n");
    insert_var(node->left->value, stack_size);
    push(node->left->left->left->value, file);
    node->left = NULL;
    printf("EQUALS\n");

  }
  if(strcmp(node->value, "(") == 0){

  }
  if(node->type == OPERATOR){
    if(node->value[0] == '='){

    } else {
      generate_operator_code(node, syscall_number, file);
    }
  } 
  if(node->type == INT){
    fprintf(file, "  mov rax, %s\n", node->value);
    push("rax", file);
    //fprintf(file, "  mov rax, %d\n", syscall_number);
    //fprintf(file, "  mov rdi, %s\n", node->value);
    //fprintf(file, "  syscall\n");
  }
  if(node->type == IDENTIFIER){
    if(syscall_number == 60){
       fprintf(file, "  mov rax, %d\n", syscall_number);
       display_vars();
       printf("IDENFITIER TYPE: %s\n", node->value);
       fprintf(file, "  push QWORD [rsi + %zu]", search_var(node->value)->data * 8);
       //push("QWORD [rsi + " + search_var(node->value)->data * 8 + "]", file);
       pop("rdi", file);
       fprintf(file, "  syscall\n");
    } 
  }
  if(strcmp(node->value, ")") == 0){

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

  fprintf(file, "section .text\n");
  fprintf(file, "  global _start\n\n");
  fprintf(file, "_start:\n");
  //print_tree(root, 0, "root");

  traverse_tree(root, 0, file, 0);
  fclose(file);

  return 0;
}

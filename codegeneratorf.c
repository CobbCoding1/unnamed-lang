#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "lexerf.h"
#include "parserf.h"
#include "./hashmap/hashmap.h"

Node *generate_operator_code(Node *node, int syscall_number, FILE *file){
   Node *tmp = node;
    fprintf(file, "  mov rax, %s\n", node->left->value);
    int did_loop = 0;
    while(tmp->right->type == OPERATOR){
      did_loop = 1;
      char *oper = search(tmp->value[0])->data;
      tmp = tmp->right;
      fprintf(file, "  mov rbx, %s\n", tmp->left->value);
      if(strcmp(oper, "mul") == 0 || strcmp(oper, "div") == 0){
        fprintf(file, "  %s rbx\n", oper);
        fprintf(file, "  mov rdi, rax\n");
      } else {
        fprintf(file, "  %s rax, rbx\n", oper);
        fprintf(file, "  mov rdi, rax\n");
      }
    }
    if(did_loop){
      if(tmp->value[0] == '*' || tmp->value[0] == '/'){
        fprintf(file, "  mov rax, rdi\n");
        fprintf(file, "  mov rbx, %s\n", tmp->right->value);
        fprintf(file, "  %s rbx\n", search(tmp->value[0])->data);
        fprintf(file, "  mov rdi, rax\n");
      } else {
        fprintf(file, "  %s rdi, %s\n", search(tmp->value[0])->data, tmp->right->value);
      }
    } else {
      fprintf(file, "  mov rbx, %s\n", tmp->right->value);
      fprintf(file, "  %s rbx\n", search(tmp->value[0])->data);
      fprintf(file, "  mov rdi, rax\n");
    }

    fprintf(file, "  mov rax, %d\n", syscall_number);
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
    //fprintf(file, "  mov rax, 60\n");
  }
  if(strcmp(node->value, "(") == 0){

  }
  if(node->type == OPERATOR){
    generate_operator_code(node, syscall_number, file);
  } 
  if(node->type == INT){
    fprintf(file, "  mov rdi, %s\n", node->value);
  }
  if(strcmp(node->value, ")") == 0){

  }
  if(strcmp(node->value, ";") == 0){
    fprintf(file, "  syscall\n");
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

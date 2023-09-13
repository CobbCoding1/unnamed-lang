section .text
  global _start

_start:
  mov rax, 60
  mov rdi, 2 
  syscall

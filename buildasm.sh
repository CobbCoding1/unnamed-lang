nasm -f elf64 generated.asm -o generated.o
ld generated.o -o generated

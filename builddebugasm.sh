nasm -f elf64 -g -F dwarf -o test.o generated.asm
ld test.o -o test

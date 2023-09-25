nasm -f elf64 generated.asm -o generated.o
gcc generated.o -o generated -lc -no-pie

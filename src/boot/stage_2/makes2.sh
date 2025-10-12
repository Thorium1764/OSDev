#!/bin/bash

BUILD=/home/tobias/code/OSDev/build

for f in *.c; do
    [ -f "$f" ] || continue    # skip if no .c files
    obj="$BUILD/${f%.c}.o"
    gcc -m32 -ffreestanding -c "$f" -o "$obj"
done


for f in *.asm; do
    [ -f "$f" ] || continue    # skip if no .asm files
    obj="$BUILD/${f%.asm}.o"
    nasm -f elf "$f" -o "$obj"
done

gcc -m32 -ffreestanding -nostdlib -T linker.ld -o "$BUILD/stage2.elf" "$BUILD"/*.o

objcopy -O binary "$BUILD/stage2.elf" "$BUILD/stage2.bin"
# something

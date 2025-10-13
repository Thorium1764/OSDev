#!/bin/bash

set -e

BUILD=/home/tobias/code/OSDev/build

mkdir -p $BUILD

for f in *.c; do
    [ -f "$f" ] || continue    # skip if no .c files
    obj="$BUILD/${f%.c}.o"
    gcc -m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector -fno-builtin -nostdinc -c "$f" -o "$obj"
done


for f in *.asm; do
    [ -f "$f" ] || continue    # skip if no .asm files
    obj="$BUILD/${f%.asm}.o"
    nasm -f elf32 "$f" -o "$obj"
done

gcc -m32 -ffreestanding -static -fno-pie -nostdlib -T linker.ld -o "$BUILD/stage2.bin" "$BUILD"/*.o
# something

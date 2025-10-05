#!/bin/bash

/opt/watcom/binl/wcc -2 -s -zu -zls -ms -fo=build/kernel.obj src/kernel_c/kernel.c && \
/opt/watcom/binl/wcc -2 -s -zu -zls -ms -fo=build/stdio.obj src/kernel_c/stdio.c && \
nasm -f obj -o build/main.obj src/kernel_c/main.asm && \
nasm -f obj -o build/printc.obj src/kernel_c/printc.asm && \
/opt/watcom/binl/wlink NAME build/kernel.bin FILE build/kernel.obj FILE build/stdio.obj FILE build/main.obj FILE build/printc.obj @link.lnk && \
nasm src/boot/bootloader.asm -f bin -o build/bootloader.bin && \
dd if=/dev/zero of=build/main_floppy.img bs=512 count=2880 && \
mkfs.fat -F 12 -n "NBOS" build/main_floppy.img && \
dd if=build/bootloader.bin of=build/main_floppy.img conv=notrunc && \
mcopy -i build/main_floppy.img build/kernel.bin "::kernel.bin"






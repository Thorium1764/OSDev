#!/bin/bash

set -e

dd if=/dev/zero of=build/main_floppy.img bs=512 count=2880 && \
mkfs.fat -F 32 -n "NBOS" build/main_floppy.img && \
dd if=build/bootloader.bin of=build/main_floppy.img conv=notrunc && \
mcopy -i build/main_floppy.img build/stage2.bin "::stage2.bin"






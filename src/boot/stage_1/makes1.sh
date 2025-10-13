#!/bin/bash

set -e

BUILD = /home/tobias/code/OSDev/build
nasm -f bin bootloader.asm -o $BUILD/bootloader.bin

#pragma once
#include "stdint.h"

#define ASM __attribute__((cdecl))

uint8_t ASM x86_DiskRead(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, void* Data);


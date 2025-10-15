#pragma once
#include "stdint.h"

void __attribute__((cdecl)) i686_out(uint16_t port, uint8_t value);
uint8_t __attribute__((cdecl)) i686_in(uint16_t port);
uint8_t __attribute__((cdecl)) i686_EnableInterrupts();
uint8_t __attribute__((cdecl)) i686_DisableInterrupts();

void i686_iowait();
void __attribute__((cdecl)) i686_Panic();

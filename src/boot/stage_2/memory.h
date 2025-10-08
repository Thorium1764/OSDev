#pragma once
#include "stdint.h"

void* memcpy(void* dest, const void* src, uint16_t num);
void* memset(void* addr, int value, uint16_t num);
int8_t memcmp(const void* ptr1, const void* ptr2, uint16_t num);

void* segoffset_to_linear(void* addr);

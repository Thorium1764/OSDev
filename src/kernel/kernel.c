#include "stdio.h"

void __attribute__((cdecl, noreturn)) start(void)
{
   puts("Im in\r\n");
   for (;;);
}

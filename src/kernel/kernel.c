#include "stdio.h"
#include "memory.h"
#include "bootparams.h"

extern uint8_t __bss_start;
extern uint8_t __end;

void __attribute__((cdecl, noreturn, section(".entry"))) start(BootParams* bootparams)
{  
   memset(&__bss_start, 0, (&__end) - (&__bss_start));

   puts("Im in\r\n");
   for (;;);
}

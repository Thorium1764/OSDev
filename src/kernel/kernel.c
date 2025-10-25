#include "stdio.h"
#include "memory.h"
#include "bootparams.h"
#include "disk.h"
#include "fat.h"
#include "stdint.h"

#define DEBUG_LOAD ((void*)(0x5000))
#define DEBUG_ADDR ((void*)(0x45000))
#define DEBUG_SIZE 0x5000

extern uint8_t __bss_start;
extern uint8_t __end;

void __attribute__((cdecl, noreturn, section(".entry"))) start(BootParams* bootparams, Disk* disk)
{  
   memset(&__bss_start, 0, (&__end) - (&__bss_start));

   puts("Hello World from the Kernel!\r\n");

   FAT_FILE* file = FAT_OPEN(disk, "/debug.txt");
   uint32_t read;
   uint8_t* addr = DEBUG_ADDR;

   putn(589394839);

   while((read = FAT_READ(disk, file, DEBUG_SIZE, DEBUG_LOAD))){
      memcpy(addr, DEBUG_LOAD, read);
      addr += read;
   }
   putn(2);

   FAT_CLOSE(file);
   puts((char*)addr);

   putn(3);

   for (;;);
}

#include "stdint.h"
#include "memdefs.h"
#include "bootparams.h"
#include "stdio.h"
#include "string.h"
#include "disk.h"
#include "bin.h"
#include "x86.h"
#include "fat.h"
#include "memory.h"

uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_KERNEL_LOAD;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

BootParams bootParams;

typedef void (*KernelStart)(BootParams* bootpara); //the pointer is now interpreted as a function and the cpu will jmp to the address

void __attribute__((cdecl)) start(uint16_t bootDrive)
{
   clrscr();

   Disk disk;

   if(!DiskInit(&disk, bootDrive))
   {
      puts("Disk initialization error\r\n");
      goto error_loop;
   }

   if (!FAT_INIT(&disk)){
      puts("FAT: initialization error!\r\n");
      goto error_loop;
   }

   bootParams.BootDevice = bootDrive;

   puts("DEBUG: 5\r\n");
   
   KernelStart entryPoint;

   if (!BIN_Read(&disk, "/kernel.bin", (void**)&entryPoint))
   {
      puts("Kernel read failed, booting halted\r\n");
      goto error_loop;
   }

   puts("DEBUG: 6\r\n");

   entryPoint(&bootParams);

error_loop:
   puts("FATAL ERROR");
   for(;;);
}

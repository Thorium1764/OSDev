#include "stdint.h"
#include "memdefs.h"
#include "bootparams.h"
#include "stdio.h"
#include "string.h"
#include "disk.h"
#include "memdetect.h"
#include "mbr.h"
#include "x86.h"
#include "fat.h"
#include "memory.h"

uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

BootParams bootParams;

typedef void (*KernelStart)(BootParams* bootpara);

void __attribute__((cdecl)) start(uint16_t bootDrive, void* partition)
{
   clrscr();

   Disk disk;

   if(!DiskInit(&disk, bootDrive))
   {
      puts("Disk initialization error\r\n");
      goto error_loop;
   }



error_loop:
   for(;;);
}

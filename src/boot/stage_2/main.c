#include "stdint.h"
#include "memdefs.h"
#include "bootparams.h"
#include "stdio.h"
#include "string.h"
#include "disk.h"
#include "bin.h"
#include "memdetect.h"
#include "mbr.h"
#include "x86.h"
#include "fat.h"
#include "memory.h"

uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

BootParams bootParams;

typedef void (*KernelStart)(BootParams* bootpara); //the pointer is now interpreted as a function and the cpu will jmp to the address

void __attribute__((cdecl)) start(uint16_t bootDrive, void* partition)
{
   clrscr();

   Disk disk;

   if(!Disk_Init(&disk, bootDrive))
   {
      puts("Disk initialization error\r\n");
      goto error_loop;
   }

   Partition part;
   detectPartition(&part, &disk, partition);

   // do fat init
   
   bootParams.BootDevice = bootDrive;
   MemDetect(&bootParams.Memory);
   
   KernelStart entryPoint;

   if (!BIN_Read(&part, "/root/kernel.bin", (void**)&entryPoint))
   {
      puts("Kernel read failed, booting halted\r\n");
      goto error_loop;
   }

   entryPoint(&bootParams);

error_loop:
   for(;;);
}

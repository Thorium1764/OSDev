#pragma once
#include "stdint.h"

#define ASM __attribute__((cdecl))

void ASM x86_out(uint16_t port, uint8_t value);
uint8_t ASM x86_in(uint16_t port);

uint8_t ASM x86_Disk_GetDriveParams(uint8_t drive, uint8_t* type, uint16_t* cylinders, uint16_t* sectors, uint16_t* heads);

uint8_t ASM x86_DiskReset(uint8_t drive);

uint8_t ASM x86_DiskRead(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, void* Data);

typedef struct
{
   uint64_t base;
   uint64_t length;
   uint32_t type;
   uint32_t ACPI;
} E820_MemBlock;

enum E820_Block_Type
{
   USABLE = 1,
   RESERVED = 2,
   RECLAIMABLE = 3,
   ACPI_NVS = 4,
   BAD_MEMORY = 5,
};

int ASM x86_GetNextE820BLock(E820_MemBlock* block, uint32_t* continuation_id);

#include "mbr.h"
#include "stdint.h"
#include "fat.h"
#include "stdio.h"
#include "disk.h"
#include "memdefs.h"

#define SECTOR_SIZE 512
#define ROOT_DIR_HANDLE -1
#define FAT_CACHE_SIZE 5
#define MAX_FILE_HANDLES 8

typedef struct
{
   // ebr
   uint8_t DriveNumber;
   uint8_t Reserved;
   uint8_t Signature;
   uint32_t VolumeID;
   uint8_t VolumeLabel[11];
   uint8_t SystemID[8];
} __attribute__((packed)) FAT_EBR;

typedef struct
{
   uint32_t SectorsPerFat;
   uint16_t Flags;
   uint16_t FatVersion;
   uint32_t RootDirCluster;
   uint16_t InfoSector;
   uint16_t BackupBootSector;
   uint8_t  Reserved[12];
   FAT_EBR EBR;
} __attribute__((packed)) FAT32_EBR;

typedef struct 
{
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    union {
        FAT_EBR EBR12_16;
        FAT32_EBR EBR32;
    };
} __attribute__((packed)) FAT_BootSector;

typedef struct
{
   uint8_t Buffer[SECTOR_SIZE];
   uint8_t Opened;
   FAT_FILE Public;
   uint32_t FirstCluster;
   uint32_t CurrCluster;
   uint32_t CurrSectInCluster;
} FAT_FileData;

typedef struct
{
   union
   {
      FAT_BootSector BootSector;
      uint8_t BootSectorBytes[SECTOR_SIZE];
   } Boot_Sector;
   FAT_FileData RootDir;
   FAT_FileData OpenedFiles[MAX_FILE_HANDLES];
   uint8_t FAT_Cache[FAT_CACHE_SIZE * SECTOR_SIZE];
   uint32_t FAT_Cache_Pos;
} FAT_DATA;

static FAT_DATA* Data;
static uint32_t DataSectionLba;
static uint8_t FatType;
static uint32_t TotalSectors;
static uint32_t SectorsPerFat;

uint8_t FAT_ReadBS(Partition* disk)
{
   return Part_ReadSectors(disk,0, 1, Data->Boot_Sector.BootSectorBytes);
}

uint8_t FAT_ReadFat(Partition* disk, uint64_t lba)
{
   return Part_ReadSectors(disk, Data->Boot_Sector.BootSector.ReservedSectors + lba, FAT_CACHE_SIZE, Data->FAT_Cache);
}

void FAT_Detect(Partition* disk)
{
   uint32_t dataCluster = (TotalSectors - DataSectionLba) / Data->Boot_Sector.BootSector.SectorsPerCluster;
   if (dataCluster < 0xFF5)
      FatType = 12;
   else if (Data->Boot_Sector.BootSector.SectorsPerFat != 0)
      FatType = 16;
   else FatType = 32;
}

uint8_t FAT_INIT(Partition* disk)
{
   Data = (FAT_DATA*)MEMORY_FAT_ADDR;

   if (!FAT_ReadBS(disk))
   {
      puts("FAT: failed to read boot sector\r\n");
      return 0;
   }

   Data->FAT_Cache_Pos = 0xFFFFFFFF;
   TotalSectors = Data->Boot_Sector.BootSector.TotalSectors;
   if (TotalSectors == 0) // fat32 
      TotalSectors = Data->Boot_Sector.BootSector.LargeSectorCount;

   uint8_t isFat32 = 0;
   SectorsPerFat = Data->Boot_Sector.BootSector.SectorsPerFat;
   if (SectorsPerFat == 0){ //FAT32
      isFat32 = 1;
      SectorsPerFat = Data->Boot_Sector.BootSector.EBR32.SectorsPerFat;
   }



}







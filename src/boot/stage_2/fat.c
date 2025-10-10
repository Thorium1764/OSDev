#include "mbr.h"
#include "stdint.h"
#include "fat.h"
#include "stdio.h"
#include "disk.h"
#include "memdefs.h"
#include <math.h>


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


uint32_t FAT_cluster_to_lba(uint32_t cluster)
{
   return DataSectionLba + (cluster - 2) * Data->Boot_Sector.BootSector.SectorsPerCluster;
}

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

   uint32_t root_dir_lba;
   uint32_t root_dir_size;
   if (isFat32) {
      DataSectionLba = Data->Boot_Sector.BootSector.ReservedSectors + SectorsPerFat * Data->Boot_Sector.BootSector.FatCount;
      root_dir_lba = FAT_cluster_to_lba(Data->Boot_Sector.BootSector.EBR32.RootDirCluster);
      root_dir_size = 0;
   } else {
      root_dir_lba = Data->Boot_Sector.BootSector.ReservedSectors + SectorsPerFat * Data->Boot_Sector.BootSector.FatCount;
      root_dir_size = sizeof(FAT_DirEntry) * Data->Boot_Sector.BootSector.DirEntryCount;
      uint32_t root_dir_sectors = (root_dir_size + Data->Boot_Sector.BootSector.BytesPerSector - 1) / Data->Boot_Sector.BootSector.BytesPerSector;
      DataSectionLba = root_dir_lba + root_dir_sectors;
   }

   Data->RootDir.Public.Handle = ROOT_DIR_HANDLE;
   Data->RootDir.Public.isDir = 1;
   Data->RootDir.Public.Pos = 0;
   Data->RootDir.Public.Size = sizeof(FAT_DirEntry) * Data->Boot_Sector.BootSector.DirEntryCount;
   Data->RootDir.Opened = 0;
   Data->RootDir.FirstCluster = root_dir_lba;
   Data->RootDir.CurrCluster = root_dir_lba;
   Data->RootDir.CurrSectInCluster = 0;

   if (!Part_ReadSectors(disk,  root_dir_lba, 1, Data->RootDir.Buffer)){
      puts("FAT: root directory read failed\r\n");
      return 0;
   }
   
   FAT_Detect(disk);

   for (int i = 0; i < MAX_FILE_HANDLES; i++)
      Data->OpenedFiles[i].Opened = 0;

   return 1;
}

void FAT_CLOSE(FAT_FILE *file){
   if (file->Handle == ROOT_DIR_HANDLE){
      file->Pos = 0;
      Data->RootDir.CurrCluster = Data->RootDir.FirstCluster;
   } else {
      Data->OpenedFiles[file->Handle].Opened = 0;
   }
}

FAT_FILE* FAT_OpenEntry(Partition* disk, FAT_DirEntry* entry)
{
   int16_t handle = -1;
   for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++){
        if (Data->OpenedFiles[i].Opened)
            handle = i;
   }

   if (handle < 0){
      puts("FAT: out of file handles\r\n");
      return 0;
   }

   FAT_FileData* fd = &Data->OpenedFiles[handle];
   fd->Public.Handle = handle;
   fd->Public.isDir = (entry->Attributes & FAT_DIR) != 0;
   fd->Public.Pos = 0;
   fd->Public.Size = entry->Size;
   fd->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
   fd->CurrCluster = fd->FirstCluster;
   fd->CurrSectInCluster = 0;

   if (!Part_ReadSectors(disk, FAT_cluster_to_lba(fd->CurrCluster), 1, fd->Buffer)) {
      puts("FAT: entry open failed - read error: cluster = ");
      putn(fd->CurrCluster);
      puts(" lba = ");
      putn(FAT_cluster_to_lba(fd->CurrCluster));
      puts("\r\n");
      for (int i = 0; i < 11; i++)
         putn(entry->Name[i]);
      puts("\r\n");
      return 0;
   }

   fd->Opened = 1;
   return &fd->Public;
}

uint32_t FAT_NextCluster(Partition* disk, uint32_t currentCluster)
{
   uint32_t index;
   if (FatType == 12)
      index = currentCluster * 3 / 2; // 1.5 bytes
   else if (FatType == 16)
      index = currentCluster * 2; // 2 bytes
   else if (FatType == 32)
      index = currentCluster * 4; // 4 bytes

   uint32_t fatIndex = index / SECTOR_SIZE;
   if (fatIndex < Data->FAT_Cache_Pos || fatIndex >= Data->FAT_Cache_Pos + FAT_CACHE_SIZE){
      FAT_ReadFat(disk, fatIndex);
      Data->FAT_Cache_Pos = fatIndex;
   }

   index -= (Data->FAT_Cache_Pos * SECTOR_SIZE);
}








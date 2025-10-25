#include "stdint.h"
#include "fat.h"
#include "stdio.h"
#include "disk.h"
#include "memory.h"
#include "minmax.h"
#include "string.h"


#define SECTOR_SIZE 512
#define ROOT_DIR_HANDLE -1
#define MAX_FILE_HANDLES 10
#define MAX_PATH_SIZE 256


/* fat32
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
} __attribute__((packed)) FAT32_EBR;*/

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

   // ebr
   uint8_t DriveNumber;
   uint8_t Reserved;
   uint8_t Signature;
   uint32_t VolumeID;
   uint8_t VolumeLabel[11];
   uint8_t SystemID[8];
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
} FAT_DATA;

static FAT_DATA* Data;
static uint32_t DataSectionLba;
static uint8_t* Fat = NULL;


uint8_t FAT_ReadBS(Disk* disk)
{
   return DiskRead(disk,0, 1, Data->Boot_Sector.BootSectorBytes);
}

uint8_t FAT_ReadFat(Disk* disk)
{
   return DiskRead(disk, Data->Boot_Sector.BootSector.ReservedSectors, Data->Boot_Sector.BootSector.SectorsPerFat, Fat);
}

uint32_t FAT_cluster_to_lba(uint32_t cluster)
{
   return DataSectionLba + (cluster - 2) * Data->Boot_Sector.BootSector.SectorsPerCluster;
}

void FAT_CLOSE(FAT_FILE *file){
   if (file->Handle == ROOT_DIR_HANDLE){
      file->Pos = 0;
      Data->RootDir.CurrCluster = Data->RootDir.FirstCluster;
   } else {
      Data->OpenedFiles[file->Handle].Opened = 0;
   }
}

FAT_FILE* FAT_OpenEntry(Disk* disk, FAT_DirEntry* entry)
{
   int16_t handle = -1;
   for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++){
        if (!Data->OpenedFiles[i].Opened)
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
   fd->FirstCluster = entry->FirstClusterHigh & 0xFFF; //so it was that easy
   fd->CurrCluster = fd->FirstCluster;
   fd->CurrSectInCluster = 0;

   if (!DiskRead(disk, FAT_cluster_to_lba(fd->CurrCluster), 1, fd->Buffer)) {
      puts("FAT: opening entry failed - read error: cluster = ");
      putn(fd->CurrCluster);
      puts("; lba = ");
      putn(FAT_cluster_to_lba(fd->CurrCluster));
      puts("; entry = ");
      puts("\r\nwhile opening: \"");
      for (int i = 0; i < 12; i++)
         putc(entry->Name[i]);
      puts("\"\r\n");
      return 0;
   }

   fd->Opened = 1;
   return &fd->Public;
}

uint32_t FAT_NextCluster(uint32_t currentCluster)
{
   uint32_t index = currentCluster * 3 / 2;
   if (currentCluster % 2 == 0)
      return (*(uint16_t*)(Fat + index)) & 0x0FFF;
   else 
      return (*(uint16_t*)(Fat + index)) >> 4;
}

uint32_t FAT_READ(Disk *disk, FAT_FILE *file, uint32_t size, void *data)
{
   FAT_FileData* fd = (file->Handle == ROOT_DIR_HANDLE)
      ? &Data->RootDir
      : &Data->OpenedFiles[file->Handle];

   uint8_t* byte_data = (uint8_t*)data;
   if (!fd->Public.isDir || (fd->Public.isDir && fd->Public.Size != 0))
       size = min(size, fd->Public.Size - fd->Public.Pos);

   while (size > 0){
      uint32_t remaining = SECTOR_SIZE - (fd->Public.Pos % SECTOR_SIZE);
      uint32_t to_take = min(remaining, size);

      memcpy(byte_data, fd->Buffer + fd->Public.Pos % SECTOR_SIZE, to_take);
      byte_data += to_take;
      fd->Public.Pos += to_take;
      size -= to_take;

      if (remaining == to_take){
         if (fd->Public.Handle == ROOT_DIR_HANDLE){
            ++fd->CurrCluster;
            if (!DiskRead(disk, fd->CurrCluster, 1, fd->Buffer)){
               puts("FAT: error reading!\r\n");
               break;
            }
         } else {
            if (++fd->CurrSectInCluster >= Data->Boot_Sector.BootSector.SectorsPerCluster){
               fd->CurrSectInCluster = 0;
               fd->CurrCluster = FAT_NextCluster(fd->CurrCluster);
            }

            if (fd->CurrCluster >= 0xFF8){
               fd->Public.Size = fd->Public.Pos;
               break;
            }

            if (!DiskRead(disk, FAT_cluster_to_lba(fd->CurrCluster) + fd->CurrSectInCluster, 1, fd->Buffer)){
               puts("FAT: error reading!\r\n");
               break;
            }
         }
      }
   }
   return byte_data - (uint8_t*)data;
}

uint8_t FAT_READ_ENTRY(Disk *disk, FAT_FILE *file, FAT_DirEntry *entry)
{
   return FAT_READ(disk, file, sizeof(FAT_DirEntry), entry) == sizeof(FAT_DirEntry);
}

void FAT_GetShortName(const char* name, char shortName[12])
{
   memset(shortName, ' ', 12);
   *(shortName + 11) = '\0';
   const char* ext = strchr(name, '.');
   if (ext == NULL)
      ext = name + 11;

   for (int i = 0; i < 8 && *(name + i) && name + i < ext; i++)
      *(shortName + i) = toupper(*(name + i));

   if (ext != name + 11){
      for (int i = 0; i < 3 && *(ext + i + 1); i++)
         *(shortName + i + 8) = toupper(*(ext + i + 1));
   }
}
      
uint8_t FAT_FIND_FILE(Disk* disk, FAT_FILE* file, const char* name, FAT_DirEntry* entry_out)
{
   char shortName[12];

   FAT_DirEntry entry;
   FAT_GetShortName(name, shortName);

   while (FAT_READ_ENTRY(disk, file, &entry)){
      if (memcmp(shortName, entry.Name, 11) == 0){
         *entry_out = entry;
         return 1;
      }
   }
   return 0;
}

FAT_FILE* FAT_OPEN(Disk* disk, const char* path)
{
   char name[MAX_PATH_SIZE];

   if (*path == '/')
      path++;

   FAT_FILE* current = &Data->RootDir.Public;
   while (*path){
      uint8_t isLast = 0;
      const char* delim = strchr(path, '/');
      if (delim != NULL) {
         memcpy(name, path, delim - path);
         name[delim - path + 1] = '\0';
         path = delim + 1;
      } else {
         unsigned len = strlen(path);
         memcpy(name, path, len);
         name[len + 1] = '\0';
         path += len;
         isLast = 1;
      }
      FAT_DirEntry entry;
      if (FAT_FIND_FILE(disk, current, name, &entry)){
         FAT_CLOSE(current);

         if (!isLast && entry.Attributes & FAT_DIR == 0) {
            puts("FAT: ");
            puts(name);
            puts(" is not a directory!\r\n");
            return NULL;
         }
         current = FAT_OpenEntry(disk, &entry);
      } else {
         FAT_CLOSE(current);
         puts("FAT: ");
         puts(name);
         puts(" not found\r\n");

         return NULL;
      }
   }
   return current;
}

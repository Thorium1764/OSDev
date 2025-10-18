#include "stdint.h"
#include "mbr.h"

uint8_t BIN_Read(Disk* disk, const char* path, void** entryPoint);

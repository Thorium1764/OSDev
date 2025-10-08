#include "stdint.h"
#include "mbr.h"

uint8_t BIN_Read(Partition* part, const char* path, void* loadAddress, void** entryPoint);

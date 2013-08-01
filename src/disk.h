//-----------------------------------------------------------------------------
// Software that is described herein is for illustrative purposes only
// which provides customers with programming information regarding the
// products. This software is supplied "AS IS" without any warranties.
// NXP Semiconductors assumes no responsibility or liability for the
// use of the software, conveys no license or title under any patent,
// copyright, or mask work right to the product. NXP Semiconductors
// reserves the right to make changes in the software without
// notification. NXP Semiconductors also make no representation or
// warranty that such application will be suitable for the specified
// use without further testing or modification.
//-----------------------------------------------------------------------------

#ifndef __DISK_H__
#define __DISK_H__

#include "type.h"
#include <stdint.h>

#define MSC_MemorySize  ( BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE + USER_FLASH_SIZE )
#define MSC_BlockSize   512
#define MSC_BlockCount  (MSC_MemorySize / MSC_BlockSize)

#define MAX_ROOT_DIR_ENTRIES 16
#define ROOT_DIR_ENTRIES 4
#define BOOT_SECT_SIZE  MSC_BlockSize
#define ROOT_DIR_SIZE   MSC_BlockSize
#define FAT_SIZE        (MSC_BlockSize * 2)

#define BLOCKS_PER_CLUSTER  64
#define NO_OF_CLUSTERS     (MSC_BlockCount/BLOCKS_PER_CLUSTER)

#define DIRECTORY_ENTRY_SIZE 32

typedef struct FatBootSector {
    uint8_t       bootjmp[3];
    uint8_t       oem_name[8];
    uint16_t      bytes_per_sector;
    uint8_t       sectors_per_cluster;
    uint16_t      reserved_sector_count;
    uint8_t       table_count;
    uint16_t      root_entry_count;
    uint16_t      total_sectors_16;
    uint8_t       media_type;
    uint16_t      table_size_16;
    uint16_t      sectors_per_track;
    uint16_t      head_side_count;
    uint32_t        hidden_sector_count;
    uint32_t        total_sectors_32;

    //this will be cast to it's specific type once the driver actually knows what type of FAT this is.
    uint8_t       extended_section[54];

} __attribute__((packed)) FatBootSector_t;

typedef struct FatDirectoryEntry {
    uint8_t filename[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_ms;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t accessed_date;
    uint16_t first_cluster_high_16;
    uint16_t modification_time;
    uint16_t modification_date;
    uint16_t first_cluster_low_16;
    uint32_t filesize;
}  __attribute__((packed)) FatDirectoryEntry_t;

/* RAM to store the file allocation table */
extern uint8_t FAT[FAT_SIZE];

/* FAT12 Root directory entries */
extern FatDirectoryEntry_t DIRECTORY_ENTRIES[ROOT_DIR_ENTRIES];

extern const FatBootSector_t BOOT_SECTOR;

#endif  /* __DISK_H__ */

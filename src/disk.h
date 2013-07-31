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

#define MSC_MemorySize  ( BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE + USER_FLASH_SIZE )
#define MSC_BlockSize   512
#define MSC_BlockCount  (MSC_MemorySize / MSC_BlockSize)

#define BOOT_SECT_SIZE  MSC_BlockSize
#define ROOT_DIR_SIZE   (MSC_BlockSize * 1)
#define FAT_SIZE        (MSC_BlockSize * 2)

#define BLOCKS_PER_CLUSTER  64
#define NO_OF_CLUSTERS     (MSC_BlockCount/BLOCKS_PER_CLUSTER)

#define DIR_ENTRY 64

typedef struct FatBootSector {
    unsigned char       bootjmp[3];
    unsigned char       oem_name[8];
    unsigned short      bytes_per_sector;
    unsigned char       sectors_per_cluster;
    unsigned short      reserved_sector_count;
    unsigned char       table_count;
    unsigned short      root_entry_count;
    unsigned short      total_sectors_16;
    unsigned char       media_type;
    unsigned short      table_size_16;
    unsigned short      sectors_per_track;
    unsigned short      head_side_count;
    unsigned int        hidden_sector_count;
    unsigned int        total_sectors_32;

    //this will be cast to it's specific type once the driver actually knows what type of FAT this is.
    unsigned char       extended_section[54];

} __attribute__((packed)) FatBootSector_t;


extern unsigned char  Fat_RootDir[FAT_SIZE + ROOT_DIR_SIZE];
extern const unsigned char RootDirEntry[DIR_ENTRY];
extern const FatBootSector_t BOOT_SECTOR;

#endif  /* __DISK_H__ */

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

extern unsigned char  Fat_RootDir[FAT_SIZE + ROOT_DIR_SIZE];
extern const unsigned char RootDirEntry[DIR_ENTRY];
extern const unsigned char BootSect[];

#endif  /* __DISK_H__ */

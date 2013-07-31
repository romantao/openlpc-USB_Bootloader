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

#include "disk.h"


const FatBootSector_t BOOT_SECTOR = {
    {0xEB, 0x3C, 0x90},
    {0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30},
    MSC_BlockSize,
    BLOCKS_PER_CLUSTER,
    1,
    1,
    16,
    // this is the block count which can only be known at runtime if we want to
    // keep things flexible - it's maximized here but will be smaller when
    // handled in BlockDevRead
    0x03EC,
    0xF8,
    2,
    1,
    1
};

/* FAT12 Root directory entry constants */
const unsigned char RootDirEntry[DIR_ENTRY] = {
    // 8.3 file name
    'L', 'P', 'C', '1', '7', '5', '9',' ', ' ', ' ', ' ',
    // file attributes
    0x28,
    // reserved for Windows NT
    0x00,
    // creation time in tenths of a second
    0x00,
    // time of file creation
    0x00,0x00,
    // date of file creation
    0x00,0x00,
    // last accessed date
    0x00,0x00,
    // high 16 bits of the first cluster number (always 0 for FAT12)
    0x00,0x00,
    // last modification time
    0x00,0x00,
    // last modification date
    0x00,0x00,
    // low 16 bits of the cluster number
    0x00,0x00,
    // file size in bytes
    0x00,0x00,0x00,0x00,

    // 8.3 file name
    'F', 'I', 'R', 'M', 'W', 'A', 'R', 'E', 'B', 'I', 'N',
    // file attributes
    0x20,
    // reserved for Windows NT
    0x18,
    // creation time in tenths of a second
    0xbc,
    // time of file creation
    0x41,0x97,
    // date of file creation
    0x37,0x38,
    // last accessed date
    0x37,0x38,
    // high 16 bits of the first cluster number (always 0 for FAT12)
    0x00,0x00,
    // last modification time
    0x3d,0x6e,
    // last modification date
    0x2b,0x38,
    // low 16 bits of the cluster number
    0x02,0x00,
    // file size in bytes
    0x00,0xD0,0x07,0x00,
 };

/* RAM to store the file allocation table */
unsigned char Fat_RootDir[FAT_SIZE + ROOT_DIR_SIZE];

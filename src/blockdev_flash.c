//*****************************************************************************
//   +--+
//   | ++----+
//   +-++    |
//     |     |
//   +-+--+  |
//   | +--+--+
//   +----+    Copyright (c) 2009 Code Red Technologies Ltd.
//
// blockdev_flash.c - bottom layer for LPCUSB stack mass storage, providing
//                    access to LPC1768 flash memory for RDB1768 bootloader
//
// Software License Agreement
//
// The software is owned by Code Red Technologies and/or its suppliers, and is
// protected under applicable copyright laws.  All rights are reserved.  Any
// use in violation of the foregoing restrictions may subject the user to criminal
// sanctions under applicable laws, as well as to civil liability for the breach
// of the terms and conditions of this license.
//
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// USE OF THIS SOFTWARE FOR COMMERCIAL DEVELOPMENT AND/OR EDUCATION IS SUBJECT
// TO A CURRENT END USER LICENSE AGREEMENT (COMMERCIAL OR EDUCATIONAL) WITH
// CODE RED TECHNOLOGIES LTD.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "sbl_config.h"
#include "sbl_iap.h"
#include "type.h"
#include "disk.h"
#include "log.h"

#include <string.h>

#define BLOCKSIZE        512

extern bool user_flash_erased; // from main.c

static bool FIRST_BLOCK_WRITTEN = false;
static int FORCE_FIRST_CLUSTER_WORKAROUND_OFFSET = 0;

int BlockDevGetSize(uint32_t *pdwDriveSize) {
    *pdwDriveSize = (512 * 1024)- sector_start_map[USER_START_SECTOR];
    return 0;
}

int BlockDevWrite(uint32_t dwAddress, uint8_t * pbBuf) {
    uint32_t offset = 512 * dwAddress;
    // first four bytes are reserved
    if(offset < BOOT_SECT_SIZE) {
        debug("Disallowing write to the boot sector");
    } else if(offset < (BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE)) {
        debug("Modifying a root directory entry in RAM disk");
        for(uint32_t i = 0; i < BLOCKSIZE; i++) {
            Fat_RootDir[(offset + i) - BOOT_SECT_SIZE] = pbBuf[i];

            // erasing a file, mark first byte of entry with 0xe5
            if(pbBuf[i] == 0xe5 ) {
                if((offset+i) == BOOT_SECT_SIZE + FAT_SIZE + 32 ) {
                    // Delete user flash when firmware.bin is erased
                    if(user_flash_erased == false ) {
                        erase_user_flash();
                        user_flash_erased = true;
                    }
                }
            }
        }
    } else {
        // Some OSs (e.g. Linux and OS X) start writing the file from the 2nd
        // cluster instead of the first. Normally that's fine since they just
        // write the correct starting cluster number into the FAT entry -
        // however since our FAT entries are transient and aren't actually read
        // when the bootloader starts up (they're stored in RAM anyway), the
        // bootloader just assumes the host wrote the firmware starting from
        // cluster 2 - using 'cp' in Linux it'll start it at 3, however. This
        // workaround detects if the first write from the host is coming in at
        // the 3rd cluster and sets an offset for all of the remaining writes.
        // This makes the assumption that writes will come in sequentially,
        // which so far when using 'cp' or the file browser has been true. A
        // more proper way to fix this issue would be to actually store the FAT
        // directory entires in flash somewhere, read it when the bootloader
        // starts, and jump to the correct cluster number.
        if(!FIRST_BLOCK_WRITTEN && offset == (0x8000 +
                    BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE)) {
            FORCE_FIRST_CLUSTER_WORKAROUND_OFFSET = -0x8000;
            debug("Detecting a firmware write not starting from first "
                    "cluster - adjusting offset");
        }
        offset += FORCE_FIRST_CLUSTER_WORKAROUND_OFFSET;
        FIRST_BLOCK_WRITTEN = true;

        write_flash((unsigned *)((uint8_t*)USER_FLASH_START + (offset - (
                        BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE))),
                    (char *)pbBuf, BLOCKSIZE);
    }
    return 0;
}

int BlockDevRead(uint32_t dwAddress, uint8_t * pbBuf) {
    uint32_t offset = 512 * dwAddress;
    uint8_t data;
    for(unsigned int i = 0; i < BLOCKSIZE; i++) {
        if(offset < BOOT_SECT_SIZE) {

            switch (offset) {
                case 19:

                    data = (uint8_t)(MSC_BlockCount & 0xFF);
                    break;
                case 20:

                    data = (uint8_t)((MSC_BlockCount >> 8) & 0xFF);
                    break;
                case 510:
                    data = 0x55;
                    break;
                case 511:
                    data = 0xAA;
                    break;
                default:
                    if(offset > 29 ) {
                        data = 0x0;
                    } else {
                        data = BootSect[offset];
                    }
                    break;
            }

        } else if(offset < (BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE)) {
            data = Fat_RootDir[offset - BOOT_SECT_SIZE];
        } else {
            data = *((uint8_t*)USER_FLASH_START + (offset - (BOOT_SECT_SIZE +
                            FAT_SIZE + ROOT_DIR_SIZE)));
        }

        pbBuf[i] = data;
        offset++;
    }
    return 0;
}

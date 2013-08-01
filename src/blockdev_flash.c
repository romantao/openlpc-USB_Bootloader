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

static int FORCE_FIRST_CLUSTER_WORKAROUND_OFFSET = 0;
unsigned block_written_map[1024];

int BlockDevGetSize(uint32_t *pdwDriveSize) {
    *pdwDriveSize = (512 * 1024)- sector_start_map[USER_START_SECTOR];
    return 0;
}

int BlockDevWrite(uint32_t dwAddress, uint8_t * pbBuf) {
    uint32_t offset = 512 * dwAddress;
    // first four bytes are reserved
    if(offset < BOOT_SECT_SIZE) {
        debug("Disallowing write to the boot sector");
    } else if(offset < BOOT_SECT_SIZE + FAT_SIZE) {
        debug("Modifying the FAT itself");
        for(uint32_t i = 0; i < BLOCKSIZE; i++) {
            FAT[(offset + i) - BOOT_SECT_SIZE] = pbBuf[i];
        }
    } else if(offset < BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE) {
        debug("Modifying directory entry %d", ((BOOT_SECT_SIZE + FAT_SIZE +
                    ROOT_DIR_SIZE) - offset) % DIRECTORY_ENTRY_SIZE);

        for(uint32_t i = 0; i < BLOCKSIZE; i++) {
            ((uint8_t*)&DIRECTORY_ENTRIES)[(offset + i) - BOOT_SECT_SIZE -
                    FAT_SIZE] = pbBuf[i];
        }

        for(int i = 0; i < MAX_ROOT_DIR_ENTRIES; i++) {
            FatDirectoryEntry_t* directory_entry = &DIRECTORY_ENTRIES[i];

            // erasing a file, mark first byte of entry with 0xe5
            if(directory_entry->filename[0] == 0xe5 &&
                        !strncmp(&(directory_entry->filename[1]), "IRMWARE", 7) &&
                        !user_flash_erased) {
                // Delete user flash when firmware.bin is erased
                erase_user_flash();
                user_flash_erased = true;
            }

            if(directory_entry->first_cluster_low_16 == 3) {
                // TODO explain this
                debug("Fudging the first cluster for %s",
                        directory_entry->filename)
                directory_entry->first_cluster_low_16 = 2;
            }

            if(directory_entry->filename[0] == NULL || directory_entry->filename[0] == 0xe5) {
                continue;
            } else if(directory_entry->attributes == 0xf) {
                debug("Found a VFAT long file name entry...");
                for(int j = 0; j < DIRECTORY_ENTRY_SIZE; j++) {
                    debug_no_newline("%02x ", ((uint8_t*)directory_entry)[j]);
                }
                debug("");
                continue;
            }
            debug("filename: %s, attributes: 0x%x, first cluster: %d, size: %d",
                    directory_entry->filename,
                    directory_entry->attributes,
                    directory_entry->first_cluster_low_16,
                    directory_entry->filesize);
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
        if(offset == (0x8000 + BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE)) {
            bool bogus_file_up_front = false;
            for(int i = 0; i < 9; i++) {
                bogus_file_up_front = bogus_file_up_front || !block_written_map[i];
                if(bogus_file_up_front) {
                    break;
                }
            }
            if(!block_written_map[0] || bogus_file_up_front) {
                FORCE_FIRST_CLUSTER_WORKAROUND_OFFSET = -0x8000;
                debug("Detecting a firmware write not starting from first "
                        "cluster - adjusting offset");
                memset(block_written_map, 0, 1024);
            }
        }

        offset += FORCE_FIRST_CLUSTER_WORKAROUND_OFFSET;

        if(offset < BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE) {
            debug("Attempting to write at beginning of flash after fixing "
                    "offset, bailing!");
            return 0;
        }

        unsigned* flash_address = (unsigned *)((uint8_t*)USER_FLASH_START +
                (offset - (BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE)));
        unsigned int block_array_index = ((unsigned)flash_address -
                0x10000) / BLOCKSIZE;
        /* debug("flash address 0x%02x", flash_address); */
        /* debug("block array index: %d", block_array_index); */
        if(block_array_index > 1024) {
            /* debug("requested block is corrupted, bailing on write request"); */
        } else if(block_written_map[block_array_index]) {
            debug("Address 0x%02x already written, blocking second write",
                    (unsigned)flash_address);
        } else {
            write_flash(flash_address, (char *)pbBuf, BLOCKSIZE);
            block_written_map[block_array_index] = true;
        }
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
                        data = ((char*)&BOOT_SECTOR)[offset];
                    }
                    break;
            }

        } else if(offset < BOOT_SECT_SIZE + FAT_SIZE) {
            data = FAT[offset - BOOT_SECT_SIZE];
        } else if(offset < BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE) {
            uint32_t directory_offset = offset - BOOT_SECT_SIZE - FAT_SIZE;
            if(directory_offset > (ROOT_DIR_ENTRIES * DIRECTORY_ENTRY_SIZE)) {
                data = 0;
            } else {
                data = ((uint8_t*)&DIRECTORY_ENTRIES)[directory_offset];
            }
        } else {
            data = *((uint8_t*)USER_FLASH_START + (offset - (BOOT_SECT_SIZE +
                            FAT_SIZE + ROOT_DIR_SIZE)));
        }

        pbBuf[i] = data;
        offset++;
    }
    return 0;
}

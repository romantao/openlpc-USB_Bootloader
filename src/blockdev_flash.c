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


#include "sbl_config.h"
#include "sbl_iap.h"
#include "type.h"
#include "disk.h"

#include <string.h>

extern BOOL  user_flash_erased; // from main_bootloader.c

int BlockDevGetSize(U32 *pdwDriveSize)
{
    *pdwDriveSize = (512 * 1024)- sector_start_map[USER_START_SECTOR];

    return 0;

}

/*
int BlockDevWrite(U32 dwAddress, U8 * pbBuf)
{
    static U32 lba = 0;

    if (lba < 1) {
        // Can't write boot sector
    } else if (lba < 4) {
        // wants to write the FAT or root dir
        memcpy(&Fat_RootDir[MSC_BlockSize * (lba - 1)], pbBuf, MSC_BlockSize);

    } else if(lba < MSC_BlockCount){
        unsigned int addr = USER_FLASH_START + MSC_BlockSize * (lba - 4);
        write_flash((unsigned *)addr, pbBuf, MSC_BlockSize);
    } else {
        // out of bounds!
        return 0;
    }
    lba++;
    return 0;

}*/


int BlockDevWrite(U32 dwAddress, U8 * pbBuf)
{

    BYTE * firmware;
    firmware = (BYTE *)USER_FLASH_START;

    U32 offset;

    U32 length =512;

    U32 i;

    offset = 512* dwAddress;

    if (offset < BOOT_SECT_SIZE)
    {
      // Can't write boot sector
    }
    else if (offset < (BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE))
         {
           for ( i = 0; i<length; i++)
           {
             Fat_RootDir[(offset+i) - BOOT_SECT_SIZE] = pbBuf[i];

             if ( pbBuf[i] == 0xe5 )
             {
                 if ( (offset+i) == BOOT_SECT_SIZE + FAT_SIZE + 32 )
                 {
                    // Delete user flash when firmware.bin is erased
                    if( user_flash_erased == FALSE )
                    {
                      erase_user_flash();
                      user_flash_erased = TRUE;
                    }
                 }

             }
           }
         }
         else
         {
           write_flash((unsigned *)(firmware + (offset - (BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE))),(char *)pbBuf,length);
           // [pbBuf casted to signed char pointer as that is what write_flash routine expects]
         }
    return 0;

}



int BlockDevRead(U32 dwAddress, U8 * pbBuf)
{
    U32 offset;

      unsigned int i;
      BYTE data;
      BYTE * firmware;
      firmware = (BYTE *)USER_FLASH_START;

        U32 length =512;

    offset = 512 * dwAddress;


      for ( i = 0; i<length; i++)
      {
        if (offset < BOOT_SECT_SIZE)
        {

          switch (offset)
          {
            case 19:

              data = (BYTE)(MSC_BlockCount & 0xFF);
            break;
          case 20:

              data = (BYTE)((MSC_BlockCount >> 8) & 0xFF);
            break;
            case 510:
              data = 0x55;
            break;
            case 511:
              data = 0xAA;
            break;
            default:
            if ( offset > 29 )
            {
              data = 0x0;
            }
            else
            {
              data = BootSect[offset];
            }
            break;
          }

        }
        else if (offset < (BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE))
             {
               data = Fat_RootDir[offset - BOOT_SECT_SIZE];
             }
             else
             {
                 data = *(firmware + (offset - (BOOT_SECT_SIZE + FAT_SIZE + ROOT_DIR_SIZE)));
               }

        pbBuf[i] = data;
        offset++;
      }
    return 0;
}





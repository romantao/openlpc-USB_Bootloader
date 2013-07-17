#include <stdint.h>
#include <stdbool.h>
#include "disk.h"
#include "sbl_iap.h"
#include "sbl_config.h"

#include "LPC17xx.h"

bool  user_flash_erased;

// USB mass storage driver - in msc_usb_start.c
void usb_msc_start (void);

/*****************************************************************************
 * enter_usb_isp() is the routine called if the bootloader determines that the
 * USB interface needs starting to allow the user to upload a new application
 *  into the LPC1768's flash memory
 *****************************************************************************/
void enter_usb_isp(void) {
    uint32_t n, m , next_cluster;

    user_flash_erased = false;

    // Generate File Allocation Table to save Flash space
    // First Two FAT entries are reserved
    Fat_RootDir[0]= 0xF8;
    Fat_RootDir[1]= 0xFF;
    Fat_RootDir[2]= 0xFF;
    /* Start cluster of a file is indicated by the Directory entry = 2 */
    m = 3;
    for(n = 3;n < NO_OF_CLUSTERS+2;n+=2) {
        if(n == ((NO_OF_CLUSTERS+2)-1)) {
          next_cluster = 0xFFF;
        } else {
          next_cluster = n + 1;
        }
          Fat_RootDir[m] = (uint8_t)n & 0xFF;
          Fat_RootDir[m+1] = (((uint8_t)next_cluster & 0xF) << 4) | ((uint8_t)(n>>8)&0xF);
          Fat_RootDir[m+2] = (uint8_t)(next_cluster >> 4) & 0xFF;
        m = m+3;
    }

    /* Copy root directory entries */
    for(n = 0; n < DIR_ENTRY ; n++) {             /* Copy Initial Disk Image */
        Fat_RootDir[(FAT_SIZE+n)] = RootDirEntry[n];  /*   from Flash to RAM     */
    }

    /* Correct file size entry for file firmware.bin */
    Fat_RootDir[FAT_SIZE+60] = (uint8_t)(USER_FLASH_SIZE & 0xFF);
    Fat_RootDir[FAT_SIZE+61] = (uint8_t)(USER_FLASH_SIZE >> 8);
    Fat_RootDir[FAT_SIZE+62] = (uint8_t)(USER_FLASH_SIZE >> 16);
    Fat_RootDir[FAT_SIZE+63] = (uint8_t)(USER_FLASH_SIZE >> 24);


    // Start up LPCUSB mass storage system to allow user to copy
    // their application binary to the LPC1768's flash.
    usb_msc_start ();

    // Note - should never actually return from usb_msc_start ().
}

/*********************************
 * Main entry point for bootloader
 *********************************/

int main (void) {
    // Check to see if there is a user application in the LPC1768's flash memory.
    if(user_code_present()) {
        // There is an application, but need to check if user is pressing the button
        // to indicate they want to upload a new application.
        check_isp_entry_pin();
    }

    // User code not present or isp entry requested
    enter_usb_isp();

    // Note - should never actually return from enter_usb_isp ().
    while (1);        // loop forever
    return 0;
}

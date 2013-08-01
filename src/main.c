#include <stdint.h>
#include <stdbool.h>
#include "disk.h"
#include "sbl_iap.h"
#include "sbl_config.h"

#include "LPC17xx.h"

bool user_flash_erased;

// USB mass storage driver - in msc_usb_start.c
void usb_msc_start(void);

/*****************************************************************************
 * enter_usb_isp() is the routine called if the bootloader determines that the
 * USB interface needs starting to allow the user to upload a new application
 *  into the LPC1768's flash memory
 *****************************************************************************/
void enter_usb_isp(void) {
    user_flash_erased = false;

    // Generate File Allocation Table to save Flash space
    // First Two FAT entries are reserved
    FAT[0] = 0xF8;
    FAT[1] = 0xFF;
    FAT[2] = 0xFF;
    /* Start cluster of a file is indicated by the Directory entry = 2 */
    uint32_t m = 3;
    uint32_t next_cluster;
    for(uint32_t n = 3; n < NO_OF_CLUSTERS + 2; n += 2) {
        if(n == (NO_OF_CLUSTERS + 2) - 1) {
            next_cluster = 0xFFF;
        } else {
            next_cluster = n + 1;
        }
        FAT[m] = (uint8_t)n & 0xFF;
        FAT[m + 1] = (((uint8_t)next_cluster & 0xF) << 4) | ((uint8_t)(n>>8)&0xF);
        FAT[m + 2] = (uint8_t)(next_cluster >> 4) & 0xFF;
        m += 3;
    }

    /* Correct file size entry for file firmware.bin since user flash size isn't
     * know until runtime.
     */
    DIRECTORY_ENTRIES[1].filesize = USER_FLASH_SIZE;

    // Start up LPCUSB mass storage system to allow user to copy
    // their application binary to the LPC1768's flash.
    usb_msc_start ();

    // Note - should never actually return from usb_msc_start ().
}

/*********************************
 * Main entry point for bootloader
 *********************************/

int main (void) {
    initialize_logging();
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

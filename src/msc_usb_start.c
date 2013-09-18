/*
    LPCUSB, an USB device driver for LPC microcontrollers
    Copyright (C) 2006 Bertrik Sikken (bertrik@sikken.nl)

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. The name of the author may not be used to endorse or promote products
       derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*********************************************************************
 * Code Red Technologies - modification of LPCUSB mass storage example
 * to provide a flash-based USB secondary bootloader for RDB1768
 * development board fitted with NXP LPC1768 MCU.
 *********************************************************************/

#include <stdint.h>
#include "usbapi.h"
#include "log.h"
#include "msc_bot.h"
#include "disk.h"

#define BAUD_RATE    115200
#define MAX_PACKET_SIZE    64
#define LE_WORD(x)        ((x)&0xFF),((x)>>8)

static uint8_t abClassReqData[4];

static const uint8_t abDescriptors[] = {

// device descriptor
    0x12,
    DESC_DEVICE,
    LE_WORD(0x0200),        // bcdUSB
    0x00,                    // bDeviceClass
    0x00,                    // bDeviceSubClass
    0x00,                    // bDeviceProtocol
    MAX_PACKET_SIZE0,        // bMaxPacketSize
    LE_WORD(0xFFFF),        // idVendor
    LE_WORD(0x0003),        // idProduct
    LE_WORD(0x0100),        // bcdDevice
    0x01,                    // iManufacturer
    0x02,                    // iProduct
    0x03,                    // iSerialNumber
    0x01,                    // bNumConfigurations

// configuration descriptor
    0x09,
    DESC_CONFIGURATION,
    LE_WORD(32),            // wTotalLength
    0x01,                    // bNumInterfaces
    0x01,                    // bConfigurationValue
    0x00,                    // iConfiguration
    0xC0,                    // bmAttributes
    0x32,                    // bMaxPower

// interface
    0x09,
    DESC_INTERFACE,
    0x00,                    // bInterfaceNumber
    0x00,                    // bAlternateSetting
    0x02,                    // bNumEndPoints
    0x08,                    // bInterfaceClass = mass storage
    0x06,                    // bInterfaceSubClass = transparent SCSI
    0x50,                    // bInterfaceProtocol = BOT
    0x00,                    // iInterface
// EP
    0x07,
    DESC_ENDPOINT,
    MSC_BULK_IN_EP,            // bEndpointAddress
    0x02,                    // bmAttributes = bulk
    LE_WORD(MAX_PACKET_SIZE),// wMaxPacketSize
    0x00,                    // bInterval
// EP
    0x07,
    DESC_ENDPOINT,
    MSC_BULK_OUT_EP,        // bEndpointAddress
    0x02,                    // bmAttributes = bulk
    LE_WORD(MAX_PACKET_SIZE),// wMaxPacketSize
    0x00,                    // bInterval

// string descriptors
    0x04,
    DESC_STRING,
    LE_WORD(0x0409),

    0x10,
    DESC_STRING,
    'D', 0, 'e', 0, 'v', 0, 'B', 0, 'o', 0, 'a', 0,'r',0,'d','0',

    0x28,
    DESC_STRING,
    'L', 0, 'P', 0, 'C', 0, '1', 0, '7', 0, '5', 0, '9', 0, ' ', 0,'B',0,'o',0,'o',0,'t',0,'l',0,'o',0,'a',0,'d',0,'e',0,'r',0,

// terminating zero
    0
};


/*************************************************************************
    HandleClassRequest
    ==================
        Handle mass storage class request

**************************************************************************/
static bool HandleClassRequest(TSetupPacket *pSetup, int *piLen, uint8_t **ppbData) {
    if(pSetup->wIndex != 0) {
        debug("Invalid idx %X", pSetup->wIndex);
        return false;
    }

    if(pSetup->wValue != 0) {
        debug("Invalid val %X", pSetup->wValue);
        return false;
    }

    switch (pSetup->bRequest) {

    // get max LUN
    case 0xFE:
        *ppbData[0] = 0;        // No LUNs
        *piLen = 1;
        break;

    // MSC reset
    case 0xFF:
        if(pSetup->wLength > 0) {
            return false;
        }
        MSCBotReset();
        break;

    default:
        debug("Unhandled class");
        return false;
    }
    return true;
}


void usb_msc_start (void) {

    debug("Initialising USB stack");

    // initialise stack
    USBInit();

    // enable bulk-in interrupts on NAKs
    // these are required to get the BOT protocol going again after a STALL
    USBHwNakIntEnable(INACK_BI);

    // register descriptors
    USBRegisterDescriptors(abDescriptors);

    // register class request handler
    USBRegisterRequestHandler(REQTYPE_TYPE_CLASS, HandleClassRequest, abClassReqData);

    // register endpoint handlers
    USBHwRegisterEPIntHandler(MSC_BULK_IN_EP, MSCBotBulkIn);
    USBHwRegisterEPIntHandler(MSC_BULK_OUT_EP, MSCBotBulkOut);

    debug("Starting USB communication");

    // connect to bus
    USBHwConnect(true);

    // call USB interrupt handler continuously
    while (1) {
        USBHwISR();
    }
}

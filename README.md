openlpc-USB_Bootloader
======================

This repository contains a USB bootloader compatible with the LPC17xx
microcontroller. The lineage of this bootloader:

* Originally based off of NXP's [AN10866 LPC1700 secondary
  bootloader](http://www.lpcware.com/content/nxpfile/an10866-lpc1700-secondary-usb-bootloader)
* Code Red modified the bootloader to use the BSD licensed LPCUSB library,
  instead of the restrictively licensed USB stack from Keil (which could only be
  used with the uVision/ARM development tools).
* Modifications made for OpenLPC project (not sure on the details)
* Chris Peplin replaced Code Red's fork of the LPCUSB library with a Git
  submodule pointing to a branch in the LPCUSB library that supports the
  LPC17xx, and re-used as many example files from the LPCUSB project directly.
* Chris also replaced the startup files with a Git submodule pointer to the ARM
  CDL project.

## Installing the Bootloader

    $ script/bootstrap.sh
    $ cd src
    $ make

Assuming you have an Olimex ARM-OCD-USB JTAG adapter plugged in, connected to
the board and the board powered up:

    $ make flash

## Building User Firmware

See the simple program in the `example` directory for an example of how to build
a program for the bootloader.

The only change required is your linker script - instead of starting the user
program at flash memory location 0, start it at `0x10000`, e.g.:

    FLASH (rx) : ORIGIN = 0x10000, LENGTH = 512K - 0x10000

If you are using the `SystemInit()` function from the ARM CDL, be aware that it
sets the vector table offset register (`VTOR`) back to `0`, so interrupts will
not work. A workaround is to manually set the `VTOR` to the location of your
interrupt vectors at startup:

    void Reset_Handler(void) {
        SystemInit();
        SCB->VTOR = (uint32_t) interrupt_vectors;
        ...
    }

This workaround doesn't have any negative effects if you use the same code
running on bare metal.

## Flashing User Code

Note that the `firmware.bin` file will always look the same (with an old
modification date and the same `firmware.bin` filename), even if you reflash and
remount the bootloader. It's not a true filesystem, we are just creating a fake
file so you can write to it from standard file browser tools.

### Windows

To flash, hold down the bootloader entry button while plugging into USB or
hitting the reset button. A USB drive should appear.

* Delete the firmware.bin file
* Copy your new firmware.bin over (the filename doesn't matter)
* Unmount and reset the microcontroller

### Mac OS X

There is an [issue](https://github.com/openxc/openlpc-USB_Bootloader/issues/6)
with writing the firmware using Finder right now, so instead you must use the
command line.

To flash, hold down the bootloader entry button while plugging into USB or
hitting the reset button. A USB drive should appear.

Copy your new firmware.bin over the top of the existing firmware.bin from the
command line:

  $ cp newfirmware.bin /Volumes/LPC1759/firmware.bin

Eject and reset the microcontroller.

### Linux

Mounting the USB and copying over the firmware [does not
work](http://dangerousprototypes.com/docs/LPC_ARM_quick_start#Bootloaders) from
Linux. You need to use `mdel` and `mcopy` from the `mtools` package.

To flash, hold down the bootloader entry button while powering on. Then:

    $ sudo mdel -i /dev/sdc ::/firmware.bin
    $ sudo mcopy -i /dev/sdc firmware.bin ::/firmware.bin

where `/dev/sdc` is the device name of the LPC17xx. No need to unmount or
anything after that, just reset the board.

There's a utility script at `script/flash.sh` to automate these two steps,
since sometimes it can take a few seconds before the LPC17xx filesystem can be
written, and you have to keep retrying the mdel command. Run it like so:

    $ ../script/flash.sh /dev/sdc blink.bin

## License

The LPCUSB library is made availble under the BSD license. It is linked to from
this project as a Git submodule.

The core of the bootloader is originally developed by NXP, and is licensed under
NXP's permissive example code license:

    Software that is described herein is for illustrative purposes only
    which provides customers with programming information regarding the
    products. This software is supplied "AS IS" without any warranties.
    NXP Semiconductors assumes no responsibility or liability for the
    use of the software, conveys no license or title under any patent,
    copyright, or mask work right to the product. NXP Semiconductors
    reserves the right to make changes in the software without
    notification. NXP Semiconductors also make no representation or
    warranty that such application will be suitable for the specified
    use without further testing or modification.

A few remaining pieces were developed by Code Red, and are available under a
more restrictive license (`main.c` and `blockdev_flash.c`):

    The software is owned by Code Red Technologies and/or its suppliers, and is
    protected under applicable copyright laws.  All rights are reserved.  Any
    use in violation of the foregoing restrictions may subject the user to criminal
    sanctions under applicable laws, as well as to civil liability for the breach
    of the terms and conditions of this license.

    THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
    OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
    USE OF THIS SOFTWARE FOR COMMERCIAL DEVELOPMENT AND/OR EDUCATION IS SUBJECT
    TO A CURRENT END USER LICENSE AGREEMENT (COMMERCIAL OR EDUCATIONAL) WITH
    CODE RED TECHNOLOGIES LTD.

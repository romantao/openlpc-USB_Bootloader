openlpc-USB_Bootloader
======================

LPC17xx USB bootloader based on a Mass Storage Device

## Installing the Bootloader

    $ git submodule update --init
    $ cd src
    $ make

Assuming you have an Olimex ARM-OCD-USB JTAG adapter plugged in, connected to
the board and the board powered up:

    $ make flash

## Flashing User Code

### Windows / OS X

To flash, hold down the bootloader entry button while plugging into USB or
hitting the reset button. A USB drive should appear.

* Delete the firmware.bin file
* Copy your new firmware.bin over (the filename doesn't matter)
* Unmount and reset the MCU


### Linux

Mounting the USB and copying over the firmware [does not
work](http://dangerousprototypes.com/docs/LPC_ARM_quick_start#Bootloaders) from
Linux. You need to use `mdel` and `mcopy` from the `mtools` package.

To flash, hold down the bootloader entry button while powering on. Then:

    $ sudo mdel -i /dev/sdc ::/firmware.bin
    $ sudo mcopy -i /dev/sdc firmware.bin ::/firmware.bin

where `/dev/sdc` is the device name of the LPC17xx. No need to unmount or
anything after that, just reset the board.

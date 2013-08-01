#!/bin/bash
#
# Automate re-flashing an LPC17xx device running the USB bootloader from Linux.
#
# Example:
#    $ ./flash.sh /dev/sdc new-firmware.bin
#

DEVICE=$1
FIRMWARE=$2

FIRMWARE_PATH=::/firmware.bin

echo "Copying new firmware..."
sudo mcopy -o -i $DEVICE $FIRMWARE $FIRMWARE_PATH
while [ $? != 0 ]; do
    sleep 2

    echo "Re-trying copying new firwmare..."
    sudo mcopy -o -i $DEVICE $FIRMWARE $FIRMWARE_PATH
done
echo "Done."

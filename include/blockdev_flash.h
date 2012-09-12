//*****************************************************************************
//   +--+
//   | ++----+
//   +-++    |
//     |     |
//   +-+--+  |
//   | +--+--+
//   +----+    Copyright (c) 2009 Code Red Technologies Ltd.
//
// blockdev_flash.h - header file for bottom layer for LPCUSB stack mass
//                    storage, providing access to LPC1768 flash memory for
//                    RDB1768 bootloader.
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

#ifndef  _BLOCKDEV_FLASH_H
#define  _BLOCKDEV_FLASH_H

#include "lpcusb_type.h"

int BlockDevGetSize(U32 *pdwDriveSize);
int BlockDevWrite(U32 dwAddress, U8 * pbBuf);
int BlockDevRead(U32 dwAddress, U8 * pbBuf);

#endif // _BLOCKDEV_FLASH_H

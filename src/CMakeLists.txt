CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_MODULE_PATH    ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
SET(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Toolchains/Toolchain-lpc1759.cmake)
SET(LINKER_SCRIPT_FILE   ${CMAKE_CURRENT_SOURCE_DIR}/../basefiles/lpc1759NOBL.ld)

PROJECT(LPC17xx_USB_Bootloader C)

INCLUDE(lpc21isp)

#
#
#

SET(CMSIS_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/../CMSIS)
SET(BASEFILES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../basefiles)
SET(USB_DIR	  ${CMAKE_CURRENT_SOURCE_DIR}/usbstack)

INCLUDE_DIRECTORIES(
	.
	./include
	${USB_DIR}/include
	${CMSIS_DIR}/include

)


ADD_DEFINITIONS(-Wall -Wextra -O2 -ffunction-sections -fdata-sections)

ADD_EXECUTABLE(main
	./src/main.c
	./src/msc_bot.c
	./src/msc_scsi.c
	./src/msc_usb_start.c
	./src/sbl_iap.c
	./src/blockdev_flash.c
	./src/diskimage.c
	./usbstack/src/usbcontrol.c
	${USB_DIR}/src/usbhw_lpc.c
	${USB_DIR}/src/usbinit.c
	${USB_DIR}/src/usbstdreq.c
	${CMSIS_DIR}/src/core_cm3.c
	${CMSIS_DIR}/src/system_lpc17xx.c
	${BASEFILES_DIR}/startup_lpc17xx.c
)

TARGET_LINK_LIBRARIES(main -lm)

GENERATE_HEX(main)

LPC21ISP(main /dev/ttyUSB0 115200 12000)


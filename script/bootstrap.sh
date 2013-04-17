#!/usr/bin/env bash

set -e

CYGWIN_PACKAGES="git, curl, libsasl2, ca-certificates, make, gcc4, patchutils"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $DIR/..

KERNEL=`uname`
if [ ${KERNEL:0:7} == "MINGW32" ]; then
    OS="windows"
elif [ ${KERNEL:0:6} == "CYGWIN" ]; then
    OS="cygwin"
elif [ $KERNEL == "Darwin" ]; then
    OS="mac"
else
    OS="linux"
    if ! command -v lsb_release >/dev/null 2>&1; then
        # Arch Linux
        if command -v pacman>/dev/null 2>&1; then
            sudo pacman -S lsb-release
        fi
    fi

    DISTRO=`lsb_release -si`
fi

_cygwin_error() {
    echo
    echo "${bldred}Missing \"$1\"${txtrst} - run the Cygwin installer again and select the base package set:"
    echo "    $CYGWIN_PACKAGES"
    echo "After installing the packages, re-run this bootstrap script."
    die
}

if [ $OS == "cygwin" ] && ! command -v tput >/dev/null 2>&1; then
    _cygwin_error "ncurses"
fi

txtrst=$(tput sgr0) # reset
bldred=${txtbld}$(tput setaf 1)
bldgreen=${txtbld}$(tput setaf 2)

die() {
    echo >&2 "${bldred}$@${txtrst}"
    exit 1
}

_pushd() {
    pushd $1 > /dev/null
}

_popd() {
    popd > /dev/null
}

_wait() {
    if [ -z $CI ]; then
        echo "Press Enter when done"
        read
    fi
}

_install() {
    if [ $OS == "cygwin" ]; then
        _cygwin_error $1
    elif [ $OS == "mac" ]; then
        # brew exists with 1 if it's already installed
        set +e
        brew install $1
        set -e
    else
        if ! command -v lsb_release >/dev/null 2>&1; then
            echo
            echo "Missing $1 - install it using your distro's package manager or build from source"
            _wait
        else
            if [ $DISTRO == "arch" ]; then
                sudo pacman -S $1
            elif [ $DISTRO == "Ubuntu" ]; then
                sudo apt-get update -qq
                sudo apt-get install $1 -y
            else
                echo
                echo "Missing $1 - install it using your distro's package manager or build from source"
                _wait
            fi
        fi
    fi
}


download() {
    url=$1
    filename=$2
    curl $url -L --O $filename
}

if [ `id -u` == 0 ]; then
    die "Error: running as root - don't use 'sudo' with this script"
fi

if ! command -v curl >/dev/null 2>&1; then
    if [ $OS == "cygwin" ]; then
        _cygwin_error "curl"
    else
        _install curl
    fi
fi

echo "Storing all downloaded dependencies in the \"dependencies\" folder"

DEPENDENCIES_FOLDER="dependencies"
mkdir -p $DEPENDENCIES_FOLDER

if [ $OS == "windows" ]; then
    die "Sorry, the bootstrap script for compiling from source doesn't support the Windows console - try Cygwin."
fi

if [ $OS == "mac" ] && ! command -v brew >/dev/null 2>&1; then
    echo "Installing Homebrew..."
    ruby -e "$(curl -fsSkL raw.github.com/mxcl/homebrew/go)"
fi

if ! command -v make >/dev/null 2>&1; then
    if [ $OS == "cygwin" ]; then
        _cygwin_error "make"
    elif [ $OS == "mac" ]; then
            echo "Missing 'make' - install the Xcode CLI tools"
	    die
    else
        if [ $DISTRO == "arch" ]; then
            sudo pacman -S base-devel
        elif [ $DISTRO == "Ubuntu" ]; then
            sudo apt-get update -qq
            sudo apt-get install build-essential -y
        fi
    fi
fi

if ! command -v git >/dev/null 2>&1; then
    if [ $OS == "cygwin" ]; then
        _cygwin_error "git"
    elif [ $OS == "mac" ]; then
        _install git
    fi
fi

echo "Updating Git submodules..."

# git submodule update is a shell script and expects some lines to fail
set +e
if ! git submodule update --init --quiet; then
    echo "Unable to update git submodules - try running \"git submodule update\" to see the full error"
    echo "If git complains that it \"Needed a single revision\", run \"rm -rf src/libs\" and then try the bootstrap script again"
    if [ $OS == "cygwin" ]; then
        echo "In Cygwin this may be true (ignore if you know ca-certifications is installed:"
        _cygwin_error "ca-certificates"
    fi
    die
fi
set -e

# ARM / LPC17XX Dependencies

if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then

    echo "Installing GCC for ARM Embedded..."

    GCC_ARM_BASENAME="gcc-arm-none-eabi-4_7-2012q4-20121208"
    if [ $OS == "linux" ]; then
        GCC_ARM_FILE="$GCC_ARM_BASENAME-linux.tar.bz2"
    elif [ $OS == "mac" ]; then
        GCC_ARM_FILE="$GCC_ARM_BASENAME-mac.tar.bz2"
    elif [ $OS == "cygwin" ]; then
        GCC_ARM_FILE="$GCC_ARM_BASENAME-win32.exe"
    fi

    GCC_ARM_URL="https://launchpad.net/gcc-arm-embedded/4.7/4.7-2012-q4-major/+download/$GCC_ARM_FILE"
    GCC_ARM_DIR="gcc-arm-embedded"

    _pushd $DEPENDENCIES_FOLDER
    if ! test -e $GCC_ARM_FILE
    then
        download $GCC_ARM_URL $GCC_ARM_FILE
    fi

    mkdir -p $GCC_ARM_DIR
    _pushd $GCC_ARM_DIR
    if [ $OS == "cygwin" ]; then
        chmod a+x ../$GCC_ARM_FILE
        INSTALL_COMMAND="cygstart.exe ../$GCC_ARM_FILE"
        PROGRAM_FILES_BASE="/cygdrive/c/"
        PROGRAM_FILES="Program Files"
        PROGRAM_FILES_64="Program Files (x86)"
        TRAILING_DIRNAME="GNU Tools ARM Embedded/4.7 2012q4/"
        GCC_INNER_DIR="$PROGRAM_FILES_BASE/$PROGRAM_FILES_64/$TRAILING_DIRNAME"
        if ! test -d "$GCC_INNER_DIR"; then
            GCC_INNER_DIR="$PROGRAM_FILES_BASE/$PROGRAM_FILES/$TRAILING_DIRNAME"
        fi
    else
        GCC_INNER_DIR="gcc-arm-none-eabi-4_7-2012q4"
        INSTALL_COMMAND="tar -xjf ../$GCC_ARM_FILE"
    fi

    if ! test -d "$GCC_INNER_DIR"
    then
        $INSTALL_COMMAND
        if [ $OS == "cygwin" ]; then
            echo -n "Press Enter when the GCC for ARM Embedded installer is finished"
            read
        fi
    fi

    if [ $OS == "cygwin" ]; then
        GCC_INNER_DIR="$PROGRAM_FILES_BASE/$PROGRAM_FILES_64/$TRAILING_DIRNAME"
        if ! test -d "$GCC_INNER_DIR"; then
            GCC_INNER_DIR="$PROGRAM_FILES_BASE/$PROGRAM_FILES/$TRAILING_DIRNAME"
            if ! test -d "$GCC_INNER_DIR"; then
                die "GCC for ARM isn't installed in the expected location."
            fi
        fi
    fi

    if ! test -d arm-none-eabi; then
        echo "Copying GCC binaries to local dependencies folder..."
        cp -R "$GCC_INNER_DIR"/* .
    fi

    _popd
    _popd

fi

if [ -z $CI ] && ! command -v openocd >/dev/null 2>&1; then

    ## Download OpenOCD for flashing ARM via JTAG
    _pushd $DEPENDENCIES_FOLDER

    echo "Installing OpenOCD..."
    if [ $OS == "linux" ]; then
        _install "openocd"
    elif [ $OS == "mac" ]; then
        _install libftdi
        _install libusb
        set +e
        brew install --enable-ft2232_libftdi open-ocd
        set -e
    elif [ $OS == "cygwin" ]; then
        echo
        echo "Missing OpenOCD and it's not trivial to install in Windows - you won't be able to program the ARM platform (not required for the chipKIT translator)"
    fi
    _popd
fi

FTDI_USB_DRIVER_PLIST=/System/Library/Extensions/FTDIUSBSerialDriver.kext/Contents/Info.plist
if [ -z $CI ]  && [ $OS == "mac" ] && [ -e $FTDI_USB_DRIVER_PLIST ]; then
    if grep -q "Olimex OpenOCD JTAG A" $FTDI_USB_DRIVER_PLIST; then
        sudo sed -i "" -e "/Olimex OpenOCD JTAG A/{N;N;N;N;N;N;N;N;N;N;N;N;N;N;N;N;d;}" $FTDI_USB_DRIVER_PLIST
        FTDI_USB_DRIVER_MODULE=/System/Library/Extensions/FTDIUSBSerialDriver.kext/
        # Driver may not be loaded yet, but that's OK - don't exit on error.
        set +e
        sudo kextunload $FTDI_USB_DRIVER_MODULE
        set -e
        sudo kextload $FTDI_USB_DRIVER_MODULE
    fi
fi

if [ $OS == "cygwin" ] && ! command -v ld >/dev/null 2>&1; then
    _cygwin_error "gcc4"
fi

popd

echo
echo "${bldgreen}All developer dependencies installed, ready to compile.$txtrst"

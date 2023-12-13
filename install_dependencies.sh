#!/bin/bash

set -eu -o pipefail # fail on error and report it, debug all lines

sudo -n true
test $? -eq 0 || exit 1 "you should have sudo privilege to run this script"

sudo apt-get install -y \
	libusb-1.0-0-dev \
	linux-headers-`uname -r` \
        build-essential \
        libwxbase3.0-dev \
        libwxgtk3.0-gtk3-dev


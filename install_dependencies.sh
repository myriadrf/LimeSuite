#!/bin/bash

set -eu -o pipefail # fail on error and report it, debug all lines

if [ "$EUID" -ne 0 ]; then
  echo "You should run this script as root"
  exit 1
fi

apt-get install -y \
  libusb-1.0-0-dev \
  "linux-headers-$(uname -r)" \
  build-essential \
  libwxbase3.0-dev \
  libwxgtk3.0-gtk3-dev \
  cmake

#!/bin/bash

set -eu -o pipefail # fail on error and report it, debug all lines

if [ "$EUID" -ne 0 ]; then
  echo "You should run this script as root"
  exit 1
fi

apt-get install -y \
  libusb-1.0-0-dev \
  build-essential \
  libwxgtk3.2-dev \
  cmake

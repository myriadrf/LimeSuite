#!/bin/bash

set -eu -o pipefail # fail on error and report it, debug all lines

if [ "$EUID" -ne 0 ]; then
  echo "You should run this script as root"
  exit 1
fi

. /etc/os-release

libwx="libwxgtk3.2-dev"

if [[ ($ID == "ubuntu" && ${VERSION_ID:0:2} -le 22) || ($ID == "debian" && $VERSION_ID -le 11) ]]; then
  libwx="libwxbase3.0-dev libwxgtk3.0-gtk3-dev"
fi

apt-get install -y \
  libusb-1.0-0-dev \
  build-essential \
  $libwx \
  cmake

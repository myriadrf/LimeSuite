#!/bin/bash

# Use this script to convert the master icon to several scaled versions
# which are used for the freedesktop.org xdg-icon-resource associations.
# This script depends on imagemagick to provide the convert utility.
# The icons are checked into the repository, and this script is not installed.
# Keep it around to update the desktop icons if the master icon is changed.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

MASTER_ICON="${DIR}/../src/resources/LMS_ICO.ico[0]"

for size in 16 22 32 48 64 128; do
    echo "Generating lime-suite-${size}.png..."
    convert "${MASTER_ICON}" -geometry ${size} "${DIR}/lime-suite-${size}.png"
done
echo "Done!"

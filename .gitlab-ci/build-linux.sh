#!/bin/sh

set -e

cmake . -B build
make -C build --no-print-directory

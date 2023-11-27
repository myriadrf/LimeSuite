#!/bin/sh

set -e

cmake . -B build -DBUILD_SHARED_LIBS=OFF
make -C build --no-print-directory
ctest --test-dir build --output-on-failure

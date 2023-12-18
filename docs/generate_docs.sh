#!/bin/bash
## Must be run from within the venv.

# Usage:
# ./generate_docs.sh - to just regenerate the main docs
# ./generate_docs.sh clean - to clean all the build files
# ./generate_docs.sh rebuild - to rebuild the doxygen definitions
# ./generate_docs.sh clean rebuild - to clean the build files and rebuild the definitions

set -e

if [[ ! -d "_build" || ! -d "apidoc" || ! -d "../build/xml" ]]; then
    set "rebuild"
fi

if [[ $1 == "clean" ]]; then
    echo "Deleting _build folder"
    rm -rf _build

    echo "Deleting apidoc folder"
    rm -rf apidoc

    shift
    if [[ $1 == "" ]]; then
        exit 0
    fi
fi

if [[ $1 == "rebuild" ]]; then
    cmake -S .. -B ../build
    make --no-print-directory -C ../build -j"$(nproc)" doc
    breathe-apidoc --generate class --members --force --output-dir apidoc ../build/xml/
    python add_undoc_members.py
fi

make -j"$(nproc)" html

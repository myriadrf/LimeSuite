#!/bin/sh

# this scripts makes an absolute softlink a relative one
# needed to make a debian/ubuntu install a valid sysroot

target=$1

printf "doing re re ln on %s" "$target"

src=$(readlink "$target")
link=$target

rm -f "$link"
ln -vs --relative "$src" "$link"

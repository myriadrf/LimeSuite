#!/bin/sh

# this scripts makes an absolute softlink a relative one
# needed to make a debian/ubuntu install a valid sysroot

target=$1

echo doing re re ln on $target

src=`ls -l $target | awk '{print $11}'`
link=`ls -l $target | awk '{print $9}'`

rm -f $link
ln -vs --relative $src $link


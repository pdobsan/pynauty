#!/bin/bash

# Ref.: https://github.com/pypa/manylinux
#
echo Fixing wheel tag ...
echo Warning: this works only on Archlinux or derivatives

TAG=manylinux2010
TAG=manylinux2014
# should work on any distro based on glibc>=x.y
# TAG=manylinux_x_y

X=$(pacman -Q glibc | cut -d ' ' -f 2 | cut -d - -f 1 | cut -d . -f 1)
Y=$(pacman -Q glibc | cut -d ' ' -f 2 | cut -d - -f 1 | cut -d . -f 2)
TAG=manylinux\_$X\_$Y
# echo $X $Y $TAG

ORIG=$(ls pynauty-*-linux_x86_64.whl)
NEW=$(echo $ORIG | sed -e "s/linux/$TAG/")

mv -f $ORIG $NEW

echo ... done.

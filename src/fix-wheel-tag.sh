#!/bin/bash

# Ref.: https://github.com/pypa/manylinux
#

echo Fixing wheel tag ...
if type pacman &> /dev/null
then
  echo ... on Archlinux ...
  X=$(pacman -Q glibc | cut -d ' ' -f 2 | cut -d - -f 1 | cut -d . -f 1)
  Y=$(pacman -Q glibc | cut -d ' ' -f 2 | cut -d - -f 1 | cut -d . -f 2)
  TAG=manylinux\_$X\_$Y
else
  if [ `uname -s` = Linux ]; then
    echo ... on Linux ...
    TAG=manylinux2014
  else
    echo ... not on Linux, bailing out ...
    exit 0
  fi
fi

ORIG=$(ls pynauty-*-linux_*.whl)
NEW=$(echo $ORIG | sed -e "s/linux/$TAG/")

echo $ORIG '-->' $NEW
mv -f $ORIG $NEW

echo ... done.

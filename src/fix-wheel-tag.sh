#!/bin/bash

# Ref.: https://github.com/pypa/manylinux
#

echo Fixing wheel tag ...
if [ `uname -s` = Linux ]; then
  echo ... on Linux ...
  X=$(ldd --version | head -1 | cut -d ' ' -f 4 | cut -d . -f 1)
  Y=$(ldd --version | head -1 | cut -d ' ' -f 4 | cut -d . -f 2)
  TAG=manylinux\_$X\_$Y
else
  echo ... not on Linux, bailing out ...
  exit 0
fi

ORIG=$(ls pynauty-*-linux_*.whl)
NEW=$(echo $ORIG | sed -e "s/linux/$TAG/")

echo $ORIG '-->' $NEW
mv -f $ORIG $NEW

echo ... done.

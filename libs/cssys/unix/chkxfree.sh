#!/bin/sh
# Test for XFree86 headers and features

# Find the X11 directory
if [ -d /usr/X11 ]; then
  X11_PATH="/usr/X11"
elif [ -d /usr/X11R6 ]; then
  X11_PATH="/usr/X11R6"
elif [ -d /usr/openwin ]; then
  X11_PATH="/usr/openwin"
elif [ -d /usr/lib/X11 ]; then
  X11_PATH="/usr/lib/X11"
else
  echo "$0: Cannot find X11 directory!" >&2
  exit 1
fi
echo "X11_PATH = "${X11_PATH}

# find out if Xfree86 is installed, enable VideoMode extension if so.
# detected by existence of the XFree86 server binary.
if [ -n "${X11_PATH}" ]; then
  [ -f "${X11_PATH}/lib/libXxf86vm.so" ] && echo "USE_XFREE86VM = yes"
fi


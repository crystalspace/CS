#!/bin/sh
#==============================================================================
# Check for presence of X11 and determine its features.
#
# IMPORTS
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    X11_PATH
#	Makefile variable emitted to the standard output stream.  Value is the
#	base path of the X11 installation, otherwise the variable is not set.
#    USE_XFREE86VM
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if XFree86 is installed and supports the VideoMode extension,
#	otherwise the variable is not set.
#==============================================================================

# Find the X11 directory

msg_checking "for X11"

X11_PATH=''
if [ -d /usr/X11 ]; then
  X11_PATH="/usr/X11"
elif [ -d /usr/X11R6 ]; then
  X11_PATH="/usr/X11R6"
elif [ -d /usr/openwin ]; then
  X11_PATH="/usr/openwin"
elif [ -d /usr/lib/X11 ]; then
  X11_PATH="/usr/lib/X11"
fi

if [ -n "${X11_PATH}" ]; then
  echo "X11_PATH = ${X11_PATH}"
  msg_result "${X11_PATH}"
else
  msg_result "no"
fi

# Find out if XFree86 is installed, enable VideoMode extension if so.
# Detected by existence of the XFree86 server binary.

if [ -n "${X11_PATH}" ]; then
  msg_checking "for XFree86"
  if [ -f "${X11_PATH}/lib/libXxf86vm.so" ]; then
    echo "USE_XFREE86VM = yes"
    msg_result "yes"
  else
    msg_result "no"
  fi
fi

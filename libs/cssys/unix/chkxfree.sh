#!/bin/sh
#==============================================================================
# Check for presence of X11 and determine its features.
#
# IMPORTS
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    X11_OK
#	Shell variable set to 1 if X11 is available, 0 otherwise.
#    X11.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if X11 is available, otherwise the variable is not yet.
#    X11_PATH
#	Makefile variable emitted to the standard output stream.  Value is the
#	base path of the X11 installation, otherwise the variable is not set.
#    X_CFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	compiler flags needed to build X11 clients.
#    X_LIBS
#	Makefile variable emitted to the standard output stream.  Value is the
#	linker flags needed to build X11 clients.  (X_LIBS is a misnomer
#	considering that it only lists linker flags -- not actual libraries.
#	Although, X_LFLAGS would have been a more sensible name, X_LIBS was
#	chosen for consistency with Autoconf's naming scheme.)
#    X_PRE_LIBS
#	Makefile variable emitted to the standard output stream.  Value is the
#	list of libraries which must appear before -lX11.
#    X_EXTRA_LIBS
#	Makefile variable emitted to the standard output stream.  Value is the
#	list of libraries which must appear after -lX11.
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

if [ -n "$X11_PATH" ]; then
  X11_OK=1
  echo "X11.AVAILABLE = yes"
  echo "X11_PATH = ${X11_PATH}"
  echo "X_CFLAGS = -I${X11_PATH}/include"
  echo "X_LIBS = -L${X11_PATH}/lib"
  echo "X_PRE_LIBS ="
  echo "X_EXTRA_LIBS="
  msg_result "${X11_PATH}"
else
  X11_OK=0
  msg_result "no"
fi

# Find out if XFree86 is installed, enable VideoMode extension if so.
# Detected by existence of the XFree86 server binary.

if [ ${X11_OK} -eq 1 ]; then
  msg_checking "for XFree86"
  if [ -f "${X11_PATH}/lib/libXxf86vm.so" -o -f "${X11_PATH}/lib/libXxf86vm.a" ]; then
    echo "USE_XFREE86VM = yes"
    msg_result "yes"
  else
    msg_result "no"
  fi
fi

postcondition '${X11_OK} -eq 0 -o ${X11_OK} -eq 1'

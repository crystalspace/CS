#! /bin/sh
#
# This script file is used to autodetect some parameters
# needed for building Crystal Space on various Unix flavours.
#
# Arguments: $1 is operating system subtype (linux, solaris, freebsd etc)
#            $2 is user-preferred install dir, argument can be left out
#
# The configuration (a makefile fragment) is piped to stdout
# and errors are piped to stderr.
#

SYSTYPE=$1
INSTALL_DIR=$2

BIN_DIR=bin
UNIX_DIR=libs/cssys/unix

# find out architecture
. ${BIN_DIR}/arch.sh

# Check for compiler
. ${BIN_DIR}/comptest.sh

# Check for tools
. ${BIN_DIR}/chktools.sh

# Analyse system headers
. ${BIN_DIR}/chkheadr.sh

# Test for presence of SVGALIB.
echo "#include <vga.h>" > conftest.cpp
${CXX} -c conftest.cpp 2>/dev/null && echo "PLUGINS += video/canvas/svgalib"
rm -f conftest.cpp conftest.o

# Remove dummy remains
rm -f conftest.asm conftest.o

# Find the X11 directory
. ${UNIX_DIR}/chkxfree.sh

# test for endianess
. ${BIN_DIR}/endtest.sh

# test for phyton
. ${BIN_DIR}/haspythn.sh

# find install dir
[ -z "${INSTALL_DIR}" ] && INSTALL_DIR=/usr/local/crystal
echo "INSTALL_DIR = ${INSTALL_DIR}"

exit 0

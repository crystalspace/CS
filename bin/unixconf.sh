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

# Find the directory where script is located
SCRIPT_NAME=`basename $0`
SCRIPT_DIR=`expr $0 : "\(.*\)/${SCRIPT_NAME}"`
[ -z "${SCRIPT_DIR}" ] && SCRIPT_DIR="./"
SCRIPT_DIR=`(cd ${SCRIPT_DIR} > /dev/null; pwd)` # Convert to absolute path.

# find out architecture
. ${SCRIPT_DIR}/arch.sh

# Check for compiler
. ${SCRIPT_DIR}/comptest.sh

# Check for tools
. ${SCRIPT_DIR}/chktools.sh

# Analyse system headers
. ${SCRIPT_DIR}/chkheadr.sh

# Test for presence of SVGALIB.
echo "#include <vga.h>" > conftest.cpp
${CXX} -c conftest.cpp 2>/dev/null && echo "PLUGINS += video/canvas/svgalib"
rm -f conftest.cpp conftest.o

# Remove dummy remains
rm -f conftest.asm conftest.o

# Find the X11 directory
. ${SCRIPT_DIR}/chkxfree.sh

# test for endianess
. ${SCRIPT_DIR}/endtest.sh

# test for phyton
. ${SCRIPT_DIR}/haspythn.sh

# find install dir
[ -z "${INSTALL_DIR}" ] && INSTALL_DIR=/usr/local/crystal
echo "INSTALL_DIR = ${INSTALL_DIR}"

exit 0

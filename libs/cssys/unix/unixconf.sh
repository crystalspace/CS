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

# Configuration helper functions.
. ${BIN_DIR}/confutil.sh

# find out architecture
. ${BIN_DIR}/arch.sh

# Check for compiler
. ${BIN_DIR}/comptest.sh

# Check for tools
. ${BIN_DIR}/chktools.sh

# Analyse system headers
. ${BIN_DIR}/chkheadr.sh

# test for endianess
. ${BIN_DIR}/endtest.sh

# Check for Pthread
. ${BIN_DIR}/chkpthrd.sh

if [ -n "${PTHREAD_OK}" -a ${PTHREAD_OK} -eq 1 ]; then
  echo "THREADS = pthread"
  echo "THREADS.INC = csthrd.h"
fi

# Check for Readline (required by some Python installations).
. ${BIN_DIR}/chkrdlin.sh

# Check for Phyton.
. ${BIN_DIR}/chkpythn.sh

# Test for presence of SVGALIB.
msg_checking "for svgalib"
echo "#include <vga.h>" > conftest.cpp
${CXX} -c conftest.cpp 2>/dev/null
if [ $? -eq 0 ]; then
  echo "PLUGINS += video/canvas/svgalib"
  msg_result "yes"
else
  msg_result "no"
fi

# Test for presence of CAL3D.
msg_checking "cal3d"
echo "#include <cal3d/cal3d.h>" > conftest.cpp
${CXX} -c conftest.cpp 2>/dev/null
if [ $? -eq 0 ]; then
  echo "HAS_CAL3D = yes"
  msg_result "yes"
else
  msg_result "no"
fi

rm -f conftest.cpp conftest.o conftest.obj conftest.exe conftest conftest.asm

# Find the X11 directory
. ${UNIX_DIR}/chkxfree.sh

# Does for special OpenGL extensions.
. ${UNIX_DIR}/chkogl.sh

# Find install directory.
if [ -z "${INSTALL_DIR}" ]; then
  INSTALL_DIR=/usr/local/crystal
fi
echo "INSTALL_DIR = ${INSTALL_DIR}"

exit 0

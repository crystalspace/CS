#!/bin/sh
#==============================================================================
# A compiler capability testing script for Apple/NeXT.
# Copyright (C)2001,2002 by Eric Sunshine <sunshine@sunshineco.com>
#
# The output of this script (a makefile fragment) is configuration information
# needed for building Crystal Space.  It is piped to stdout, and errors are
# piped to stderr.
#==============================================================================

BIN_DIR=bin

# Helper function for finding programs.
. ${BIN_DIR}/checkprog.sh

# Check for compiler and compiler capabilities.
. ${BIN_DIR}/comptest.sh

# We also compile Objective-C and Objective-C++.
echo "OBJC = ${CC}"
echo "OBJCXX = ${CXX}"

# Check for tools.
. ${BIN_DIR}/chktools.sh

# Analyze system header files.
. ${BIN_DIR}/chkheadr.sh

# Check for Phyton.
. ${BIN_DIR}/haspythn.sh


#------------------------------------------------------------------------------
# Check if the linker recognizes "-multiply_defined suppress".  This is
# required for MacOS/X 10.1 to avoid warning messages when linking a program
# with "-framework Foundation" if that program does not actually employ any
# Objective-C.
#------------------------------------------------------------------------------
echo "int main(void) { return 0; }" > conftest.c
${CC} -c conftest.c && \
${LINK} -o conftest conftest.o -multiply_defined suppress 2>/dev/null && \
  echo "NEXT.LFLAGS.CONFIG += -multiply_defined suppress"


#------------------------------------------------------------------------------
# Clean up.
#------------------------------------------------------------------------------
rm -f conftest*
exit 0

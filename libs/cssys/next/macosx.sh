#!/bin/sh
#==============================================================================
# A compiler capability testing script for MacOS/X.
# Copyright (C)2001 by Eric Sunshine <sunshine@sunshineco.com>
#
# Arguments: $1 is the name of the C compiler.
#
# The output of this script (a makefile fragment) is configuration information
# needed for building Crystal Space.  It is piped to stdout, and errors are
# piped to stderr.
#==============================================================================
CC=$1

#------------------------------------------------------------------------------
# Check if the linker recognizes "-multiply_defined suppress".  This is
# required for MacOS/X 10.1 to avoid warning messages when linking a program
# with "-framework Foundation" if that program does not actually employ any
# Objective-C.
#------------------------------------------------------------------------------
echo "int main(void) { return 0; }" > conftest.c
${CC} -c conftest.c && \
${CC} -o conftest conftest.o -multiply_defined suppress 2>/dev/null && \
  echo "NEXT.LFLAGS.CONFIG += -multiply_defined suppress"

#------------------------------------------------------------------------------
# Clean up.
#------------------------------------------------------------------------------
rm -f conftest*
exit 0

#! /bin/sh
#==============================================================================
# A compiler capability testing script for Apple/NeXT.
# Copyright (C)2001,2002 by Eric Sunshine <sunshine@sunshineco.com>
#
# The output of this script (a makefile fragment) is configuration information
# needed for building Crystal Space.  It is piped to stdout, and errors are
# piped to stderr.
#==============================================================================

BIN_DIR=bin

# Configuration helper functions.
. ${BIN_DIR}/confutil.sh

# Check for compiler and compiler capabilities.
. ${BIN_DIR}/comptest.sh

# We also compile Objective-C and Objective-C++.
msg_checking "for Objective-C compiler"
echo "OBJC = ${CC} -c"
msg_result "${CC}"

msg_checking "for Objective-C++ compiler"
echo "OBJCXX = ${CXX} -c"
msg_result "${CXX}"

# Check for tools.
. ${BIN_DIR}/chktools.sh

# Analyze system header files.
. ${BIN_DIR}/chkheadr.sh

# Check for zlib.
. ${BIN_DIR}/chkzlib.sh

# Check for libpng.
. ${BIN_DIR}/chkpng.sh

# Check for libjpeg.
. ${BIN_DIR}/chkjpeg.sh

# Check for Pthread (required by some Python installations).
. ${BIN_DIR}/chkpthrd.sh

# Check for Readline (required by some Python installations).
. ${BIN_DIR}/chkrdlin.sh

# Check for Phyton.
. ${BIN_DIR}/chkpythn.sh


#------------------------------------------------------------------------------
# Check if the linker recognizes "-multiply_defined suppress".  This is
# required for MacOS/X 10.1 to avoid warning messages when linking a program
# with "-framework Foundation" if that program does not actually employ any
# Objective-C.
#------------------------------------------------------------------------------
msg_checking "for Apple/NeXT linker flags"
echo "int main(void) { return 0; }" > conftest.c
${CC} -c conftest.c && \
${LINK} -o conftest conftest.o -multiply_defined suppress 2>/dev/null
if [ $? -eq 0 ]; then
  echo "NEXT.LFLAGS.CONFIG += -multiply_defined suppress"
  msg_result "-multiply_defined suppress"
else
  msg_result "no"
fi


#------------------------------------------------------------------------------
# Clean up.
#------------------------------------------------------------------------------
rm -f conftest.c conftest.cpp conftest.o conftest.obj conftest.exe conftest
exit 0

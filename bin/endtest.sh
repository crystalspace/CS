#!/bin/sh
#
# This script file is used to autodetect the endianess of the host platform.
#
# Arguments: $1 is the name of the C++ compiler (gcc, c++, etc.)
#
# The output of this script (a makefile fragment) is configuration
# information needed for building Crystal Space.  It is pipied to stdout,
# and errors are piped to stderr.

echo "int main() { long x = 0x12; return *(unsigned char *)&x == 0x12; }" > comptest.cpp

${CXX} -o comptest comptest.cpp 2>/dev/null || echo "endtest.sh: cannot compile testcase" >&2
if test -f ./comptest; then
    if ./comptest; then
	echo "CS_BIG_ENDIAN = 1"
    else
	echo "CS_LITTLE_ENDIAN = 1"
    fi
fi

rm -f comptest.cpp comptest comptest.exe

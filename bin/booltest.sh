#!/bin/sh
#
# This script file is used to autodetect whether or not the C++ compiler
# has a built-in 'bool' type.
#
# Arguments: $1 is the name of the C++ compiler (gcc, c++, etc.)
#
# The output of this script (a makefile fragment) is configuration
# information needed for building Crystal Space.  It is pipied to stdout,
# and errors are piped to stderr.

CXX=$1

echo "int main() { bool b = true; return (int)b; }" > booltest.cpp

${CXX} -c booltest.cpp 2>/dev/null || echo "NEED_FAKE_BOOL = yes"

rm -f booltest.cpp booltest.o

exit 0

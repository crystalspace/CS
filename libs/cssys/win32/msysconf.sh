#! /bin/sh
#
# This script file is used to autodetect some parameters
# needed for building Crystal Space on various Unix flavours.
#
# Arguments: $1 is user-preferred install dir, argument can be left out
#
# The configuration (a makefile fragment) is piped to stdout
# and errors are piped to stderr.
#

INSTALL_DIR=$1

CC=gcc
CXX=gcc

BIN_DIR=bin

# Helper function for finding apps
. ${BIN_DIR}/checkprog.sh

# find out architecture
. ${BIN_DIR}/arch.sh

# Check for compiler
#. ${BIN_DIR}/comptest.sh

# Check for tools
. ${BIN_DIR}/chktools.sh

# Analyse system headers
. ${BIN_DIR}/chkheadr.sh

# test for endianess
. ${BIN_DIR}/endtest.sh

# Check for python
. ${BIN_DIR}/haspythn.sh

# Test if we need to link explicitly with libmingex.a.  Older versions of MinGW
# did not have this library, whereas newer interim verions supply it but do not
# link automatically with it.  The very newest versions (presumably) link with
# libmingex.a automatically.  To see if libmingex.a is required, we try using
# opendir(), which exists in libming32.a for older releases, and in libmingex.a
# for newer releases.

cat << EOF > comptest.cpp
#include <dirent.h>
int main() { DIR* p = opendir("."); return p != 0; }
EOF

if ${CXX} -o comptest comptest.cpp 2>/dev/null; then
    true # Do not need any extra flags.
else
    ${CXX} -o comptest comptest.cpp -lmingwex 2>/dev/null && \
        echo "MINGW_LIBS += -lmingwex"
fi

rm -f comptest.cpp comptest.o comptest.obj comptest.exe comptest

exit 0

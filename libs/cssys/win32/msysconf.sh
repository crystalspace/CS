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
BIN_DIR=bin

# Helper function for finding apps
. ${BIN_DIR}/checkprog.sh

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

# Check for python
. ${BIN_DIR}/haspythn.sh


#------------------------------------------------------------------------------
# Check if the compiler recognizes -fvtable-thunks.  As of gcc 3.x, this option
# is no longer supported.  Unfortunately, rather than returning an error code,
# the compiler merely prints a warning message, so we need to capture the error
# output as well.
#------------------------------------------------------------------------------

echo "int main() { return 0; }" > comptest.cpp
${CXX} -c -fvtable-thunks comptest.cpp 2>comptest.log
if [ $? -eq 0 ]; then
    if [ ! -s "comptest.log" ]; then
	echo "CFLAGS.SYSTEM += -fvtable-thunks"
    fi
fi


#------------------------------------------------------------------------------
# Test if we need to link explicitly with libmingex.a.  Older versions of MinGW
# did not have this library, whereas newer interim verions supply it but do not
# link automatically with it.  The very newest versions (presumably) link with
# libmingex.a automatically.  To see if libmingex.a is required, we try using
# opendir(), which exists in libming32.a for older releases, and in libmingex.a
# for newer releases.
#------------------------------------------------------------------------------

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


#------------------------------------------------------------------------------
# Clean up.
#------------------------------------------------------------------------------
rm -f comptest.cpp comptest.o comptest.obj comptest.exe comptest comptest.log
exit 0

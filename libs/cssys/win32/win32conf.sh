#! /bin/sh
#==============================================================================
# A compiler capability testing script for Mingw and Cygwin.
#
# The output of this script (a makefile fragment) is configuration information
# needed for building Crystal Space.  It is piped to stdout, and errors are
# piped to stderr.
#==============================================================================

INSTALL_DIR=$1
BIN_DIR=bin

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

# Check for Readline (required by some Python installations).
. ${BIN_DIR}/chkrdlin.sh

# Check for python
. ${BIN_DIR}/chkpythn.sh


#------------------------------------------------------------------------------
# Check if the compiler recognizes -fvtable-thunks.  As of gcc 3.x, this option
# is no longer supported.  Unfortunately, rather than returning an error code,
# the compiler merely prints a warning message, so we need to capture the error
# output as well.
#------------------------------------------------------------------------------

msg_checking "if -fvtable-thunks is needed"

echo "int main() { return 0; }" > comptest.cpp

${CXX} -c -fvtable-thunks comptest.cpp 2>comptest.log
if [ $? -eq 0 ]; then
  if [ ! -s "comptest.log" ]; then
    echo "CFLAGS.SYSTEM += -fvtable-thunks"
    msg_result "yes"
  else
    msg_result "no"
  fi
else
  msg_result "no"
fi


#------------------------------------------------------------------------------
# For Mingw, test if we need to link explicitly with libmingwex.a.  Older
# versions of MinGW did not have this library, whereas newer interim verions
# supply it but do not link automatically with it.  The very newest versions
# (presumably) link with libmingwex.a automatically.  To see if libmingwex.a is
# required, we try using opendir(), which exists in libming32.a for older
# releases, and in libmingwex.a for newer releases.
#------------------------------------------------------------------------------

msg_checking "if -lmingwex is needed"

cat << EOF > comptest.cpp
#include <dirent.h>
int main() { DIR* p = opendir("."); return p != 0; }
EOF

${CXX} -o comptest comptest.cpp 2>/dev/null
if [ $? -eq 0 ]; then
  msg_result "no"
else
  ${CXX} -o comptest comptest.cpp -lmingwex 2>/dev/null
  if [ $? -eq 0 ]; then
    echo "MINGW_LIBS += -lmingwex"
    msg_result "yes"
  else
    msg_result "no"
  fi
fi


#------------------------------------------------------------------------------
# Clean up.
#------------------------------------------------------------------------------
rm -f comptest.cpp comptest.o comptest.obj comptest.exe comptest comptest.log
exit 0

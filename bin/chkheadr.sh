#! /bin/sh
#==============================================================================
# Copyright (C)2000,2002 by Eric Sunshine <sunshine@sunshineco.com>
#
# Check header files for facilities and types.
#
# IMPORTS
#    CXX
#	Shell or environment variable used to compile a program.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    CS_USE_FAKE_SOCKLEN_TYPE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if socklen_t is unknown, otherwise the variable is not set.
#==============================================================================

#------------------------------------------------------------------------------
# Test for presence of socklen_t.  If not available, emit makefile variable
# CS_USE_FAKE_SOCKLEN_TYPE=yes.
#------------------------------------------------------------------------------

precondition '-n "${CXX}"'

msg_checking "for socklen_t" 

cat << EOF > comptest.cpp
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#define BSD_COMP 1
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
int main() { socklen_t x = 0; return (int)x; }
EOF

${CXX} -c comptest.cpp 2>/dev/null
if [ $? -eq 0 ]; then
  msg_result "yes"
else
  msg_result "no"
  echo "CS_USE_FAKE_SOCKLEN_TYPE = yes"
fi

rm -f comptest.cpp comptest.o

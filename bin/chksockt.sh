#! /bin/sh
#==============================================================================
# Copyright (C)2000,2002,2003 by Eric Sunshine <sunshine@sunshineco.com>
#
# Check for socket-related facilities.
#
# IMPORTS
#    CXX
#	Shell or environment variable used to compile a program.
#    LINK
#	Shell or environment variable used to link an executable.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    SOCKET.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if the socket API is available, otherwise the variable is not
#	set.
#    SOCKET.CFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	compiler flags required for socket use.
#    SOCKET.LFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	linker flags required for socket use.
#    CS_USE_FAKE_SOCKLEN_TYPE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if socklen_t is unknown, otherwise the variable is not set.
#==============================================================================

precondition '-n "${CXX}"'
precondition '-n "${LINK}"'

#------------------------------------------------------------------------------
# Check if programs which use sockets must link with a special library.
# Common libraries:
#	libsocket.a - Some Unix platforms.
#	libsocket.a libnsl.a - Solaris
#	wsock32.lib - Windows
#------------------------------------------------------------------------------

SOCKET_OK=0
SOCKET_CFLAGS=''
SOCKET_LFLAGS=''

msg_checking "for socket support"

cat << EOF > socktest.cpp
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif
int main() { return connect(0,0,0); }
EOF

${LINK} -o socktest socktest.cpp 1>/dev/null 2>&1
if [ $? -eq 0 ]; then
  SOCKET_OK=1
else
  ${LINK} -o socktest socktest.cpp -lsocket 1>/dev/null 2>&1
  if [ $? -eq 0 ]; then
    SOCKET_OK=1
    SOCKET_LFLAGS='-lsocket'
  else
    ${LINK} -o socktest socktest.cpp -lsocket -lnsl 1>/dev/null 2>&1
    if [ $? -eq 0 ]; then
      SOCKET_OK=1
      SOCKET_LFLAGS='-lsocket -lnsl'
    else
      ${LINK} -o socktest socktest.cpp -lwsock32 1>/dev/null 2>&1
      if [ $? -eq 0 ]; then
        SOCKET_OK=1
        SOCKET_LFLAGS='-lwsock32'
      fi
    fi
  fi
fi

rm -f socktest.cpp socktest.o socktest.obj socktest.exe socktest

if [ ${SOCKET_OK} -eq 1 ]; then
  echo "SOCKET.AVAILABLE = yes"
  echo "SOCKET.CFLAGS = ${SOCKET_CFLAGS}"
  echo "SOCKET.LFLAGS = ${SOCKET_LFLAGS}"
  msg_result "yes"
  if [ -n "${SOCKET_CFLAGS}" ]; then
    msg_inform "socket cflags... ${SOCKET_CFLAGS}"
  fi
  if [ -n "${SOCKET_LFLAGS}" ]; then
    msg_inform "socket lflags... ${SOCKET_LFLAGS}"
  fi
else
  msg_result "no"
fi

postcondition '${SOCKET_OK} -eq 0 -o ${SOCKET_OK} -eq 1'


#------------------------------------------------------------------------------
# Test for presence of socklen_t.  If not available, emit makefile variable
# CS_USE_FAKE_SOCKLEN_TYPE=yes.
#------------------------------------------------------------------------------

if [ ${SOCKET_OK} -eq 1 ]; then
  msg_checking "for socklen_t" 

  cat << EOF > socktest.cpp
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#define BSD_COMP 1
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
int main() { socklen_t x = 0; return (int)x; }
EOF

  ${CXX} -c socktest.cpp 2>/dev/null
  if [ $? -eq 0 ]; then
    msg_result "yes"
  else
    msg_result "no"
    echo "CS_USE_FAKE_SOCKLEN_TYPE = yes"
  fi

  rm -f socktest.cpp socktest.o socktest.obj socktest.exe socktest
fi

#! /bin/sh
#==============================================================================
# Copyright (C)2002 by Eric Sunshine <sunshine@sunshineco.com>
#
# Determine if Posix pthread capability is available and, if so, which link
# flags are required.
#
# IMPORTS
#    LINK
#	Shell or environment variable used to link an executable.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    PTHREAD_OK
#	Shell variable set to 1 if pthread is available, 0 otherwise.
#    PTHREAD_CFLAGS
#	Shell variable containing the compiler flags required for pthread.
#    PTHREAD_LFLAGS
#	Shell variable containing the linker flags required for pthread.
#    PTHREAD.AVIALABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if pthread is available, otherwise the variable is not set.
#    PTHREAD.CFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	compiler flags required for pthread.
#    PTHREAD.LFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	linker flags required for pthread.
#==============================================================================

precondition '-n "${LINK}"'

PTHREAD_OK=0
PTHREAD_CFLAGS=''
PTHREAD_LFLAGS=''

msg_checking "for pthread"

cat << EOF > testpthd.cpp
#include <pthread.h>
void* worker(void* p) { (void)p; return p; }
int main()
{
  pthread_t tid;
  pthread_create(&tid, 0, worker, 0);
  return 0;
}
EOF

${LINK} -o testpthd testpthd.cpp 1>/dev/null 2>&1
if [ $? -eq 0 ]; then
  PTHREAD_OK=1
else
  ${LINK} -o testpthd testpthd.cpp -lpthread 1>/dev/null 2>&1
  if [ $? -eq 0 ]; then
    PTHREAD_OK=1
    PTHREAD_LFLAGS='-lpthread'
  else
    ${LINK} -o testpthd testpthd.cpp -pthread 1>/dev/null 2>&1
    if [ $? -eq 0 ]; then
      PTHREAD_OK=1
      PTHREAD_LFLAGS='-pthread'
    else
      ${LINK} -o testpthd testpthd.cpp -pthread -lpthread 1>/dev/null 2>&1
      if [ $? -eq 0 ]; then
        PTHREAD_OK=1
        PTHREAD_LFLAGS='-pthread -lpthread'
      fi
    fi
  fi
fi

rm -f testpthd.cpp testpthd.o testpthd.obj testpthd.exe testpthd

if [ ${PTHREAD_OK} -eq 1 ]; then
  echo "PTHREAD.AVAILABLE = yes"
  echo "PTHREAD.CFLAGS = ${PTHREAD_CFLAGS}"
  echo "PTHREAD.LFLAGS = ${PTHREAD_LFLAGS}"
  msg_result "yes"
  if [ -n "${PTHREAD_CFLAGS}" ]; then
    msg_inform "pthread cflags... ${PTHREAD_CFLAGS}"
  fi
  if [ -n "${PTHREAD_LFLAGS}" ]; then
    msg_inform "pthread lflags... ${PTHREAD_LFLAGS}"
  fi
else
  msg_result "no"
fi

postcondition '${PTHREAD_OK} -eq 0 -o ${PTHREAD_OK} -eq 1'

#! /bin/sh
#==============================================================================
# Copyright (C)2002 by Eric Sunshine <sunshine@sunshineco.com>
#
# Determine if the readline and history facilities are available and, if so,
# which link flags are required.
#
# IMPORTS
#    LINK
#	Shell or environment variable used to link an executable.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    READLINE_OK
#	Shell variable set to 1 if readline is available, 0 otherwise.
#    READLINE_CFLAGS
#	Shell variable containing the compiler flags required for readline.
#    READLINE_LFLAGS
#	Shell variable containing the linker flags required for readline.
#    READLINE.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if readline is available, otherwise the variable is not set.
#    READLINE.CFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	compiler flags required for readline.
#    READLINE.LFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	linker flags required for readline.
#==============================================================================

precondition '-n "${LINK}"'

READLINE_OK=0
READLINE_CFLAGS=''
READLINE_LFLAGS=''

msg_checking "for readline"

cat << EOF > testrdln.cpp
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
int main()
{
  add_history(readline(""));
  return 0;
}
EOF

${LINK} -o testrdln testrdln.cpp 1>/dev/null 2>&1
if [ $? -eq 0 ]; then
  READLINE_OK=1
else
  ${LINK} -o testrdln testrdln.cpp -lreadline 1>/dev/null 2>&1
  if [ $? -eq 0 ]; then
    READLINE_OK=1
    READLINE_LFLAGS='-lreadline'
  else
    ${LINK} -o testrdln testrdln.cpp -lreadline -lhistory 1>/dev/null 2>&1
    if [ $? -eq 0 ]; then
      READLINE_OK=1
      READLINE_LFLAGS='-lreadline -lhistory'
    else
      ${LINK} -o testrdln testrdln.cpp -lreadline -lcurses 1>/dev/null 2>&1
      if [ $? -eq 0 ]; then
        READLINE_OK=1
        READLINE_LFLAGS='-lreadline -lcurses'
      else
        ${LINK} -o testrdln testrdln.cpp -lreadline -lhistory -lcurses \
	  1>/dev/null 2>&1
        if [ $? -eq 0 ]; then
          READLINE_OK=1
          READLINE_LFLAGS='-lreadline -lhistory -lcurses'
        fi
      fi
    fi
  fi
fi

rm -f testrdln.cpp testrdln.o testrdln.obj testrdln.exe testrdln

if [ ${READLINE_OK} -eq 1 ]; then
  echo "READLINE.AVAILABLE = yes"
  echo "READLINE.CFLAGS = ${READLINE_CFLAGS}"
  echo "READLINE.LFLAGS = ${READLINE_LFLAGS}"
  msg_result "yes"
  if [ -n "${READLINE_CFLAGS}" ]; then
    msg_inform "readline cflags... ${READLINE_CFLAGS}"
  fi
  if [ -n "${READLINE_LFLAGS}" ]; then
    msg_inform "readline lflags... ${READLINE_LFLAGS}"
  fi
else
  msg_result "no"
fi

postcondition '${READLINE_OK} -eq 0 -o ${READLINE_OK} -eq 1'

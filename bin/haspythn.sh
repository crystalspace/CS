#!/bin/sh
#
# This script file is used to automatically detect if the Python language is
# installed.  If the user has not already specified the location of Python's
# headers and library via PYTHON_INC and PYTHON_LIB, then a search is
# performed to locate these directories.
#
# The configuration (a makefile fragment) is piped to stdout and errors are
# piped to stderr.
#

SOURCES="/usr /usr/local /usr/local/python"

# Find the Python header and library directories.  In the event that multiple
# header and library directories are located, choose the the final
# lexicographically sorted entry from each list.  (Or stated in C terminology,
# do not 'break' out of the 'for' loops.)

if [ -z "${PYTHON_INC}" ]; then
  for i in ${SOURCES}; do
    FILES=`ls -d ${i}/include/python* 2>/dev/null`
    if [ -n "${FILES}" ]; then
      for j in ${FILES}; do
        PYTHON_INC=${j}
      done
    fi
  done
fi

if [ -z "${PYTHON_LIB}" ]; then
  for i in ${SOURCES}; do
    FILES=`ls -d ${i}/lib/python* 2>/dev/null`
    if [ -n "${FILES}" ]; then
      for j in ${FILES}; do
        PYTHON_LIB=${j}
        done
    fi
  done
fi

if [ -n "${PYTHON_INC}" -a -n "${PYTHON_LIB}" ]; then
  echo "PYTHON_INC = ${PYTHON_INC}"
  echo "PYTHON_LIB = ${PYTHON_LIB}"
  echo "PLUGINS.DYNAMIC += cscript/cspython"
fi

exit 0

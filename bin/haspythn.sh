#!/bin/sh
#
# This script file is used to automatically detect if the Python
# language / interpreter is installed.
#
# The configuration (a makefile fragment) is piped to stdout
# and errors are piped to stderr.
#

# Find the Python header/library directory
[ -z "${PYTHON_INC}" ] && PYTHON_INC=`ls -d /usr/include/python* 2>/dev/null`
[ -z "${PYTHON_INC}" ] && PYTHON_INC=`ls -d /usr/local/include/python* 2>/dev/null`
[ -z "${PYTHON_LIB}" ] && PYTHON_LIB=`ls -d /usr/lib/python* 2>/dev/null`
[ -z "${PYTHON_LIB}" ] && PYTHON_LIB=`ls -d /usr/local/lib/python* 2>/dev/null`

if [ -n "${PYTHON_INC}" -a -n "${PYTHON_LIB}" ]; then
# echo "Found Python headers in ${PYTHON_INC}, libs in ${PYTHON_LIB}" >&2
  echo "PYTHON_INC = ${PYTHON_INC}"
  echo "PYTHON_LIB = ${PYTHON_LIB}"
fi

exit 0

#! /bin/sh
#==============================================================================
# Copyright (C)2000-2002 by Eric Sunshine <sunshine@sunshineco.com>
#
# Automatically detect if the Python language SDK is installed.  If the user
# has not already specified the location of Python's headers and library via
# PYTHON_INC and PYTHON_LIB, then a search is performed to locate these
# directories.  Once located, a validity check is performed to ensure that the
# Python SDK is usable.
#
# IMPORTS
#    LINK
#	Shell or environment variable used to link an executable.
#    PYTHON_INC
#	Optional environment variable indicating the Python include directory.
#	If not specified, it will be determined dynamically.
#    PYTHON_LIB
#	Optional environment variable indicating the Python library name.  This
#	name, of the form "python2.2" is used to both locate the library
#	directory and to identify the actual link library.  If not specified,
#	it will be determined dynamically.
#    PTHREAD_CFLAGS
#	Shell or environment variable containing compiler flags for pthread.
#    PTHREAD_LFLAGS
#	Shell or environment variable containing linker flags for pthread.
#    READLINE_CFLAGS
#	Shell or environment variable containing compiler flags for readline.
#    READLINE_LFLAGS
#	Shell or environment variable containing linker flags for readline.
#    checktool()
#	Shell function which checks if the program mentioned as its sole
#	argument can be found in the PATH.
#    msg_*()
#	Functions for reporting progress to users.
#    shellprotect()
#	Shell function which escapes special characters in pathnames.
#
# EXPORTS
#    PYTHON_OK
#	Shell variable set to 1 if Python is available, 0 otherwise.
#    PYTHON_CFLAGS
#	Shell variable containing the compiler flags required for Python.
#    PYTHON_LFLAGS
#	Shell variable containing the linker flags required for Python.
#    PYTHON.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if Python is available, otherwise the variable is not set.
#    PYTHON.CFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	compiler flags required for Python.
#    PYTHON.LFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	linker flags required for Python.
#==============================================================================

precondition '-n "${LINK}"'

PYTHON_OK=0
PYTHON_CFLAGS=''
PYTHON_LFLAGS=''

# If Python executable is in the user's path, then try that path before any
# others.

PYTHON_BIN=`checktool python`
if [ -n "${PYTHON_BIN}" ]; then
    PYTHON_BIN=`echo ${PYTHON_BIN} | sed -e 's:/bin/python::'`
    PYTHON_DIRS=`shellprotect "${PYTHON_BIN}"`
fi

# A list of common directories in which a Python installation might be found.
# User-extension directories (such as /usr/local") are purposely searched first
# so that, if the user has installed a version of Python newer than a system-
# supplied version, we will find the user-installed one.

PYTHON_DIRS="${PYTHON_DIRS} /usr/local/python /usr/local /usr"

# Find the Python header and library directories.  In the event that multiple
# header and library directories are located, choose the the final
# lexicographically sorted entry from each list.  (Or stated in C terminology,
# do not 'break' out of the 'for' loops.)

msg_checking "for Python include"
if [ -z "${PYTHON_INC}" ]; then
  for i in ${PYTHON_DIRS}; do
    FILES=`ls -d ${i}/include/python* 2>/dev/null`
    if [ -n "${FILES}" ]; then
      for j in ${FILES}; do
        PYTHON_INC=${j}
      done
    fi
  done
fi
if [ -n "${PYTHON_INC}" ]; then
  msg_result "${PYTHON_INC}"
else
  msg_result "no"
fi

msg_checking "for Python lib"
if [ -z "${PYTHON_LIB}" ]; then
  for i in ${PYTHON_DIRS}; do
    FILES=`ls -d ${i}/lib/python* 2>/dev/null`
    if [ -n "${FILES}" ]; then
      for j in ${FILES}; do
        PYTHON_LIB=${j}
        done
    fi
  done
fi
if [ -n "${PYTHON_LIB}" ]; then
  msg_result "${PYTHON_LIB}"
else
  msg_result "no"
fi

# If Python include and library directories were located, try building a simple
# Python program to ensure that the installation is usable.  Some versions of
# Python require linking with the Python's `util' library, so this is tested.
# Various Python installations may also require the `pthread', `readline', and
# `dl' libraries, so these are tested as well.

if [ -n "${PYTHON_INC}" -a -n "${PYTHON_LIB}" ]; then
  msg_checking "if Python SDK is okay"

  PYBASE=`basename "${PYTHON_LIB}"`
  PYLIBS="${PYBASE} "`echo "${PYBASE}" | sed 's/\.//g'`
  PYCFLAGS="-I"`shellprotect "${PYTHON_INC}"`" ${PTHREAD_CFLAGS} ${READLINE_CFLAGS}"
  PYLFLAGS="-L"`shellprotect "${PYTHON_LIB}/config"`" -L"`shellprotect "${PYTHON_LIB}"`" ${PYLIB} ${PTHREAD_LFLAGS} ${READLINE_LFLAGS}"
  PYFLAGS="${PYCFLAGS} ${PYLFLAGS}"
  PYEXTRACFLAGS=''
  PYEXTRALFLAGS=''

  cat << EOF > testpy.cpp
  #include <Python.h>
  int main()
  {
    Py_Initialize();
    Py_Finalize();
    return 0;
  }
EOF

  for lib in ${PYLIBS}; do
    PYLIB="-l${lib}"

    eval "${LINK} -o testpy testpy.cpp ${PYLIB} ${PYFLAGS}" >/dev/null 2>&1
    if [ $? -eq 0 ]; then
      PYTHON_OK=1
    else
      eval "${LINK} -o testpy testpy.cpp ${PYLIB} ${PYFLAGS} -lutil" >/dev/null 2>&1
      if [ $? -eq 0 ]; then
        PYTHON_OK=1
        PYEXTRALFLAGS='-lutil'
      else
        eval "${LINK} -o testpy testpy.cpp ${PYLIB} ${PYFLAGS} -ldl" >/dev/null 2>&1
        if [ $? -eq 0 ]; then
          PYTHON_OK=1
          PYEXTRALFLAGS='-ldl'
        else
          eval "${LINK} -o testpy testpy.cpp ${PYLIB} ${PYFLAGS} -lutil -ldl" >/dev/null 2>&1
          if [ $? -eq 0 ]; then
            PYTHON_OK=1
            PYEXTRALFLAGS='-lutil -ldl'
          fi
        fi
      fi
    fi

    if [ ${PYTHON_OK} -eq 1 ]; then
      break
    fi
  done

  rm -f testpy.cpp testpy.o testpy.obj testpy.exe testpy

  if [ ${PYTHON_OK} -eq 1 ] ; then
    PYTHON_CFLAGS="${PYCFLAGS} ${PYEXTRACFLAGS}"
    PYTHON_LFLAGS="${PYLIB} ${PYLFLAGS} ${PYEXTRALFLAGS}"
    echo "PYTHON.AVAILABLE = yes"
    echo "PYTHON.CFLAGS = ${PYTHON_CFLAGS}"
    echo "PYTHON.LFLAGS = ${PYTHON_LFLAGS}"
    msg_result "yes"
    if [ -n "${PYTHON_CFLAGS}" ]; then
      msg_inform "python cflags... ${PYTHON_CFLAGS}"
    fi
    if [ -n "${PYTHON_LFLAGS}" ]; then
      msg_inform "python lflags... ${PYTHON_LFLAGS}"
    fi
  else
    msg_result "no"
  fi
fi

postcondition '${PYTHON_OK} -eq 0 -o ${PYTHON_OK} -eq 1'

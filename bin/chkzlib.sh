#! /bin/sh
#==============================================================================
# Copyright (C)2002 by Eric Sunshine <sunshine@sunshineco.com>
#
# Determine if the ZLIB library is available and, if so, which compiler and
# link flags are required.
#
# IMPORTS
#    LINK
#	Shell or environment variable used to link an executable.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    ZLIB_OK
#	Shell variable set to 1 if zlib is available, 0 otherwise.
#    ZLIB_CFLAGS
#	Shell variable containing the compiler flags required for zlib.
#    ZLIB_LFLAGS
#	Shell variable containing the linker flags required for zlib.
#    ZLIB.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if zlib is available, otherwise the variable is not set.
#    ZLIB.CFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	compiler flags required for zlib.
#    ZLIB.LFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	linker flags required for zlib.
#==============================================================================

precondition '-n "${LINK}"'

ZLIB_OK=0
ZLIB_CFLAGS=''
ZLIB_LFLAGS=''

# A list of header:library directory tuples.  An attempt is made to build a
# small test program using the specified directories.  The tuples may include
# wildcards for cases when the zlib directory is suffixed by a version
# number.
#
# Current cases:
# - Bare `:' in order to test using no special directories for the case when
#   the compiler and linker default search paths are sufficient.
# - /usr/local for compilers and linkers which do not search this by default.
# - In case the user does not have permission to install zlib as root, we also
#   search the CS/lib/zlib directory.
# - The Fink package management facility for MacOS/X installs in /sw by
#   default.
ZLIB_DIRS=': /usr/local/include:/usr/local/lib libs/zlib*:libs/zlib* \
  /sw/include:/sw/lib'

msg_checking "for zlib"

cat << EOF > testzlib.cpp
#include <zlib.h>
int main()
{
  zlibVersion();
  return 0;
}
EOF

for tuple in ${ZLIB_DIRS}; do
  # Extra echo is used to expand wildcards.
  inc_dir=`echo "${tuple}" | sed 's/:.*//'`
  inc_dir=`echo ${inc_dir}`
  lib_dir=`echo "${tuple}" | sed 's/.*://'`
  lib_dir=`echo ${lib_dir}`

  if [ -n "${inc_dir}" ]; then
    ZLIB_CFLAGS="-I${inc_dir}"
  fi
  if [ -n "${lib_dir}" ]; then
    ZLIB_LFLAGS="-L${lib_dir}"
  fi

  ZLIB_LFLAGS="${ZLIB_LFLAGS} -lz"
  ${LINK} -o testzlib testzlib.cpp ${ZLIB_CFLAGS} ${ZLIB_LFLAGS} \
    1>/dev/null 2>&1
  if [ $? -eq 0 ]; then
    ZLIB_OK=1
    break
  fi
done

rm -f testzlib.cpp testzlib.o testzlib.obj testzlib.exe testzlib

if [ ${ZLIB_OK} -eq 1 ]; then
  echo "ZLIB.AVAILABLE = yes"
  echo "ZLIB.CFLAGS = ${ZLIB_CFLAGS}"
  echo "ZLIB.LFLAGS = ${ZLIB_LFLAGS}"
  msg_result "yes"
  if [ -n "${ZLIB_CFLAGS}" ]; then
    msg_inform "zlib cflags... ${ZLIB_CFLAGS}"
  fi
  if [ -n "${ZLIB_LFLAGS}" ]; then
    msg_inform "zlib lflags... ${ZLIB_LFLAGS}"
  fi
else
  msg_result "no"
fi

postcondition '${ZLIB_OK} -eq 0 -o ${ZLIB_OK} -eq 1'

#! /bin/sh
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
# Determine if the JNG/MNG library is available and, if so, which compiler and
# link flags are required.
#
# IMPORTS
#    ZLIB_CFLAGS
#	Shell or environment variable specifying compiler flags for zlib.
#    ZLIB_LFLAGS
#	Shell or environment variable specifying linker flags for zlib.
#    JPEG_CFLAGS
#	Shell or environment variable specifying compiler flags for libjpeg.
#    JPEG_LFLAGS
#	Shell or environment variable specifying linker flags for libjpeg.
#    LINK
#	Shell or environment variable used to link an executable.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    MNG.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if MNG is available, otherwise the variable is not set.
#    MNG.CFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	compiler flags required for MNG.
#    MNG.LFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	linker flags required for MNG.
#==============================================================================

precondition '-n "${LINK}"'
precondition '${ZLIB_OK} -eq 0 -o ${ZLIB_OK} -eq 1'
precondition '${JPEG_OK} -eq 0 -o ${JPEG_OK} -eq 1'

MNG_OK=0
MNG_CFLAGS=''
MNG_LFLAGS=''

# A list of header:library directory tuples.  An attempt is made to build a
# small test program using the specified directories.  The tuples may include
# wildcards for cases when the libmng directory is suffixed by a version
# number.
#
# Current cases:
# - Bare `:' in order to test using no special directories for the case when
#   the compiler and linker default search paths are sufficient.
# - /usr/local for compilers and linkers which do not search this by default.
# - In case the user does not have permission to install libmng as root, we
#   also search the CS/lib/libmng directory.
# - The Fink package management facility for MacOS/X installs in /sw by
#   default.
MNG_DIRS=': /usr/local/include:/usr/local/lib libs/libmng*:libs/libmng* \
  /sw/include:/sw/lib'

msg_checking "for libmng"

cat << EOF > testmng.cpp
#include <libmng.h>
int main()
{
  mng_version_release();
  return 0;
}
EOF

for tuple in ${MNG_DIRS}; do
  # Extra echo is used to expand wildcards.
  inc_dir=`echo "${tuple}" | sed 's/:.*//'`
  inc_dir=`echo ${inc_dir}`
  lib_dir=`echo "${tuple}" | sed 's/.*://'`
  lib_dir=`echo ${lib_dir}`

  if [ -n "${inc_dir}" ]; then
    MNG_CFLAGS="-I${inc_dir}"
  fi
  if [ -n "${lib_dir}" ]; then
    MNG_LFLAGS="-L${lib_dir}"
  fi

  MNG_LFLAGS="${MNG_LFLAGS} -lmng"

  if [ -n "${ZLIB_CFLAGS}" ]; then
    MNG_CFLAGS="${MNG_CFLAGS} ${ZLIB_CFLAGS}"
  fi
  if [ -n "${ZLIB_LFLAGS}" ]; then
    MNG_LFLAGS="${MNG_LFLAGS} ${ZLIB_LFLAGS}"
  fi

  if [ -n "${JPEG_CFLAGS}" ]; then
    MNG_CFLAGS="${MNG_CFLAGS} ${JPEG_CFLAGS}"
  fi
  if [ -n "${JPEG_LFLAGS}" ]; then
    MNG_LFLAGS="${MNG_LFLAGS} ${JPEG_LFLAGS}"
  fi

  ${LINK} -o testmng testmng.cpp ${MNG_CFLAGS} ${MNG_LFLAGS} 1>/dev/null 2>&1
  if [ $? -eq 0 ]; then
    MNG_OK=1
    break
  fi
done

rm -f testmng.cpp testmng.o testmng.obj testmng.exe testmng

if [ ${MNG_OK} -eq 1 ]; then
  echo "MNG.AVAILABLE = yes"
  echo "MNG.CFLAGS = ${MNG_CFLAGS}"
  echo "MNG.LFLAGS = ${MNG_LFLAGS}"
  msg_result "yes"
  if [ -n "${MNG_CFLAGS}" ]; then
    msg_inform "libmng cflags... ${MNG_CFLAGS}"
  fi
  if [ -n "${MNG_LFLAGS}" ]; then
    msg_inform "libmng lflags... ${MNG_LFLAGS}"
  fi
else
  msg_result "no"
fi

postcondition '${MNG_OK} -eq 0 -o ${MNG_OK} -eq 1'

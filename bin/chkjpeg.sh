#! /bin/sh
#==============================================================================
# Copyright (C)2002 by Eric Sunshine <sunshine@sunshineco.com>
#
# Determine if the JPEG library is available and, if so, which compiler and
# link flags are required.
#
# IMPORTS
#    LINK
#	Shell or environment variable used to link an executable.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    JPEG.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if JPEG is available, otherwise the variable is not set.
#    JPEG.CFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	compiler flags required for JPEG.
#    JPEG.LFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	linker flags required for JPEG.
#==============================================================================

precondition '-n "${LINK}"'

JPEG_OK=0
JPEG_CFLAGS=''
JPEG_LFLAGS=''

# A list of header:library directory tuples.  An attempt is made to build a
# small test program using the specified directories.  The tuples may include
# wildcards for cases when the libjpeg directory is suffixed by a version
# number.
#
# Current cases:
# - Bare `:' in order to test using no special directories for the case when
#   the compiler and linker default search paths are sufficient.
# - /usr/local for compilers and linkers which do not search this by default.
# - In case the user does not have permission to install libjpeg as root, we
#   also search the CS/lib/libjpeg and CS/lib/jpeg directories.
# - The Fink package management facility for MacOS/X installs in /sw by
#   default.
JPEG_DIRS=': /usr/local/include:/usr/local/lib libs/*jpeg*:libs/*jpeg* \
  /sw/include:/sw/lib'

msg_checking "for libjpeg"

cat << EOF > testjpeg.cpp
extern "C" {
#include <stddef.h>
#include <stdio.h>
#include <jpeglib.h>
}
int main()
{
  jpeg_create_compress(0);
  return 0;
}
EOF

for tuple in ${JPEG_DIRS}; do
  # Extra echo is used to expand wildcards.
  inc_dir=`echo "${tuple}" | sed 's/:.*//'`
  inc_dir=`echo ${inc_dir}`
  lib_dir=`echo "${tuple}" | sed 's/.*://'`
  lib_dir=`echo ${lib_dir}`

  if [ -n "${inc_dir}" ]; then
    JPEG_CFLAGS="-I${inc_dir}"
  fi
  if [ -n "${lib_dir}" ]; then
    JPEG_LFLAGS="-L${lib_dir}"
  fi

  JPEG_LFLAGS="${JPEG_LFLAGS} -ljpeg"
  ${LINK} -o testjpeg testjpeg.cpp ${JPEG_CFLAGS} ${JPEG_LFLAGS} \
    1>/dev/null 2>&1
  if [ $? -eq 0 ]; then
    JPEG_OK=1
    break
  fi
done

rm -f testjpeg.cpp testjpeg.o testjpeg.obj testjpeg.exe testjpeg

if [ ${JPEG_OK} -eq 1 ]; then
  echo "JPEG.AVAILABLE = yes"
  echo "JPEG.CFLAGS = ${JPEG_CFLAGS}"
  echo "JPEG.LFLAGS = ${JPEG_LFLAGS}"
  msg_result "yes"
  if [ -n "${JPEG_CFLAGS}" ]; then
    msg_inform "libjpeg cflags... ${JPEG_CFLAGS}"
  fi
  if [ -n "${JPEG_LFLAGS}" ]; then
    msg_inform "libjpeg lflags... ${JPEG_LFLAGS}"
  fi
else
  msg_result "no"
fi

postcondition '${JPEG_OK} -eq 0 -o ${JPEG_OK} -eq 1'

#! /bin/sh
#==============================================================================
# Copyright (C)2002 by Eric Sunshine <sunshine@sunshineco.com>
#
# Determine if the PNG library is available and, if so, which compiler and link
# flags are required.
#
# IMPORTS
#    ZLIB_CFLAGS
#	Shell or environment variable specifying compiler flags for zlib.
#    ZLIB_LFLAGS
#	Shell or environment variable specifying linker flags for zlib.
#    LINK
#	Shell or environment variable used to link an executable.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    PNG.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if PNG is available, otherwise the variable is not set.
#    PNG.CFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	compiler flags required for PNG.
#    PNG.LFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	linker flags required for PNG.
#==============================================================================

precondition '-n "${LINK}"'
precondition '${ZLIB_OK} -eq 0 -o ${ZLIB_OK} -eq 1'

PNG_OK=0
PNG_CFLAGS=''
PNG_LFLAGS=''

# A list of header:library directory tuples.  An attempt is made to build a
# small test program using the specified directories.  The tuples may include
# wildcards for cases when the libpng directory is suffixed by a version
# number.
#
# Current cases:
# - Bare `:' in order to test using no special directories for the case when
#   the compiler and linker default search paths are sufficient.
# - /usr/local for compilers and linkers which do not search this by default.
# - In case the user does not have permission to install libpng as root, we
#   also search the CS/lib/libpng directory.
# - The Fink package management facility for MacOS/X installs in /sw by
#   default.
PNG_DIRS=': /usr/local/include:/usr/local/lib libs/libpng*:libs/libpng* \
  /sw/include:/sw/lib'

msg_checking "for libpng"

cat << EOF > testpng.cpp
#include <png.h>
int main()
{
  png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  return 0;
}
EOF

for tuple in ${PNG_DIRS}; do
  # Extra echo is used to expand wildcards.
  inc_dir=`echo "${tuple}" | sed 's/:.*//'`
  inc_dir=`echo ${inc_dir}`
  lib_dir=`echo "${tuple}" | sed 's/.*://'`
  lib_dir=`echo ${lib_dir}`

  if [ -n "${inc_dir}" ]; then
    PNG_CFLAGS="-I${inc_dir}"
  fi
  if [ -n "${lib_dir}" ]; then
    PNG_LFLAGS="-L${lib_dir}"
  fi

  PNG_LFLAGS="${PNG_LFLAGS} -lpng"

  if [ -n "${ZLIB_CFLAGS}" ]; then
    PNG_CFLAGS="${PNG_CFLAGS} ${ZLIB_CFLAGS}"
  fi
  if [ -n "${ZLIB_LFLAGS}" ]; then
    PNG_LFLAGS="${PNG_LFLAGS} ${ZLIB_LFLAGS}"
  fi

  ${LINK} -o testpng testpng.cpp ${PNG_CFLAGS} ${PNG_LFLAGS} 1>/dev/null 2>&1
  if [ $? -eq 0 ]; then
    PNG_OK=1
    break
  fi
done

rm -f testpng.cpp testpng.o testpng.obj testpng.exe testpng

if [ ${PNG_OK} -eq 1 ]; then
  echo "PNG.AVAILABLE = yes"
  echo "PNG.CFLAGS = ${PNG_CFLAGS}"
  echo "PNG.LFLAGS = ${PNG_LFLAGS}"
  msg_result "yes"
  if [ -n "${PNG_CFLAGS}" ]; then
    msg_inform "libpng cflags... ${PNG_CFLAGS}"
  fi
  if [ -n "${PNG_LFLAGS}" ]; then
    msg_inform "libpng lflags... ${PNG_LFLAGS}"
  fi
else
  msg_result "no"
fi

postcondition '${PNG_OK} -eq 0 -o ${PNG_OK} -eq 1'

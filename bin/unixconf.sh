#! /bin/sh
#
# This script file is used to autodetect some parameters
# needed for building Crystal Space on various Unix flavours.
#
# Arguments: $1 is operating system subtype (linux, solaris, freebsd etc)
#            $2 is user-preferred install dir, argument can be left out
#
# The configuration (a makefile fragment) is piped to stdout
# and errors are piped to stderr.
#

SYSTYPE=$1
INSTALL_DIR=$2

# Find the directory where script is located
SCRIPT_NAME=`basename $0`
SCRIPT_DIR=`expr $0 : "\(.*\)/${SCRIPT_NAME}"`
[ -z "${SCRIPT_DIR}" ] && SCRIPT_DIR="./"
SCRIPT_DIR=`(cd ${SCRIPT_DIR} > /dev/null; pwd)` # Convert to absolute path.

# First get a string describing current machine and processor types
# Initially set to reasonable defaults
MACHINE=`uname -m 2>/dev/null`
CPU=`uname -p 2>/dev/null`
# Now find more specific
case ${SYSTYPE} in
  linux*)
    CPU=`cat /proc/cpuinfo | sed -ne "/^cpu	/p" | sed -e "s/.*://"`
    ;;
esac

# If CPU is empty or unknown set it at least to MACHINE
[ -z "${CPU}" ] && CPU=$MACHINE
[ "${CPU}" = "unknown" ] && CPU=$MACHINE

# If MACHINE is contains 'sun' then set it to CPU so that
# the processor is correctly catched
# IP30 came up on an IRIX machine (Dmitry)
case $MACHINE in
  *sun*)	MACHINE=$CPU  ;;
  *IP[0-9][0-9]*)	MACHINE=$CPU  ;;
esac

# Now check processor type: add more checks here as needed
case $MACHINE in
  *ppc*)	echo "PROC = POWERPC" ;;
  *i[3-9]86*)	echo "PROC = INTEL" ;;
  *ia64*)	echo "PROC = UNKNOWN" ;;
  *sparc*)	echo "PROC = SPARC" ;;
  *mips*)	echo "PROC = MIPS" ;;
  *alpha*)	echo "PROC = ALPHA" ;;
  *)		echo "UNKNOWN MACHINE TYPE: Please fix $0!" >&2
		exit 1
esac

# Find the C++ compiler
[ -z "${CXX}" ] && CXX=`which gcc 2>&1 | grep -v "[Nn]o"`
[ -z "${CXX}" ] && CXX=`which egcs 2>&1 | grep -v "[Nn]o"`
[ -z "${CXX}" ] && CXX=`which g++ 2>&1 | grep -v "[Nn]o"`
[ -z "${CXX}" ] && CXX=`which c++ 2>&1 | grep -v "[Nn]o"`

if [ -z "${CXX}" ]; then
  echo "$0: Cannot find an installed C++ compiler!" >&2
  exit 1
fi

CXX=`basename ${CXX}`
echo "CC = ${CXX} -c"
echo "CXX = ${CXX} -c"
echo "LINK = ${CXX}"

# Create a dummy C++ program
echo "int main () {}" >conftest.cpp

# Check for machine-specific C compiler flags
(echo "$CPU" | grep -s 686 >/dev/null && ${CXX} -c -mpentiumpro -march=i686 conftest.cpp && echo "CFLAGS.SYSTEM += -mpentiumpro -march=i686") || \
(echo "$CPU" | grep -s [5-6]86 >/dev/null && ${CXX} -c -mpentium -march=i586 conftest.cpp && echo "CFLAGS.SYSTEM += -mpentium -march=i586") || \
(echo "$CPU" | grep -s [3-9]86 >/dev/null && ${CXX} -c -m486 conftest.cpp && echo "CFLAGS.SYSTEM += -m486") || \
(echo "$MACHINE" | grep -s alpha >/dev/null && ${CXX} -c -mieee conftest.cpp && echo "CFLAGS.SYSTEM += -mieee")

# Check for GCC-version-specific command-line options
${CXX} -c -fno-exceptions conftest.cpp 2>/dev/null && echo "CFLAGS.SYSTEM += -fno-exceptions"
${CXX} -c -fno-rtti conftest.cpp 2>/dev/null && echo "CFLAGS.SYSTEM += -fno-rtti"

# Remove dummy remains
rm -f conftest.cpp conftest.o

# Check for socklen_t
echo "#include <unistd.h>" > socktest.cpp
echo "#include <sys/types.h>" >> socktest.cpp
echo "#include <sys/socket.h>" >> socktest.cpp
echo "#define BSD_COMP 1" >> socktest.cpp
echo "#include <sys/ioctl.h>" >> socktest.cpp
echo "#include <netinet/in.h>" >> socktest.cpp
echo "#include <netdb.h>" >> socktest.cpp
echo "int main() { socklen_t x = 0; return (int)x; }" >> socktest.cpp

${CXX} -c socktest.cpp 2>/dev/null || echo "CS_USE_FAKE_SOCKLEN_TYPE = yes"

rm -f socktest.cpp socktest.o

# Test for presence of SVGALIB.
echo "#include <vga.h>" > conftest.cpp
${CXX} -c conftest.cpp 2>/dev/null && echo "PLUGINS += video/canvas/svgalib"
rm -f conftest.cpp conftest.o

# Create a dummy NASM program
echo "%xdefine TEST" >conftest.asm

# Check if NASM is installed and if it has the right version
NASMBIN=`which nasm 2>&1 | grep -v "[Nn]o"`

if [ -n "${NASMBIN}" ]; then
  # Well, we really should check here for obj format...
  # but we'll use ELF as it really doesn't matter
  ${NASMBIN} -f elf conftest.asm -o conftest.o 2>/dev/null && echo "NASM.INSTALLED = yes"
fi

# Remove dummy remains
rm -f conftest.asm conftest.o

# Check if makedep is installed and is the right version
[ -z "${MAKEDEP}" ] && MAKEDEP=`which makedep 2>&1 | grep -v "[Nn]o"`
if [ -n "${MAKEDEP}" ]; then
  echo "DEPEND_TOOL = mkdep"
  MAKEDEP_VERSION=`makedep -V | sed -e "s/.*Version *//"`
  if [ `expr "${MAKEDEP_VERSION}" ">" 0.0.0` = "1" ]; then
    echo "MAKEDEP.INSTALLED = yes"
  fi
fi

# Find the X11 directory
if [ -d /usr/X11 ]; then
  X11_PATH="/usr/X11"
elif [ -d /usr/X11R6 ]; then
  X11_PATH="/usr/X11R6"
elif [ -d /usr/openwin ]; then
  X11_PATH="/usr/openwin"
elif [ -d /usr/lib/X11 ]; then
  X11_PATH="/usr/lib/X11"
else
  echo "$0: Cannot find X11 directory!" >&2
  exit 1
fi
echo "X11_PATH = "${X11_PATH}

# find out if Xfree86 is installed, enable VideoMode extension if so.
# detected by existence of the XFree86 server binary.
[ -f ${X11_PATH}/bin/XFree86 -o -f /etc/X11/XF86Config ] &&
echo "USE_XFREE86VM = yes"

# find a writable install dir, unless user has done an override
([ ${INSTALL_DIR} ] && echo "INSTALL_DIR = ${INSTALL_DIR}") ||
([ -w /usr/local ] && echo "INSTALL_DIR = /usr/local/crystal") ||
echo "INSTALL_DIR = "${HOME}"/crystal"

sh ${SCRIPT_DIR}/comptest.sh ${CXX}
sh ${SCRIPT_DIR}/endtest.sh ${CXX}
sh ${SCRIPT_DIR}/haspythn.sh ${CXX}

exit 0

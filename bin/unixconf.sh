#! /bin/sh
#
# This script file is used to autodetect some parameters
# needed for building Crystal Space on various Unix flavours.
#
# Arguments: $1 is operating system subtype (linux, solaris, freebsd etc)
#
# The configuration (a makefile fragment) is piped to stdout
# and errors are piped to stderr.
#

# First get a string describing current machine and processor types
# Initially set to reasonable defaults
MACHINE=`uname -m 2>/dev/null`
CPU=`uname -p 2>/dev/null`
# Now find more specific
case $1 in
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
  *IP30*)	MACHINE=$CPU  ;;
esac

# Now check processor type: add more checks here as needed
case $MACHINE in
  *ppc*)	echo "PROC = POWERPC" ;;
  *i[3-9]86*)	echo "PROC = INTEL" ;;
  *sparc*)	echo "PROC = SPARC" ;;
  *mips*)	echo "PROC = MIPS" ;;
  *)		echo "UNKNOWN MACHINE TYPE: Please fix $0!" >&2
		exit 1
esac

# Find the C++ compiler
CXX=`which g++`
[ -z "${CXX}" ] && CXX=`which egcs`
[ -z "${CXX}" ] && CXX=`which gcc`
[ -z "${CXX}" ] && CXX=`which c++`

if [ -z "${CXX}" ]; then
  echo "$0: Cannot find an installed C++ compiler!" >&2
  exit 1
fi

echo "CXX = "`basename ${CXX}`

# Create a dummy C++ program
echo "int main () {}" >conftest.cpp

# Check for machine-specific C compiler flags
(echo "$CPU" | grep -q 686 && ${CXX} -c -mpentiumpro -march=i686 conftest.cpp && echo "CFLAGS.SYSTEM += -mpentiumpro -march=i686") || \
(echo "$CPU" | grep -q [5-6]86 && ${CXX} -c -mpentium -march=i586 conftest.cpp && echo "CFLAGS.SYSTEM += -mpentium -march=i586") || \
(echo "$CPU" | grep -q [3-9]86 && ${CXX} -c -m486 conftest.cpp && echo "CFLAGS.SYSTEM += -m486 ")

# Check for GCC-version-specific command-line options
${CXX} -c -fno-exceptions conftest.cpp 2>/dev/null && echo "CFLAGS.SYSTEM += -fno-exceptions"
${CXX} -c -fno-rtti conftest.cpp 2>/dev/null && echo "CFLAGS.SYSTEM += -fno-rtti"

# Remove dummy remains
rm -f conftest.cpp conftest.o

# Create a dummy NASM program
echo "%xdefine TEST" >conftest.asm

# Check if NASM is installed and if it has the right version
[ -z "${NASM}" ] && NASM=`which nasm`

if [ -n "${NASM}" ]; then
  echo "NASM = "`basename ${NASM}`
  # Well, we really should check here for obj format...
  # but we'll use ELF as it really doesn't matter
  ${NASM} -f elf conftest.asm -o conftest.o 2>/dev/null && echo "USE_NASM = yes"
fi

# Remove dummy remains
rm -f conftest.asm conftest.o

# Look where is X11 directory
# lib/ or include/ ? (Dmitry)
([ -d /usr/X11 ] && echo "X11_PATH = /usr/X11") || \
([ -d /usr/X11R6 ] && echo "X11_PATH = /usr/X11R6") || \
([ -d /usr/openwin ] && echo "X11_PATH = /usr/openwin") || \
([ -d /usr/lib/X11 ] && echo "X11_PATH = /usr/lib/X11") || \
(echo "$0: Cannot find X11 directory!" >&2 && exit 1)

exit $?

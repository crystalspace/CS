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

SYSTYPE=$1

# Find the directory where script is located
SCRIPT_NAME=`basename $0`
SCRIPT_DIR=`expr $0 : "\(.*\)/${SCRIPT_NAME}"`
[ -z "${SCRIPT_DIR}" ] && SCRIPT_DIR="./"
SCRIPT_DIR=`(cd ${SCRIPT_DIR}; pwd)`	# Convert to absolute path.

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
  *sparc*)	echo "PROC = SPARC" ;;
  *mips*)	echo "PROC = MIPS" ;;
  *alpha*)	echo "PROC = ALPHA" ;;
  *)		echo "UNKNOWN MACHINE TYPE: Please fix $0!" >&2
		exit 1
esac

# Find the C++ compiler
CXX=`which g++ | grep -v "no g++"`
[ -z "${CXX}" ] && CXX=`which egcs | grep -v "no egcs"`
[ -z "${CXX}" ] && CXX=`which gcc | grep -v "no gcc"`
[ -z "${CXX}" ] && CXX=`which c++ | grep -v "no c++"`

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
(echo "$CPU" | grep -q [3-9]86 && ${CXX} -c -m486 conftest.cpp && echo "CFLAGS.SYSTEM += -m486") || \
(echo "$MACHINE" | grep -q alpha && ${CXX} -c -mieee conftest.cpp && echo "CFLAGS.SYSTEM += -mieee")
 
# Check for GCC-version-specific command-line options
${CXX} -c -fno-exceptions conftest.cpp 2>/dev/null && echo "CFLAGS.SYSTEM += -fno-exceptions"
${CXX} -c -fno-rtti conftest.cpp 2>/dev/null && echo "CFLAGS.SYSTEM += -fno-rtti"

# Remove dummy remains
rm -f conftest.cpp conftest.o

# Create a dummy NASM program
echo "%xdefine TEST" >conftest.asm

# Check if NASM is installed and if it has the right version
[ -z "${NASM}" ] && NASM=`which nasm | grep -v "no nasm"`

if [ -n "${NASM}" ]; then
  echo "NASM = "`basename ${NASM}`
  # Well, we really should check here for obj format...
  # but we'll use ELF as it really doesn't matter
  ${NASM} -f elf conftest.asm -o conftest.o 2>/dev/null && echo "USE_NASM = yes"
fi

# Remove dummy remains
rm -f conftest.asm conftest.o

# Check if makedep is installed and is the right version
[ -z "${MAKEDEP}" ] && MAKEDEP=`which makedep | grep -v "no makedep"`
if [ -n "${MAKEDEP}" ]; then
  echo "DEPEND_TOOL = mkdep"
  MAKEDEP_VERSION=`makedep -V | sed -e "s/.*Version *//"`
  if [ "${MAKEDEP_VERSION}" ">" "0.0.0" ]; then
    echo "DEPEND_TOOL.INSTALLED = yes"
  fi
fi

# Look where is X11 directory
([ -d /usr/X11 ] && echo "X11_PATH = /usr/X11") || \
([ -d /usr/X11R6 ] && echo "X11_PATH = /usr/X11R6") || \
([ -d /usr/openwin ] && echo "X11_PATH = /usr/openwin") || \
([ -d /usr/lib/X11 ] && echo "X11_PATH = /usr/lib/X11") || \
(echo "$0: Cannot find X11 directory!" >&2 && exit 1)

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

exec ${SCRIPT_DIR}/booltest.sh ${CXX}

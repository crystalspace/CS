#! /bin/sh
#==============================================================================
# Determine if various tools are available.
#
# IMPORTS
#    checktool()
#	Shell function which checks if the program mentioned as its sole
#	argument can be found in the PATH.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    NASM.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if nasm is available, otherwise the variable is not set.
#    DEPEND_TOOL
#	Makefile varaible emitted to the standard output stream.  Value is
#	"mkdep" if a special dependency generation tool was located.
#    MAKEDEP.AVAILABLE
#	Makefile variable emitted to the standard ouput stream.  Value is "yes"
#	if the customized Crystal Space "makedep" program is available.
#==============================================================================

# Check if NASM is installed and has the correct version.
[ -z "${NASMBIN}" ] && NASMBIN=`checktool nasm`
if [ -n "${NASMBIN}" ]; then
  msg_checking "for nasm extensions"
  echo "%xdefine TEST" >conftest.asm
  # Well, we really should check here for obj format,
  # but we'll use ELF as it really doesn't matter.
  ${NASMBIN} -f elf conftest.asm -o conftest.o 2>/dev/null
  if [ $? -eq 0 ]; then
    echo "NASM.AVAILABLE = yes"
    msg_result "yes"
  else
    msg_result "no"
  fi
  rm -f conftest.asm conftest.o
fi

# Check if makedep is installed and has the correct version.
[ -z "${MAKEDEP}" ] && MAKEDEP=`checktool makedep`
if [ -n "${MAKEDEP}" ]; then
  msg_checking "for makedep extensions"
  echo "DEPEND_TOOL = mkdep"
  MAKEDEP_VERSION=`makedep -V | sed -e "s/.*Version *//"`
  if [ `expr "${MAKEDEP_VERSION}" ">" 0.0.0` = "1" ]; then
    echo "MAKEDEP.AVAILABLE = yes"
    msg_result "yes"
  else
    msg_result "no"
  fi
fi

# Check for Bison and Flex.
BISONBIN=`checktool bison`
if [ -n "${BISONBIN}" ]; then
    echo "BISONBIN = ${BISONBIN}"
fi

FLEXBIN=`checktool flex`
if [ -n "${FLEXBIN}" ]; then
    echo "FLEXBIN = ${FLEXBIN}"
fi

# Check for Swig and Swig extensions.
SWIGBIN=`checktool swig`
if [ -n "${SWIGBIN}" ]; then
    echo "SWIGBIN = ${SWIGBIN}"
fi

LUASWIGBIN=`checktool luaswig`
if [ -n "${SWIGBIN}" ]; then
    echo "LUASWIGBIN = ${LUASWIGBIN}"
fi

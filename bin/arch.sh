#! /bin/sh
#==============================================================================
# Auto-detect the architecture (CPU/machine) of the host platform.
#
# IMPORTS
#    SYSTYPE
#	Optional operating system type (for example, "linux", "solaris",
#	"freebsd", etc.).  While the system type is not strictly necessary, if
#	present, it does allow this script to glean more detailed information
#	about the host than otherwise obtainable from `uname'.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    CPU
#	Shell variable giving the host processor type.
#    MACHINE
#	Shell variable giving the host machine (hardware) type.
#    PROC
#	Shell and makefile variable emitted to the standard output stream.
#	Value is a well-known processor type, such as "X86", "POWERPC",
#	"SPARC", "M68K", etc.
#==============================================================================

msg_checking "architecture"

# First get a string describing current machine and processor types.
# Initially set to reasonable defaults.
if [ -z "$MACHINE" ]; then
  MACHINE=`uname -m 2>/dev/null`
fi
if [ -z "$CPU" ]; then
  CPU=`uname -p 2>/dev/null`
  # Now find more specific
  case ${SYSTYPE} in
    linux*)
      if [ -r /proc/cpuinfo ]; then
        CPU=`cat /proc/cpuinfo | sed -ne "/^cpu     /p" | sed -e "s/.*://"`
      fi
      ;;
    hurd*)
      CPU=`dpkg --print-architecture 2>/dev/null`
      ;;
    netbsd*)
      [ $CPU = "i386" ] && CPU=`dmesg|grep cpu0|sed "s/.*\(.86\)-class.*/\1/"`
      ;;
  esac
fi

# If CPU is empty or unknown set it at least to MACHINE
[ -z "${CPU}" -o "${CPU}" = "unknown" ] && CPU=$MACHINE

# Special cases:
# - If MACHINE is contains 'sun' then set it to CPU so that the processor is
#   correctly identified.
# - IP30 came up on an IRIX machine (Dmitry).
# - Power Macintosh
case $MACHINE in
  *sun*)          MACHINE=$CPU ;;
  *IP[0-9][0-9]*) MACHINE=$CPU ;;
  *[Mm]acintosh*) MACHINE=$CPU ;;
esac

# Emit the PROC makefile variable.
if [ -z "${PROC}" ]; then
  # Add more checks here as needed.
  case $MACHINE in
    *[Pp][Pp][Cc]*)                 PROC="POWERPC" ;;
    *[Pp][Oo][Ww][Ee][Rr][Pp][Cc]*) PROC="POWERPC" ;;
    *[Ii][3-9]86*)                  PROC="X86"     ;;
    *[Ii]86pc*)                     PROC="X86"     ;;
    *ia64*)                         PROC="UNKNOWN" ;;
    *sparc*)                        PROC="SPARC"   ;;
    *mips*)                         PROC="MIPS"    ;;
    *alpha*)                        PROC="ALPHA"   ;;
    *arm*)                          PROC="ARM"     ;;
    *s390*)                         PROC="S390"    ;;
    *parisc*)                       PROC="HPPA"    ;;
    *[Mm]68[Kk]*)                   PROC="M68K"    ;;
  esac
fi

if [ -n "${PROC}" ]; then
  echo "PROC = ${PROC}"
  msg_result "${PROC}"
else
  msg_result "unknown"
  msg_inform "Unable to determine hardware architecture.   Terminating!"
  exit 1
fi

postcondition '-n "${CPU}"'
postcondition '-n "${MACHINE}"'
postcondition '-n "${PROC}"'

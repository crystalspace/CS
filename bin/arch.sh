#!/bin/sh
# Try to find out on which architecture (cpu/machine) we're running

# First get a string describing current machine and processor types
# Initially set to reasonable defaults
MACHINE=`uname -m 2>/dev/null`
CPU=`uname -p 2>/dev/null`
# Now find more specific
case ${SYSTYPE} in
  linux*)
    CPU=`cat /proc/cpuinfo | sed -ne "/^cpu     /p" | sed -e "s/.*://"`
    ;;
  hurd*)
    CPU=`dpkg --print-architecture 2>/dev/null`
    ;;
  netbsd*)
    [ $CPU = "i386" ] && CPU=`dmesg|grep cpu0|sed "s/.*\(.86\)-class.*/\1/"`
    ;;
esac

# If CPU is empty or unknown set it at least to MACHINE
[ -z "${CPU}" ] && CPU=$MACHINE
[ "${CPU}" = "unknown" ] && CPU=$MACHINE

# If MACHINE is contains 'sun' then set it to CPU so that
# the processor is correctly catched
# IP30 came up on an IRIX machine (Dmitry)
case $MACHINE in
  *sun*)          MACHINE=$CPU ;;
  *IP[0-9][0-9]*) MACHINE=$CPU ;;
esac

# Now check processor type: add more checks here as needed
case $MACHINE in
  *ppc*)        PROC="POWERPC" ;;
  *i[3-9]86*)   PROC="X86"     ;;
  *i86pc*)      PROC="X86"     ;;
  *ia64*)       PROC="UNKNOWN" ;;
  *sparc*)      PROC="SPARC"   ;;
  *mips*)       PROC="MIPS"    ;;
  *alpha*)      PROC="ALPHA"   ;;
  *arm*)        PROC="ARM"     ;;
  *)            echo "UNKNOWN MACHINE TYPE: Please fix $0!" >&2
                exit 1         ;;
esac
echo "PROC = ${PROC}"


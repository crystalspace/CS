#!/bin/sh
# Checks for an installed nasm

# Create a dummy NASM program
echo "%define TEST" >conftest.asm

# Check if NASM is installed and if it has the right version
NASMBIN=`which nasm 2>&1 | grep -v "[Nn]o"`

if [ -n "${NASMBIN}" ]; then
  # Well, we really should check here for obj format...
  # but we'll use ELF as it really doesn't matter
  ${NASMBIN} -f elf conftest.asm -o conftest.o 2>/dev/null && echo "NASM.INSTALLED = yes"
  rm -f conftest.asm conftest.o
fi

# Check if makedep is installed and is the right version
[ -z "${MAKEDEP}" ] && MAKEDEP=`which makedep 2>&1 | grep -v "[Nn]o"`
if [ -n "${MAKEDEP}" ]; then
  echo "DEPEND_TOOL = mkdep"
  MAKEDEP_VERSION=`makedep -V | sed -e "s/.*Version *//"`
  if [ `expr "${MAKEDEP_VERSION}" ">" 0.0.0` = "1" ]; then
    echo "MAKEDEP.INSTALLED = yes"
  fi
fi


#!/bin/awk -f
#
# This script is intended to extract all useful information from output of
#
# objdump --stabs
#
# command. This is known to work only with GNU binutils
# and ELF executable format.

# Function that converts from hexadecimal to decimal
function hexval (str)
{
  gsub (/^[ 	]*/, "", str)
  str = tolower (str)
  len = length (str);
  val = 0
  for (idx = 1; idx <= len; idx++)
  {
    dig = index ("0123456789abcdef", substr (str, idx, 1)) - 1
    if (dig < 0)
      break
    val = (val * 16) + dig
  }
  return val
}

# The startup function that performs initialization
BEGIN {
  FS = " ";
  modname = ""; modaddr = -1;
  funname = ""; funaddr = -1;
  labname = ""; labaddr = -1;
  baseaddr = 0;
}

# Lines with Type == "SO" contain information about source files
$2 == "SO" {
  ma = hexval($5)
  mn = $7
  if (ma == modaddr)
    modname = modname mn
  else
    modname = mn
  modaddr = ma
}

# The lines with Type == "FUN" contain information about functions
$2 == "FUN" {
  if (modaddr != -1)
  {
    printf "S %x %s\n", modaddr, modname
    modaddr = -1
  }

  funaddr = hexval($5)
  if (labaddr == funaddr)
    funname = labname
  else
  {
    funname = $7
    if (colidx = index (funname, ":"))
      funname = substr (funname, 1, colidx - 1)
  }
}

# The lines with Type == "SLINE" contain information
# about source code line addresses
$2 == "SLINE" {
  if (funaddr != -1)
  {
    printf "F %x %s\n", funaddr, funname
    baseaddr = funaddr
    funaddr = -1
  }

  addr = hexval($5) + baseaddr
  line = $4
  printf "L %x %d\n", addr, line
}

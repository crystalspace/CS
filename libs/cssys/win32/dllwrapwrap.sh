#!/bin/sh

# dllwrapwrap.
# wrapper around dllwrap to create export definition files and work around
# the inability of dllwrap to export functions with _declspec(dllexport) from
# libraries...

# Syntax:
#   dllwrapwrap [plugin-name] [other parameters ...]

dllname=$1
shift
params=$*

deffile=out/win32/x86/${dllname}.def
echo EXPORTS > ${deffile}
echo  ${dllname}_scfInitialize >> ${deffile}
echo  ${dllname}_scfFinalize >> ${deffile}
dllwrap --def ${deffile} ${params}
rm -f ${deffile}


#!/bin/bash
#
# Usage: memdb.sh [debug-executable] {memdbg-options}
#
# The {memdbg-options} parameter is a string consisting of optional letters.
# Each letter enables a specific debugger mode. The following options are
# implemented:
#
# (a)
#     Fill memory after allocation with garbage?
#     This is highly recommended as it is not too expensive but allows to
#     detect cases when memory is used without being initialized first.
# (f)
#     Do we want to fill freed memory with garbage and leave it non-freed?
#     WARNING: This is quite expensive in memory terms!
# (d)
#     Do we want debugger breakpoints when detecting serious erros?
#     The debugger will break inside operator new or delete right before
#     exiting back to user program. By stepping a few you can detect
#     the place where error occured.
# (v)
#     Do we want memory debugger to print an information string each
#     time new or delete is called? It is recommended you to redirect
#     stdout when using this flag as output to console will immensely
#     slow down your program.
# (s)
#     Do we want a summary sheet at the end of program?
#     The sheet will list the summary number of memory allocations,
#     memory frees, the peak memory consumption and lots of other
#     useful information.
# (l)
#     Do we want a list of unfreed memory blocks at the end of program?
#     The list will also contain the location where the corresponding
#     memory block was allocated.
# (b)
#     Detect writes past the block bounds? This is implemented by allocating
#     slightly bigger blocks than actually requested, and by filling those
#     inter-block spaces with some well-known value. When block is freed, the
#     space between blocks is checked to contains same well-known value.
# (L) Redirect (append) output from console to a log file called memdbg.log.
#
# Default options:	aslbL
# Extensive checking:	aslbLf
# Full throttle:	aslbLfdv
#

NM=/emx/bin/nm

# Find a tool along the PATH and type an error message if it is not found
find ( )
{
	[ -z "`eval echo \\$$1`" ] && eval $1=`$which $2`

	if [ -z "`eval echo \\$$1`" ]; then
		echo "ERROR: The $2 tool is required to run this script,"
		echo "       but cannot be found along your PATH. Either"
		echo "       set the $1 variable to point to it, or add"
		echo "       its path to the PATH variable."
		exit
	fi
}

# Find a tool and check if it is a GNU tool
findgnu ( )
{
	find $*
	if [ -z "`eval \\$$1 --version | grep GNU`" ]; then
		echo "WARNING: GNU $2 required for this script to work!"
		echo "         The script will attempt to run existing $1,"
		echo "         but good results are not guaranteed."
	fi
}

if [ -z "$*" ]; then
	echo "ERROR: No executable filename given"
	exit
fi

mapfile="memdbg.map"
executable=$1
shift
options=$*
[ -z "$options" ] && options="aslbL"

# Some horrible which's will output lots of garbage
# instead of silently returning an empty string
which="type -path"

#
# First some system-specific issues.
#
# On ELF systems (Linux, Solaris) debugging symbols are put in a separate
# .stabs section of executable. Unfortunately, nm does not display the contents
# of .stabs section, thus we're forced to use GNU objdump on such systems.
#

findgnu AWK gawk

# Try to find a suitable method for building the map file
case `uname` in
	Linux* | Solaris*)
		find CXXFILT c++filt
		findgnu OBJDUMP objdump
		filter="awk -f bin/memdbg-elf.awk"
		dumpsym="$OBJDUMP --stabs"
		;;
	OS/2 | OS2*)
		find CXXFILT c++filt
		findgnu NM nm
		filter="awk -f bin/memdbg.awk"
		dumpsym="$NM -anf sysv"
		;;
	*)
		# This is supposed to be the most working method
		CXXFILT=cat
		findgnu OBJDUMP objdump
		filter="awk -f bin/memdbg-d.awk"
		dumpsym="$OBJDUMP --debugging"
		;;
esac

# If map file is older than executable, build it
if [ ! -f $mapfile -o $executable -nt $mapfile ]; then
	echo "Building map file $mapfile ..."
	echo "O $options" >$mapfile
	$dumpsym $executable | $filter | $CXXFILT >>$mapfile
	echo "done."
fi

# If executable name contains no path, prepend ./
[ -z "`echo $executable | grep /`" ] && executable=./$executable
$executable

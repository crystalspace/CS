#!/bin/sh
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

if [ -z "$*" ]; then
	echo "No executable filename given"
	exit
fi

mapfile="memdbg.map"
executable=$1
shift
options=$*

[ -z "$options" ] && options="aslbL"

# If map file is older than executable, build it
if [ ! -f $mapfile -o $executable -nt $mapfile ]; then
	echo "Building map file $mapfile ..."
	nm --numeric-sort -f sysv --debug-syms --demangle $executable | \
	awk -f bin/memdbg.awk -v OPTIONS=$options >$mapfile
	echo "done."
fi

$executable

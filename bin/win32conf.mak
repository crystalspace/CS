# This makefile assists in the determination of shells for win32gcc.
# below, part is copied from the CS/Makefile in order to get the same
# conditions here. 

# The following two symbols are intended to be used in "echo" commands.
# config.mak can override them depending on configured platform's requirements.
"='
|=|
-include ../config.mak

# parameters for recursive make calls
RECMAKEFLAGS=--no-print-directory --no-builtin-rules
MAKEFLAGS+=r

testecho:
	echo $"testing$">>conftest.2

# This is the makefile for mingw compiler (Native win32/gcc)

# Friendly names for building environment
DESCRIPTION.mingw = Mingw (GNU C/C++)


# Choose which drivers you want to build/use
# cs2d/ddraw6 cs2d/openglwin cs3d/direct3d5 cs3d/direct3d6 cs3d/opengl
#
DRIVERS=cs2d/ddraw cs3d/software

#  csnetdrv/null csnetman/null csnetman/simple \
#  cssnddrv/null cssndrdr/null cssndrdr/software

# Uncomment the following to get an startup console window
#CONSOLE_FLAGS = -DWIN32_USECONSOLE

#---------------------------------------------------- rootdefines & defines ---#

.SUFFIXES: .exe .dll

# Processor type.
PROC=INTEL

# Operating system
OS=NT4

# Compiler -- Mingw
COMP=GCC

# Command to update a target
#UPD=bin/nt4upd.bat $@ DEST

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

# Typical extension for executables on this system (e.g. EXE=.exe)
EXE=.exe

# Typical extension for dynamic libraries on this system.
DLL=.dll

# Typical prefix for library filenames
LIB_PREFIX=lib

# Extra libraries needed on this system (beside drivers)
LIBS.EXE=

# Where can the Zlib library be found on this system?
Z_LIBS=-Llibs/zlib -lz

# Where can the PNG library be found on this system?
PNG_LIBS=-Llibs/libpng -lpng

# Where can the JPG library be found on this system?
JPG_LIBS=-Llibs/libjpeg -ljpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Does this system require libsocket.a?
NEED_SOCKET_LIB=no

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-Ilibs/zlib -Ilibs/libpng -Ilibs/libjpeg

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-Wall -fvtable-thunks $(CFLAGS.SYSTEM) -DOS_NT4

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-s -O6 -fomit-frame-pointer

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-gdb

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=-mwindows

# Flags for the linker which are used when optimizing.
LFLAGS.optimize=-s

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL=

# Typical extension for objects and static libraries
LIB=.a
define AR
  @rm -f $@
  ar
endef
ARFLAGS=cr

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f win32 -DEXTERNC_UNDERSCORE

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = libs/cssys/mingw/printf.cpp libs/cssys/mingw/timing.cpp \
  libs/cssys/mingw/dir.cpp libs/cssys/mingw/mingw.cpp \
  libs/cssys/mingw/loadlib.cpp support/gnu/getopt.c support/gnu/getopt1.c
SRC.SYS_CSSYS_EXE=libs/cssys/mingw/exeentry.cpp
SRC.SYS_CSSYS_DLL=libs/cssys/mingw/dllentry.cpp

# Where to put the dynamic libraries on this system?
OUTDLL=

# The C compiler - mingw
CC=gcc -c

# The C++ compiler - mingw
CXX=c++ -c

# The linker.
LINK=gcc

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).

MKDIR = mkdir $(subst /,\,$(@:/=))

# The command to remove all specified files.
RM=rm -f

# The command to remove a directory tree.
RMDIR=rm -rf

# For using sockets we should link with sockets library
NETSOCK_LIBS=

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=-e "s/\.ob*j*\:/\$$O:/g"

# Flags for linking a GUI and a console executable
LFLAGS.EXE=
LFLAGS.CONSOLE.EXE=

# We don't need separate directories for dynamic libraries
OUTSUFX.yes=

# Use gcc to build dependencies
DEPEND_TOOL=cc

endif # ifeq ($(MAKESECTION),defines)

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL)))
"=
|=³
endif

SYSHELP += \
  $(NEWLINE)echo $"  make mingwnt4   Prepare for building under and for $(DESCRIPTION.mingw)$"

endif # ifeq ($(MAKESECTION),confighelp)

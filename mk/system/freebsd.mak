# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Choose which drivers you want to build/use
DRIVERS=cs2d/softx cs3d/software csnetdrv/null csnetman/null csnetman/simple \
  cssnddrv/null cssndrdr/null cssndrdr/software csnetdrv/sockets cssnddrv/oss

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor. Can be one of: INTEL, SPARC, POWERPC, M68K, UNKNOWN
PROC=INTEL

# Operating system. Can be one of: SOLARIS, LINUX, IRIX, BSD, UNIX, DOS, MACOS, AMIGAOS, WIN32, OS2, BE
OS=BSD

# Compiler. Can be one of: GCC, WCC (Watcom C++), MPWERKS, VC (Visual C++), UNKNOWN
COMP=GCC

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

# Typical extension for executables on this system (e.g. EXE=.exe)
EXE=

# Typical extension for dynamic libraries on this system.
DLL=.so

# Typical extension for static libraries
LIB=.a

# Typical prefix for library filenames
LIB_PREFIX=lib

# Does this system require libsocket.a?
NEED_SOCKET_LIB=no

# Extra libraries needed on this system.
LIBS.EXE+=-L/usr/local/lib -lm

# Where can the Zlib library be found on this system?
Z_LIBS=-lz

# Where can the PNG library be found on this system?
PNG_LIBS=-lpng

# Where can the JPG library be found on this system?
JPG_LIBS=-ljpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-I/usr/local/include

# General flags for the compiler which are used in any case.
# CFLAGS.GENERAL=-Wall -DNO_ASSEMBLER
CFLAGS.GENERAL=-Wall

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-O6 -fomit-frame-pointer -malign-loops=2 -malign-jumps=2 -malign-functions=2

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g3 -gstabs

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g3

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL=-Wl,-shared

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f aoutb -DEXTERNC_UNDERSCORE

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = libs/cssys/unix/unix.cpp libs/cssys/unix/loadlib.cpp \
  libs/cssys/general/printf.cpp libs/cssys/unix/utiming.cpp \
  libs/cssys/general/fopen.cpp \
  support/gnu/getopt.c support/gnu/getopt1.c
SRC.SYS_CSSYS_DLL = libs/cssys/unix/dummy.cpp

# Where to put the dynamic libraries on this system?
OUTDLL=

# The C compiler.
CC=gcc -c

# The C++ compiler.
CXX=g++ -c

# The linker.
LINK=g++

# The library (archive) manager
AR=ar
ARFLAGS=cr

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
MKDIR=mkdir $(@:/=)

# The command to remove all specified files.
RM=rm -f

# The command to remove a directory tree.
RMDIR=rm -rf

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=

# If we don't use -fpic we don't need separate output directories
ifeq ($(CFLAGS.DLL),)
OUTSUFX.yes=
endif

#==================================================
# Extra operation system dependent options.
#==================================================

# Include support for the XSHM extension in Crystal Space.
# Note that you don't need to disable this if you don't have XSHM
# support in your X server because Crystal Space will autodetect
# the availability of XSHM.
DO_SHM=yes

endif # ifeq ($(MAKESECTION),defines)

#---------------------------------------------------------------- configure --
ifeq ($(MAKESECTION),configure)

configure:
	bin/unixconf.sh freebsd >>config.mak

endif # ifeq ($(MAKESECTION),configure)

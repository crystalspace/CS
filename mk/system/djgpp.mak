################################################################################
#                       This is the makefile for DOS/DJGPP
################################################################################

# Enable for GCC later than 2.7.2 (2.8.0 and up)
#NO_EXCEPTIONS=-fno-exceptions

DRIVERS=cs2d/dosraw cs3d/software csnetdrv/null csnetman/null \
  cssnddrv/null cssndrdr/null

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

.SUFFIXES: .exe

# Processor type
PROC=INTEL

# "Operating system", if it can be called so :-/
OS=DOS

# Compiler
COMP=GCC

# Always override "USE_DLL" to "no" even if specified from command line
override USE_DLL=no

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

vpath %.s libs/cssys/djgpp

# Typical extension for executables on this system (e.g. EXE=.exe)
EXE=.exe

# Typical extension for dynamic libraries on this system.
DLL=

# Typical extension for static libraries
LIB=.a
AR=ar
ARFLAGS=cr

# Typical prefix for library filenames
LIB_PREFIX=lib

# Extra libraries needed on this system.
LIBS.EXE=-lm

# Where can the Zlib library be found on this system?
Z_LIBS=-Llibs/zlib -lz

# Where can the PNG library be found on this system?
PNG_LIBS=-Llibs/libpng -lpng

# Where can the JPG library be found on this system?
JPG_LIBS=-Llibs/libjpeg -ljpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-Ilibs/zlib -Ilibs/libpng -Ilibs/libjpeg

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-Wall -m486 -fno-rtti $(NO_EXCEPTIONS)

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-O6 -s -fomit-frame-pointer -mno-ieee-fp \
 -malign-loops=2 -malign-jumps=2 -malign-functions=2

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g3

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=

# Flags for the linker which are used when optimizing.
LFLAGS.optimize=-s

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL=

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = libs/cssys/djgpp/djgpp.cpp libs/cssys/djgpp/printf.cpp \
	libs/cssys/djgpp/djmousys.s libs/cssys/djgpp/djkeysys.s \
	libs/cssys/general/fopen.cpp libs/cssys/general/timing.cpp

# Where to put the dynamic libraries on this system?
OUTDLL=

# Does this OS have native COM support?
NATIVE_COM=no

# The C compiler.
CC=gcc -c

# The C++ compiler.
CXX=gcc -c

# The linker.
LINK=gcc

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
MKDIR=-mkdir $(subst /,\,$(@:/=))

# The command to remove all specified files.
RM=rm -f

# The command to remove a directory tree.
RMDIR=rm -rf

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=

endif # ifeq ($(MAKESECTION),defines)

#---------------------------------------------------------------- configure --
ifeq ($(MAKESECTION),configure)

configure:
	@echo "If you have GCC >= 2.8.0 you can uncomment the"
	@echo "NO_EXCEPTIONS variable at start of mk/system/djgpp.mak"
	@echo "since this will make executable files much smaller"

endif # ifeq ($(MAKESECTION),configure)

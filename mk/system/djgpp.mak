################################################################################
#                       This is the makefile for DOS/DJGPP
################################################################################

# Friendly names for building environment
DESCRIPTION.djgpp = DOS with DJGPP

DRIVERS=cs3d/software csnetdrv/null csnetman/null cssnddrv/null cssndrdr/null

ifeq ($(USE_ALLEGRO),yes)
  DRIVERS += cs2d/alleg2
else
  DRIVERS += cs2d/dosraw
endif

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

.SUFFIXES: .exe

# Processor type
PROC=INTEL

# "Operating system", if it can be called so :-/
OS=DOS

# Compiler
COMP=GCC

# The command to update target
UPD=bin/dosupd.bat $(subst /,\,$@ DEST)

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
CFLAGS.GENERAL=-Wall $(CFLAGS.SYSTEM)

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-s -O6 -fomit-frame-pointer

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

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f coff -DEXTERNC_UNDERSCORE

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = libs/cssys/djgpp/djgpp.cpp libs/cssys/djgpp/printf.cpp \
	libs/cssys/djgpp/djmousys.s libs/cssys/djgpp/djkeysys.s \
	libs/cssys/general/timing.cpp

# Where to put the dynamic libraries on this system?
OUTDLL=

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

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make djgpp        Prepare for building under and for $(DESCRIPTION.djgpp)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION)/$(ROOTCONFIG),rootdefines/config)

# Always override USE_SHARED_PLUGINS for DOS to "no"
override USE_SHARED_PLUGINS = no

SYSCONFIG=bin/dosconf.bat
# Check if "echo" executable is not installed (thus using dumb COMMAND.COM's echo)
ifeq ($(shell echo ""),"")
SYSCONFIG += $(NEWLINE)type bin\dosconf.var>>config.tmp
endif

endif # rootdefines & config

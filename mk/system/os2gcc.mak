################################################################################
#                        This is the makefile for OS2/EMX
#                       *** (tested only with pgcc-1.0) ***
################################################################################

#---------------------------------------------------
# NOTE: OS/2 makefiles for libjpeg, libpng and zlib
# can be found in system/os2 subdirectory
#---------------------------------------------------

# Choose which drivers you want to build/use
DRIVERS=cs2d/csdive cs3d/software csnetdrv/null csnetman/null csnetman/simple \
  cssnddrv/null cssndrdr/null cssndrdr/software

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

.SUFFIXES: .exe .dll .lib .res .rc

# The path for system-specific sources
vpath %.cpp libs/cssys/os2

# Processor type. Most OS/2 systems runs on Intel's, I'm not sure IBM
# will ever ressurect the PowerPC version ... :-(
PROC=INTEL

# Operating system
OS=OS2

# Compiler
COMP=GCC

# System-dependent help commands
SYSMODIFIERSHELP = \
  echo $"  USE_OMF=yes$|no    Use OMF object module format (yes) or a.out format (no)$"
SYSMODIFIERS="USE_OMF=$(USE_OMF)"

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

# Typical extension for executables on this system (e.g. EXE=.exe)
EXE=.exe

# Typical extension for dynamic libraries on this system.
DLL=.dll

# Typical prefix for library filenames
LIB_PREFIX=

# Extra libraries needed on this system (beside drivers)
LIBS.EXE=

# Where can the Zlib library be found on this system?
Z_LIBS=-Llibs/zlib -lzdll

# Where can the PNG library be found on this system?
PNG_LIBS=-Llibs/libpng -lpngdll

# Where can the JPG library be found on this system?
JPG_LIBS=-Llibs/libjpeg -ljpegdll

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-Ilibs/zlib -Ilibs/libpng -Ilibs/libjpeg

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-Wall -Zmt $(CFLAGS.SYSTEM)

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-O6 -fomit-frame-pointer

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=-Zmt

# Flags for the linker which are used when optimizing.
LFLAGS.optimize=-s -Zsys -Zsmall-conv

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g -Zcrtdll -Zstack 512

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg -Zcrtdll -Zstack 512

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL=-Zdll

# Typical extension for objects and static libraries
ifeq ($(USE_OMF),yes)
  O=.obj
  LIB=.lib
  AR=emxomfar
  ARFLAGS=cr
  CFLAGS.GENERAL += -Zomf
  LFLAGS.GENERAL += -Zomf
else
  LIB=.a
  AR=ar
  ARFLAGS=cr
endif

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = libs/cssys/general/printf.cpp libs/cssys/general/timing.cpp \
  libs/cssys/general/fopen.cpp libs/cssys/os2/csos2.cpp \
  libs/cssys/os2/loadlib.cpp libs/cssys/os2/scancode.cpp \
  support/gnu/getopt.c support/gnu/getopt1.c
SRC.SYS_CSSYS_DLL=libs/cssys/os2/dllentry.cpp

# Where to put the dynamic libraries on this system?
OUTDLL=

# Does this OS have native COM support?
NATIVE_COM=no

# The C compiler (autodetected)
#CC=gcc -c

# The C++ compiler (autodetected)
#CXX=gcc -c

# The linker.
LINK=$(CC)

# The Resource Compiler (autodetected)
#RC=rc
RCFLAGS=-r -I libs $(RCFLAGS.SYSTEM)

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
ifneq (,$(findstring cmd,$(SHELL)))
  MKDIR = mkdir $(subst /,\,$(@:/=))
else
  MKDIR = mkdir $@
endif

# The command to remove all specified files.
RM=rm -f

# The command to remove a directory tree.
RMDIR=rm -rf

# For using sockets we should link with sockets library
NETSOCK_LIBS=-lsocket

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=-e 's/\.ob*j*\:/$$O:/g'

# Override linker with os2link.cmd
LINK=@cmd /c bin\\os2link.cmd OUT=$(OUT) DESCRIPTION=[$(DESCRIPTION.$@)]
LFLAGS.CONSOLE.EXE=CONSOLE

endif # ifeq ($(MAKESECTION),defines)

#---------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION),configure)

export SHELL

# For some (very strange) reason the following include is ignored by make???
USE_OMF = yes
-include config.mak

configure:
	@echo USE_OMF = $(USE_OMF)>>config.mak
	@cmd /c bin\\os2conf.cmd>>config.mak

endif # ifeq ($(MAKESECTION),configure)

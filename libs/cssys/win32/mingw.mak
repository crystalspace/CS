# This is the makefile for Mingw compiler (gcc Native Win32)

# Friendly names for building environment
DESCRIPTION.mingw = Win32 with Mingw GCC


# Choose which drivers you want to build/use
# video/canvas/ddraw6 video/canvas/ddraw video/renderer/direct3d5
# video/renderer/direct3d6 video/renderer/opengl
#
PLUGINS+= video/canvas/ddraw video/renderer/software \
	font/renderer/csfont
# font/renderer/csfont font/renderer/freefont
# sound/renderer/software video/canvas/ddraw video/renderer/direct3d5 \
# video/renderer/direct3d6 video/renderer/opengl
#
#  Uncomment the following to get a startup console window
CONSOLE_FLAGS = -DWIN32_USECONSOLE

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

.SUFFIXES: .exe .dll

# Processor type.
PROC=INTEL

# Operating system
OS=WIN32

# Compiler
COMP=GCC

# Use Xavier Trocchus DO_DINPUT_KEYBOARD?  set to no if you do not wish to
# use DirectInput.
# If you select "yes", then you must change include\cssys\win32\volatile.h
# to reflect this by uncommenting #define DO_DINPUT_KEYBOARD
DO_DINPUT=no

# Command to update a target
#UPD=bin\dosupd.bat $@ DEST

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

include mk/dos.mak

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
NEED_SOCKET_LIB=

#Accomodate differences between static and dynamic libraries
ifeq ($(USE_SHARED_PLUGINS),no)
	CFLAGS.GENERAL=-DCS_STATIC_LINKED
endif

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-Ilibs/zlib -Ilibs/libpng -Ilibs/libjpeg

# General flags for the compiler which are used in any case.
# Default is Ix386:
CFLAGS.GENERAL+= -fomit-frame-pointer -fvtable-thunks \
		-DWIN32_VOLATILE -Wall $(CFLAGS.SYSTEM)

# If using Pentium II
#CFLAGS.GENERAL+=-Dpentium -fomit-frame-pointer -fvtable-thunks \
#		-DWIN32_VOLATILE -Wall $(CFLAGS.SYSTEM)

# If using Pentium Pro or better (Recommended for MMX builds)
#CFLAGS.GENERAL+=-Dpentiumpro -fomit-frame-pointer -fvtable-thunks \
#		-DWIN32_VOLATILE -Wall $(CFLAGS.SYSTEM)

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-s -O3

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
# -mwindows is required for COMP_GCC build
LFLAGS.GENERAL= -mwindows

# Flags for the linker which are used when optimizing.
LFLAGS.optimize=

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

ifeq ($(USE_SHARED_PLUGINS),yes)
# Flags for the linker which are used when building a shared library.
LFLAGS.DLL=--dll
endif

# Typical extension for objects
O=.o

# Typical extension for static libraries
LIB=.a

LIB_SUFFIX=

# Setup 'lib' prefix for static library references
LIB_PREFIX=lib

define AR
  @rm -f $@
  ar
endef
ARFLAGS=cr

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f win32 -DEXTERNC_UNDERSCORE

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = libs/cssys/win32/printf.cpp \
  libs/cssys/win32/timing.cpp libs/cssys/win32/dir.cpp \
  libs/cssys/win32/win32.cpp libs/cssys/win32/loadlib.cpp \
  support/gnu/getopt.c support/gnu/getopt1.c
SRC.SYS_CSSYS_EXE=libs/cssys/win32/exeentry.cpp
SRC.SYS_CSSYS_DLL=libs/cssys/win32/dllentry.cpp

# The C compiler
CC=gcc -c

# The C++ compiler
CXX=c++ -c

# The linker.
LINK=c++

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).

# For using sockets we should link with sockets library
NETSOCK_LIBS=

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=-e "s/\.ob*j*\:/\$$O:/g"

# Flags for linking a GUI and a console executable
LFLAGS.EXE=
LFLAGS.CONSOLE.EXE=

# Use makedep to build dependencies
#DEPEND_TOOL=mkdep
DEPEND_TOOL=cc

endif # ifeq ($(MAKESECTION),defines)

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL)))
"=
|=³
endif

SYSHELP += \
  $(NEWLINE)echo $"  make mingw        Prepare for building under and for $(DESCRIPTION.mingw)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure ---#
ifeq ($(ROOTCONFIG),config)

SYSCONFIG=

endif # ifeq ($(ROOTCONFIG),config)

# This is the makefile for Mingw compiler (gcc Native Win32)

# Friendly names for building environment
DESCRIPTION.mingw = Win32 with Mingw GCC


# Choose which drivers you want to build/use
# video/canvas/ddraw6 video/canvas/ddraw 
# video/renderer/direct3d5 video/renderer/direct3d6
# video/renderer/opengl
#
PLUGINS += video/renderer/software video/canvas/ddraw
# sound/renderer/software
# video/renderer/direct3d5
# video/renderer/direct3d6
#video/renderer/opengl

ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL)))
"=
|=³
endif

override DO.MAKE.VOLATILE=$(subst \#,\\\#,$(VOLATILE_H.ALL))

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

.SUFFIXES: .exe .dll .a

# Processor type.
PROC=INTEL

# Operating system
OS=WIN32

# Compiler
COMP=GCC

# Command to update a target
#UPD=bin\dosupd.bat $@ DEST

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

# OpenGL settings for use with OpenGL Drivers...untested
#SGI OPENGL SDK v1.1 for Win32
#OPENGL.LIBS.DEFINED = -lopengl -lglut
#MS OpenGL
#OPENGL.LIBS.DEFINED = -lopengl32 -lglut32

# Extra libraries needed on this system (beside drivers)
LIBS.EXE=

# Where can the Zlib library be found on this system?
Z_LIBS=-Llibs/zlib -lz -mwindows

# Where can the PNG library be found on this system?
PNG_LIBS=-Llibs/libpng -lpng

# Where can the JPG library be found on this system?
JPG_LIBS=-Llibs/libjpeg -ljpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Does this system require libsocket.a?
NEED_SOCKET_LIB=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-Ilibs/zlib -Ilibs/libpng -Ilibs/libjpeg

# General flags for the compiler which are used for Ix386.
#CFLAGS.GENERAL+= -fomit-frame-pointer -fvtable-thunks \
#		-Wall $(CFLAGS.SYSTEM)

# General flags which are used when using Pentium II
#CFLAGS.GENERAL+=-Dpentium -fomit-frame-pointer -fvtable-thunks \
#		-Wall $(CFLAGS.SYSTEM)

# If using Pentium Pro or better (Recommended for MMX builds)
CFLAGS.GENERAL+=-Dpentiumpro -fomit-frame-pointer -fvtable-thunks \
		-Wall $(CFLAGS.SYSTEM)

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-s -O3

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL =

# Flags for the linker which are used when optimizing.
LFLAGS.optimize=

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

ifeq ($(USE_SHARED_PLUGINS),yes)
# Flags for the linker which are used when building a shared library.
LFLAGS.DLL=--dll
LIB=.dll
else
# Typical extension for static libraries
LIB=.a
endif

# Typical extension for object files
O=.o

# Typical extension for assembler files
ASM=.asm

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
  support/gnu/getopt.c support/gnu/getopt1.cpp
SRC.SYS_CSSYS_EXE=libs/cssys/win32/exeentry.cpp
SRC.SYS_CSSYS_DLL=libs/cssys/win32/dllentry.cpp

# The C compiler for Mingw/GCC
CC=gcc -c

# The C++ compiler for Mingw
# For internal compiler error handling
#CXX=c++ -c --save-temp
CXX=c++ -c

# The linker for Mingw/G++
LINK=c++

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
MKDIR = mkdir $(subst /,\,$(@:/=))

# For using sockets we should link with sockets library
NETSOCK_LIBS=

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=

#Use CC to build Dependencies
DEPEND_TOOL=cc

# Flags for linking a GUI and a console executable
LFLAGS.EXE=

endif # ifeq ($(MAKESECTION),defines)

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make mingw        Prepare for building under and for $(DESCRIPTION.mingw)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure ---#
ifeq ($(ROOTCONFIG),config)

SYSCONFIG=

endif # ifeq ($(ROOTCONFIG),config)

# This is the makefile for Mingw compiler (Minimalist C++ Native Gnu-Win32)
#  Static Library Port portion completed July 17, 2000

# Friendly names for building environment
DESCRIPTION.mingw = Win32 with Mingw GCC


# Choose which drivers you want to build/use
# video/canvas/ddraw6 video/canvas/ddraw 
# video/renderer/software video/renderer/direct3d5
# video/renderer/direct3d6 video/renderer/opengl
#
PLUGINS += video/canvas/ddraw video/renderer/software

ifeq ($(DO_SOUND),yes)
PLUGINS += sound/renderer/software
endif

# video/renderer/direct3d5
# video/renderer/direct3d6

# We don't need extra directories for dynamic libraries
OUTSUFX.yes=

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

.SUFFIXES: .exe .dll .def .a

# Processor type.
PROC=INTEL

# Operating system
OS=WIN32

# Compiler
COMP=GCC

# Command to update a target
#UPD=

# Win32 console equivalent of Unix rmdir
RMDIR=deltree /Y

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

# OpenGL settings for use with OpenGL Drivers...untested
#SGI OPENGL SDK v1.1.1 for Win32
#OPENGL.LIBS.DEFINED = -lopengl -lglut

#MS OpenGL
#OPENGL.LIBS.DEFINED = -lopengl32 -lglut32

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

# Need to override some things due to limitations of Win32 input line
ifdef DO_DEPEND
SRC1.CSENGINE = $(wildcard libs/csengine/*.cpp)
SRC2.CSENGINE = $(wildcard libs/csengine/*/*.cpp)

dep:	$(OUTOS)csengine1.dep
$(OUTOS)csengine1.dep: $(SRC1.CSENGINE)
	$(DO.DEP1) $(CFLAGS.CSENGINE) $(DO.DEP2)

dep:	$(OUTOS)csengine2.dep
$(OUTOS)csengine2.dep: $(SRC2.CSENGINE)
	$(DO.DEP1) $(CFLAGS.CSENGINE) $(DO.DEP2)

DEPPART1 = csengine1.dep
DEPPART2 = csengine2.dep
MERGE = copy

merge:	$(OUTOS)csengine.dep
$(OUTOS)csengine.dep: $(MERGE) $(OUTOS)$(DEPPART1)+$(OUTOS)$(DEPPART2) $(OUTOS)csengine.dep

endif


# Indicate where special include files can be found.
CFLAGS.INCLUDE=-Ilibs/zlib -Ilibs/libpng -Ilibs/libjpeg

# General flags for the compiler which are used for Ix386.
#CFLAGS.GENERAL+= -fvtable-thunks -Wall $(CFLAGS.SYSTEM)

# General flags which are used when using Pentium II
#CFLAGS.GENERAL+=-Dpentium -mthreads -fvtable-thunks -Wall $(CFLAGS.SYSTEM)

# If using Pentium Pro or better (Recommended for MMX builds)
CFLAGS.GENERAL+=-Dpentiumpro -mthreads -fvtable-thunks -Wall $(CFLAGS.SYSTEM)

# Flags for the compiler which are used when optimizing.
#CFLAGS.optimize=-s -O3
CFLAGS.optimize= -O3

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-p -O -g

# Flags for the compiler which are used when building a shared/dynamic library.
CFLAGS.DLL= -shared

# General flags for the linker which are used in any case.
LFLAGS.GENERAL = -mconsole -mwindows

# Flags for the linker which are used when building a shared/dynamic library.
LFLAGS.DLL =

# Flags for the linker which are used when optimizing.
LFLAGS.optimize=

# Flags for the linker which are used when debugging.
LFLAGS.debug=

# Flags for the linker which are used when profiling.
LFLAGS.profile=-p

# Typical extension for assembler files
ASM=.asm


ifneq ($(NASM.INSTALLED),no)
#
# System-dependent flags to pass to NASM
#   NASM is not supported for Mingw, use gnu assembler instead
#
NASMFLAGS.SYSTEM=-f win32 -DEXTERNC_UNDERSCORE
endif

# The C compiler for Mingw/GCC
CC=gcc -c

# The C++ compiler for Mingw
CXX=c++ -c

# The linker for Mingw/G++
LINK=c++

ifeq ($(USE_SHARED_PLUGINS),yes)

# ----- Construction Zone for .dlls ---------------------
#
#  This section under construction for defining and creating .def
# files.
#
# -------------------------------------------------------

# Build .def files using AR
#
	MAKE_DLL=yes
	DLL=.dll
	DEF=.def
	DLL_SUFFIX=dll

#
# Whenever AR is invoked, create a .def file
# Ideally we should be able to also create the .dll
# immediately after the .def file is created
#

  LIB=$(DEF)
	override ARFLAGS.@=
	override LFLAGS.@= -mdll -o $@
	
	define AR
		dlltool 
	endef
	ARFLAGS=--export-all-symbols --output-def $@

# A command format for generating a .dll from a .def
# Assumes .def file already exists
#
# c++ -shared *.o [*.def] -o *.dll

override LFLAGS.@=-mdll -o

CPP=c++

BUILD.DLL = $(CPP) $(LFLAGS.@) $@ $^

#(OUT)%.def: %.dll
#	$(DO.BUILD.DLL)  

#	DO.SHARED.PLUGINS = $(DO.BUILD.DLL)
 
else

#
# Setup 'lib' prefix for static library references
#
  LIB_PREFIX=lib

# Typical extension for static libraries
#
  LIB=.a

# Explicitly define of static linking mode
#
  LFLAGS.GENERAL+= -static

define AR
  @rm -f $@
  ar
endef
ARFLAGS=cr
endif

# ----- End Construction Zone ----------------------------

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = libs/cssys/win32/printf.cpp \
  libs/cssys/win32/timing.cpp libs/cssys/win32/dir.cpp \
  libs/cssys/win32/win32.cpp libs/cssys/win32/loadlib.cpp \
  support/gnu/getopt.c support/gnu/getopt1.cpp
#
# Disabled exeentry.cpp as it was generating duplicate ModuleHandle references
#SRC.SYS_CSSYS_EXE=libs/cssys/win32/exeentry.cpp
SRC.SYS_CSSYS_DLL=libs/cssys/win32/dllentry.cpp


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

# Flags for linking a GUI such as V
LFLAGS.EXE=

# Flags for linking a separate console executable
LFLAGS.CONSOLE.EXE=

endif # ifeq ($(MAKESECTION),defines)

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make mingw        Prepare for building under and for $(DESCRIPTION.mingw)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION),rootdefines)

#ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL)))
"=
|=³
#endif

ifeq ($(ROOTCONFIG),config)
SYSCONFIG=
endif # ifeq ($(ROOTCONFIG),config)
#---------------------------------------------------------------- volatile ---#

ifeq ($(ROOTCONFIG),volatile)


endif # ifeq ($(ROOTCONFIG),volatile)

endif # ifeq ($(MAKESECTION),rootdefines)

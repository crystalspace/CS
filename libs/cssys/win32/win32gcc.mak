# This is the makefile for MinGW32 compiler (gcc for Win32)

# Friendly names for building environment
DESCRIPTION.win32gcc = Windows with MinGW32 GNU C/C++
DESCRIPTION.OS.win32gcc = Win32

# Choose which drivers you want to build/use
PLUGINS+= cscript/cspython
PLUGINS+= sound/renderer/software
PLUGINS+= video/canvas/ddraw
PLUGINS+= video/canvas/ddraw8
#video/renderer/direct3d8

# if u have the following line uncommented make sure one  
# LIBS.OPENGL.SYSTEM is set below or you have a custom 
# opengl dll installed as GL.dll (e.g. MESA)
PLUGINS+= video/canvas/openglwin video/renderer/opengl

# uncomment the line below to build the sound driver
#PLUGINS+= sound/driver/waveoutsd

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

.SUFFIXES: .exe .dll

# Processor type.
PROC=X86

# Operating system
OS=WIN32

# Compiler
COMP=GCC

# Command to update a target
UPD=bin\dosupd.bat $@ DEST

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/dos.mak

# Typical object file extension
O=.o

# Typical prefix for library filenames
LIB_PREFIX=lib

# Extra libraries needed on this system (beside drivers)
LIBS.EXE=

# OpenGL settings for use with OpenGL Drivers...untested
#SGI OPENGL SDK v1.1.1 for Win32
#LIBS.OPENGL.SYSTEM = $(LFLAGS.l)opengl $(LFLAGS.l)glut

# MS OpenGL
LIBS.OPENGL.SYSTEM=$(LFLAGS.l)opengl32 $(LFLAGS.l)glut32

# Socket library
LIBS.SOCKET.SYSTEM=$(LFLAGS.l)wsock32

# Sound library
LIBS.SOUND.SYSTEM=$(LFLAGS.l)dsound $(LFLAGS.l)winmm

# Python library
LIBS.CSPYTHON.SYSTEM=$(LFLAGS.l)python15

# Where can the Zlib library be found on this system?
Z_LIBS=$(LFLAGS.l)z

# Where can the PNG library be found on this system?
PNG_LIBS=$(LFLAGS.l)png

# Where can the JPG library be found on this system?
JPG_LIBS=$(LFLAGS.l)jpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
# for instance where your dx includes are
CFLAGS.INCLUDE=
#$(CFLAGS.I)/dx7asdk/dxf/include 

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-Wall $(CFLAGS.SYSTEM) -fvtable-thunks -pipe

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-s -O3 -ffast-math
ifneq ($(shell gcc --version),2.95.2)
# WARNING: mingw32 2.95.2 has a bug that causes incorrect code
# to be generated if you use -fomit-frame-pointer
CFLAGS.optimize += -fomit-frame-pointer
endif

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g

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
LFLAGS.DLL=-q -s --no-export-all-symbols --dllname $*

# Typical extension for objects and static libraries
LIB=.a
define AR
  rm -f $@
  ar
endef
ARFLAGS=cr

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f win32 $(CFLAGS.D)EXTERNC_UNDERSCORE

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = \
  libs/cssys/win32/win32.cpp \
  libs/cssys/win32/dir.cpp \
  libs/cssys/win32/timing.cpp \
  libs/cssys/win32/loadlib.cpp \
  libs/cssys/win32/instpath.cpp \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/printf.cpp \
  libs/cssys/general/getopt.cpp

# The C compiler
CC=gcc -c

# The C++ compiler
CXX=gcc -c

# The linker.
LINK=gcc

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL))$(findstring cmd,$(SHELL))$(findstring CMD,$(SHELL)))
  MKDIR = mkdir $(subst /,\,$(@:/=))
else
  MKDIR = mkdir $@
endif

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=-e "s/\.ob*j*\:/\$$O:/g"

# Flags for linking a GUI and a console executable
LFLAGS.EXE=-mwindows
# commenting out the following line will make the -noconsole option work
# but the only way to redirect output will be WITH -noconsole (wacky :-)
# and the console will not start minimized if a shortcut says it should
LFLAGS.EXE+=-mconsole
LFLAGS.CONSOLE.EXE=

# Use makedep to build dependencies
DEPEND_TOOL=mkdep

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# How to make a shared AKA dynamic library
DO.SHARED.PLUGIN.CORE = \
  dllwrap $(LFLAGS.DLL) $(LFLAGS.@) $(^^) $(L^) $(LIBS) $(LFLAGS) -mwindows

# Commenting out the following line will make the -noconsole option work
# but the only way to redirect output will be WITH -noconsole (wacky :-)
# and the console will not start minimized if a shortcut says it should
DO.SHARED.PLUGIN.CORE += -mconsole

endif # ifeq ($(MAKESECTION),postdefines)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL)))
"=
|=³
endif

SYSHELP += \
  $(NEWLINE)echo $"  make win32gcc     Prepare for building on $(DESCRIPTION.win32gcc)$"

endif # ifeq ($(MAKESECTION),confighelp)

#--------------------------------------------------------------- configure ---#
ifeq ($(ROOTCONFIG),config)

SYSCONFIG=bin\win32conf.bat mingw32

endif # ifeq ($(ROOTCONFIG),config)

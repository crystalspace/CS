# This is the makefile for MinGW32 compiler (gcc for Win32)
# Optionally works with the MSYS environment.

# Friendly names for building environment
DESCRIPTION.win32gcc = Windows with MinGW32 GNU C/C++
DESCRIPTION.OS.win32gcc = Win32

# Choose which drivers you want to build/use
PLUGINS += sound/renderer/software
PLUGINS += video/canvas/ddraw
#PLUGINS += video/canvas/ddraw8
#PLUGINS.DYNAMIC +=video/format/avi
#PLUGINS.DYNAMIC +=video/format/codecs/opendivx

# If you have the following line uncommented make sure one
# LIBS.OPENGL.SYSTEM is set below or you have a custom
# opengl dll installed as GL.dll (e.g. MESA)
PLUGINS += video/canvas/openglwin video/renderer/opengl

# Uncomment the line below to build the sound driver
PLUGINS += sound/driver/waveoutsd

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
ifeq (,$(MSYSTEM))
UPD=libs\cssys\win32\winupd.bat $@ DEST
endif

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/dos.mak

# Typical object file extension
O=.o

# Typical prefix for library filenames
LIB_PREFIX=lib

# Extra libraries needed on this system (beside drivers)
# MINGW_LIBS comes from the local config.mak and is set up by msysconf.sh.
LIBS.EXE= $(LFLAGS.l)gdi32 $(MINGW_LIBS) $(LFLAGS.l)shell32 

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

# Lua library
LIBS.CSLUA.SYSTEM=$(LFLAGS.l)lua $(LFLAGS.l)lualib

# Freetype library
LIBS.FREETYPE.SYSTEM=$(LFLAGS.l)ttf

# Where can the Zlib library be found on this system?
Z_LIBS=$(LFLAGS.L)libs/zlib $(LFLAGS.l)z

# Where can the PNG library be found on this system?
PNG_LIBS=$(LFLAGS.L)libs/libpng $(LFLAGS.l)png

# Where can the JPG library be found on this system?
JPG_LIBS=$(LFLAGS.L)libs/libjpeg $(LFLAGS.l)jpeg

# Where can the MNG library be found on this system?
MNG_LIBS=$(LFLAGS.L)libs/libmng $(LFLAGS.l)mng

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
# for instance where your dx includes are
CFLAGS.INCLUDE=$(CFLAGS.I)/usr/include/directx \
  $(CFLAGS.I)libs/zlib $(CFLAGS.I)libs/libpng $(CFLAGS.I)libs/libjpeg
# $(CFLAGS.I)/dx7asdk/dxf/include

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

# Extra linker flags used by MSYS
ifneq (,$(MSYSTEM))
LFLAGS.GENERAL=$(LFLAGS.L)/usr/lib/w32api
endif

# Flags for the linker which are used when optimizing.
LFLAGS.optimize=-s

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for dllwrap in optimize mode
DFLAGS.optimize = -s

# Flags for dllwrap in debug mode
DFLAGS.debug = -g3

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL=$(DFLAGS.$(MODE)) -q --no-export-all-symbols --dllname $*

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
  libs/cssys/win32/timing32.cpp \
  libs/cssys/win32/loadlib.cpp \
  libs/cssys/win32/instpath.cpp \
  libs/cssys/win32/sysroot.cpp \
  libs/cssys/win32/mmap.cpp \
  libs/cssys/win32/vfsplat.cpp \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/getopt.cpp \
  libs/cssys/general/printf.cpp \
  libs/cssys/general/runloop.cpp \
  libs/cssys/general/sysinit.cpp 

# The C compiler
CC=gcc -c

# The C++ compiler
CXX=g++ -c

# The linker.
LINK=g++

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

# If SHELL is the Windows COMMAND or CMD, then we need "bash" for scripts.
# Also, multiple commands are separated with "&" vs ";" w/ bash.
ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL))$(findstring cmd,$(SHELL))$(findstring CMD,$(SHELL)))
  RUN_SCRIPT = bash
  COMMAND_DELIM = &
else
  COMMAND_DELIM = ;
endif

# How to make shared libs for cs-config
LINK.PLUGIN=dllwrap
PLUGIN.POSTFLAGS=-mwindows -mconsole

# How to make a shared AKA dynamic library

DLLWRAPWRAP = $(RUN_SCRIPT) libs/cssys/win32/dllwrapwrap.sh

ifeq ($(MODE),debug)
	RCFLAGS=-DCS_DEBUG
else
	RCFLAGS=
endif

COMPILE_RES = windres --include-dir include $(RCFLAGS) 
MAKEVERSIONINFO = $(RUN_SCRIPT) libs/cssys/win32/mkverres.sh
MERGERES = $(RUN_SCRIPT) libs/cssys/win32/mergeres.sh

DO.SHARED.PLUGIN.CORE = \
  $(MAKEVERSIONINFO) $(OUT)/$(@:$(DLL)=-version.rc) \
    "$(DESCRIPTION.$*)" $(COMMAND_DELIM) \
  $(MERGERES) $(OUT)/$(@:$(DLL)=-rsrc.rc) ./ \
    $(OUT)/$(@:$(DLL)=-version.rc) $($@.WINRSRC) $(COMMAND_DELIM) \
  $(COMPILE_RES) -i $(OUT)/$(@:$(DLL)=-rsrc.rc) \
    -o $(OUT)/$(@:$(DLL)=-rsrc.o) $(COMMAND_DELIM) \
  $(DLLWRAPWRAP) $* $(LFLAGS.DLL) $(LFLAGS.@) $(^^) \
    $(OUT)/$(@:$(DLL)=-rsrc.o) $(L^) $(LIBS) $(LFLAGS) --driver-name=$(LINK) \
    -mwindows
#DO.SHARED.PLUGIN.CORE = \
#  $(DLLWRAPWRAP) $* $(LFLAGS.DLL) $(LFLAGS.@) $(^^) \
#    $(L^) $(LIBS) $(LFLAGS) -mwindows

# Commenting out the following line will make the -noconsole option work
# but the only way to redirect output will be WITH -noconsole (wacky :-)
# and the console will not start minimized if a shortcut says it should
DO.SHARED.PLUGIN.CORE += -mconsole

DO.LINK.EXE = \
	$(MAKEVERSIONINFO) $(OUT)/$(@:$(EXE)=-version.rc) \
	  "$(DESCRIPTION.$*)" $(COMMAND_DELIM) \
	$(MERGERES) $(OUT)/$(@:$(EXE)=-rsrc.rc) ./ \
	  $(OUT)/$(@:$(EXE)=-version.rc) $($@.WINRSRC) $(COMMAND_DELIM) \
	$(COMPILE_RES) -i $(OUT)/$(@:$(EXE)=-rsrc.rc) \
	  -o $(OUT)/$(@:$(EXE)=-rsrc.o) $(COMMAND_DELIM) \
	$(LINK) $(LFLAGS) $(LFLAGS.EXE) $(LFLAGS.@) $(^^) \
	  $(OUT)/$(@:$(EXE)=-rsrc.o) $(L^) $(LIBS) $(LIBS.EXE.PLATFORM)
DO.LINK.CONSOLE.EXE = $(DO.LINK.EXE)

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
ifeq ($(MAKESECTION),rootdefines) # Makefile includes us twice with valid
ifeq ($(ROOTCONFIG),config)	  # ROOTCONFIG, but we only need to run once.

ifneq (,$(MSYSTEM))
SYSCONFIG += $(NEWLINE)sh libs/cssys/win32/msysconf.sh $(INSTALL_DIR)>>config.tmp
else
SYSCONFIG=libs\cssys\win32\winconf.bat mingw32
endif

endif # ifeq ($(ROOTCONFIG),config)
endif # ifeq ($(MAKESECTION),rootdefines)

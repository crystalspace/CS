# This is the makefile for Mingw+MSYS and Cygwin (gcc for Win32)

# Friendly names for building environment
DESCRIPTION.win32gcc = Windows with Mingw+MSYS or Cygwin
DESCRIPTION.OS.win32gcc = Win32

# Choose which drivers you want to build/use
PLUGINS += sound/renderer/software
PLUGINS += video/canvas/ddraw
PLUGINS += video/canvas/ddraw8
#PLUGINS.DYNAMIC +=video/format/avi
#PLUGINS.DYNAMIC +=video/format/codecs/opendivx

ifeq ($(GL.AVAILABLE),yes)
PLUGINS += video/canvas/openglwin
endif

PLUGINS += sound/driver/waveoutsd

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/dos.mak

# Typical object file extension
O=.o

# Typical prefix for library filenames
LIB_PREFIX=lib

# Extra libraries needed on this system (beside drivers)
# LIBS.SYSTEM comes from the local config.mak and is set up by the Autoconf
# configure script.
LIBS.EXE = \
  $(LFLAGS.L)/usr/lib/w32api \
  $(LFLAGS.l)gdi32 \
  $(LIBS.SYSTEM) \
  $(LFLAGS.l)shell32 

# Sound library
LIBS.SOUND.SYSTEM = $(LFLAGS.l)dsound $(LFLAGS.l)winmm

# Lua library
LIBS.CSLUA.SYSTEM = $(LFLAGS.l)lua $(LFLAGS.l)lualib

# Freetype library
LIBS.FREETYPE.SYSTEM = $(LFLAGS.l)ttf

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL = $(CFLAGS.SYSTEM) $(CSTHREAD.CFLAGS) -pipe

# Indicate where special include files can be found.
# for instance where your dx includes are
CFLAGS.INCLUDE= $(CFLAGS.I)/usr/include/directx

# Flags for the compiler which are used when profiling.
CFLAGS.profile = -pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL =

# General flags for the linker which are used in any case.
LFLAGS.GENERAL = $(CSTHREAD.LFLAGS)

# Flags for the linker which are used when optimizing.
LFLAGS.optimize += -s

# Flags for the linker which are used when profiling.
LFLAGS.profile = -pg

# Flags for linking DLLs in optimize mode
DFLAGS.optimize += -s

# Flags for linking DLLs in debug mode
DFLAGS.debug = -Xlinker --export-all-symbols

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL = $(DFLAGS.$(MODE)) -shared

# Typical extension for objects and static libraries
LIB = .a
define AR
  rm -f $@
  ar
endef
ARFLAGS = cr

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM = -f win32 $(CFLAGS.D)EXTERNC_UNDERSCORE

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = $(wildcard libs/cssys/win32/*.cpp) \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/getopt.cpp \
  libs/cssys/general/printf.cpp \
  libs/cssys/general/runloop.cpp \
  libs/cssys/general/sysinit.cpp \
  $(CSTHREAD.SRC)
INC.SYS_CSSYS = $(wildcard libs/cssys/win32/*.h) $(CSTHREAD.INC)

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND = -e "s/\.ob*j*\:/\$$O:/g"

# Flags for linking a GUI and a console executable
ifeq ($(MODE),debug)
  LFLAGS.EXE = -mconsole
else
  LFLAGS.EXE = -mwindows
endif
# Commenting out the following line will make the -noconsole option work
# but the only way to redirect output will be WITH -noconsole (wacky :-)
# and the console will not start minimized if a shortcut says it should
#LFLAGS.EXE += -mconsole
LFLAGS.CONSOLE.EXE = -mconsole

# Use makedep to build dependencies
DEPEND_TOOL = mkdep

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

COMMAND_DELIM = ;

# How to make shared libs for cs-config
LINK.PLUGIN = $(LINK)
PLUGIN.POSTFLAGS = -mwindows -mconsole

# How to make a shared (a.k.a. dynamic) library.
ifeq ($(MODE),debug)
  RCFLAGS = -DCS_DEBUG
else
  RCFLAGS =
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
  $(LINK.PLUGIN) $(LFLAGS.DLL) $(LFLAGS.@) $(^^) \
    $(OUT)/$(@:$(DLL)=-rsrc.o) $(L^) $(LIBS) $(LFLAGS) \
    -mwindows

# Commenting out the following line will make the -noconsole option work
# but the only way to redirect output will be WITH -noconsole (wacky :-)
# and the console will not start minimized if a shortcut says it should
#DO.SHARED.PLUGIN.CORE += -mconsole

DO.LINK.EXE = \
  $(MAKEVERSIONINFO) $(OUT)/$(@:$(EXE)=-version.rc) \
    "$(DESCRIPTION.$*)" $(COMMAND_DELIM) \
  $(MERGERES) $(OUT)/$(@:$(EXE)=-rsrc.rc) ./ \
    $(OUT)/$(@:$(EXE)=-version.rc) $($@.WINRSRC) $(COMMAND_DELIM) \
  $(COMPILE_RES) -i $(OUT)/$(@:$(EXE)=-rsrc.rc) \
    -o $(OUT)/$(@:$(EXE)=-rsrc.o) $(COMMAND_DELIM) \
  $(LINK) $(LFLAGS) $(LFLAGS.EXE) $(LFLAGS.@) $(^^) \
    $(OUT)/$(@:$(EXE)=-rsrc.o) $(L^) $(LIBS) $(LIBS.EXE.PLATFORM)

DO.LINK.CONSOLE.EXE = \
  $(MAKEVERSIONINFO) $(OUT)/$(@:$(EXE)=-version.rc) \
    "$(DESCRIPTION.$*)" $(COMMAND_DELIM) \
  $(MERGERES) $(OUT)/$(@:$(EXE)=-rsrc.rc) ./ \
    $(OUT)/$(@:$(EXE)=-version.rc) $($@.WINRSRC) $(COMMAND_DELIM) \
  $(COMPILE_RES) -i $(OUT)/$(@:$(EXE)=-rsrc.rc) \
    -o $(OUT)/$(@:$(EXE)=-rsrc.o) $(COMMAND_DELIM) \
  $(LINK) $(LFLAGS) $(LFLAGS.CONSOLE.EXE) $(LFLAGS.@) $(^^) \
    $(OUT)/$(@:$(EXE)=-rsrc.o) $(L^) $(LIBS) $(LIBS.EXE.PLATFORM)

endif # ifeq ($(MAKESECTION),postdefines)

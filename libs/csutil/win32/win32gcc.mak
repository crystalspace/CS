# This is the makefile for Mingw/MSYS and Cygwin (gcc for Win32)

# Friendly names for build environment
DESCRIPTION.win32gcc = Windows with Mingw/MSYS or Cygwin
DESCRIPTION.OS.win32gcc = Win32

PLUGINS += video/canvas/ddraw
ifeq ($(GL.AVAILABLE),yes)
PLUGINS += video/canvas/openglwin
endif
PLUGINS += sound/driver/waveoutsd
PLUGINS += sound/renderer/ds3d

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include $(SRCDIR)/mk/dos.mak

# Typical object file extension
O=.o

# Typical prefix for library filenames
LIB_PREFIX=lib

# <cs-config>
# Extra libraries needed on this system (beside drivers)
# LIBS.SYSTEM comes from the local config.mak and is set up by the Autoconf
# configure script.
LIBS.EXE.PLATFORM += \
  $(LFLAGS.L)/usr/lib/w32api \
  $(LFLAGS.l)gdi32 \
  $(LIBS.SYSTEM) \
  $(LFLAGS.l)shell32 
# </cs-config>

# Extra libraries needed for Python cspace module.
PYTHMOD.LIBS.PLATFORM = gdi32 shell32

# Sound library
LIBS.SOUND.SYSTEM = $(LFLAGS.l)dsound $(LFLAGS.l)winmm

# Freetype library
LIBS.FREETYPE.SYSTEM = $(LFLAGS.l)ttf

# <cs-config>
# General flags for the compiler which are used in any case.
CFLAGS.GENERAL = $(CFLAGS.SYSTEM) $(CSTHREAD.CFLAGS)
# </cs-config>

# <cs-config>
# Indicate where special include files can be found.
# for instance where your dx includes are
CFLAGS.INCLUDE= $(CFLAGS.I)/usr/include/directx
# </cs-config>

# Flags for the compiler which are used when profiling.
CFLAGS.profile = -pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL +=

# General flags for the linker which are used in any case.
# <cs-config>
LFLAGS.GENERAL = $(LFLAGS.SYSTEM)
# </cs-config>

# Flags for the linker which are used when optimizing.
LFLAGS.optimize += -s

# Flags for the linker which are used when profiling.
LFLAGS.profile = -pg

# Flags for linking DLLs in optimize mode
DFLAGS.optimize += -s

# Flags for linking DLLs in debug mode
DFLAGS.debug =

# Flags for the linker which are used when building a shared library.
# <cs-config>
TARGET.RAW = $(basename $(notdir $@))
TARGET.RAW.UPCASE = $(basename $(notdir $(UPCASE)))
LFLAGS.DLL += $(DFLAGS.$(MODE))
# </cs-config>

# Typical extension for objects and static libraries
LIB = .a
define AR
  rm -f $@
  ar
endef
ARFLAGS = cr

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM = -f win32 $(CFLAGS.D)EXTERNC_UNDERSCORE

# System dependent source files included into csutil library
SRC.SYS_CSUTIL = $(wildcard $(SRCDIR)/libs/csutil/win32/*.cpp) \
  $(SRCDIR)/libs/csutil/generic/appdir.cpp \
  $(SRCDIR)/libs/csutil/generic/csprocessorcap.cpp \
  $(SRCDIR)/libs/csutil/generic/findlib.cpp \
  $(SRCDIR)/libs/csutil/generic/getopt.cpp \
  $(SRCDIR)/libs/csutil/generic/pluginpaths.cpp \
  $(SRCDIR)/libs/csutil/generic/resdir.cpp \
  $(SRCDIR)/libs/csutil/generic/runloop.cpp \
  $(CSTHREAD.SRC)
INC.SYS_CSUTIL = $(wildcard $(SRCDIR)/libs/csutil/win32/*.h) $(CSTHREAD.INC)

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND = -e "s/\.ob*j*\:/\$$O:/g"

# <cs-config>
LFLAGS.EXE = -mconsole
# </cs-config>

# Use makedep to build dependencies
DEPEND_TOOL = mkdep

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# How to make shared libs
# <cs-config>
LINK.PLUGIN = $(LINK)
PLUGIN.POSTFLAGS = -mwindows -mconsole
COMMAND_DELIM = ;
# </cs-config>

# How to make a shared (a.k.a. dynamic) library.
ifeq ($(MODE),debug)
  RCFLAGS = -DCS_DEBUG
else
  RCFLAGS =
endif

COMPILE_RES = $(CMD.WINDRES) --use-temp-file --include-dir include $(RCFLAGS) 
MAKEVERSIONINFO = $(RUN_SCRIPT) $(SRCDIR)/libs/csutil/win32/mkverres.sh
MERGERES = $(RUN_SCRIPT) $(SRCDIR)/libs/csutil/win32/mergeres.sh
MAKEMETADATA = $(RUN_SCRIPT) $(SRCDIR)/libs/csutil/win32/mkmetadatares.sh

ifeq ($(EMBED_META),yes)
METARC = $(OUT)/$(@:$(DLL)=-meta.rc)
# Replace/override the default preamble which simply copies .csplugin file.
DO.SHARED.PLUGIN.PREAMBLE = \
  $(MAKEMETADATA) $(METARC) $(INF.$(TARGET.RAW.UPCASE)) $(COMMAND_DELIM)
else
METARC =
endif

DO.SHARED.PLUGIN.PREAMBLE += \
  $(MAKEVERSIONINFO) $(OUT)/$(@:$(DLL)=-version.rc) \
    "$(DESCRIPTION.$(TARGET.RAW))" \
    "$(SRCDIR)/include/csver.h" $(COMMAND_DELIM) \
  $(MERGERES) $(OUT)/$(@:$(DLL)=-rsrc.rc) $(SRCDIR) $(SRCDIR) \
    $(OUT)/$(@:$(DLL)=-version.rc) $($@.WINRSRC) $(METARC) \
    $(COMMAND_DELIM) \
  $(COMPILE_RES) -i $(OUT)/$(@:$(DLL)=-rsrc.rc) \
    --include-dir "$(SRCDIR)/include" \
    -o $(OUT)/$(@:$(DLL)=-rsrc.o) $(COMMAND_DELIM)

DO.SHARED.PLUGIN.CORE = \
  $(LINK.PLUGIN) $(LFLAGS.DLL) $(LFLAGS.@) $(^^) \
    $(OUT)/$(@:$(DLL)=-rsrc.o) $(L^) $(LIBS) $(LFLAGS)

# <cs-config>
DO.SHARED.PLUGIN.POSTAMBLE = -mwindows
# </cs-config>

# Commenting out the following line will make the -noconsole option work
# but the only way to redirect output will be WITH -noconsole (wacky :-)
# and the console will not start minimized if a shortcut says it should
#DO.SHARED.PLUGIN.CORE += -mconsole

DO.LINK.EXE = \
  $(MAKEVERSIONINFO) $(OUT)/$(@:$(EXE)=-version.rc) \
    "$(DESCRIPTION.$(TARGET.RAW))" "$(SRCDIR)/include/csver.h" \
    $(COMMAND_DELIM) \
  $(MERGERES) $(OUT)/$(@:$(EXE)=-rsrc.rc) $(SRCDIR) $(SRCDIR) \
    $(OUT)/$(@:$(EXE)=-version.rc) $($@.WINRSRC) $(COMMAND_DELIM) \
  $(COMPILE_RES) -i $(OUT)/$(@:$(EXE)=-rsrc.rc) \
    --include-dir="$(SRCDIR)/include" \
    -o $(OUT)/$(@:$(EXE)=-rsrc.o) $(COMMAND_DELIM) \
  $(LINK) $(LFLAGS) $(LFLAGS.EXE) $(LFLAGS.@) $(^^) \
    $(OUT)/$(@:$(EXE)=-rsrc.o) $(L^) $(LIBS)

DO.LINK.CONSOLE.EXE = $(DO.LINK.EXE)

endif # ifeq ($(MAKESECTION),postdefines)

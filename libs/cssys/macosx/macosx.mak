#==============================================================================
# This is the system makefile for Apple MacOS/X.
# Copyright (C)1998-2002 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================

# Choose which drivers you want to build/use
PLUGINS += video/canvas/macosx/cocoa
PLUGINS += video/canvas/macosx/coregraphics
PLUGINS += video/canvas/macosx/opengl
PLUGINS += video/renderer/opengl
PLUGINS += sound/driver/coreaudio
PLUGINS += sound/renderer/software

DESCRIPTION.macosx = MacOS/X
DESCRIPTION.OS.macosx = MacOS/X

# Avoid linker complain about weak vs. non-weak frameworks.
export MACOSX_DEPLOYMENT_TARGET=10.2

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

PROC = POWERPC

# Operating system.
OS = MACOSX

# Operating system family: UNIX (for Unix or Unix-like platforms), WIN32, etc.
OS_FAMILY = UNIX

# Compiler. Can be one of: GCC, MPWERKS, VC (Visual C++), UNKNOWN
COMP = GCC

# Application wrapper support.
MACOSX.APP_EXE  = $@/Contents/MacOS/$(notdir $(basename $@))
MACOSX.APP_ICON = libs/cssys/macosx/appicon.icns
MACOSX.APP_DIR  = .
MACOSX.APP_EXT  = .app

# Plug-in component support.
MACOSX.PLUGIN_DIR = components
MACOSX.PLUGIN_EXT = .csplugin

# Apple can not use the x86 assembly in CS.
override DO_ASM = no

# The extensive memory debugger facility in cssysdef.h is incompatible with
# the Apple compiler.
override EXTENSIVE_MEMDEBUG = no

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/unix.mak

# Add support for Objective-C (.m) and Objective-C++ (.mm) source code.
.SUFFIXES: .m .mm

# How to compile an Objective-C .m source
DO.COMPILE.M = $(OBJC) $(CFLAGS.@) $(<<) $(CFLAGS) $(CFLAGS.INCLUDE)

# How to compile an Objective-C++ .mm source
DO.COMPILE.MM = $(OBJCXX) $(CFLAGS.@) $(<<) $(CFLAGS) $(CFLAGS.INCLUDE)

# Directories containing platform-specific source code.
MACOSX.SOURCE_PATHS = libs/cssys/macosx

# Select MacOS/X config file for inclusion with install.
TO_INSTALL.CONFIG += data/config/macosx.cfg

# Extension for applications on this system
EXE = $(MACOSX.APP_EXT)

# Extension for dynamic libraries on this system.
DLL = $(MACOSX.PLUGIN_EXT)

# Extra libraries needed on this system.
LIBS.EXE =

# Extra libraries needed only for executables (not plug-ins)
LIBS.EXE.PLATFORM =

# Socket library
LIBS.SOCKET.SYSTEM =

# Additional audio libraries.
SOUND_LIBS =

# Indicate where special include files can be found.
CFLAGS.INCLUDE = $(addprefix $(CFLAGS.I),$(MACOSX.SOURCE_PATHS))

# General flags for the compiler which are used in any case.  The "config"
# flags are determined at configuration time and come from CS/config.mak.
CFLAGS.GENERAL = \
  -force_cpusubtype_ALL \
  $(MACOSX.CFLAGS.CONFIG) \
  $(CFLAGS.SYSTEM) \
  $(CSTHREAD.CFLAGS) \
  -Wno-precomp -fno-common -pipe

# Special option for the software 3D renderer to force it to ARGB mode
CFLAGS.PIXEL_LAYOUT = -DCS_24BIT_PIXEL_LAYOUT=CS_24BIT_PIXEL_ARGB

# OpenGL support.
CFLAGS.GL3D = -DCS_OPENGL_PATH=OpenGL
CFLAGS.GLRENDER3D = $(CFLAGS.GL3D)
LIBS.OPENGL.SYSTEM = -framework OpenGL

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize = -O3

# Flags for the compiler which are used when debugging.
CFLAGS.debug = -g

# Flags for the compiler which are used when profiling.
CFLAGS.profile = -pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL =

# General flags for the linker which are used in any case.  The "config" flags
# are determined at configuration time and come from CS/config.mak.
LFLAGS.GENERAL = $(MACOSX.LFLAGS.CONFIG) $(CSTHREAD.LFLAGS)

# Flags for the linker which are used when debugging.
LFLAGS.debug = -g

# Flags for the linker which are used when profiling.
LFLAGS.profile = -pg

# Flags for the linker which are used when building a graphical executable.
LFLAGS.EXE = -framework AppKit -framework Foundation

# Flags for the linker which are used when building a console executable.
LFLAGS.CONSOLE.EXE = -framework AppKit -framework Foundation

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL = -bundle -framework AppKit -framework Foundation

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM =

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = $(wildcard \
  $(addsuffix /*.cpp,$(MACOSX.SOURCE_PATHS)) \
  $(addsuffix /*.c,$(MACOSX.SOURCE_PATHS)) \
  $(addsuffix /*.m,$(MACOSX.SOURCE_PATHS)) \
  $(addsuffix /*.mm,$(MACOSX.SOURCE_PATHS))) \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/getopt.cpp \
  libs/cssys/general/printf.cpp \
  libs/cssys/general/sysroot.cpp \
  $(CSTHREAD.SRC)

# Where to put dynamic libraries on this system?
OUTDLL = $(MACOSX.PLUGIN_DIR)

# The library (archive) manager
AR = libtool
ARFLAGS = -static -o

# The stripper :-)
STRIP = strip

# We don't need separate directories for dynamic libraries
OUTSUFX.yes =

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Add support for Objective-C (.m) source code.
$(OUT)/%$O: %.m
	$(DO.COMPILE.M)

# Add support for Objective-C++ (.mm) source code.
$(OUT)/%$O: %.mm
	$(DO.COMPILE.MM)

OBJ.CSSYS = $(addprefix $(OUT)/,$(notdir $(subst .s,$O,$(subst .c,$O,\
  $(subst .cpp,$O,$(subst .mm,$O,$(SRC.CSSYS:.m=$O)))))))

vpath %.m  libs/cssys $(filter-out libs/cssys/general/,$(sort $(dir $(SRC.SYS_CSSYS)))) libs/cssys/general
vpath %.mm libs/cssys $(filter-out libs/cssys/general/,$(sort $(dir $(SRC.SYS_CSSYS)))) libs/cssys/general

# Override default method of creating a GUI application.  For Cocoa, we need
# to place the executable inside an application wrapper.
define DO.LINK.EXE
  sh libs/cssys/macosx/appwrap.sh $(notdir $(basename $@)) $(MACOSX.APP_DIR) $(MACOSX.APP_ICON)
  $(NEWLINE)$(LINK) $(LFLAGS) $(LFLAGS.EXE) -o $(MACOSX.APP_EXE) $(^^) $(L^) $(LIBS) $(LIBS.EXE.PLATFORM)
  touch $@
endef

endif # ifeq ($(MAKESECTION),postdefines)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += $(NEWLINE)echo $"  make macosx       Prepare for building on $(DESCRIPTION.macosx)$"

endif # ifeq ($(MAKESECTION),confighelp)

#--------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION),rootdefines) # Makefile includes us twice with valid
ifeq ($(ROOTCONFIG),config)	  # ROOTCONFIG, but we only need to run once.

SYSCONFIG += \
  $(NEWLINE)sh libs/cssys/macosx/osxconf.sh>>config.tmp
  $(NEWLINE)echo override DO_ASM = $(DO_ASM)>>config.tmp

endif # ifeq ($(ROOTCONFIG),config)

ifeq ($(ROOTCONFIG),volatile)

MAKE_VOLATILE_H += \
  $(NEWLINE)echo $"\#define OS_MACOSX_DESCRIPTION "MacOS/X"$">>volatile.tmp \
  $(NEWLINE)echo $"\#define OS_MACOSX_PLUGIN_DIR "$(MACOSX.PLUGIN_DIR)/"$">>volatile.tmp \
  $(NEWLINE)echo $"\#define OS_MACOSX_PLUGIN_EXT "$(MACOSX.PLUGIN_EXT)"$">>volatile.tmp

endif # ifeq ($(ROOTCONFIG),volatile)
endif # ifeq ($(MAKESECTION),rootdefines)

#==============================================================================
# This is the system makefile for Apple MacOS/X.
# Copyright (C)1998-2004 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================

PLUGINS += video/canvas/macosx/cocoa
PLUGINS += video/canvas/macosx/coregraphics
ifeq ($(GL.AVAILABLE),yes)
PLUGINS += video/canvas/macosx/opengl
endif
PLUGINS += sound/driver/coreaudio

DESCRIPTION.macosx = MacOS/X
DESCRIPTION.OS.macosx = MacOS/X

# Avoid linker complaint about weak vs. non-weak frameworks.
# <cs-config>
export MACOSX_DEPLOYMENT_TARGET = 10.2
# </cs-config>

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Application wrapper support.
# <cs-config>
MACOSX.APP_EXE  = $@/Contents/MacOS/$(notdir $(basename $@))
MACOSX.APP_ICON = $(SRCDIR)/libs/csutil/macosx/appicon.icns
MACOSX.APP_DIR  = .
# </cs-config>

# Apple can not use the x86 assembly in CS.
override DO_ASM = no

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include $(SRCDIR)/mk/unix.mak

# Add support for Objective-C (.m) and Objective-C++ (.mm) source code.
.SUFFIXES: .m .mm

# How to compile an Objective-C .m source
DO.COMPILE.M = $(DO.OBJC) $(CFLAGS.@) $(<<) $(CFLAGS) $(CFLAGS.INCLUDE)

# How to compile an Objective-C++ .mm source
DO.COMPILE.MM = $(DO.OBJCXX) $(CFLAGS.@) $(<<) $(CFLAGS) $(CFLAGS.INCLUDE)

# Directories containing platform-specific source code.
MACOSX.SOURCE_PATHS = $(SRCDIR)/libs/csutil/macosx

# Select MacOS/X config file for inclusion with install.
TO_INSTALL.CONFIG += $(SRCDIR)/data/config/macosx.cfg

# Extension for applications on this system
EXE = .app

# Extension for dynamic libraries on this system.
DLL = .csbundle

# <cs-config>
# Extra libraries needed on this system.
LIBS.EXE =
# </cs-config>

# <cs-config>
# Extra libraries needed only for executables (not plug-ins)
LIBS.EXE.PLATFORM =
# </cs-config>

# Indicate where special include files can be found.
CFLAGS.INCLUDE = $(addprefix $(CFLAGS.I),$(MACOSX.SOURCE_PATHS))

# General flags for the compiler which are used in any case.  The "config"
# flags are determined at configuration time and come from CS/config.mak.
# <cs-config>
CFLAGS.GENERAL = \
  $(MACOSX.CFLAGS.CONFIG) \
  $(CFLAGS.SYSTEM) \
  $(CSTHREAD.CFLAGS)
# </cs-config>

# Special option for the software 3D renderer to force it to ARGB mode
CFLAGS.PIXEL_LAYOUT = -DCS_24BIT_PIXEL_LAYOUT=CS_24BIT_PIXEL_ARGB

# Flags for the compiler which are used when profiling.
CFLAGS.profile = -pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL =

# General flags for the linker which are used in any case.  The "config" flags
# are determined at configuration time and come from CS/config.mak.
# <cs-config>
LFLAGS.GENERAL = $(LFLAGS.SYSTEM) $(MACOSX.LFLAGS.CONFIG) $(CSTHREAD.LFLAGS)
# </cs-config>

# Flags for the linker which are used when profiling.
LFLAGS.profile = -pg

# Flags for the linker which are used when building a graphical executable.
# <cs-config>
LFLAGS.EXE = -framework AppKit -framework Foundation
# </cs-config>

# Flags for the linker which are used when building a console executable.
# <cs-config>
LFLAGS.CONSOLE.EXE = -framework AppKit -framework Foundation
# </cs-config>

# Flags for the linker which are used when building a shared library.
# <cs-config>
LFLAGS.DLL = -bundle -framework AppKit -framework Foundation
# </cs-config>

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM =

# System dependent source files included into csutil library
SRC.SYS_CSUTIL = $(wildcard \
  $(addsuffix /*.cpp,$(MACOSX.SOURCE_PATHS)) \
  $(addsuffix /*.c,$(MACOSX.SOURCE_PATHS)) \
  $(addsuffix /*.m,$(MACOSX.SOURCE_PATHS)) \
  $(addsuffix /*.mm,$(MACOSX.SOURCE_PATHS))) \
  $(SRCDIR)/libs/csutil/generic/apppath.cpp \
  $(SRCDIR)/libs/csutil/generic/csprocessorcap.cpp \
  $(SRCDIR)/libs/csutil/generic/findlib.cpp \
  $(SRCDIR)/libs/csutil/generic/getopt.cpp \
  $(SRCDIR)/libs/csutil/generic/pathutil.cpp \
  $(SRCDIR)/libs/csutil/generic/pluginpaths.cpp \
  $(SRCDIR)/libs/csutil/generic/printf.cpp \
  $(SRCDIR)/libs/csutil/generic/scanplugins.cpp \
  $(SRCDIR)/libs/csutil/generic/sysroot.cpp \
  $(CSTHREAD.SRC)
INC.SYS_CSUTIL = \
  $(wildcard $(addsuffix /*.h,$(MACOSX.SOURCE_PATHS))) $(CSTHREAD.INC)

# Where to put dynamic libraries on this system?
OUTDLL =

# The library (archive) manager
# <cs-config>
AR = libtool
ARFLAGS = -static -o
# </cs-config>

# MacOS/X Panther expects us to use the -s option with ranlib.
# <cs-config>
ifeq (,$(RANLIB))
RANLIB = ranlib
endif
RANLIB += -s
# </cs-config>

# The stripper :-)
STRIP = strip

# We don't need separate directories for dynamic libraries
OUTSUFX.yes =

# Extra libraries needed for Python cspace module.
PYTHMOD.LFLAGS.PLATFORM = -framework AppKit -framework Foundation

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Add support for Objective-C (.m) source code.
$(OUT)/%$O: %.m
	$(DO.COMPILE.M)

# Add support for Objective-C++ (.mm) source code.
$(OUT)/%$O: %.mm
	$(DO.COMPILE.MM)

OBJ.SYS_CSUTIL = $(addprefix $(OUT)/,$(notdir $(subst .s,$O,$(subst .c,$O,\
  $(subst .cpp,$O,$(subst .mm,$O,$(SRC.SYS_CSUTIL:.m=$O)))))))

vpath %.m  $(SRCDIR)/libs/csutil $(filter-out $(SRCDIR)/libs/csutil/generic/,$(sort $(dir $(SRC.SYS_CSUTIL)))) $(SRCDIR)/libs/csutil/generic
vpath %.mm $(SRCDIR)/libs/csutil $(filter-out $(SRCDIR)/libs/csutil/generic/,$(sort $(dir $(SRC.SYS_CSUTIL)))) $(SRCDIR)/libs/csutil/generic

# Override default method of creating a GUI application.  For Cocoa, we need
# to place the executable inside an application wrapper.
# <cs-config>
define DO.LINK.EXE
  sh $(SRCDIR)/libs/csutil/macosx/appwrap.sh $(notdir $(basename $@)) $(MACOSX.APP_DIR) $(MACOSX.APP_ICON) $(SRCDIR)/include/csver.h
  $(NEWLINE)$(LINK) $(LFLAGS) $(LFLAGS.EXE) -o $(MACOSX.APP_EXE) $(^^) $(L^) $(LIBS)
  touch $@
endef
# </cs-config>

endif # ifeq ($(MAKESECTION),postdefines)

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
MACOSX.APP_ICON = $(SRCDIR)/$(DIR.CSUTIL)/macosx/appicon.icns
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
CFLAGS.INCLUDE = $(CFLAGS.I)$(SRCDIR)/$(DIR.CSUTIL)/macosx

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
  $(SRCDIR)/$(DIR.CSUTIL)/macosx/*.cpp \
  $(SRCDIR)/$(DIR.CSUTIL)/macosx/*.c \
  $(SRCDIR)/$(DIR.CSUTIL)/macosx/*.m \
  $(SRCDIR)/$(DIR.CSUTIL)/macosx/*.mm) \
  $(SRCDIR)/$(DIR.CSUTIL)/generic/apppath.cpp \
  $(SRCDIR)/$(DIR.CSUTIL)/generic/csprocessorcap.cpp \
  $(SRCDIR)/$(DIR.CSUTIL)/generic/findlib.cpp \
  $(SRCDIR)/$(DIR.CSUTIL)/generic/getopt.cpp \
  $(SRCDIR)/$(DIR.CSUTIL)/generic/pathutil.cpp \
  $(SRCDIR)/$(DIR.CSUTIL)/generic/pluginpaths.cpp \
  $(SRCDIR)/$(DIR.CSUTIL)/generic/printf.cpp \
  $(SRCDIR)/$(DIR.CSUTIL)/generic/scanplugins.cpp \
  $(SRCDIR)/$(DIR.CSUTIL)/generic/sysroot.cpp \
  $(CSTHREAD.SRC)
INC.SYS_CSUTIL = $(wildcard \
  $(SRCDIR)/$(DIR.CSUTIL)/macosx/*.h $(SRCDIR)/include/csutil/macosx/*.h) \
  $(CSTHREAD.INC)

# Where to put dynamic libraries on this system?
OUTDLL =

# The library (archive) manager
# <cs-config>
ifneq (,$(APPLE_LIBTOOL))
AR = $(APPLE_LIBTOOL)
else
AR = /usr/bin/libtool
endif
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

ifndef DIR.CSUTIL
DIR.CSUTIL = libs/csutil
endif
ifndef OUT.CSUTIL
OUT.CSUTIL = $(OUT)/$(DIR.CSUTIL)
endif

# Add support for Objective-C (.m) source code.
$(OUT.CSUTIL)/%$O: $(SRCDIR)/$(DIR.CSUTIL)/macosx/%.m
	$(DO.COMPILE.M)

$(OUT)/%$O: %.m
	$(DO.COMPILE.M)

# Add support for Objective-C++ (.mm) source code.
$(OUT.CSUTIL)/%$O: $(SRCDIR)/$(DIR.CSUTIL)/macosx/%.mm
	$(DO.COMPILE.MM)

$(OUT)/%$O: %.mm
	$(DO.COMPILE.MM)

# Add support C and C++ in platform-specific directory.
$(OUT.CSUTIL)/%$O: $(SRCDIR)/$(DIR.CSUTIL)/macosx/%.c
	$(DO.COMPILE.C)

$(OUT.CSUTIL)/%$O: $(SRCDIR)/$(DIR.CSUTIL)/macosx/%.cpp
	$(DO.COMPILE.CPP)

# Override the default setting of OBJ.SYS_CSUTIL since we must include object
# files from Objective-C and Objective-C++ sources.
OBJ.SYS_CSUTIL = $(addprefix $(OUT.CSUTIL)/,$(notdir $(subst .s,$O,\
  $(subst .c,$O,$(subst .cpp,$O,$(subst .mm,$O,$(SRC.SYS_CSUTIL:.m=$O)))))))

# Override default method of creating a GUI application.  For Cocoa, we need
# to place the executable inside an application wrapper.
# <cs-config>
define DO.LINK.EXE
  sh $(SRCDIR)/$(DIR.CSUTIL)/macosx/appwrap.sh $(notdir $(basename $@)) $(MACOSX.APP_DIR) $(MACOSX.APP_ICON) $(SRCDIR)/include/csver.h
  $(NEWLINE)$(LINK) $(LFLAGS) $(LFLAGS.EXE) -o $(MACOSX.APP_EXE) $(^^) $(L^) $(LIBS)
  touch $@
endef
# </cs-config>

endif # ifeq ($(MAKESECTION),postdefines)

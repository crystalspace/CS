#==============================================================================
# This is the system makefile for MacOS/X Server, OpenStep, and NextStep.
# Copyright (C)1998-2001 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================

# Only one of the cover makefiles should be including this file. Ignore others.
ifeq ($(NEXT.FRIEND),yes)

# Choose which drivers you want to build/use
PLUGINS += $(NEXT.PLUGINS) video/canvas/next

ifneq ($(NEXT.TARGET),)
  DESCRIPTION.$(NEXT.TARGET) := $(NEXT.DESCRIPTION)
  DESCRIPTION.OS.$(NEXT.TARGET) = $(NEXT.DESCRIPTION.OS)
endif

#----------------------------------------- rootdefines, defines, config ------#
ifneq ($(findstring defines,$(MAKESECTION))$(findstring config,$(ROOTCONFIG)),)

PROC.m68k  = M68K
PROC.i386  = X86
PROC.sparc = SPARC
PROC.hppa  = HPPA
PROC.ppc   = POWERPC

NEXT.TARGET_ARCHS := $(sort $(filter $(NEXT.ARCHS),$(TARGET_ARCHS)))

ifeq ($(strip $(NEXT.TARGET_ARCHS)),)
NEXT.TARGET_ARCHS := $(shell /usr/bin/arch)
endif

endif # MAKESECTION is rootdefines, defines; ROOTCONFIG is config

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

SYSMODIFIERS += $(NEWLINE)echo $"  TARGET_ARCHS="$(NEXT.TARGET_ARCHS)"$"

endif # ifeq ($(MAKESECTION),rootdefines)

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor. Can be one of: X86, SPARC, POWERPC, M68K, HPPA, UNKNOWN
# May use TARGET_ARCHS to specify multiple architectures at once.
# Ex. TARGET_ARCHS="m68k i386 sparc hppa"
PROC = $(subst $(SPACE),_,$(foreach arch,$(NEXT.TARGET_ARCHS),$(PROC.$(arch))))

# Operating system.
OS = NEXT

# Operating system family: UNIX (for Unix or Unix-like platforms), WIN32, etc.
OS_FAMILY = UNIX

# Compiler. Can be one of: GCC, MPWERKS, VC (Visual C++), UNKNOWN
COMP = GCC

# Plug-in component support.
NEXT.PLUGIN_DIR = components/
NEXT.PLUGIN_EXT = .csplugin

# None of the NeXT platforms can grok the assembly used by CS
override DO_ASM = no

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/unix.mak

# Add support for Objective-C (.m) source code.
.SUFFIXES: .m

# How to compile a .m source
DO.COMPILE.M = $(OBJC) $(CFLAGS.@) $(<<) $(CFLAGS) $(CFLAGS.INCLUDE)

# Multi-architecture binary (MAB) support.
NEXT.ARCH_FLAGS = $(foreach arch,$(NEXT.TARGET_ARCHS),-arch $(arch))

# Select appropriate source directories (macosxs, openstep, nextstep, shared).
# NOTE: List "shared" directory last so search proceeds: specific -> general.
NEXT.SHARED = shared
NEXT.SEARCH_PATH = $(NEXT.SOURCE_DIRS) $(NEXT.SHARED)
NEXT.SOURCE_PATHS = $(addprefix libs/cssys/next/,$(NEXT.SEARCH_PATH))

# Select next config file for inclusion with install.
TO_INSTALL.CONFIG += data/config/next.cfg

# Typical extension for dynamic libraries on this system.
DLL = $(NEXT.PLUGIN_EXT)

# Extra libraries needed on this system.
LIBS.EXE =

# Extra libraries needed only for executables (not plug-ins)
LIBS.EXE.PLATFORM = $(NEXT.LIBS)

# Socket library
LIBS.SOCKET.SYSTEM =

# Where can the Zlib library be found on this system?
Z_LIBS = $(LFLAGS.l)z

# Where can the PNG library be found on this system?
PNG_LIBS = $(LFLAGS.l)png

# Where can the JPG library be found on this system?
JPG_LIBS = $(LFLAGS.l)jpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS =

# Indicate where special include files can be found.
CFLAGS.INCLUDE = $(NEXT.INCLUDE_DIRS) \
  $(addprefix $(CFLAGS.I),$(NEXT.SOURCE_PATHS)) \
  $(CFLAGS.I)libs/zlib $(CFLAGS.I)libs/libpng $(CFLAGS.I)libs/libjpeg

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL = $(NEXT.CFLAGS.GENERAL) $(NEXT.ARCH_FLAGS) \
  -Wno-precomp -fno-common -pipe

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize = $(NEXT.CFLAGS.OPTIMIZE)

# Flags for the compiler which are used when debugging.
CFLAGS.debug = -g $(NEXT.CFLAGS.DEBUG)

# Flags for the compiler which are used when profiling.
CFLAGS.profile = -pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL = $(NEXT.CFLAGS.DLL)

# General flags for the linker which are used in any case.
LFLAGS.GENERAL = $(NEXT.ARCH_FLAGS) $(NEXT.LFLAGS.GENERAL) \
  $(LFLAGS.L)libs/zlib $(LFLAGS.L)libs/libpng $(LFLAGS.L)libs/libjpeg

# Flags for the linker which are used when debugging.
LFLAGS.debug = -g

# Flags for the linker which are used when profiling.
LFLAGS.profile = -pg

# Flags for the linker which are used when building a graphical executable.
LFLAGS.EXE = $(NEXT.LFLAGS.EXE)

# Flags for the linker which are used when building a console executable.
LFLAGS.CONSOLE.EXE = $(NEXT.LFLAGS.EXE)

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL = $(NEXT.LFLAGS.DLL)

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM =

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = $(wildcard \
  $(addsuffix /*.cpp,$(NEXT.SOURCE_PATHS)) \
  $(addsuffix /*.c,$(NEXT.SOURCE_PATHS))) \
  $(addsuffix /*.m,$(NEXT.SOURCE_PATHS))) \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/instpath.cpp \
  libs/cssys/general/getopt.cpp \
  libs/cssys/general/printf.cpp

# Where to put dynamic libraries on this system?
OUTDLL = $(NEXT.PLUGIN_DIR)

# The C compiler.
CC = cc -c

# The C++ compiler.
CXX = cc -c

# The Objective-C compiler.
OBJC = cc -c

# The linker.
LINK = cc

# The library (archive) manager
AR = libtool
ARFLAGS = -static -o

# The stripper :-)
STRIP = strip

# We don't need separate directories for dynamic libraries
OUTSUFX.yes =

endif # ifeq ($(MAKESECTION),defines)

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),postdefines)

# Add support for Objective-C (.m) source code.
$(OUT)%$O: %.m
	$(DO.COMPILE.M)

OBJ.CSSYS = $(addprefix $(OUT),$(notdir \
  $(subst .s,$O,$(subst .c,$O,$(subst .cpp,$O,$(SRC.CSSYS:.m=$O))))))

vpath %.m libs/cssys $(sort $(dir $(SRC.SYS_CSSYS)))

# Multiple -arch flags cause the compiler to barf when generating dependency
# information, so we filter out -arch commands.  This step is performed under
# `postdefines' rather than `defines' since we want to use the existing
# definition of DO.DEP1 as the basis of our augmented DO.DEP1.
DO.DEP1 := $(subst $(NEXT.ARCH_FLAGS),,$(DO.DEP1))

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND = \
  -e "s/\.cpp\.o \:/\.o\:/g" \
  -e "s/\.c\.o \:/\.o\:/g" \
  -e "s/\.m\.o \:/\.o\:/g"

endif # ifeq ($(MAKESECTION),postdefines)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

# This makefile can be included more than once, but help messages should be
# appended only once.
ifndef ALREADY_INCLUDED
ALREADY_INCLUDED = 1

SYSHELP += $(NEXT.SYSHELP)

# System-dependent help commands
SYSMODIFIERSHELP += \
  $(NEWLINE)echo $"  TARGET_ARCHS="$(sort $(NEXT.ARCHS.ALL))" $" \
  $(NEWLINE)echo $"      Specific to: $(strip $(NEXT.DESCRIPTION.ALL))$" \
  $(NEWLINE)echo $"      Target architectures to build.  If not specified, then the current$" \
  $(NEWLINE)echo $"      architecture is used.  Possible values are:$" \
  $(NEXT.ARCHS.HELP)

# Ensure that these variables are simply expanded by using :=
# This will cause += to also perform simple expansion, which is necessary
# since this makefile is included multiple times.
NEXT.SYSHELP :=
NEXT.ARCHS.HELP :=
NEXT.ARCHS.ALL :=

endif # ALREADY_INCLUDED

NEXT.SYSHELP += \
  $(NEWLINE)echo $"  make $(NEXT.TARGET.DESCRIPTION)     Prepare for building on $(DESCRIPTION.$(NEXT.TARGET))$"
NEXT.ARCHS.HELP += \
  $(NEWLINE)echo $"          $(NEXT.ARCHS)  [$(NEXT.DESCRIPTION)]$"
NEXT.ARCHS.ALL += $(NEXT.ARCHS)
NEXT.DESCRIPTION.ALL := $(NEXT.DESCRIPTION.ALL)$(NEXT.COMMA) $(NEXT.DESCRIPTION)
NEXT.COMMA = ,

endif # ifeq ($(MAKESECTION),confighelp)

#--------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION),rootdefines) # Makefile includes us twice with valid
ifeq ($(ROOTCONFIG),config)	  # ROOTCONFIG, but we only need to run once.

ifneq ($(strip $(TARGET_ARCHS)),)
  SYSCONFIG += $(NEWLINE)echo TARGET_ARCHS = $(NEXT.TARGET_ARCHS)>>config.tmp
endif
SYSCONFIG += \
  $(NEWLINE)bin/comptest.sh cc>>config.tmp \
  $(NEWLINE)bin/haspythn.sh>> config.tmp \
  $(NEWLINE)echo override DO_ASM = $(DO_ASM)>>config.tmp

endif # ifeq ($(ROOTCONFIG),config)

ifeq ($(ROOTCONFIG),volatile)

MAKE_VOLATILE_H += \
  $(NEWLINE)echo $"\#define OS_NEXT_$(NEXT.FLAVOR)$">>volatile.tmp \
  $(NEWLINE)echo $"\#define OS_NEXT_DESCRIPTION "$(NEXT.DESCRIPTION)"$">>volatile.tmp \
  $(NEWLINE)echo $"\#define OS_NEXT_PLUGIN_DIR "$(NEXT.PLUGIN_DIR)"$">>volatile.tmp \
  $(NEWLINE)echo $"\#define OS_NEXT_PLUGIN_EXT "$(NEXT.PLUGIN_EXT)"$">>volatile.tmp

endif # ifeq ($(ROOTCONFIG),volatile)
endif # ifeq ($(MAKESECTION),rootdefines)

endif # ifeq ($(NEXT.FRIEND),yes)

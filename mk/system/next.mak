# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Only one of the cover makefiles should be including this file.  Ignore others.
ifeq ($(NEXT.FRIEND),yes)

# We don't want assembly for now
override DO_ASM=no

# Choose which drivers you want to build/use
DRIVERS=cs2d/next cs3d/software csnetdrv/null csnetdrv/sockets \
  csnetman/null csnetman/simple cssnddrv/null cssndrdr/null

ifneq ($(NEXT.TARGET),)
DESCRIPTION.$(NEXT.TARGET):=$(NEXT.DESCRIPTION)
endif

#------------------------------------------ rootdefines, defines, config ------#
ifneq ($(findstring defines,$(MAKESECTION))$(findstring config,$(ROOTCONFIG)),)

PROC.m68k  = M68K
PROC.i386  = INTEL
PROC.sparc = SPARC
PROC.hppa  = HPPA
PROC.ppc   = POWERPC

NEXT.TARGET_ARCHS := $(sort $(filter $(NEXT.ARCHS),$(TARGET_ARCHS)))

ifeq ($(strip $(NEXT.TARGET_ARCHS)),)
NEXT.TARGET_ARCHS := $(shell /usr/bin/arch)
endif

endif # MAKESECTION is rootdefines, defines; ROOTCONFIG is config

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

SYSMODIFIERS=TARGET_ARCHS="$(NEXT.TARGET_ARCHS)"

endif # ifeq ($(MAKESECTION),rootdefines)

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor. Can be one of: INTEL, SPARC, POWERPC, M68K, HPPA, UNKNOWN
# May use TARGET_ARCHS to specify multiple architectures at once.
# Ex. TARGET_ARCHS="m68k i386 sparc hppa"

PROC=$(subst $(SPACE),_,$(foreach arch,$(NEXT.TARGET_ARCHS),$(PROC.$(arch))))

# Operating system. Can be one of: SOLARIS, LINUX, IRIX, BSD, UNIX, DOS, MACOS, AMIGAOS, WIN32, OS2, BE, NEXT
OS=NEXT

# Compiler. Can be one of: GCC, WCC (Watcom C++), MPWERKS, VC (Visual C++), UNKNOWN
COMP=GCC

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

# Multi-architecture binary (MAB) support.
NEXT.ARCH_FLAGS=$(foreach arch,$(NEXT.TARGET_ARCHS),-arch $(arch))

# Select appropriate source directories (macosxs, openstep, nextstep, shared).
# NOTE: List "shared" directory last so search proceeds: specific -> general.
NEXT.SHARED=shared
NEXT.SEARCH_PATH=$(NEXT.SOURCE_DIRS) $(NEXT.SHARED)
NEXT.SOURCE_PATHS=$(addprefix libs/cssys/next/,$(NEXT.SEARCH_PATH))

# Typical extension for executables on this system (e.g. EXE=.exe)
EXE=

# Typical extension for dynamic libraries on this system.
DLL=.so

# Typical extension for static libraries
LIB=.a

# Typical prefix for library filenames
LIB_PREFIX=lib

# Does this system require libsocket.a?
NEED_SOCKET_LIB=no

# Extra libraries needed on this system.
LIBS.EXE=$(NEXT.LIBS)

# Where can the Zlib library be found on this system?
Z_LIBS=$(LFLAGS.L)libs/zlib $(LFLAGS.l)z

# Where can the PNG library be found on this system?
PNG_LIBS=$(LFLAGS.L)libs/libpng $(LFLAGS.l)png

# Where can the JPG library be found on this system?
JPG_LIBS=$(LFLAGS.L)libs/libjpeg $(LFLAGS.l)jpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=$(NEXT.INCLUDE_DIRS) \
  $(addprefix $(CFLAGS.I),$(NEXT.SOURCE_PATHS)) \
  $(CFLAGS.I)libs/zlib $(CFLAGS.I)libs/libpng $(CFLAGS.I)libs/libjpeg

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=$(NEXT.CFLAGS.GENERAL) $(NEXT.ARCH_FLAGS) \
  -ObjC++ -fno-common -pipe

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-O4 -finline-functions

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g $(NEXT.CFLAGS.DEBUG)

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=$(NEXT.CFLAGS.DLL)

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=$(NEXT.ARCH_FLAGS) $(NEXT.LFLAGS.GENERAL)

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a graphical executable.
LFLAGS.EXE= $(NEXT.LFLAGS.EXE)

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL= $(NEXT.LFLAGS.DLL)

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = $(wildcard $(addsuffix /*.cpp,$(NEXT.SOURCE_PATHS))) \
  libs/cssys/general/printf.cpp

# Where to put the dynamic libraries on this system?
OUTDLL=

# The C compiler.
CC=cc -c

# The C++ compiler.
CXX=cc -c

# The linker.
LINK=cc

# The library (archive) manager
AR=libtool
ARFLAGS=-static -o

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
MKDIR=mkdir $(@:/=)

# The command to remove all specified files.
RM=rm -f

# The command to remove a directory tree.
RMDIR=rm -rf

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=

#==================================================
# Extra operation system dependent options.
#==================================================

# Include support for the XSHM extension in Crystal Space.
# Note that you don't need to disable this if you don't have XSHM
# support in your X server because Crystal Space will autodetect
# the availability of XSHM.
DO_SHM=no

# We don't need separate directories for dynamic libraries
OUTSUFX.yes=

endif # ifeq ($(MAKESECTION),defines)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Other platforms link all $(LIBS) into DLL, but doing so breaks DLLs on
# NeXT, so a custom link statement is required.
DO.SHARED.PLUGIN  = $(LINK) $(LFLAGS.DLL) $(LFLAGS.@) $(^^) $(LFLAGS)
DO.SHARED.LIBRARY = $(LINK) $(LFLAGS.DLL) $(LFLAGS.@) $(^^) $(LFLAGS)

endif # ifeq ($(MAKESECTION),postdefines)

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

# Since this makefile can be included more than once, disallow addition of
# help messages more than once.
ifndef ALREADY_INCLUDED
ALREADY_INCLUDED = 1

SYSHELP += $(NEXT.SYSHELP)

# System-dependent help commands
SYSMODIFIERSHELP += \
  $(NEWLINE)echo $"  TARGET_ARCHS="$(sort $(NEXT.ARCHS.ALL))" ($(strip $(NEXT.DESCRIPTION.ALL)))$" \
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
  $(NEWLINE)echo $"  make $(NEXT.TARGET.DESCRIPTION)     Prepare for building under and for $(DESCRIPTION.$(NEXT.TARGET))$"
NEXT.ARCHS.HELP += \
  $(NEWLINE)echo $"          $(NEXT.ARCHS)  [$(NEXT.DESCRIPTION)]$"
NEXT.ARCHS.ALL += $(NEXT.ARCHS)
NEXT.DESCRIPTION.ALL := $(NEXT.DESCRIPTION.ALL)$(NEXT.COMMA) $(NEXT.DESCRIPTION)
NEXT.COMMA = ,

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION),rootdefines) # Makefile includes us twice with valid
ifeq ($(ROOTCONFIG),config)	  # ROOTCONFIG, but we only need to run once.

SYSCONFIG += $(NEWLINE)bin/booltest.sh "cc -ObjC++" >> config.tmp
SYSCONFIG += $(NEWLINE)bin/haspythn.sh >> config.tmp
SYSCONFIG += $(NEWLINE)echo override DO_ASM = $(DO_ASM)>>config.tmp
ifneq ($(strip $(TARGET_ARCHS)),)
  SYSCONFIG += $(NEWLINE)echo TARGET_ARCHS = $(NEXT.TARGET_ARCHS)>>config.tmp
endif

endif # ifeq ($(ROOTCONFIG),config)

ifeq ($(ROOTCONFIG),volatile)

MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define OS_NEXT_$(NEXT.FLAVOR)$">>volatile.tmp
MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define OS_NEXT_DESCRIPTION "$(NEXT.DESCRIPTION)"$">>volatile.tmp
MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define CS_NO_MODULE_LOCKING$">>volatile.tmp
MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define USGISH$">>volatile.tmp

endif # ifeq ($(ROOTCONFIG),volatile)
endif # ifeq ($(MAKESECTION),rootdefines)

endif # ifeq ($(NEXT.FRIEND),yes)

# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Loadable modules not currently supported.
override USE_DLL=no

# Choose which drivers you want to build/use
DRIVERS=cs2d/next2d cs3d/software csnetdrv/null csnetdrv/sockets \
  csnetman/null csnetman/simple cssnddrv/null cssndrdr/null

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# System-dependent help commands
define SYSMODIFIERSHELP
  echo "  TARGET_ARCHS=\"$(NEXT.ARCHS)\""
  echo "      Target architectures to build.  Default if not specified is `/usr/bin/arch`."
  echo "      Possible values are: $(NEXT.ARCHS)"
endef

endif # ifeq ($(MAKESECTION),rootdefines)

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor. Can be one of: INTEL, SPARC, POWERPC, M68K, HPPA, UNKNOWN
# May use TARGET_ARCHS to specify multiple architectures at once.
# Ex. TARGET_ARCHS="m68k i386 sparc hppa"

PROC.m68k  = M68K
PROC.i386  = INTEL
PROC.sparc = SPARC
PROC.hppa  = HPPA
PROC.ppc   = POWERPC

NEXT.TARGET_ARCHS:=$(sort $(filter $(NEXT.ARCHS),$(TARGET_ARCHS)))

ifeq ($(strip $(NEXT.TARGET_ARCHS)),)
NEXT.TARGET_ARCHS:=$(shell /usr/bin/arch)
endif

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

# Select appropriate source directories (rhapsody, openstep, nextstep, shared).
# NOTE: List "shared" directory last so search proceeds: specific -> general.
NEXT.SHARED=shared
NEXT.SOURCE_PATHS=$(addprefix libs/cssys/next/,$(NEXT.SOURCE_DIRS) $(NEXT.SHARED))

# Typical extension for executables on this system (e.g. EXE=.exe)
EXE=

# Typical extension for dynamic libraries on this system.
DLL=.dl

# Typical extension for static libraries
LIB=.a

# Typical prefix for library filenames
LIB_PREFIX=lib

# Does this system require libsocket.a?
NEED_SOCKET_LIB=no

# Extra libraries needed on this system.
LIBS.EXE=$(NEXT.LIBS)

# Where can the Zlib library be found on this system?
Z_LIBS=-lz

# Where can the PNG library be found on this system?
PNG_LIBS=-lpng

# Where can the JPG library be found on this system?
JPG_LIBS=-ljpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=$(NEXT.INCLUDE_DIRS) $(addprefix -I,$(NEXT.SOURCE_PATHS)) \
  -Ilibs/zlib -Ilibs/libpng -Ilibs/libjpeg

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=$(NEXT.CFLAGS) $(NEXT.ARCH_FLAGS) \
  $(CFLAGS.D)OS_NEXT_$(NEXT.FLAVOR) \
  $(CFLAGS.D)OS_NEXT_DESCRIPTION='"$(NEXT.DESCRIPTION)"' \
  -ObjC++ -fno-common -pipe $(CFLAGS.D)NO_ASSEMBLER 

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-O4

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=-dynamic

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=$(NEXT.ARCH_FLAGS) -Llibs/zlib -Llibs/libpng -Llibs/libjpeg

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL= # FIXME

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = $(wildcard $(addsuffix /*.cpp,$(NEXT.SOURCE_PATHS))) \
  libs/cssys/next/shared/loadlib.cpp \
  support/gnu/getopt.c support/gnu/getopt1.c

# Where to put the dynamic libraries on this system?
OUTDLL=

# Does this OS have native COM support?
NATIVE_COM=no

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

#---------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION),configure)

.PHONY: configure
configure:

endif # ifeq ($(MAKESECTION),configure)

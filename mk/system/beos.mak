# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Friendly names for building environment
DESCRIPTION.beos = BeOS

# Choose which drivers you want to build/use
DRIVERS=cs3d/software cs2d/be cs2d/openglbe cs3d/opengl \
  csnetdrv/null csnetdrv/sockets csnetman/null csnetman/simple \
  cssnddrv/null cssndrdr/null
# Uncomment the following if you want to build/use Glide.
# DRIVERS+=cs2d/beglide2 cs3d/glide2

# We don't want assembly for now
override DO_ASM=no

# Until this is auto-detected, we assume it to be true.
override BUGGY_EGCS_COMPILER=yes

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor. Can be one of: INTEL, SPARC, POWERPC, M68K, UNKNOWN
PROC=INTEL

# Operating system. Can be one of: SOLARIS, LINUX, IRIX, BSD, UNIX, DOS, MACOS, AMIGAOS, WIN32, OS2, BE
OS=BE

# Compiler. Can be one of: GCC, WCC (Watcom C++), MPWERKS, VC (Visual C++), UNKNOWN
COMP=GCC

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
LIBS.EXE+=-lroot -lbe -lgame

# Where can the Zlib library be found on this system?
Z_LIBS=-lz

# Where can the PNG library be found on this system?
PNG_LIBS=-lpng

# Where can the JPG library be found on this system?
JPG_LIBS=-Llibs/libjpeg -ljpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=$(CFLAGS.I). $(CFLAGS.I)include $(CFLAGS.I)libs \
 $(CFLAGS.I)apps $(CFLAGS.I)support $(CFLAGS.I)libs/libjpeg

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-Wall -Wno-multichar -Wno-ctor-dtor-privacy 

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-O3 

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g -O0

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=

# Flags for the linker which are used when debugging.
LFLAGS.debug= -gdwarf-2

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL= -nostart

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS=$(wildcard libs/cssys/be/*.cpp) libs/cssys/general/printf.cpp

# Where to put dynamic libraries on this system?
OUTDLL=add-ons/

# The C compiler.
CC=gcc -c

# The C++ compiler.
CXX=gcc -c

# The linker.
LINK=gcc

# The 'make' executable. This should be the GNU make!
MAKE=make

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
MKDIR=mkdir '$(@:/=)'

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

# Defineds for OpenGL 3D driver
OPENGL.LIBS.DEFINED=1
CFLAGS.GL3D+=$(CFLAGS.I)/boot/develop/headers/be/opengl 
LIBS.LOCAL.GL3D+=$(LFLAGS.l)GL

endif # ifeq ($(MAKESECTION),defines)

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make beos         Prepare for building under and for $(DESCRIPTION.beos)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION)/$(ROOTCONFIG),rootdefines/config)

SYSCONFIG += $(NEWLINE)bin/haspythn.sh >> config.tmp
SYSCONFIG += $(NEWLINE)echo override DO_ASM = $(DO_ASM)>>config.tmp
SYSCONFIG += $(NEWLINE)echo override BUGGY_EGCS_COMPILER = $(BUGGY_EGCS_COMPILER)>>config.tmp

endif # rootdefines & config

#--------------------------------------------------------------- volatile.h ---#
ifeq ($(MAKESECTION)/$(ROOTCONFIG),rootdefines/volatile)

MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define NO_SOCKETS_SUPPORT$">>volatile.tmp
MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define USGISH$">>volatile.tmp

endif # rootdefines & volatile

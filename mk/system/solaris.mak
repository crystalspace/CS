# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Friendly names for building environment
DESCRIPTION.solaris = Solaris

# Choose which drivers you want to build/use
DRIVERS=cs2d/softx cs3d/software csnetdrv/null csnetdrv/sockets \
  csnetman/null csnetman/simple cssnddrv/null \
  cssndrdr/null cssndrdr/software

# uncomment the following to build GGI 2D driver
#DRIVERS+=cs2d/ggi
# uncomment the following to build OpenGL stuff
DRIVERS+=cs3d/opengl cs2d/openglx

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor is autodetected and written to config.mak
#PROC=SPARC

# Operating system. Can be one of: SOLARIS, LINUX, IRIX, BSD, UNIX, DOS, MACOS, AMIGAOS, WIN32, OS2, BE
OS=SOLARIS

# Compiler. Can be one of: GCC, WCC (Watcom C++), MPWERKS, VC (Visual C++), UNKNOWN
COMP=GCC

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

# Typical extension for executables on this system (e.g. EXE=.exe)
EXE=

# Typical extension for dynamic libraries on this system.
DLL=.so

# Typical extension for static libraries
LIB=.a

# Typical prefix for library filenames
LIB_PREFIX=lib

# Does this system require libsocket.a?
NEED_SOCKET_LIB=yes

# Extra libraries needed on this system.
LIBS.EXE=-ldl -lnsl -lm

# Where can the Zlib library be found on this system?
Z_LIBS=-Llibs/zlib -lz

# Where can the PNG library be found on this system?
PNG_LIBS=-Llibs/libpng -lpng

# Where can the JPG library be found on this system?
JPG_LIBS=-Llibs/libjpeg -ljpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-Ilibs/zlib -Ilibs/libpng -Ilibs/libjpeg

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-Wall $(CFLAGS.SYSTEM)

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-O -fomit-frame-pointer

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g3 -gstabs

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g3

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a shared library.
#LFLAGS.DLL=-Wl,-G -nostdlib -lgcc -lc
LFLAGS.DLL=-Wl,-G

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS=libs/cssys/unix/unix.cpp libs/cssys/unix/loadlib.cpp \
  libs/cssys/general/printf.cpp libs/cssys/unix/utiming.cpp \
  libs/cssys/general/fopen.cpp support/gnu/getopt.c support/gnu/getopt1.c

# Where to put the dynamic libraries on this system?
OUTDLL=

# The C compiler.
CC=gcc -c

# The C++ compiler.
CXX=gcc -c

# The linker.
LINK=gcc

# The library (archive) manager
AR=ar
ARFLAGS=cr

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

# If we don't use -fpic we don't need separate output directories
ifeq ($(CFLAGS.DLL),)
OUTSUFX.yes=
endif

#==================================================
# Extra operation system dependent options.
#==================================================

# Include support for the XSHM extension in Crystal Space.
# Note that you don't need to disable this if you don't have XSHM
# support in your X server because Crystal Space will autodetect
# the availability of XSHM.
DO_SHM=yes

endif # ifeq ($(MAKESECTION),defines)

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make solaris      Prepare for building under and for $(DESCRIPTION.solaris)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure ---#
ifeq ($(ROOTCONFIG),config)

SYSCONFIG=bin/unixconf.sh solaris >>config.tmp

endif # ifeq ($(ROOTCONFIG),config)

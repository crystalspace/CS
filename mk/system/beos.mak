# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Choose which drivers you want to build/use
DRIVERS=cs3d/software cs2d/be2d csnetdrv/null csnetman/null \
  cssnddrv/null cssndrdr/null
# Uncomment the following if you want to build/use OpenGL
# DRIVERS+=cs3d/opengl cs2d/openglbe

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
JPG_LIBS=-ljpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE= -I/boot/develop/headers/be/opengl $(CFLAGS.I). \
 $(CFLAGS.I)./include $(CFLAGS.I)./libs $(CFLAGS.I)./apps $(CFLAGS.I)./support

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
SRC.SYS_CSSYS=libs/cssys/be/csbe.cpp libs/cssys/be/loadlib.cpp \
 libs/cssys/general/printf.cpp libs/cssys/general/fopen.cpp libs/cssys/be/loadlib.cpp

# Does this OS have native COM support?
NATIVE_COM=no

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

endif # ifeq ($(MAKESECTION),defines)

#---------------------------------------------------------------- configure --
ifeq ($(MAKESECTION),configure)

configure:

endif # ifeq ($(MAKESECTION),configure)


# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Friendly names for building environment
DESCRIPTION.solaris = Solaris
DESCRIPTION.OS.solaris = Solaris

# Choose which 2D/3D driver combinations you want to build/use
PLUGINS+=video/renderer/software video/canvas/softx
PLUGINS+=video/renderer/opengl video/canvas/openglx
PLUGINS+=video/canvas/linex

# Uncomment some of the following if you have a special MESA version that uses
# some of the following hardware/software renderers. Also set the entry Driver
# in section Display of opengl.cfg
#PLUGINS+=video/canvas/openglx/glide
#PLUGINS+=video/canvas/openglx/svga
#PLUGINS+=video/canvas/openglx/empty

# Uncomment the following to build GGI 2D driver
#PLUGINS+=video/canvas/ggi

# Uncomment the following to build sound renderer
#PLUGINS+=sound/renderer/software

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor is autodetected and written to config.mak
#PROC=SPARC

# Operating system. Can be one of:
# NEXT, SOLARIS, LINUX, IRIX, BSD, UNIX, DOS, MACOS, WIN32, OS2, BE
OS=SOLARIS

# Operating system family: UNIX (for Unix or Unix-like platforms), WIN32, etc.
OS_FAMILY=UNIX

# Compiler. Can be one of: GCC, MPWERKS, VC (Visual C++), UNKNOWN
COMP=GCC

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/unix.mak

# Does this system require libsocket.a?
NEED_SOCKET_LIB=yes

# Extra libraries needed on this system.
LIBS.EXE=-ldl -lnsl -lm

# Where can the Zlib library be found on this system?
Z_LIBS=-L/usr/local/lib -Llibs/zlib -lz

# Where can the PNG library be found on this system?
PNG_LIBS=-L/usr/local/lib -Llibs/libpng -lpng

# Where can the JPG library be found on this system?
JPG_LIBS=-L/usr/local/lib -Llibs/libjpeg -ljpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-I/usr/local/include -Ilibs/zlib -Ilibs/libpng -Ilibs/libjpeg

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
SRC.SYS_CSSYS=libs/cssys/unix/unix.cpp libs/cssys/unix/utiming.cpp \
  libs/cssys/unix/loadlib.cpp libs/cssys/general/findlib.cpp \
  libs/cssys/general/printf.cpp libs/cssys/general/getopt.cpp

# The C compiler.
#CC=gcc -c

# The C++ compiler.
#CXX=gcc -c

# The linker.
#LINK=gcc

# Use makedep to build dependencies
DEPEND_TOOL=mkdep

endif # ifeq ($(MAKESECTION),defines)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make solaris      Prepare for building on $(DESCRIPTION.solaris)$"

endif # ifeq ($(MAKESECTION),confighelp)

#--------------------------------------------------------------- configure ---#
ifeq ($(ROOTCONFIG),config)

SYSCONFIG=bin/unixconf.sh solaris >>config.tmp

endif # ifeq ($(ROOTCONFIG),config)

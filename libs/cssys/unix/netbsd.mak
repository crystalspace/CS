# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Friendly names for building environment
DESCRIPTION.netbsd = NetBSD
DESCRIPTION.OS.netbsd = NetBSD

# Choose which drivers you want to build/use
PLUGINS+=video/canvas/softx sound/renderer/software sound/driver/oss
PLUGINS+=video/renderer/opengl video/canvas/openglx
PLUGINS+=video/canvas/linex

# The X-Window plugin
PLUGINS+=video/canvas/xwindow
# Shared Memory Plugin
PLUGINS+=video/canvas/xextshm
# Video Modes Plugin
PLUGINS+=video/canvas/xextf86vm

# video support
# formats (this is the wrapping format for the video data)
# Microsofts AVI
PLUGINS+=video/format/avi
# CODECS (some formats are dynamic, that is they need codecs to encod/decode
# data) OpenDivX : you need an additional library you can get from
# www.projectmayo.com
#PLUGINS+=video/format/codecs/opendivx                                                                  
PLUGINS+=video/format/codecs/rle                                                                  

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor. Can be one of: X86, SPARC, POWERPC, M68K, UNKNOWN
PROC=X86

# Operating system. Can be one of:
# NEXT, SOLARIS, LINUX, IRIX, BSD, UNIX, DOS, MACOS, WIN32, OS2, BE
OS=BSD

# Operating system family: UNIX (for Unix or Unix-like platforms), WIN32, etc.
OS_FAMILY=UNIX

# Compiler. Can be one of: GCC, MPWERKS, VC (Visual C++), UNKNOWN
COMP=GCC

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/unix.mak

# Extra libraries needed on this system.
LIBS.EXE+=$(LFLAGS.L)/usr/pkg/lib $(LFLAGS.l)m

# Socket library
LIBS.SOCKET.SYSTEM=

# Where can the Zlib library be found on this system?
Z_LIBS=$(LFLAGS.l)z

# Where can the PNG library be found on this system?
PNG_LIBS=$(LFLAGS.l)png

# Where can the JPG library be found on this system?
JPG_LIBS=$(LFLAGS.l)jpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=$(CFLAGS.I)/usr/pkg/include $(CFLAGS.I)/usr/include

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-Wall

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-O6 -fomit-frame-pointer -malign-loops=2 -malign-jumps=2 \
  -malign-functions=2 -ffast-math

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g3 -ggdb

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=-Wl,-E

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g3

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL=-Wl,-shared

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f elf 

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = \
  libs/cssys/unix/unix.cpp \
  libs/cssys/unix/utiming.cpp \
  libs/cssys/unix/loadlib.cpp \
  libs/cssys/unix/instpath.cpp \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/getopt.cpp \
  libs/cssys/general/printf.cpp \
  libs/cssys/general/runloop.cpp \
  libs/cssys/general/sysinit.cpp

# The C compiler.
#CC=gcc -c

# The C++ compiler.
#CXX=g++ -c

# The linker.
#LINK=gcc

# Use makedep to build dependencies
DEPEND_TOOL=mkdep

endif # ifeq ($(MAKESECTION),defines)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make netbsd       Prepare for building on $(DESCRIPTION.netbsd)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure --#
ifeq ($(MAKESECTION),rootdefines) # Makefile includes us twice with valid
ifeq ($(ROOTCONFIG),config)	  # ROOTCONFIG, but we only need to run once.

SYSCONFIG += $(NEWLINE)bin/unixconf.sh netbsd>>config.tmp

endif # ifeq ($(ROOTCONFIG),config)

ifeq ($(ROOTCONFIG),volatile)

MAKE_VOLATILE_H += \
  $(NEWLINE)echo $"\#define CS_UNIX_PLUGIN_REQUIRES_MAIN$">>volatile.tmp

endif # ifeq ($(ROOTCONFIG),volatile)
endif # ifeq ($(MAKESECTION),rootdefines)

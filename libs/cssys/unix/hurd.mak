# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Friendly names for building environment
DESCRIPTION.hurd = Debian GNU/Hurd
DESCRIPTION.OS.hurd = Debian GNU/Hurd

ifeq ($(X11.AVAILABLE),yes)
  PLUGINS+=video/canvas/softx
  PLUGINS+=video/canvas/linex
  ifeq ($(GL.AVAILABLE),yes)
    PLUGINS+=video/canvas/openglx
  endif

  # The X-Window plugin
  PLUGINS+=video/canvas/xwindow
  # Shared Memory Plugin
  PLUGINS+=video/canvas/xextshm
  # Video Modes Plugin
  PLUGINS+=video/canvas/xextf86vm
endif

# video support
# formats (this is the wrapping format for the video data)
# Microsofts AVI
PLUGINS+=video/format/avi
# CODECS (some formats are dynamic, that is they need codecs to encod/decode
# data) OpenDivX : you need an additional library you can get from
# www.projectmayo.com
#PLUGINS+=video/format/codecs/opendivx

# Uncomment the following to build SDL 2D driver
#PLUGINS+=video/canvas/sdl

# Uncomment the following to build sound drivers
#PLUGINS+=sound/driver/oss sound/renderer/software

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor is autodetected and written to config.mak
#PROC=

# Operating system. Can be one of:
# MACOSX, SOLARIS, LINUX, IRIX, BSD, UNIX, WIN32
OS=HURD

# Operating system family: UNIX (for Unix or Unix-like platforms), WIN32, etc.
OS_FAMILY=UNIX

# Compiler. Can be one of: GCC, MPWERKS, VC (Visual C++), UNKNOWN
COMP=GCC

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/unix.mak

# Extra libraries needed on this system.
LIBS.EXE+=$(LFLAGS.l)dl $(LFLAGS.l)m

# Indicate where special include files can be found.
CFLAGS.INCLUDE=

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=$(CFLAGS.SYSTEM) $(CSTHREAD.CFLAGS)

# Flags for the compiler which are used when optimizing.
ifeq ($(PROC),X86)
  CFLAGS.optimize=-O6 -fomit-frame-pointer -ffast-math
else
  CFLAGS.optimize=-O -fomit-frame-pointer
endif

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g3 -gstabs

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=$(CSTHREAD.LFLAGS)

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g3

LFLAGS.optimize=-s

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a shared library.
#LFLAGS.DLL=-Wl,-shared -nostdlib $(LFLAGS.l)gcc
LFLAGS.DLL=-Wl,-shared

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f elf

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS= $(wildcard libs/cssys/unix/*.cpp) \
  libs/cssys/general/sysroot.cpp \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/getopt.cpp \
  libs/cssys/general/printf.cpp \
  libs/cssys/general/runloop.cpp \
  libs/cssys/general/sysinit.cpp \
  $(CSTHREAD.SRC)

# Use makedep to build dependencies
DEPEND_TOOL=mkdep

endif # ifeq ($(MAKESECTION),defines)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make hurd         Prepare for building on $(DESCRIPTION.hurd)$"

endif # ifeq ($(MAKESECTION),confighelp)

#--------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION),rootdefines) # Makefile includes us twice with valid
ifeq ($(ROOTCONFIG),config)	  # ROOTCONFIG, but we only need to run once.

SYSCONFIG += $(NEWLINE)sh libs/cssys/unix/unixconf.sh hurd $(INSTALL_DIR)>>config.tmp

endif # ifeq ($(ROOTCONFIG),config)

ifeq ($(ROOTCONFIG),volatile)

MAKE_VOLATILE_H += \
  $(NEWLINE)echo $"\#define CS_UNIX_PLUGIN_REQUIRES_MAIN$">>volatile.tmp

endif # ifeq ($(ROOTCONFIG),volatile)
endif # ifeq ($(MAKESECTION),rootdefines)

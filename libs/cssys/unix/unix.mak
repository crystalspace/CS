# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Friendly names for building environment
DESCRIPTION.unix = Unix
DESCRIPTION.OS.unix = Unix

# Choose the 2D/3D drivers you want to use.
ifeq ($(SVGA.AVAILABLE),yes)
PLUGINS += video/canvas/svgalib
endif

ifeq ($(X11.AVAILABLE),yes)
  PLUGINS += video/canvas/softx
  PLUGINS += video/canvas/linex
  ifeq ($(GL.AVAILABLE),yes)
    PLUGINS += video/canvas/openglx
    PLUGINS += video/renderer/opengl-ext
  endif

  # The X-Window plugin
  PLUGINS += video/canvas/xwindow
  # Shared Memory Plugin
  PLUGINS += video/canvas/xextshm
  # Video Modes Plugin
  PLUGINS += video/canvas/xextf86vm
endif

# Video support.
# Formats (this is the wrapping format for the video data).
# Microsofts AVI
PLUGINS += video/format/avi
# CODECS (some formats are dynamic, that is they need codecs to encode/decode
# data) OpenDivX: you need an additional library you can get from
# www.projectmayo.com
#PLUGINS += video/format/codecs/opendivx
#PLUGINS += video/format/codecs/divx4
PLUGINS += video/format/codecs/rle

# Uncomment the following to build SDL 2D canvas
#PLUGINS + =video/canvas/sdl

# Sound drivers.
PLUGINS += sound/driver/oss sound/renderer/software

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Operating system. Can be one of:
# MACOSX, UNIX, WIN32
OS = UNIX

# Operating system family: UNIX (for Unix or Unix-like platforms), WIN32, etc.
OS_FAMILY = UNIX

# Compiler. Can be one of: GCC, VC (Visual C++), UNKNOWN
COMP = GCC

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/unix.mak

# Extra libraries needed on this system.
ifndef LIBS.EXE
  LIBS.EXE += $(LFLAGS.l)dl $(LFLAGS.l)m 
endif

# Indicate where special include files can be found.
ifndef CFLAGS.INCLUDE
  CFLAGS.INCLUDE =
endif

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL = $(CFLAGS.SYSTEM) $(CSTHREAD.CFLAGS)

# Flags for the compiler which are used when optimizing.
ifndef CFLAGS.optimize
  CFLAGS.optimize = -O3 -fomit-frame-pointer
endif

# Flags for the compiler which are used when debugging.
ifndef CFLAGS.debug
  CFLAGS.debug = -g3
endif

# Flags for the compiler which are used when profiling.
ifndef CFLAGS.profile
  CFLAGS.profile = -pg -O -g
endif

# Flags for the compiler which are used when building a shared library.
ifndef CFLAGS.DLL
  CFLAGS.DLL = $(CFLAGS.D)CS_UNIX_PLUGIN_REQUIRES_MAIN
endif

# General flags for the linker which are used in any case.
LFLAGS.GENERAL = $(LFLAGS.L)/usr/local/lib $(CSTHREAD.LFLAGS)

# Flags for the linker which are used when debugging.
ifndef LFLAGS.debug
  LFLAGS.debug = -g3
endif

# Flags for the linker which are used in optimize mode.
ifdef LFLAGS.optimize
 LFLAGS.optimize =
endif

# Flags for the linker which are used when profiling.
ifndef LFLAGS.profile
LFLAGS.profile = -pg
endif

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL = -Wl,-shared -Wl,-soname -Wl,$@

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM = -f elf

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS= $(wildcard libs/cssys/unix/*.cpp) \
  libs/cssys/general/sysroot.cpp \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/getopt.cpp \
  libs/cssys/general/printf.cpp \
  libs/cssys/general/runloop.cpp \
  libs/cssys/general/sysinit.cpp \
  $(CSTHREAD.SRC)
INC.SYS_CSSYS = $(wildcard libs/cssys/unix/*.h) $(CSTHREAD.INC)

# Use makedep to build dependencies
DEPEND_TOOL = mkdep

endif # ifeq ($(MAKESECTION),defines)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make unix         Prepare for building on $(DESCRIPTION.unix)$"

endif # ifeq ($(MAKESECTION),confighelp)

#--------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION),rootdefines) # Makefile includes us twice with valid
ifeq ($(ROOTCONFIG),config)	  # ROOTCONFIG, but we only need to run once.

SYSCONFIG += $(NEWLINE)sh libs/cssys/unix/unixconf.sh unix $(INSTALL_DIR)>>config.tmp

endif # ifeq ($(ROOTCONFIG),config)
endif # ifeq ($(MAKESECTION),rootdefines)

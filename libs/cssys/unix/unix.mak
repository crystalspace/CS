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
  # The X-Window plugin
  PLUGINS += video/canvas/xwindow
  # Shared Memory Plugin
  PLUGINS += video/canvas/xextshm
  # Video Modes Plugin
  PLUGINS += video/canvas/xextf86vm

  ifeq ($(GL.AVAILABLE),yes)
    PLUGINS += video/canvas/openglx
    PLUGINS += video/renderer/opengl-ext
  endif
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
ifeq ($(OSS.AVAILABLE),yes)
PLUGINS += sound/driver/oss sound/renderer/software
endif

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

# Flags for the compiler which are used when profiling.
ifndef CFLAGS.profile
  CFLAGS.profile = -pg -O -g
endif

# Flags for the compiler which are used when building a shared library.
ifndef CFLAGS.DLL
  CFLAGS.DLL =
endif

# General flags for the linker which are used in any case.
# <cs-config>
LFLAGS.GENERAL = $(LFLAGS.L)/usr/local/lib $(CSTHREAD.LFLAGS)
# </cs-config>

# Flags for the linker which are used when profiling.
ifndef LFLAGS.profile
LFLAGS.profile = -pg
endif

# Flags for the linker which are used when building a shared library.
# <cs-config>
LFLAGS.DLL = -Wl,-shared -Wl,-soname -Wl,$@
# </cs-config>

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

# Friendly names for building environment
DESCRIPTION.unix = Unix
DESCRIPTION.OS.unix = Unix

# Choose the 2D/3D drivers you want to use.
ifeq ($(SVGA.AVAILABLE),yes)
PLUGINS += video/canvas/svgalib
endif

ifeq ($(X11.AVAILABLE),yes)
  PLUGINS += video/canvas/softx
  # The X-Window plugin
  PLUGINS += video/canvas/xwindow
  # Shared Memory Plugin
  PLUGINS += video/canvas/xextshm
  # Video Modes Plugin
  ifeq ($(USE_XFREE86VM),yes)
    PLUGINS += video/canvas/xextf86vm
  endif
  ifeq ($(GLX.AVAILABLE),yes)
    PLUGINS += video/canvas/openglx
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

# Sound drivers.
ifeq ($(OSS.AVAILABLE),yes)
PLUGINS += sound/driver/oss sound/renderer/software
endif

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include $(SRCDIR)/mk/unix.mak

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
LFLAGS.GENERAL = $(LFLAGS.L)/usr/local/lib $(LFLAGS.SYSTEM) \
  $(CSTHREAD.LFLAGS) $(LIBBFD.LFLAGS)
# </cs-config>

# Flags for the linker which are used when profiling.
ifndef LFLAGS.profile
LFLAGS.profile = -pg
endif

# Flags for the linker which are used when building a shared library.
# <cs-config>
LFLAGS.DLL = -shared -Wl,-soname -Wl,$@
# </cs-config>

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM = -f elf

# System dependent source files included into CSSYS library
INC.SYS_CSSYS = $(wildcard $(SRCDIR)/libs/cssys/unix/*.h) $(CSTHREAD.INC)
UNIX_SRC.SYS_CSSYS = \
  $(wildcard $(SRCDIR)/libs/cssys/unix/*.cpp) \
  $(SRCDIR)/libs/cssys/general/appdir.cpp \
  $(SRCDIR)/libs/cssys/general/apppath.cpp \
  $(SRCDIR)/libs/cssys/general/csprocessorcap.cpp \
  $(SRCDIR)/libs/cssys/general/findlib.cpp \
  $(SRCDIR)/libs/cssys/general/getopt.cpp \
  $(SRCDIR)/libs/cssys/general/pathutil.cpp \
  $(SRCDIR)/libs/cssys/general/platformconfig.cpp \
  $(SRCDIR)/libs/cssys/general/printf.cpp \
  $(SRCDIR)/libs/cssys/general/resdir.cpp \
  $(SRCDIR)/libs/cssys/general/runloop.cpp \
  $(SRCDIR)/libs/cssys/general/sysroot.cpp \
  $(CSTHREAD.SRC)

ifeq ($(LIBBFD.AVAILABLE)$(OBJCOPY.AVAILABLE),yesyes)
  SRC.SYS_CSSYS = $(UNIX_SRC.SYS_CSSYS)
else
  SRC.SYS_CSSYS = $(filter-out $(SRCDIR)/libs/cssys/unix/bfdplugins.cpp, \
    $(UNIX_SRC.SYS_CSSYS)) $(SRCDIR)/libs/cssys/general/scanplugins.cpp
endif

# Use makedep to build dependencies
DEPEND_TOOL = mkdep

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(LIBBFD.AVAILABLE)$(OBJCOPY.AVAILABLE),yesyes)
  DO.SHARED.PLUGIN.PREAMBLE =
  # Blank line in macro is relevant; do not remove.
  define DO.SHARED.PLUGIN.POSTAMBLE

    $(CMD.OBJCOPY) --add-section .crystal=$(INF.INFILE) $(@)
  endef
endif

endif # ifeq ($(MAKESECTION),postdefines)

# Friendly names for building environment
DESCRIPTION.unix = Unix
DESCRIPTION.OS.unix = Unix

ifeq ($(X11.AVAILABLE),yes)
  PLUGINS += video/canvas/softx
  # The X-Window plugin
  PLUGINS += video/canvas/xwindow
  # Shared Memory plugin
  PLUGINS += video/canvas/xextshm
  # Video Modes plugin
  ifeq ($(XFREE86VM.AVAILABLE),yes)
    PLUGINS += video/canvas/xextf86vm
  endif
  ifeq ($(GLX.AVAILABLE),yes)
    PLUGINS += video/canvas/openglx
  endif
endif

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include $(SRCDIR)/mk/unix.mak

# Extra libraries needed on this system.
# <cs-config>
LIBS.EXE.PLATFORM +=
# </cs-config>

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
CFLAGS.DLL +=

# General flags for the linker which are used in any case.
# <cs-config>
LFLAGS.GENERAL = $(LFLAGS.L)/usr/local/lib $(LFLAGS.SYSTEM) $(CSTHREAD.LFLAGS)
# </cs-config>

# Flags for the linker which are used when profiling.
ifndef LFLAGS.profile
LFLAGS.profile = -pg
endif

# Flags for the linker which are used when building a shared library.
# <cs-config>
ifeq ($(LFLAGS.DLL.USE_SONAME),yes)
  LFLAGS.DLL += -Wl,-soname,$@
endif
# </cs-config>

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM = -f elf

# System dependent source files included into csutil library
INC.SYS_CSUTIL = $(wildcard $(SRCDIR)/libs/csutil/unix/*.h) $(CSTHREAD.INC)
UNIX_SRC.SYS_CSUTIL = \
  $(wildcard $(SRCDIR)/libs/csutil/unix/*.cpp) \
  $(SRCDIR)/libs/csutil/generic/appdir.cpp \
  $(SRCDIR)/libs/csutil/generic/apppath.cpp \
  $(SRCDIR)/libs/csutil/generic/csprocessorcap.cpp \
  $(SRCDIR)/libs/csutil/generic/findlib.cpp \
  $(SRCDIR)/libs/csutil/generic/getopt.cpp \
  $(SRCDIR)/libs/csutil/generic/pathutil.cpp \
  $(SRCDIR)/libs/csutil/generic/platformconfig.cpp \
  $(SRCDIR)/libs/csutil/generic/printf.cpp \
  $(SRCDIR)/libs/csutil/generic/resdir.cpp \
  $(SRCDIR)/libs/csutil/generic/runloop.cpp \
  $(SRCDIR)/libs/csutil/generic/sysroot.cpp \
  $(CSTHREAD.SRC)

ifeq ($(EMBED_META)$(LIBBFD.AVAILABLE)$(OBJCOPY.AVAILABLE),yesyesyes)
  SRC.SYS_CSUTIL = $(UNIX_SRC.SYS_CSUTIL)
else
  SRC.SYS_CSUTIL = $(filter-out $(SRCDIR)/libs/csutil/unix/bfdplugins.cpp, \
    $(UNIX_SRC.SYS_CSUTIL)) $(SRCDIR)/libs/csutil/generic/scanplugins.cpp
endif

# Use makedep to build dependencies
DEPEND_TOOL = mkdep

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(EMBED_META)$(LIBBFD.AVAILABLE)$(OBJCOPY.AVAILABLE),yesyesyes)
  # Override default preamble which simply copies .csplugin file.
  DO.SHARED.PLUGIN.PREAMBLE =
  # Blank line in macro is relevant; do not remove.
  define DO.SHARED.PLUGIN.POSTAMBLE

    $(CMD.OBJCOPY) --add-section .crystal=$(INF.INFILE) $(@)
  endef
endif

$(OUT)/instpath$O: $(SRCDIR)/libs/csutil/unix/instpath.cpp
	$(DO.COMPILE.CPP) \
	$(CFLAGS.D)CS_CONFIGDIR='"$(CS_CONFIGDIR)"' \
	$(CFLAGS.D)CS_PLUGINDIR='"$(CS_PLUGINDIR)"'

endif # ifeq ($(MAKESECTION),postdefines)

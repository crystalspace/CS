############################################
# These are the dependencies for Crystal Space.
# This is an include file for the makefiles.
# Don't use this directly as a makefile.
############################################

# The following include file should define all variables
# needed for this makefile to work.
include mk/user.mak
include mk/common.mak
-include config.mak

.PHONY: doc api depend clean cleanlib cleandep distclean

# Remove all standard suffixes to speed up make lookups
.SUFFIXES:

#--------------- Several definitions to make this file compiler-independent ---#
# Flags for C compiler to direct output to the rule target
CFLAGS.@ = -o $@
# Flags for linker to direct output to the rule target
LFLAGS.@ = -o $@
# Flags for indicating to linker an additional library path
LFLAGS.L = -L
# Flags for indicating an additional library to linker
LFLAGS.l = -l
# Command-line flag to define a macro from command-line
CFLAGS.D = -D
# Command-line flag to set include directories
CFLAGS.I = -I
# Flags for librarian to direct output to the rule target
ARFLAGS.@=$@
# Object file extension
O=.o

# The following include should re/define system-dependent variables
MAKESECTION=defines
include mk/subs.mak
include mk/nasm.mak

ifeq ($(USE_SHARED_LIBS),yes)
  DO.LIBRARY = $(DO.SHARED.LIBRARY)
  LIB_SUFFIX = $(DLL)
else
  DO.LIBRARY = $(DO.STATIC.LIBRARY)
  LIB_SUFFIX = $(LIB)
endif

.SUFFIXES: $O $(EXE) $(LIB) $(DLL) .S .c .cpp .h .asm .ash

# Define paths automatically searched for source files
vpath %.c support/gnu
vpath %.cpp support/debug

# Directory for object files
OUTBASE=out/
OUTOS=$(OUTBASE)$(OS)/
OUTPROC=$(OUTOS)$(PROC)/
OUT=$(OUTPROC)$(MODE)$(OUTSUFX.$(MAKE_DLL))/
############################################

HEADER=$(wildcard *.h */*.h */*/*.h */*/*/*.h */*/*/*/*.h)

CFLAGS.INCLUDE+=$(CFLAGS.I). $(CFLAGS.I)./include $(CFLAGS.I)./libs \
  $(CFLAGS.I)./apps $(CFLAGS.I)./support

CFLAGS=$(CFLAGS.GENERAL) $(CFLAGS.$(MODE)) $(MEM)
LFLAGS=$(LFLAGS.GENERAL) $(LFLAGS.$(MODE)) $(LFLAGS.L)$(OUT)
LIBS=$(LIBS.EXE) $(Z_LIBS)

ifeq ($(MAKE_DLL),yes)
  CFLAGS+=$(CFLAGS.DLL)
endif

# Use $(^^) instead of $^ when you need all dependencies except libraries
^^=$(filter-out %$(LIB_SUFFIX),$^)
# Use $(<<) instead of $< to allow system-dependent makefiles to override
<<=$<
# Use $(L^) to link with all libraries specified as dependencies
L^=$(addprefix $(LFLAGS.l),$(subst $(SPACE)$(LIB_PREFIX),$(SPACE),\
  $(basename $(notdir $(filter %$(LIB_SUFFIX),$+)))))

# How to compile a .c source
DO.COMPILE.C = $(CC) $(CFLAGS.@) $(<<) $(CFLAGS) $(CFLAGS.INCLUDE)
# How to compile a .cpp source
DO.COMPILE.CPP = $(CXX) $(CFLAGS.@) $(<<) $(CFLAGS) $(CFLAGS.INCLUDE)
# How to compile a GAS source
DO.COMPILE.S = $(CC) $(CFLAGS.@) -x assembler-with-cpp $(<<) $(CFLAGS) $(CFLAGS.INCLUDE)
# How to compile a NASM source
DO.COMPILE.ASM = $(NASM) $(NASM.@) $(NASMFLAGS) $(<<)
# How to make a static library
DO.STATIC.LIBRARY = $(AR) $(ARFLAGS) $(ARFLAGS.@) $(^^)
# How to make a shared/dynamic library
DO.SHARED.LIBRARY = $(LINK) $(LFLAGS.DLL) $(LFLAGS.@) $(^^) $(L^) $(LIBS) $(LFLAGS)
# How to make a static plugin
DO.STATIC.PLUGIN = $(AR) $(ARFLAGS) $(ARFLAGS.@) $(^^)
# How to make a shared plugin
DO.SHARED.PLUGIN = $(LINK) $(LFLAGS.DLL) $(LFLAGS.@) $(^^) $(L^) $(LIBS) $(LFLAGS)
# How to link a console executable
DO.LINK.CONSOLE.EXE = $(LINK) $(LFLAGS) $(LFLAGS.CONSOLE.EXE) $(LFLAGS.@) $(^^) $(L^) $(LIBS)
# How to link a graphical executable
DO.LINK.EXE = $(LINK) $(LFLAGS) $(LFLAGS.EXE) $(LFLAGS.@) $(^^) $(L^) $(LIBS)

# How to do either a dynamic or static library (depending on MAKE_DLL)
ifeq ($(MAKE_DLL),yes)
  DO.PLUGIN = $(DO.SHARED.PLUGIN)
else
  DO.PLUGIN = $(DO.SHARED.PLUGIN)
endif

# The sed script used to build dependencies
SED_DEPEND=-e "s/^\([^ ].*\)/$$\(OUT\)\1/" $(SYS_SED_DEPEND)
# How to build a source dependency file
ifndef DO.DEP
  ifeq ($(DEPEND_TOOL),cc)
    DO.DEP = $(CC) -MM $(CFLAGS) $(CFLAGS.INCLUDE) $(filter-out %.asm,$^) | sed $(SED_DEPEND) >$@
  else
    ifeq ($(DEPEND_TOOL),mkdep)
      # If mkdep is already installed, don't build it
      ifneq ($(DEPEND_TOOL.INSTALLED),yes)
        depend: mkdep
      endif
      DO.DEP = makedep $(MEM) $(subst $(CFLAGS.I),-I,$(CFLAGS.INCLUDE)) \
        $(filter-out %.asm,$^) -o $(BUCK)O -p $(OUT) -r -c -f $@
    else
      DO.DEP = echo Building dependencies is not supported on this platform
    endif
  endif
endif

# Directories for output files
OUTDIRS = $(OUTBASE) $(OUTOS) $(OUTPROC) $(OUT) $(OUTDLL)

# The following include should make additional defines using above variables
MAKESECTION = postdefines
include mk/subs.mak

# Now remove duplicate include dirs and duplicate libraries
CFLAGS.INCLUDE := $(sort $(CFLAGS.INCLUDE))
LIBS.EXE := $(sort $(LIBS.EXE))

all: $(OUTDIRS)

depend: $(OUTBASE) $(OUTOS)

distclean: clean
	-$(RM) config.mak include/volatile.h

clean:
	-$(RMDIR) $(subst /,,$(OUTBASE))
ifneq ($(strip $(OUTDLL)),)
	-$(RMDIR) $(subst /,,$(OUTDLL))
endif
	-$(RM) debug.txt
	-$(RM) ivcon.log
	-$(RM) precalc.zip

cleanlib:

cleandep:
	-$(RM) $(OUTOS)*.dep

doc:
	doc++ -H -d csdoc -f $(HEADER)

api:
	doc++ -b -H -d csapi -f \
		$(wildcard include/csengine/*.h) \
		$(wildcard include/csobject/*.h) \
		$(wildcard include/csgeom/*.h)

$(OUT)static$O: static.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.STATIC_COM)

$(OUT)%$O: %.cpp
	$(DO.COMPILE.CPP)

$(OUT)%$O: %.c
	$(DO.COMPILE.C)

$(OUT)%$O: %.S
	$(DO.COMPILE.S)

$(OUT)%$O: %.s
	$(DO.COMPILE.S)

$(OUT)%$O: %.asm
	$(DO.COMPILE.ASM)

$(OUTDIRS):
	$(MKDIR)

# The following include should define system-dependent targets
MAKESECTION=targets
include mk/subs.mak

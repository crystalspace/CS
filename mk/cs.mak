############################################
# These are the dependencies for Crystal Space.
# This is an include file for the makefiles.
# Don't use this directly as a makefile.
############################################

# The following include file should define all variables
# needed for this makefile to work. Please do not change
# the include order unless you know what you are doing!
include mk/user.mak
-include config.mak
include mk/common.mak

.PHONY: doc api depend clean cleanlib cleandep distclean html pdf classpdf pdfbook cleandoc detex zips

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

ifeq ($(USE_SHARED_PLUGINS),no)
  override MAKE_DLL=no
endif

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
  $(CFLAGS.I)./plugins $(CFLAGS.I)./apps $(CFLAGS.I)./support
  
CFLAGS=$(CFLAGS.GENERAL) $(CFLAGS.$(MODE)) $(MEM)
LFLAGS=$(LFLAGS.GENERAL) $(LFLAGS.$(MODE)) $(LFLAGS.L)$(OUT)
LIBS=$(LIBS.EXE) $(Z_LIBS)

ifeq ($(MAKE_DLL),yes)
  CFLAGS+=$(CFLAGS.DLL)
endif

# Memory debugger
ifdef MEMDBG
  # This should be filtered out
  DEP.EXE += $(OUT)memdbg$O
  # This is here because it should be the latest on the command line
  LIBS += $(OUT)memdbg$O
endif

# Use $(^^) instead of $^ when you need all dependencies except libraries
^^=$(filter-out %memdbg$O,$(filter-out %$(LIB_SUFFIX),$^))
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
  DO.PLUGIN = $(DO.STATIC.PLUGIN)
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

clean: cleandoc
	-$(RMDIR) $(subst /,,$(OUTBASE))
ifneq ($(strip $(OUTDLL)),)
	-$(RMDIR) $(subst /,,$(OUTDLL))
endif
	-$(RM) debug.txt ivcon.log precalc.zip

cleanlib:

cleandep:
	-$(RM) $(OUTOS)*.dep

CSAPI=$(wildcard include/csengine/*.h include/csobject/*.h include/csgeom/*.h)
CSDOC=$(HEADER)

pics: 
	mkdir pics
	cp newdoc/pics/* ./pics/

csapi/index.html: $(CSAPI) newdoc/html/docxxbanner.html
	doc++ -F -B newdoc/html/docxxbanner.html -j -H -d csapi -f $(CSAPI)		

api: csapi/index.html pics

csdoc/index.html: $(CSDOC) newdoc/html/docxxbanner.html
	doc++ -F -T newdoc/html/docxxbanner.html -j -H -d csdoc -f $(CSDOC)
	$(RM) csdoc/HIERjava.html
	
doc: csdoc/index.html pics

PDFLATEX=pdflatex -interaction=nonstopmode
CLASS_PDF_FILES=csgeom.pdf csengine.pdf csobject.pdf
PDF_FILES=cs-tut.pdf cs-inst.pdf cs-thry.pdf cs-def.pdf cs-debu.pdf
TEXT_FILES=cs-tut.txt cs-inst.txt cs-thry.txt cs-def.txt cs-debu.txt

VPATH=newdoc

html/index.html: $(wildcard newdoc/*.tex)
	latex2html newdoc/html
	python bin/mshelp.py crystal

html: doc html/index.html pics

%.pdf: %.tex
	$(PDFLATEX) $<

%.ttx: $(wildcard include/$(*F)/*.h)
	doc++ -t -o $(*F).ttx $(wildcard include/$(*F)/*.h)

%.pdf: %.ttx
	$(PDFLATEX) $<

classpdf: $(CLASS_PDF_FILES)

pdf: $(PDF_FILES) classpdf

pdfbook: crystal.pdf classpdf

%.txt: %.pdf
	pdftotext $<

text: $(TEXT_FILES)

cs-help-html.zip: html
	zip -9 -rp cs-help-html.zip csdoc html pics crystal.hh*

cs-help-pdf.zip: pdf
	zip -9 cs-help-pdf.zip $(PDF_FILES) $(CLASS_PDF_FILES)

cs-help-book.zip: pdfbook
	zip -9 cs-help-book.zip crystal.pdf $(CLASS_PDF_FILES)

cs-help-txt.zip: text
	zip -9 cs-help-text.zip $(TEXT_FILES)

doczips: cs-help-html.zip cs-help-pdf.zip cs-help-book.zip cs-help-txt.zip

#lacheck: $(PDF_FILES) $(CLASS_PDF_FILES) crystal.pdf
#	lacheck 

cleandoc:
	$(RM) *.aux *.log *.pdf *.idx *.toc *.ttx *.txt
	$(RM) crystal.hhc crystal.hhk crystal.hhp
	$(RMDIR) csdoc csapi html

$(OUT)static$O: static.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.STATIC_SCF)

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

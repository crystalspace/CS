DESCRIPTION.csperl5 = Crystal Space Perl5 scripting plugin
DESCRIPTION.csperl5maintainer = SWIG Perl5 files
DESCRIPTION.swigperl5gen = SWIG Perl5 files (forcibly)
DESCRIPTION.swigperl5inst = SWIG Perl5 files (install)
DESCRIPTION.swigperl5 = SWIG Perl5 files (clean)

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION), rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make csperl5      Make the $(DESCRIPTION.csperl5)$"

endif

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION), roottargets)

.PHONY: csperl5 swigperl5gen csperl5clean swigperl5clean csperl5maintainerclean

all plugins: csperl5

csperl5:
	$(MAKE_TARGET) MAKE_DLL=yes
csperl5clean:
	$(MAKE_CLEAN)
csperl5maintainerclean:
	$(MAKE_CLEAN)
ifneq (,$(SWIGBIN))
swigperl5gen:
	$(MAKE_TARGET)
swigperl5inst:
	$(MAKE_TARGET) DO_SWIGPERL5INST=yes
swigperl5clean:
	$(MAKE_CLEAN)
endif

endif

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION), postdefines)

ifeq ($(USE_PLUGINS),yes)
  CSPERL5 = $(OUTDLL)/csperl5$(DLL)
  LIB.CSPERL5 = $(foreach d,$(DEP.CSPERL5),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSPERL5)
else
  CSPERL5 = $(OUT)/$(LIBPREFIX)csperl5$(LIB)
  DEP.EXE += $(CSPERL5)
  SCF.STATIC += csperl5
  TO_INSTALL.STATIC_LIBS += $(CSPERL5)
endif

INF.CSPERL5 = $(SRCDIR)/plugins/cscript/csperl5/csperl5.csplugin
INC.CSPERL5 = $(wildcard $(addprefix $(SRCDIR)/,plugins/cscript/csperl5/*.h))
SRC.CSPERL5 = $(wildcard $(addprefix $(SRCDIR)/,plugins/cscript/csperl5/*.cpp))
OBJ.CSPERL5 = $(addprefix $(OUT)/,$(notdir $(SRC.CSPERL5:.cpp=$O)))
DEP.CSPERL5 = CSTOOL CSGEOM CSUTIL CSUTIL

PERLXSI.C = $(OUTDERIVED)/csperlxs.c
PERLXSI.O = $(OUT)/$(notdir $(PERLXSI.C:.c=$O))

SWIG.I = $(SRCDIR)/include/ivaria/cspace.i
SWIG.MOD = cspace
SWIG.PERL5.DIR = $(SRCDIR)/scripts/perl5
SWIG.PERL5.PM = $(SWIG.PERL5.DIR)/$(SWIG.MOD).pm
SWIG.PERL5.C = $(SWIG.PERL5.DIR)/cswigpl5.inc
SWIG.PERL5.PM.IN = $(OUTDERIVED)/$(notdir $(SWIG.PERL5.PM))
SWIG.PERL5.C.IN = $(OUTDERIVED)/$(notdir $(SWIG.PERL5.C))
SWIG.PERL5.CPP = $(SRCDIR)/plugins/cscript/csperl5/cswigpl5.cpp
SWIG.PERL5.O = $(OUT)/$(notdir $(SWIG.PERL5.CPP:.cpp=$O))
SWIG.PERL5.DLL = $(SWIG.PERL5.DIR)/$(SWIG.MOD)$(DLL)

CEX.CSPERL5 = perl5.cex
CIN.CSPERL5 = plugins/cscript/csperl5/perl5.cin

PERL5.CFLAGS += -Wno-unused

#MSVC.DSP += MSCSPERL5
#DSP.MSCSPERL5.NAME = csperl5
#DSP.MSCSPERL5.TYPE = plugin
#DSP.MSCSPERL5.CFLAGS = \
#  /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NO_STRICT" \
#  /D "HAVE_DES_FCRYPT" /D "PERL_IMPLICIT_CONTEXT" \
#  /D "PERL_IMPLICIT_SYS" /D "USE_PERLIO" \
#  /D "PERL_MSVCRT_READFIX" \
#MSCSPERL5 = $(CSPERL5)
#LIB.MSCSPERL5 = $(LIB.CSPERL5) $(LIBPREFIX)perl58$(LIB)
#INC.MSCSPERL5 = $(INC.CSPERL5)
#SRC.MSCSPERL5 = $(SRC.CSPERL5) $(PERLXSI.C)
#OBJ.MSCSPERL5 = $(OBJ.CSPERL5) $(PERLXSI.O)
#DEP.MSCSPERL5 = $(DEP.CSPERL5)

#MSVC.DSP += MSCSPERL5SWIG
#DSP.MSCSPERL5SWIG.NAME = csperl5s
#DSP.MSCSPERL5SWIG.TYPE = plugin
#DSP.MSCSPERL5SWIG.CFLAGS = $(DSP.MSCSPERL5.CFLAGS) /D "PERL_POLLUTE"
#MSCSPERL5SWIG = $(SWIG.PERL5.DLL)
#LIB.MSCSPERL5SWIG = $(LIB.MSCSPERL5)
#SRC.MSCSPERL5SWIG = $(SWIG.PERL5.C)
#OBJ.MSCSPERL5SWIG = $(SWIG.PERL5.O)
#DEP.MSCSPERL5SWIG = $(DEP.CSPERL5)

endif

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION), targets)

.PHONY: csperl5 swigperl5gen csperl5clean swigperl5clean csperl5maintainerclean

csperl5: $(OUTDIRS) $(CSPERL5) $(SWIG.PERL5.DLL) $(CEX.CSPERL5)

$(CSPERL5): $(OBJ.CSPERL5) $(LIB.CSPERL5) $(PERLXSI.O) $(SWIG.PERL5.DLL)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(PERL5.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

$(OUT)/%$O: $(SRCDIR)/plugins/cscript/csperl5/%.cpp
	$(DO.COMPILE.CPP) $(PERL5.CFLAGS)

ifeq ($(PERL5.EXTUTILS.EMBED.AVAILABLE),yes)
  ifeq ($(PERL5.DYNALOADER.AVAILABLE),yes)
    PERLXSI.MK = $(CMD.PERL5) -MExtUtils::Embed -e xsinit -- \
    -o $(PERLXSI.C) -std DynaLoader cspace
  else
    PERLXSI.MK = $(CMD.PERL5) -MExtUtils::Embed -e xsinit -- \
    -o $(PERLXSI.C) -std cspace
  endif
else
  PERLXSI.MK = echo "\#include \"cssys/csperlxs_fallback.inc\"" > $(PERLXSI.C)
endif

$(PERLXSI.C):
	$(PERLXSI.MK)

$(PERLXSI.O): $(PERLXSI.C)
	$(DO.COMPILE.C) $(PERL5.CFLAGS)

ifeq (,$(SWIGBIN))
$(SWIG.PERL5.PM.IN):
	-$(RM) $(SWIG.PERL5.PM.IN)
	$(CP) $(SWIG.PERL5.PM) $(SWIG.PERL5.PM.IN)
$(SWIG.PERL5.C.IN):
	-$(RM) $(SWIG.PERL5.C.IN)
	$(CP) $(SWIG.PERL5.C) $(SWIG.PERL5.C.IN)
else
$(SWIG.PERL5.PM.IN) $(SWIG.PERL5.C.IN): $(OUTDIRS)
	-$(SWIGBIN) -perl5 -c++ -shadow -const -Iinclude -I$(SRCDIR)/include \
	-module $(SWIG.MOD) -o $(SWIG.PERL5.C.IN) $(SWIG.I)
	$(SED) '/$(BUCK)Header:/d' < $(SWIG.PERL5.C.IN) > $(SWIG.PERL5.C.IN).sed
	$(RM) $(SWIG.PERL5.C.IN)
	$(MV) $(SWIG.PERL5.C.IN).sed $(SWIG.PERL5.C.IN)
endif

ifeq ($(DO_SWIGPERL5INST),yes)
swigperl5inst: $(OUTDIRS) $(SWIG.PERL5.C) $(SWIG.PERL5.PM)

$(SWIG.PERL5.C): $(SWIG.PERL5.C.IN)
	-$(RM) $@
	$(CP) $(SWIG.PERL5.C.IN) $@

$(SWIG.PERL5.PM): $(SWIG.PERL5.PM.IN)
	-$(RM) $@
	$(CP) $(SWIG.PERL5.PM.IN) $@
endif

$(SWIG.PERL5.O): $(SWIG.PERL5.CPP)
	$(filter-out -W -Wunused -Wall -Wmost,$(DO.COMPILE.CPP) \
	$(PERL5.CFLAGS) -DPERL_POLLUTE $(CFLAGS.I)$(SWIG.PERL5.DIR))

#@@@ Kludge: we leave out PREAMBLE and POSTAMPLE since we don't want metadata
$(SWIG.PERL5.DLL): $(SWIG.PERL5.O) $(LIB.CSPERL5)
	$(DO.SHARED.PLUGIN.CORE) $(PERL5.LFLAGS)

$(CEX.CSPERL5): $(CIN.CSPERL5)
	@echo Generating perl5 cs-config extension...
	$(PERL5) $(CIN.CSPERL5) \
	$"CFLAGS=$(PERL5.CFLAGS)$" $"LFLAGS=$(PERL5.LFLAGS)$" > $@

swigperl5gen: $(OUTDIRS) swigperl5clean $(SWIG.PERL5.C.IN) $(SWIG.PERL5.PM.IN)

clean: csperl5clean
maintainerclean: csperl5maintainerclean

csperl5clean: swigperl5clean
	-$(RMDIR) $(CSPERL5) $(OBJ.CSPERL5) $(OUTDLL)/$(notdir $(INF.CSPERL5)) \
	$(PERLXSI.O) $(PERLXSI.C) $(SWIG.PERL5.O) $(SWIG.PERL5.DLL)

swigperl5clean:
	-$(RM) $(SWIG.PERL5.C.IN) $(SWIG.PERL5.PM.IN)

csperl5maintainerclean: csperl5clean
	-$(RM) $(SWIG.PERL5.C) $(SWIG.PERL5.PM)

ifdef DO_DEPEND
dep: $(OUTOS)/csperl5.dep
$(OUTOS)/csperl5.dep: $(SRC.CSPERL5)
	$(DO.DEP1) $(PERL5.CFLAGS) $(DO.DEP2)
else
-include $(OUTOS)/csperl5.dep
endif

endif

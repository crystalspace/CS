DESCRIPTION.csperl5 = Crystal Space Perl v5 Scripting Plugin

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION), rootdefines)

PLUGINHELP += \
  $(NEWLINE)@echo $"  make csperl5      Make the $(DESCRIPTION.csperl5)$"

endif

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION), roottargets)

.PHONY: csperl5 csperl5clean csperl5distclean

all plugins: csperl5

csperl5:
	$(MAKE_TARGET) MAKE_DLL=yes
csperl5clean:
	$(MAKE_CLEAN)
csperl5distclean:
	$(MAKE_CLEAN)

endif

#-------------------------------------------------------------- postdefines ---#
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

INC.CSPERL5 = $(wildcard plugins/cscript/csperl5/*.h)
SRC.CSPERL5 = $(wildcard plugins/cscript/csperl5/*.cpp)
OBJ.CSPERL5 = $(addprefix $(OUT)/,$(notdir $(SRC.CSPERL5:.cpp=$O)))
DEP.CSPERL5 = CSGEOM CSSYS CSUTIL CSSYS CSUTIL

PERLXSI.C = plugins/cscript/csperl5/csperlxs.c
PERLXSI.O = $(OUT)/$(notdir $(PERLXSI.C:.c=$O))

SWIG.I = include/ivaria/cs.i
SWIG.MOD = cspace
SWIG.PERL5.PM = scripts/perl5/$(SWIG.MOD).pm
SWIG.PERL5.C = plugins/cscript/csperl5/cswigpl5.c
SWIG.PERL5.O = $(OUT)/$(notdir $(SWIG.PERL5.C:.c=$O))
SWIG.PERL5.DLL = scripts/perl5/$(SWIG.MOD)$(DLL)
DEP.SWIG.PERL5.DLL = $(DEP.CSPERL5)
SWIG.PERL5.DOC = scripts/perl5/cs_wrap.doc

CEX.CSPERL5 = perl5.cex
CIN.CSPERL5 = plugins/cscript/csperl5/perl5.cin

MSVC.DSP += CSPERL5
DSP.CSPERL5.NAME = csperl5
DSP.CSPERL5.TYPE = plugin
DSP.CSPERL5.LFLAGS = /L C:\perl\lib\CORE
DSP.CSPERL5.LIBS = perl

endif

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION), targets)

.PHONY: csperl5 csperl5clean csperl5distclean

csperl5: $(OUTDIRS) $(CSPERL5) $(CSPERL5.PM) $(CEX.PERL5)

$(CSPERL5): $(OBJ.CSPERL5) $(LIB.CSPERL5) $(PERLXSI.O) $(SWIG.PERL5.DLL)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(PERL5.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

$(OUT)/%$O: plugins/cscript/csperl5/%.cpp
	$(DO.COMPILE.CPP) $(PERL5.CFLAGS)

$(PERLXSI.O): $(PERLXSI.C)
	$(DO.COMPILE.C) $(PERL5.CFLAGS)

ifeq (,$(SWIGBIN))
  SWIGBIN = swig
endif

$(SWIG.PERL5.PM) $(SWIG.PERL5.C): $(SWIG.I)
	$(SWIGBIN) -perl5 -c++ -dascii -Sbefore -shadow -Iinclude \
	-module $(SWIG.MOD) -o $(SWIG.PERL5.C) $(SWIG.I)
	$(MV) plugins/cscript/csperl5/$(SWIG.MOD).pm $(SWIG.PERL5.PM)

$(SWIG.PERL5.O): $(SWIG.PERL5.C)
	$(filter-out -W -Wunused -Wall -Wmost,$(DO.COMPILE.CPP) $(PERL5.CFLAGS) -DPERL_POLLUTE -DNO_HANDY_PERL_MACROS)

$(SWIG.PERL5.DLL): $(SWIG.PERL5.O) $(LIB.CSPERL5)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(PERL5.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

$(CEX.PERL5): $(CIN.PERL5)
	@echo Generating perl5 cs-config extension...
	$(PERL5) $(CIN.PERL5) \
	$"CFLAGS=$(PERL5.CFLAGS)$" $"LFLAGS=$(PERL5.LFLAGS)$" > $@

clean: csperl5clean
distclean: csperl5distclean

csperl5clean:
	-$(RM) $(CSPERL5) $(OBJ.CSPERL5) $(PERLXSI.O) \
	$(CSPERL5.PM) $(SWIG.PERL5.O) $(SWIG.PERL5.DLL)

csperl5distclean: csperl5clean
	-$(RM) $(PERLXSI.C) $(SWIG.PERL5.PM) $(SWIG.PERL5.C) $(SWIG.PERL5.DOC)

ifdef DO_DEPEND
dep: $(OUTOS)/csperl5.dep
else
-include $(OUTOS)/csperl5.dep
endif

endif


DESCRIPTION.csperl5 = Crystal Space Perl5 plugin
DESCRIPTION.csperl5maintainer = SWIG Perl5 files
DESCRIPTION.swigperl5gen = SWIG Perl5 files (forcibly)
DESCRIPTION.swigperl5inst = SWIG Perl5 files (install)
DESCRIPTION.swigperl5 = SWIG Perl5 files (clean)

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION), rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make csperl5      Make the $(DESCRIPTION.csperl5)$"
ifneq (,$(CMD.SWIG))
PSEUDOHELP += \
  $(NEWLINE)echo $"  make swigperl5gen Make the $(DESCRIPTION.swigperl5gen)$" \
  $(NEWLINE)echo $"  make swigperl5inst$" \
  $(NEWLINE)echo $"                    Freeze $(DESCRIPTION.swigperl5inst)$"
endif

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
ifneq (,$(CMD.SWIG))
swigperl5gen:
	$(MAKE_TARGET)
swigperl5inst:
	$(MAKE_TARGET)
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
INC.CSPERL5 = $(wildcard $(SRCDIR)/plugins/cscript/csperl5/*.h)
SRC.CSPERL5 = $(filter-out $(SRCDIR)/plugins/cscript/csperl5/perl5mod.cpp, \
  $(wildcard $(SRCDIR)/plugins/cscript/csperl5/*.cpp))
OBJ.CSPERL5 = $(addprefix $(OUT)/,$(notdir $(SRC.CSPERL5:.cpp=$O)))
DEP.CSPERL5 = CSTOOL CSGFX CSGEOM CSUTIL

CSPERL5.DERIVED = $(OUTDERIVED)/perl5

PERLXSI.C = $(CSPERL5.DERIVED)/csperlxs.c
PERLXSI.O = $(OUT)/$(notdir $(PERLXSI.C:.c=$O))

SWIG.I = $(SRCDIR)/include/ivaria/cspace.i
SWIG.MOD = cspace
SWIG.PERL5.DIR = $(SRCDIR)/scripts/perl5
SWIG.PERL5.PM = $(SWIG.PERL5.DIR)/$(SWIG.MOD).pm
SWIG.PERL5.C = $(SWIG.PERL5.DIR)/cswigpl5.inc
SWIG.PERL5.PM.IN = $(CSPERL5.DERIVED)/$(notdir $(SWIG.PERL5.PM))
SWIG.PERL5.C.IN = $(CSPERL5.DERIVED)/$(notdir $(SWIG.PERL5.C))
SWIG.PERL5.CPP = $(SRCDIR)/plugins/cscript/csperl5/cswigpl5.cpp
SWIG.PERL5.O = $(OUT)/$(notdir $(SWIG.PERL5.CPP:.cpp=$O))
SWIG.PERL5.MOD = $(SRCDIR)/plugins/cscript/csperl5/perl5mod.cpp
SWIG.PERL5.MOD.O = $(OUT)/$(notdir $(SWIG.PERL5.MOD:.cpp=$O))
SWIG.PERL5.INSTALLDIR = $(OUTPROC)/perl5
SWIG.PERL5.DLL = $(SWIG.PERL5.INSTALLDIR)/$(SWIG.MOD)$(DLL)
SWIG.PERL5.INSTALLPM = $(SWIG.PERL5.INSTALLDIR)/$(notdir $(SWIG.PERL5.PM))

CEX.CSPERL5 = perl5.cex
CIN.CSPERL5 = $(SRCDIR)/plugins/cscript/csperl5/perl5.cin

PERL5.CFLAGS += $(CXXFLAGS.WARNING.NO_UNUSED)

OUTDIRS += $(CSPERL5.DERIVED) $(SWIG.PERL5.INSTALLDIR)

ifeq (,$(CMD.SWIG))
TO_INSTALL.SCRIPTS += $(wildcard $(SWIG.PERL5.DIR)/*.pm)
else
TO_INSTALL.SCRIPTS += \
  $(filter-out $(SWIG.PERL5.PM),$(wildcard $(SWIG.PERL5.DIR)/*.pm))
endif

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
#OBJ.MSCSPERL5SWIG = $(SWIG.PERL5.O) $(SWIG.PERL5.MOD.O)
#DEP.MSCSPERL5SWIG = $(DEP.CSPERL5)

endif

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION), targets)

.PHONY: csperl5 swigperl5gen csperl5clean swigperl5clean csperl5maintainerclean

csperl5: $(OUTDIRS) $(SWIG.PERL5.INSTALLPM) $(CSPERL5) $(SWIG.PERL5.DLL) \
  $(CEX.CSPERL5)

$(CSPERL5): $(OBJ.CSPERL5) $(LIB.CSPERL5) $(PERLXSI.O)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(PERL5.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

$(SRC.CSPERL5): $(SWIG.PERL5.C.IN)

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
  PERLXSI.MK = echo "\#include \"csutil/csperlxs_fallback.inc\"" > $(PERLXSI.C)
endif

$(PERLXSI.C):
	$(PERLXSI.MK)

$(PERLXSI.O): $(PERLXSI.C)
	$(DO.COMPILE.C) $(PERL5.CFLAGS)

ifeq (,$(CMD.SWIG))
$(SWIG.PERL5.PM.IN): $(SWIG.PERL5.PM)
	-$(RM) $(SWIG.PERL5.PM.IN)
	$(CP) $(SWIG.PERL5.PM) $(SWIG.PERL5.PM.IN)
$(SWIG.PERL5.C.IN): $(SWIG.PERL5.C)
	-$(RM) $(SWIG.PERL5.C.IN)
	$(CP) $(SWIG.PERL5.C) $(SWIG.PERL5.C.IN)
else
$(SWIG.PERL5.PM.IN) $(SWIG.PERL5.C.IN): $(SWIG.I)
	-$(CMD.SWIG) $(SWIG.FLAGS) -perl5 -c++ -shadow -const -Iinclude \
	-I$(SRCDIR)/include -module $(SWIG.MOD) -o $(SWIG.PERL5.C.IN) $(SWIG.I)
	$(SED) '/$(BUCK)Header:/d' < $(SWIG.PERL5.C.IN) >$(SWIG.PERL5.C.IN).sed
	$(RM) $(SWIG.PERL5.C.IN)
	$(MV) $(SWIG.PERL5.C.IN).sed $(SWIG.PERL5.C.IN)

swigperl5gen: $(OUTDIRS) swigperl5clean $(SWIG.PERL5.C.IN) $(SWIG.PERL5.PM.IN)

swigperl5inst: $(OUTDIRS) $(SWIG.PERL5.C) $(SWIG.PERL5.PM)

$(SWIG.PERL5.C): $(SWIG.PERL5.C.IN)
	-$(RM) $@
	$(CP) $(SWIG.PERL5.C.IN) $@

$(SWIG.PERL5.PM): $(SWIG.PERL5.PM.IN)
	-$(RM) $@
	$(CP) $(SWIG.PERL5.PM.IN) $@
endif

.PHONY: install_perlmod
install_script: install_perlmod
install_perlmod: $(SWIG.PERL5.DLL)
	@$(CMD.MKDIRS) $(INSTALL_SCRIPTS.DIR)/perl5
	@echo $"$(INSTALL_SCRIPTS.DIR)/perl5/deleteme.dir$" >> $(INSTALL_LOG)
	$(CP) $(SWIG.PERL5.DLL) $(INSTALL_SCRIPTS.DIR)/perl5
	@echo $"$(INSTALL_SCRIPTS.DIR)/perl5/$(notdir $(SWIG.PERL5.DLL))$" >> \
	$(INSTALL_LOG)

ifneq (,$(CMD.SWIG))
.PHONY: install_cspace_pm
install_script: install_cspace_pm
install_cspace_pm: $(SWIG.PERL5.PM.IN)
	@$(CMD.MKDIRS) $(INSTALL_SCRIPTS.DIR)/perl5
	@echo $"$(INSTALL_SCRIPTS.DIR)/perl5/deleteme.dir$" >> $(INSTALL_LOG)
	$(CP) $(SWIG.PERL5.PM.IN) $(INSTALL_SCRIPTS.DIR)/perl5
	@echo $"$(INSTALL_SCRIPTS.DIR)/perl5/$(notdir $(SWIG.PERL5.PM.IN))$" \
	>> $(INSTALL_LOG)
endif

$(SWIG.PERL5.O): $(SWIG.PERL5.CPP)
	$(filter-out -W -Wunused -Wall -Wmost,$(DO.COMPILE.CPP) \
	$(PERL5.CFLAGS) -DPERL_POLLUTE $(CFLAGS.I)$(CSPERL5.DERIVED))

# @@@ Kludge: We omit PREAMBLE and POSTAMBLE since we don't want metadata.
# The more correct way to do this is to invoke Perl's own extension building
# facility.  This is especially true since the ugly sans-PREAMBLE/POSTAMBLE
# hack is bound to fail on any number of platforms since platform-specific
# build rules are not guaranteed to split metadata embedding off into the
# PREAMBLE/POSTAMBLE macros.  In fact, this already fails on Windows, since
# DO.SHARED.PLUGIN.CORE on Windows tries linking with a cspace-rsrc.o file
# which does not exist.
$(SWIG.PERL5.DLL): $(SWIG.PERL5.MOD.O) $(SWIG.PERL5.O) $(LIB.CSPERL5)
	$(filter-out %-rsrc$O,$(DO.SHARED.PLUGIN.CORE)) $(PERL5.LFLAGS)

$(SWIG.PERL5.INSTALLPM): $(SWIG.PERL5.PM.IN)
	$(RM) $@
	$(CP) $(SWIG.PERL5.PM.IN) $@

$(CEX.CSPERL5): $(CIN.CSPERL5)
	@echo Generating perl5 cs-config extension...
	$(PERL5) $(CIN.CSPERL5) \
	$"CFLAGS=$(PERL5.CFLAGS)$" $"LFLAGS=$(PERL5.LFLAGS)$" > $@

clean: csperl5clean
maintainerclean: csperl5maintainerclean

csperl5clean: swigperl5clean
	-$(RMDIR) $(CSPERL5) $(OBJ.CSPERL5) \
	$(OUTDLL)/$(notdir $(INF.CSPERL5)) $(CEX.CSPERL5) \
	$(PERLXSI.O) $(PERLXSI.C) $(SWIG.PERL5.MOD.O) $(SWIG.PERL5.O) \
	$(SWIG.PERL5.DLL) $(SWIG.PERL5.INSTALLPM) $(CSPERL5.DERIVED) \
	$(SWIG.PERL5.INSTALLDIR)

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

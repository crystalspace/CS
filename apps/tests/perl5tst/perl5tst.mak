ifneq (,$(findstring csperl5,$(PLUGINS) $(PLUGINS.DYNAMIC)))

DESCRIPTION.perl5test = Crystal Space Perl5 scripting test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application
APPHELP += \
  $(NEWLINE)@echo $"  make perl5test    Make the $(DESCRIPTION.perl5test)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: perl5test perl5testclean

all apps: perl5test

perl5test:
	$(MAKE_APP)
perl5testclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

PERL5TEST.EXE = perl5test$(EXE)
DIR.PERL5TEST = apps/tests/perl5tst
OUT.PERL5TEST = $(OUT)/$(DIR.PERL5TEST)
INC.PERL5TEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PERL5TEST)/*.h))
SRC.PERL5TEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PERL5TEST)/*.cpp))
OBJ.PERL5TEST = \
  $(addprefix $(OUT.PERL5TEST)/,$(notdir $(SRC.PERL5TEST:.cpp=$O)))
DEP.PERL5TEST = CSTOOL CSUTIL
LIB.PERL5TEST = $(foreach d,$(DEP.PERL5TEST),$($d.LIB))

OUTDIRS += $(OUT.PERL5TEST)

MSVC.DSP += PERL5TEST
DSP.PERL5TEST.NAME = perl5test
DSP.PERL5TEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: perl5test perl5testclean perl5testcleandep

build.perl5test: $(OUTDIRS) $(PERL5TEST.EXE)
clean: perl5testclean

$(OUT.PERL5TEST)/%$O: $(SRCDIR)/$(DIR.PERL5TEST)/%.cpp
	$(DO.COMPILE.CPP)

$(PERL5TEST.EXE): $(OBJ.PERL5TEST) $(LIB.PERL5TEST)
	$(DO.LINK.EXE)

perl5testclean:
	-$(RM) perl5test.txt
	-$(RMDIR) $(PERL5TEST.EXE) $(OBJ.PERL5TEST)

cleandep: perl5testcleandep
perl5testcleandep:
	-$(RM) $(OUT.PERL5TEST)/perl5test.dep

ifdef DO_DEPEND
dep: $(OUT.PERL5TEST) $(OUT.PERL5TEST)/perl5test.dep
$(OUT.PERL5TEST)/perl5test.dep: $(SRC.PERL5TEST)
	$(DO.DEPEND)
else
-include $(OUT.PERL5TEST)/perl5test.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring csperl5,$(PLUGINS) $(PLUGINS.DYNAMIC)))

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

perl5test: csperl5
	$(MAKE_APP)
perl5testclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tests/perl5tst

PERL5TEST.EXE = perl5tst$(EXE)
SRC.PERL5TEST = $(wildcard apps/tests/perl5tst/*.cpp)
OBJ.PERL5TEST = $(addprefix $(OUT)/,$(notdir $(SRC.PERL5TEST:.cpp=$O)))
DEP.PERL5TEST = CSPERL5 CSTOOL CSUTIL CSSYS
LIB.PERL5TEST = $(foreach d,$(DEP.PERL5TEST),$($d.LIB))

MSVC.DSP += PERL5TEST
DSP.PERL5TEST.NAME = perl5tst
DSP.PERL5TEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: perl5test perl5testclean

all: $(PERL5TEST.EXE)
build.perl5test: $(OUTDIRS) $(PERL5TEST.EXE)
clean: perl5testclean

$(PERL5TEST.EXE): $(OBJ.PERL5TEST) $(LIB.PERL5TEST)
	$(DO.LINK.EXE)

perl5testclean:
	-$(RMDIR) $(PERL5TEST.EXE) $(OBJ.PERL5TEST)

ifdef DO_DEPEND
dep: $(OUTOS)/perl5tst.dep
$(OUTOS)/perl5tst.dep: $(SRC.PERL5TEST)
	$(DO.DEP)
else
-include $(OUTOS)/perl5tst.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring csperl5,$(PLUGINS) $(PLUGINS.DYNAMIC)))

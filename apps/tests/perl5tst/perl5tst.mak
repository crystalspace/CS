
DESCRIPTION.perl5test = Crystal Space Perl v5 Scripting Test App

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION), rootdefines)

# Application
APPHELP += \
  $(NEWLINE)@echo $"  make perl5test    Make the $(DESCRIPTION.perl5test)$"

endif

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION), roottargets)

.PHONY: perl5test perl5testclean

all apps: perl5test

clean: perl5testclean

perl5test: csperl5
	$(MAKE_TARGET)
perl5testclean:
	$(MAKE_CLEAN)

endif

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION), postdefines)

vpath % apps/tests/perl5tst

PERL5TEST = perl5tst$(EXE)
LIB.PERL5TEST = $(foreach d,$(DEP.PERL5TEST),$($d.LIB))
TO_INSTALL.EXE += $(PERL5TEST)

SRC.PERL5TEST = $(wildcard apps/tests/perl5tst/*.cpp)
OBJ.PERL5TEST = $(addprefix $(OUT)/,$(notdir $(SRC.PERL5TEST:.cpp=$O)))
DEP.PERL5TEST = CSPERL5 CSTOOL CSUTIL CSSYS

MSVC.DSP += PERL5TEST
DSP.PERL5TEST.NAME = perl5tst
DSP.PERL5TEST.TYPE = appcon
DSP.PERL5TEST.DEPEND = CSPERL5 CSTOOL CSUTIL CSSYS

endif

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION), targets)

.PHONY: perl5test perl5testclean

perl5test: $(PERL5TEST)

$(PERL5TEST): $(OBJ.PERL5TEST) $(LIB.PERL5TEST)
	$(DO.LINK.EXE)

$(OBJ.PERL5TEST): $(SRC.PERL5TEST)
	$(DO.COMPILE.CPP)

perl5testclean:
	-$(RMDIR) $(PERL5TEST) $(OBJ.PERL5TEST)

ifdef DO_DEPEND
dep: $(OUTOS)/perl5tst.dep
else
-include $(OUTOS)/perl5tst.dep
endif

endif


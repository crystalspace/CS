# Application target only valid if module is listed in PLUGINS.
ifneq (,$(findstring aws,$(PLUGINS) $(PLUGINS.DYNAMIC)))

# Application description
DESCRIPTION.mottest = Motion Manager Test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP+=$(NEWLINE)echo $"  make mottest      Make the $(DESCRIPTION.mottest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mottest mottestclean

all apps: mottest
mottest:
	$(MAKE_APP)
mottestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

MOTTEST.EXE = mottest$(EXE)
DIR.MOTTEST = apps/tests/mottest
OUT.MOTTEST = $(OUT)/$(DIR.MOTTEST)
INC.MOTTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MOTTEST)/*.h ))
SRC.MOTTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MOTTEST)/*.cpp ))
OBJ.MOTTEST = $(addprefix $(OUT.MOTTEST)/,$(notdir $(SRC.MOTTEST:.cpp=$O)))
DEP.MOTTEST = CSTOOL CSUTIL CSUTIL CSGEOM CSGFX
LIB.MOTTEST = $(foreach d,$(DEP.MOTTEST),$($d.LIB))
#CFG.MOTTEST = data/config/mottest.cfg

OUTDIRS += $(OUT.MOTTEST)

#TO_INSTALL.EXE    += $(MOTTEST.EXE)
#TO_INSTALL.CONFIG += $(CFG.MOTTEST)

MSVC.DSP += MOTTEST
DSP.MOTTEST.NAME = mottest
DSP.MOTTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.mottest mottestclean mottestcleandep

all: $(MOTTEST.EXE)
build.mottest: $(OUTDIRS) $(MOTTEST.EXE)
clean: mottestclean

$(OUT.MOTTEST)/%$O: $(SRCDIR)/$(DIR.MOTTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(MOTTEST.EXE): $(DEP.EXE) $(OBJ.MOTTEST) $(LIB.MOTTEST)
	$(DO.LINK.EXE)

mottestclean:
	-$(RM) mottest.txt
	-$(RMDIR) $(MOTTEST.EXE) $(OBJ.MOTTEST)

cleandep: mottestcleandep
mottestcleandep:
	-$(RM) $(OUT.MOTTEST)/mottest.dep

ifdef DO_DEPEND
dep: $(OUT.MOTTEST) $(OUT.MOTTEST)/mottest.dep
$(OUT.MOTTEST)/mottest.dep: $(SRC.MOTTEST)
	$(DO.DEPEND)
else
-include $(OUT.MOTTEST)/mottest.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring aws,$(PLUGINS) $(PLUGINS.DYNAMIC)))

# Application target only valid if module is listed in PLUGINS.
ifneq (,$(findstring pgserver,$(PLUGINS) $(PLUGINS.DYNAMIC)))

# Application description
DESCRIPTION.pgtest = Crystal Space PicoGUI Test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application
APPHELP += \
  $(NEWLINE)@echo $"  make pgtest       Make the $(DESCRIPTION.pgtest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: pgtest pgtestclean

all apps: pgtest

pgtest:
	$(MAKE_APP)

pgtestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

PGTEST.EXE = pgtest$(EXE)
DIR.PGTEST = apps/tests/picogui
OUT.PGTEST = $(OUT)/$(DIR.PGTEST)
INC.PGTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PGTEST)/*.h))
SRC.PGTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PGTEST)/*.cpp))
OBJ.PGTEST = $(addprefix $(OUT.PGTEST)/,$(notdir $(SRC.PGTEST:.cpp=$O)))
DEP.PGTEST = CSTOOL CSGFX CSUTIL CSUTIL
LIB.PGTEST = $(foreach d,$(DEP.PGTEST),$($d.LIB))

OUTDIRS += $(OUT.PGTEST)

MSVC.DSP += PGTEST
DSP.PGTEST.NAME = pgtest
DSP.PGTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.pgtest pgtestclean pgtestcleandep

build.pgtest: $(OUTDIRS) $(PGTEST.EXE)
clean: pgtestclean

$(OUT.PGTEST)/%$O: $(SRCDIR)/$(DIR.PGTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(PGTEST.EXE): $(DEP.EXE) $(OBJ.PGTEST) $(LIB.PGTEST)
	$(DO.LINK.EXE)

pgtestclean:
	-$(RMDIR) $(PGTEST.EXE) $(OBJ.PGTEST) pgtest.txt

cleandep: pgtestcleandep
pgtestcleandep:
	-$(RM) $(OUT.PGTEST)/pgtest.dep

ifdef DO_DEPEND
dep: $(OUT.PGTEST) $(OUT.PGTEST)/pgtest.dep
$(OUT.PGTEST)/pgtest.dep: $(SRC.PGTEST)
	$(DO.DEPEND)
else
-include $(OUT.PGTEST)/pgtest.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring pgserver,$(PLUGINS) $(PLUGINS.DYNAMIC)))

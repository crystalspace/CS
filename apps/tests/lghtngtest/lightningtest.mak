# Application description
DESCRIPTION.lghtngtest = Lightning Test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make lghtngtest   Make the $(DESCRIPTION.lghtngtest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: lghtngtest lghtngtestclean

all apps: lghtngtest
lghtngtest:
	$(MAKE_APP)
lghtngtestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

LGHTNGTEST.EXE = lghtngtest$(EXE)
DIR.LGHTNGTEST = apps/tests/lghtngtest
OUT.LGHTNGTEST = $(OUT)/$(DIR.LGHTNGTEST)
INC.LGHTNGTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.LGHTNGTEST)/*.h ))
SRC.LGHTNGTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.LGHTNGTEST)/*.cpp ))
OBJ.LGHTNGTEST = \
  $(addprefix $(OUT.LGHTNGTEST)/,$(notdir $(SRC.LGHTNGTEST:.cpp=$O)))
DEP.LGHTNGTEST = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.LGHTNGTEST = $(foreach d,$(DEP.LGHTNGTEST),$($d.LIB))

OUTDIRS += $(OUT.LGHTNGTEST)

#TO_INSTALL.EXE += $(LGHTNGTEST.EXE)

MSVC.DSP += LGHTNGTEST
DSP.LGHTNGTEST.NAME = lghtngtest
DSP.LGHTNGTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.lghtngtest lghtngtestclean lghtngtestcleandep

all: $(LGHTNGTEST.EXE)
build.lghtngtest: $(OUTDIRS) $(LGHTNGTEST.EXE)
clean: lghtngtestclean

$(OUT.LGHTNGTEST)/%$O: $(SRCDIR)/$(DIR.LGHTNGTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(LGHTNGTEST.EXE): $(DEP.EXE) $(OBJ.LGHTNGTEST) $(LIB.LGHTNGTEST)
	$(DO.LINK.EXE)

lghtngtestclean:
	-$(RM) lghtngtest.txt
	-$(RMDIR) $(LGHTNGTEST.EXE) $(OBJ.LGHTNGTEST)

cleandep: lghtngtestcleandep
lghtngtestcleandep:
	-$(RM) $(OUT.LGHTNGTEST)/lghtngtest.dep

ifdef DO_DEPEND
dep: $(OUT.LGHTNGTEST) $(OUT.LGHTNGTEST)/lghtngtest.dep
$(OUT.LGHTNGTEST)/lghtngtest.dep: $(SRC.LGHTNGTEST)
	$(DO.DEPEND)
else
-include $(OUT.LGHTNGTEST)/lghtngtest.dep
endif

endif # ifeq ($(MAKESECTION),targets)

# Application description
DESCRIPTION.g2dtest = Crystal Space canvas plugin test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP+=$(NEWLINE)echo $"  make g2dtest      Make the $(DESCRIPTION.g2dtest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: g2dtest g2dtestclean

all apps: g2dtest
g2dtest:
	$(MAKE_APP)
g2dtestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

G2DTEST.EXE = g2dtest$(EXE)
DIR.G2DTEST = apps/tests/g2dtest
OUT.G2DTEST = $(OUT)/$(DIR.G2DTEST)
INC.G2DTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.G2DTEST)/*.h))
SRC.G2DTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.G2DTEST)/*.cpp))
OBJ.G2DTEST = $(addprefix $(OUT.G2DTEST)/,$(notdir $(SRC.G2DTEST:.cpp=$O)))
DEP.G2DTEST = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.G2DTEST = $(foreach d,$(DEP.G2DTEST),$($d.LIB))

OUTDIRS += $(OUT.G2DTEST)

#TO_INSTALL.EXE += $(G2DTEST.EXE)

MSVC.DSP += G2DTEST
DSP.G2DTEST.NAME = g2dtest
DSP.G2DTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.g2dtest g2dtestclean g2dtestcleandep

build.g2dtest: $(OUTDIRS) $(G2DTEST.EXE)
clean: g2dtestclean

$(OUT.G2DTEST)/%$O: $(SRCDIR)/$(DIR.G2DTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(G2DTEST.EXE): $(DEP.EXE) $(OBJ.G2DTEST) $(LIB.G2DTEST)
	$(DO.LINK.EXE)

g2dtestclean:
	-$(RM) g2dtest.txt
	-$(RMDIR) $(G2DTEST.EXE) $(OBJ.G2DTEST)

cleandep: g2dtestcleandep
g2dtestcleandep:
	-$(RM) $(OUT.G2DTEST)/g2dtest.dep

ifdef DO_DEPEND
dep: $(OUT.G2DTEST) $(OUT.G2DTEST)/g2dtest.dep
$(OUT.G2DTEST)/g2dtest.dep: $(SRC.G2DTEST)
	$(DO.DEPEND)
else
-include $(OUT.G2DTEST)/g2dtest.dep
endif

endif # ifeq ($(MAKESECTION),targets)

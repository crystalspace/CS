# Application description
DESCRIPTION.vostest = Crystal Space VOS test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make vostest      Make the $(DESCRIPTION.vostest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: vostest vostestclean

all apps: vostest
vostest:
	$(MAKE_APP)
vostestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

VOSTEST.EXE = vostest$(EXE)
DIR.VOSTEST = apps/tests/vostest
OUT.VOSTEST = $(OUT)/$(DIR.VOSTEST)
INC.VOSTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.VOSTEST)/*.h ))
SRC.VOSTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.VOSTEST)/*.cpp ))
OBJ.VOSTEST = $(addprefix $(OUT.VOSTEST)/,$(notdir $(SRC.VOSTEST:.cpp=$O)))
DEP.VOSTEST = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.VOSTEST = $(foreach d,$(DEP.VOSTEST),$($d.LIB))

OUTDIRS += $(OUT.VOSTEST)

#TO_INSTALL.EXE += $(VOSTEST.EXE)

MSVC.DSP += VOSTEST
DSP.VOSTEST.NAME = vostest
DSP.VOSTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.vostest vostestclean vostestcleandep

build.vostest: $(OUTDIRS) $(VOSTEST.EXE)
clean: vostestclean

$(OUT.VOSTEST)/%$O: $(SRCDIR)/$(DIR.VOSTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(VOSTEST.EXE): $(DEP.EXE) $(OBJ.VOSTEST) $(LIB.VOSTEST)
	$(DO.LINK.EXE)

vostestclean:
	-$(RM) vostest.txt
	-$(RMDIR) $(VOSTEST.EXE) $(OBJ.VOSTEST)

cleandep: vostestcleandep
vostestcleandep:
	-$(RM) $(OUT.VOSTEST)/vostest.dep

ifdef DO_DEPEND
dep: $(OUT.VOSTEST) $(OUT.VOSTEST)/vostest.dep
$(OUT.VOSTEST)/vostest.dep: $(SRC.VOSTEST)
	$(DO.DEPEND)
else
-include $(OUT.VOSTEST)/vostest.dep
endif

endif # ifeq ($(MAKESECTION),targets)

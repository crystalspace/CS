# Application description
DESCRIPTION.milk2spr = Milkshape ASCII Model conversion tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make milk2spr     Make the $(DESCRIPTION.milk2spr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: milk2spr milk2sprclean

all apps: milk2spr
milk2spr:
	$(MAKE_APP)
milk2sprclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

MILK2SPR.EXE = milk2spr$(EXE.CONSOLE)
DIR.MILK2SPR = apps/import/milk2spr
OUT.MILK2SPR = $(OUT)/$(DIR.MILK2SPR)
INC.MILK2SPR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MILK2SPR)/*.h ))
SRC.MILK2SPR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MILK2SPR)/*.cpp ))
OBJ.MILK2SPR = $(addprefix $(OUT.MILK2SPR)/,$(notdir $(SRC.MILK2SPR:.cpp=$O)))
DEP.MILK2SPR = CSGFX CSUTIL CSUTIL CSGEOM
LIB.MILK2SPR = $(foreach d,$(DEP.MILK2SPR),$($d.LIB))

OUTDIRS += $(OUT.MILK2SPR)

TO_INSTALL.EXE += $(MILK2SPR.EXE)

MSVC.DSP += MILK2SPR
DSP.MILK2SPR.NAME = milk2spr
DSP.MILK2SPR.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.milk2spr milk2sprclean milk2sprcleandep

all: $(MILK2SPR.EXE)
build.milk2spr: $(OUTDIRS) $(MILK2SPR.EXE)
clean: milk2sprclean

$(OUT.MILK2SPR)/%$O: $(SRCDIR)/$(DIR.MILK2SPR)/%.cpp
	$(DO.COMPILE.CPP)

$(MILK2SPR.EXE): $(OBJ.MILK2SPR) $(LIB.MILK2SPR)
	$(DO.LINK.CONSOLE.EXE)

milk2sprclean:
	-$(RM) milk2spr.txt
	-$(RMDIR) $(MILK2SPR.EXE) $(OBJ.MILK2SPR)

cleandep: milk2sprcleandep
milk2sprcleandep:
	-$(RM) $(OUT.MILK2SPR)/milk2spr.dep

ifdef DO_DEPEND
dep: $(OUT.MILK2SPR) $(OUT.MILK2SPR)/milk2spr.dep
$(OUT.MILK2SPR)/milk2spr.dep: $(SRC.MILK2SPR)
	$(DO.DEPEND)
else
-include $(OUT.MILK2SPR)/milk2spr.dep
endif

endif # ifeq ($(MAKESECTION),targets)

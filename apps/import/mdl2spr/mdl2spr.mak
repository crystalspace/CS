# Application description
DESCRIPTION.mdl2spr = Quake model MDL/MD2 conversion tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make mdl2spr      Make the $(DESCRIPTION.mdl2spr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mdl2spr mdl2sprclean

all apps: mdl2spr
mdl2spr:
	$(MAKE_APP)
mdl2sprclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

MDL2SPR.EXE = mdl2spr$(EXE.CONSOLE)
DIR.MDL2SPR = apps/import/mdl2spr
OUT.MDL2SPR = $(OUT)/$(DIR.MDL2SPR)
INC.MDL2SPR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MDL2SPR)/*.h ))
SRC.MDL2SPR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MDL2SPR)/*.cpp ))
OBJ.MDL2SPR = $(addprefix $(OUT.MDL2SPR)/,$(notdir $(SRC.MDL2SPR:.cpp=$O)))
DEP.MDL2SPR = CSGFX CSUTIL CSUTIL CSGEOM
LIB.MDL2SPR = $(foreach d,$(DEP.MDL2SPR),$($d.LIB))

OUTDIRS += $(OUT.MDL2SPR)

TO_INSTALL.EXE += $(MDL2SPR.EXE)

MSVC.DSP += MDL2SPR
DSP.MDL2SPR.NAME = mdl2spr
DSP.MDL2SPR.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.mdl2spr mdl2sprclean mdl2sprcleandep

all: $(MDL2SPR.EXE)
build.mdl2spr: $(OUTDIRS) $(MDL2SPR.EXE)
clean: mdl2sprclean

$(OUT.MDL2SPR)/%$O: $(SRCDIR)/$(DIR.MDL2SPR)/%.cpp
	$(DO.COMPILE.CPP)

$(MDL2SPR.EXE): $(OBJ.MDL2SPR) $(LIB.MDL2SPR)
	$(DO.LINK.CONSOLE.EXE)

mdl2sprclean:
	-$(RM) mdl2spr.txt
	-$(RMDIR) $(MDL2SPR.EXE) $(OBJ.MDL2SPR)

cleandep: mdl2sprcleandep
mdl2sprcleandep:
	-$(RM) $(OUT.MDL2SPR)/mdl2spr.dep

ifdef DO_DEPEND
dep: $(OUT.MDL2SPR) $(OUT.MDL2SPR)/mdl2spr.dep
$(OUT.MDL2SPR)/mdl2spr.dep: $(SRC.MDL2SPR)
	$(DO.DEPEND)
else
-include $(OUT.MDL2SPR)/mdl2spr.dep
endif

endif # ifeq ($(MAKESECTION),targets)

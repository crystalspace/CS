# Application description
DESCRIPTION.maya2spr = Maya to CS Sprite convertor

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make maya2spr     Make the $(DESCRIPTION.maya2spr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: maya2spr maya2sprclean

all apps: maya2spr
maya2spr:
	$(MAKE_APP)
maya2sprclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

MAYA2SPR.EXE = maya2spr$(EXE.CONSOLE)
DIR.MAYA2SPR = apps/import/maya2spr
OUT.MAYA2SPR = $(OUT)/$(DIR.MAYA2SPR)
INC.MAYA2SPR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MAYA2SPR)/*.h ))
SRC.MAYA2SPR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MAYA2SPR)/*.cpp ))
OBJ.MAYA2SPR = $(addprefix $(OUT.MAYA2SPR)/,$(notdir $(SRC.MAYA2SPR:.cpp=$O)))
DEP.MAYA2SPR = CSGFX CSUTIL CSUTIL CSGEOM
LIB.MAYA2SPR = $(foreach d,$(DEP.MAYA2SPR),$($d.LIB))

OUTDIRS += $(OUT.MAYA2SPR)

TO_INSTALL.EXE += $(MAYA2SPR.EXE)

MSVC.DSP += MAYA2SPR
DSP.MAYA2SPR.NAME = maya2spr
DSP.MAYA2SPR.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.maya2spr maya2sprclean maya2sprcleandep

all: $(MAYA2SPR.EXE)
build.maya2spr: $(OUTDIRS) $(MAYA2SPR.EXE)
clean: maya2sprclean

$(OUT.MAYA2SPR)/%$O: $(SRCDIR)/$(DIR.MAYA2SPR)/%.cpp
	$(DO.COMPILE.CPP)

$(MAYA2SPR.EXE): $(OBJ.MAYA2SPR) $(LIB.MAYA2SPR)
	$(DO.LINK.CONSOLE.EXE)

maya2sprclean:
	-$(RM) maya2spr.txt
	-$(RMDIR) $(MAYA2SPR.EXE) $(OBJ.MAYA2SPR)

cleandep: maya2sprcleandep
maya2sprcleandep:
	-$(RM) $(OUT.MAYA2SPR)/maya2spr.dep

ifdef DO_DEPEND
dep: $(OUT.MAYA2SPR) $(OUT.MAYA2SPR)/maya2spr.dep
$(OUT.MAYA2SPR)/maya2spr.dep: $(SRC.MAYA2SPR)
	$(DO.DEPEND)
else
-include $(OUT.MAYA2SPR)/maya2spr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
